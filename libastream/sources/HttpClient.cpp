#include "HttpClient.h"
#include "json_helpers.h"

#include "curl/curl.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string.h>

using namespace std;
using namespace json_spirit;

#ifdef WINVER
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

struct MemoryStruct {
	char *memory;
	size_t size;
};

//#####################################################################################
// Callback curl pour ecriture en m�moire
//#####################################################################################
static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;

	mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
	if (mem->memory == NULL) {
		/* out of memory! */
		printf("not enough memory (realloc returned NULL)\n");
		return 0;
	}

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

//#####################################################################################
// Constructeur
//#####################################################################################
CHttpClient::CHttpClient()
{
	// init global de curl
	curl_global_init(CURL_GLOBAL_ALL);
}

//#####################################################################################
// Destructeur
//#####################################################################################
CHttpClient::~CHttpClient()
{
	curl_global_cleanup();
}

//#####################################################################################
// Upload d'un job
// ------------------------------------------------------------------------------------
//	\param[in] a_pC_ServerAddr : ip du serveur
//	\param[in] a_i_ServerPort : port d'�coute du serveur
//	\param[in] a_pc_Filename : nom du fichier du job � uploader
//	\param[in] a_pc_FriendlyName : nom d'usage du job
//	\param[in] a_pc_TemplateXml : contenu du template pdf
//	\param[in] a_pc_TemplateName : nom du template pdf
//	\param[in] a_pc_Formdef : formdef afp
//	\return	Id du (des) job(s) upload�(s)
//#####################################################################################
vector<S_JobInfos> CHttpClient::UploadJob(const char* a_pC_ServerAddr,
	int a_i_ServerPort,
	const char* a_pc_Filename,
	const char* a_pc_FriendlyName,
	const char* a_pc_TemplateXml,
	const char* a_pc_TemplateName,
	const char* a_pc_Formdef)throw (CHttpClientException)
{
	vector<S_JobInfos> jobInfos;
	mValue jsonValue;

	UploadJob(
		a_pC_ServerAddr,
		a_i_ServerPort,
		a_pc_Filename,
		a_pc_FriendlyName,
		a_pc_TemplateXml,
		a_pc_TemplateName,
		a_pc_Formdef,
		jsonValue);

	// on parse le buffer json de r�ponse
	if (!GetJobsFromJson(jsonValue, jobInfos))
	{
		throw CHttpClientException(1, "error parsing JSON result");
	}
	
	return jobInfos;
}

