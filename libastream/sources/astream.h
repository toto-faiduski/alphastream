#pragma once

#define ASTREAM_EXTERN

#include "SpoolerDefs.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef enum {
	ASTREAM_E_OK = 0,
	ASTREAM_E_KO    /* 1 */
}ASTREAMcode;

/* All possible error codes from all sorts of curl functions. Future versions
may return other values, stay prepared.

Always add new return codes last. Never *EVER* remove any. The return
codes must remain the same!
*/


/*
* NAME astream_get_job_info()
*
* DESCRIPTION
*
* astream_connect() blabla blablabla....
*/
ASTREAM_EXTERN ASTREAMcode astream_get_job_info(
	const char* address,
	int port,
	int id,
	S_JobInfos* pInfo);

#ifdef  __cplusplus
}
#endif
