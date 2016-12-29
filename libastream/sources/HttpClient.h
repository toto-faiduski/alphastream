#pragma once

#include "json_spirit_reader.h"
using namespace json_spirit;

#include "SpoolerDefs.h"

#include <vector>
using namespace std;

class CHttpClientException : public exception
{
	// Méthodes
public:
	CHttpClientException(const int a_i_ErrorCode, const char* a_pc_ErrorMsg) throw() : m_i_ErrorCode(a_i_ErrorCode), m_pc_ErrorMsg(a_pc_ErrorMsg) {}
	~CHttpClientException() throw() {}
	const char* what() const throw() { return m_pc_ErrorMsg; }
	int GetErrorCode() { return m_i_ErrorCode; }

	// Attributs
private:
	int m_i_ErrorCode;
	const char* m_pc_ErrorMsg;
};

class CHttpClient
{
public:
	CHttpClient();
	~CHttpClient();

	void UploadJob(const char* a_pC_ServerAddr,
		int a_i_Port,
		const char* a_pc_Filename,
		const char* a_pc_FriendlyName,
		const char* a_pc_TemplateXml,
		const char* a_pc_TemplateName,
		const char* a_pc_Formdef,
		mValue& a_jsonValue
		) throw (CHttpClientException);

	vector<S_JobInfos> UploadJob(const char* a_pC_ServerAddr,
		int a_i_Port,
		const char* a_pc_Filename,
		const char* a_pc_FriendlyName,
		const char* a_pc_TemplateXml,
		const char* a_pc_TemplateName,
		const char* a_pc_Formdef) throw (CHttpClientException);

	void GetJobInfos(const char* a_pC_ServerAddr,
		int a_i_Port,
		int a_i_IdJob,
		mValue& a_jsonValue) throw (CHttpClientException);

	S_JobInfos GetJobInfos(const char* a_pC_ServerAddr,
		int a_i_Port,
		int a_i_IdJob) throw (CHttpClientException);

	void GetJobs(const char* a_pC_ServerAddr,
		int a_i_Port,
		mValue& a_jsonValue) throw (CHttpClientException);

	vector<S_JobInfos> GetJobs(const char* a_pC_ServerAddr,
		int a_i_Port) throw (CHttpClientException);

	bool PrintJob(const char* a_pC_ServerAddr,
		int a_i_Port,
		int a_i_IdJob,
		const char* a_pc_PrintRange) throw (CHttpClientException);

	bool RemoveJob(const char* a_pC_ServerAddr,
		int a_i_Port,
		int a_i_IdJob) throw (CHttpClientException);

	bool GetJobInfosFromJson(const mValue& value, S_JobInfos* a_pst_JobInfos);
	bool GetJobsFromJson(const mValue& value, std::vector<S_JobInfos>& a_JobInfos);

};