void CHttpClient::UploadJob(const char* a_pC_ServerAddr,
									int a_i_ServerPort,
									const char* a_pc_Filename,
									const char* a_pc_FriendlyName,
									const char* a_pc_TemplateXml,
									const char* a_pc_TemplateName,
									const char* a_pc_Formdef,
									mValue& a_jsonValue)throw (CHttpClientException)
{
	CURL *curl;
	CURLcode res;
	struct curl_httppost* l_pst_formpost = NULL;
	struct curl_httppost* l_pst_lastpost = NULL;

	// on d�crit chaque part du formulaire POST
	curl_formadd(&l_pst_formpost,
		&l_pst_lastpost,
		CURLFORM_COPYNAME, "file",
		CURLFORM_FILE, a_pc_Filename,
		CURLFORM_END);
	
	if (a_pc_TemplateXml)
	{
		// r�cup du contenu du template xml
		fstream l_fs_File(a_pc_TemplateXml, ios_base::in);
		if (!l_fs_File.fail())
		{
			l_fs_File.seekg(0, ios::end);
			ifstream::pos_type l_posT_FileLen = l_fs_File.tellg();
			std::vector<char>  l_VectContentTemplateFile(l_posT_FileLen);
			l_fs_File.seekg(0, ios::beg);
			l_fs_File.read(&l_VectContentTemplateFile[0], l_posT_FileLen);
			l_fs_File.close();

			curl_formadd(&l_pst_formpost,
				&l_pst_lastpost,
				CURLFORM_COPYNAME, "template_xml",
				CURLFORM_COPYCONTENTS, reinterpret_cast<char*>(&l_VectContentTemplateFile[0]),
				CURLFORM_END);
		}
	}

	if (a_pc_TemplateName)
		curl_formadd(&l_pst_formpost,
			&l_pst_lastpost,
			CURLFORM_COPYNAME, "template_name",
			CURLFORM_COPYCONTENTS, a_pc_TemplateName,
			CURLFORM_END);

	if (a_pc_Formdef)
		curl_formadd(&l_pst_formpost,
			&l_pst_lastpost,
			CURLFORM_COPYNAME, "formdef",
			CURLFORM_COPYCONTENTS, a_pc_Formdef,
			CURLFORM_END);

	if (a_pc_FriendlyName)
		curl_formadd(&l_pst_formpost,
			&l_pst_lastpost,
			CURLFORM_COPYNAME, "friendly_name",
			CURLFORM_COPYCONTENTS, a_pc_FriendlyName,
			CURLFORM_END);

	// url de destination de la requete
	char szURL[1024];
	sprintf(szURL, "http://%s:%d/api/jobs", a_pC_ServerAddr, a_i_ServerPort);

	curl = curl_easy_init();
	if (curl)
	{
		struct MemoryStruct chunk;
		chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */
		chunk.size = 0;    /* no data at this point */

		// url de la requete
		curl_easy_setopt(curl, CURLOPT_URL, szURL);
		// part du formulaire
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, l_pst_formpost);

		// callback pour r�cup�rer la r�ponse � la requete
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

		// ex�cution de la requete HTTP
		res = curl_easy_perform(curl);
		
		// cleanup de curl
		curl_easy_cleanup(curl);

		// lib�ration des parts du formulaire
		curl_formfree(l_pst_formpost);

		if (res != CURLE_OK)
		{
			free(chunk.memory);
			throw CHttpClientException(res, curl_easy_strerror(res));
		}

		// lecture du stream json
		if (!read(chunk.memory, a_jsonValue))
		{
			free(chunk.memory);
			throw CHttpClientException(1, "error reading JSON result");
		}

		free(chunk.memory);
	}
}

//#####################################################################################
// R�cup�re les infos d'un job
// ------------------------------------------------------------------------------------
//	\param[in] a_pC_ServerAddr : ip du serveur
//	\param[in] a_i_ServerPort : port d'�coute du serveur
//	\param[in] a_pc_IdJob : id du job
//	\return	Infos sur le job
//#####################################################################################
S_JobInfos CHttpClient::GetJobInfos(const char* a_pC_ServerAddr, int a_i_Port, int a_i_IdJob) throw (CHttpClientException)
{
	S_JobInfos l_st_JobInfos = { 0 };
	mValue jsonValue;

	GetJobInfos( a_pC_ServerAddr, a_i_Port, a_i_IdJob, jsonValue);

	// on parse le buffer json de r�ponse
	if (!GetJobInfosFromJson(jsonValue, &l_st_JobInfos))
	{
		throw CHttpClientException(1, "error parsing JSON result");
	}
	return l_st_JobInfos;
}

void CHttpClient::GetJobInfos(const char* a_pC_ServerAddr, int a_i_Port, int a_i_IdJob, mValue& a_jsonValue) throw (CHttpClientException)
{
	char szURL[1024];
	sprintf(szURL, "http://%s:%d/api/jobs/%d", a_pC_ServerAddr, a_i_Port, a_i_IdJob);

	CURL *curl = curl_easy_init();
	if (curl)
	{
		struct MemoryStruct chunk;
		chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */
		chunk.size = 0;    /* no data at this point */

		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Content-Type: application/json");

		curl_easy_setopt(curl, CURLOPT_URL, szURL);
		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		// callback pour r�cup�rer la r�ponse la r�ponse � la requete
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

		// ex�cution de la requete HTTP
		CURLcode res = curl_easy_perform(curl);

		long response_code;
		if (res == CURLE_OK)
		{
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
		}
		// cleanup de curl
		curl_easy_cleanup(curl);

		// free sur les headers
		curl_slist_free_all(headers);

		if (res != CURLE_OK)
		{
			free(chunk.memory);
			throw CHttpClientException(res, curl_easy_strerror(res));
		}

		if (response_code == 404)
		{
			free(chunk.memory);
			throw CHttpClientException(response_code, "Job not found");
		}
		// lecture du stream json
		if (!read(chunk.memory, a_jsonValue))
		{
			free(chunk.memory);
			throw CHttpClientException(1, "error reading JSON result");
		}
		free(chunk.memory);
	}
}

