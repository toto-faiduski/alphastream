#include "astream.h"
#include "HttpClient.h"

ASTREAM_EXTERN ASTREAMcode astream_get_job_info(
	const char* address,
	int port,
	int id,
	S_JobInfos* pInfo)
{
	CHttpClient l_HttpClient;
	S_JobInfos l_JobInfos;
	try
	{
		l_JobInfos = l_HttpClient.GetJobInfos(
			address,
			port,
			id
		);
	}
	catch (CHttpClientException& )
	{
		return ASTREAM_E_KO;
	}
	*pInfo = l_JobInfos;
	return ASTREAM_E_OK;
}