//#####################################################################################
// Supprime un job
// ------------------------------------------------------------------------------------
//	\param[in] a_pC_ServerAddr : ip du serveur
//	\param[in] a_i_ServerPort : port d'�coute du serveur
//	\param[in] a_pc_IdJob : id du job
//	\return	
//#####################################################################################
bool CHttpClient::RemoveJob(const char* a_pC_ServerAddr, int a_i_Port, int a_i_IdJob) throw (CHttpClientException)
{
	bool l_b_JobRemoved = false;

	char szURL[1024];
	sprintf(szURL, "http://%s:%d/api/jobs/%d", a_pC_ServerAddr, a_i_Port, a_i_IdJob);

	CURL *curl = curl_easy_init();
	if (curl)
	{
		struct MemoryStruct chunk;
		chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */
		chunk.size = 0;    /* no data at this point */

		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Content-Type: application/json");

		curl_easy_setopt(curl, CURLOPT_URL, szURL);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		// callback pour r�cup�rer la r�ponse la r�ponse � la requete
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

		// ex�cution de la requete HTTP
		CURLcode res = curl_easy_perform(curl);

		long response_code;
		if (res == CURLE_OK)
		{
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
		}
		// cleanup de curl
		curl_easy_cleanup(curl);

		// free sur les headers
		curl_slist_free_all(headers);

		if (res != CURLE_OK)
		{
			free(chunk.memory);
			throw CHttpClientException(res, curl_easy_strerror(res));
		}

		if (response_code == 404)
		{
			free(chunk.memory);
			throw CHttpClientException(response_code, "Job not found");
		}

		mValue l_mV_RootValue;
		if (!read(chunk.memory, l_mV_RootValue))
		{
			free(chunk.memory);
			throw CHttpClientException(1, "error reading JSON result");
		}
		if (l_mV_RootValue.type() != bool_type)
		{
			free(chunk.memory);
			throw CHttpClientException(1, "error parsing JSON result (not a boolean)");
		}
		l_b_JobRemoved = l_mV_RootValue.get_bool();

		if (res != CURLE_OK)
		{
			free(chunk.memory);
			throw CHttpClientException(res, curl_easy_strerror(res));
		}

		free(chunk.memory);
	}

	return l_b_JobRemoved;
}

//#####################################################################################
// R�cup�re les jobs
// ------------------------------------------------------------------------------------
//	\param[in] a_pC_ServerAddr : ip du serveur
//	\param[in] a_i_ServerPort : port d'�coute du serveur
//	\return	Infos sur les jobs
//#####################################################################################
vector<S_JobInfos> CHttpClient::GetJobs(const char* a_pC_ServerAddr, int a_i_Port) throw (CHttpClientException)
{
	vector<S_JobInfos> l_st_JobInfos;
	mValue jsonValue;

	GetJobs(a_pC_ServerAddr, a_i_Port, jsonValue);

	// on parse le buffer json de r�ponse
	if (!GetJobsFromJson(jsonValue, l_st_JobInfos))
	{
		throw CHttpClientException(1, "error parsing JSON result");
	}
	return l_st_JobInfos;
}

void CHttpClient::GetJobs(const char* a_pC_ServerAddr, int a_i_Port, mValue& a_jsonValue) throw (CHttpClientException)
{
	char szURL[1024];
	sprintf(szURL, "http://%s:%d/api/jobs", a_pC_ServerAddr, a_i_Port);

	CURL *curl = curl_easy_init();
	if (curl)
	{
		struct MemoryStruct chunk;
		chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */
		chunk.size = 0;    /* no data at this point */

		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Content-Type: application/json");

		curl_easy_setopt(curl, CURLOPT_URL, szURL);
		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

		// ex�cution de la requete HTTP
		CURLcode res = curl_easy_perform(curl);

		long response_code;
		if (res == CURLE_OK)
		{
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
		}
		// cleanup de curl
		curl_easy_cleanup(curl);

		// free sur les headers
		curl_slist_free_all(headers);

		if (res != CURLE_OK)
		{
			free(chunk.memory);
			throw CHttpClientException(res, curl_easy_strerror(res));
		}
		// lecture du stream json
		if (!read(chunk.memory, a_jsonValue))
		{
			free(chunk.memory);
			throw CHttpClientException(1, "error reading JSON result");
		}

		free(chunk.memory);
	}
}


//#####################################################################################
// Ajout d'un job � la print queue
// ------------------------------------------------------------------------------------
//	\param[in] a_pC_ServerAddr : ip du serveur
//	\param[in] a_i_ServerPort : port d'�coute du serveur
//	\param[in] a_pc_IdJob : id du job
//	\param[in] a_pc_PrintRange : print range � imprimer
//	\return	True si ajouter � la print queue
//#####################################################################################
bool CHttpClient::PrintJob(const char* a_pC_ServerAddr, int a_i_Port, int a_i_JobId, const char* a_pc_PrintRange)throw (CHttpClientException)
{
	CURL *curl;
	CURLcode res;
	bool l_b_JobAddedToPQ = false;
	char szURL[1024];
	
	sprintf(szURL, "http://%s:%d/api/jobs/%d/printJob", a_pC_ServerAddr, a_i_Port, a_i_JobId);

	curl = curl_easy_init();
	if (curl)
	{
		struct MemoryStruct chunk;
		chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */
		chunk.size = 0;    /* no data at this point */

		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Content-Type: application/json");

		curl_easy_setopt(curl, CURLOPT_URL, szURL);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, a_pc_PrintRange);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(a_pc_PrintRange));

		// callback pour r�cup�rer la r�ponse la r�ponse � la requete
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

		// ex�cution de la requete HTTP
		res = curl_easy_perform(curl);

		// cleanup de curl
		curl_easy_cleanup(curl);

		// free sur les headers
		curl_slist_free_all(headers);

		mValue l_mV_RootValue;
		if (!read(chunk.memory, l_mV_RootValue))
		{
			free(chunk.memory);
			throw CHttpClientException(1, "error reading JSON result");
		}
		if (l_mV_RootValue.type()!=bool_type)
		{
			free(chunk.memory);
			throw CHttpClientException(1, "error parsing JSON result (not a boolean)");
		}
		l_b_JobAddedToPQ = l_mV_RootValue.get_bool();

		if (res != CURLE_OK)
		{
			free(chunk.memory);
			throw CHttpClientException(res, curl_easy_strerror(res));
		}

		free(chunk.memory);
	}

	return l_b_JobAddedToPQ;
}

//#####################################################################################
// JSON parser for a job
//#####################################################################################
bool CHttpClient::GetJobInfosFromJson(const mValue& value, S_JobInfos* a_pst_JobInfos)
{
	// on doit avoir un objet
	if (value.type() != obj_type)
	{
		return false;
	}

	mObject object = value.get_obj();
	mValue member;
	try
	{
		a_pst_JobInfos->m_i_JobId = get_mandatory_value<int, int_type>(object, "id");
		a_pst_JobInfos->m_str_OriginalDocName = get_mandatory_value<string, str_type>(object, "OriginalDocName");
		a_pst_JobInfos->m_str_UserFriendlyName = get_mandatory_value<string, str_type>(object, "UserFriendlyName");
		a_pst_JobInfos->m_EPreparingStatus = (EPreparingStatus)get_mandatory_value<int, int_type>(object, "PreparingStatus");

		a_pst_JobInfos->m_str_Template = get_optional_value<string, str_type>(object, "Template");
		a_pst_JobInfos->m_str_TemplateName = get_optional_value<string, str_type>(object, "TemplateName");
		a_pst_JobInfos->m_str_FormDef = get_optional_value<string, str_type>(object, "FormDef");
	}
	catch (std::runtime_error ex)
	{
		return false;
	}

	return true;
}

//#####################################################################################
// JSON parser for a job array
//#####################################################################################
bool CHttpClient::GetJobsFromJson(const mValue& value, std::vector<S_JobInfos>& a_JobInfos)
{
	// on doit avoir un tableau
	if (value.type() != array_type)
	{
		return false;
	}

	auto array = value.get_array();
	std::vector<mValue>::const_iterator itJob = array.begin();
	while (itJob != array.end())
	{
		S_JobInfos jobInfos;
		if (GetJobInfosFromJson(*itJob, &jobInfos))
			a_JobInfos.push_back(jobInfos);

		itJob++;
	}
	return true;
}

