#ifndef DBHandler_H_
#define DBHandler_H_

#include "capiot_common.h"
#include <mysql.h>

#include <CAPString.h>

#include "ThingManager.h"
#include "CentralManager.h"
#include "AppManager.h"

#ifdef __cplusplus
extern "C" {
#endif


cap_result DBHandler_OpenDB(IN SDBInfo *pstDBInfo, OUT MYSQL **ppDBconn);
cap_result DBHandler_CloseDB(MYSQL *pDBconn);

cap_result DBHandler_VerifyApiKey(IN MYSQL *pDBconn, IN cap_string strDeviceId, IN char *pszApiKey);
cap_result DBHandler_RegisterDevice(IN MYSQL *pDBconn,IN cap_string strDeviceId, IN char *pszPinCode);  
cap_result DBHandler_UnregisterDevice(IN MYSQL *pDBconn,IN cap_string strDeviceId, IN char *pszPinCode);  
cap_result DBHandler_UpdateLatestTime(IN MYSQL *pDBconn,IN cap_string strDeviceId);
cap_result DBHandler_InsertVariableHistory(IN MYSQL *pDBconn,IN cap_string strDeviceId, IN cap_string strVariableName, IN char * pszVariable);
cap_result DBHandler_InsertApplicationHistory(IN MYSQL *pDBconn,IN cap_string strDeviceId, IN cap_string strFunctionName, IN int nEcaId, IN int nErrorCode);
cap_result DBHandler_MakeConditionList(IN MYSQL *pDBconn, IN cap_string strDeviceId, IN cap_string strVariableName, IN OUT cap_handle hRelatedConditionList);
#ifdef __cplusplus
}
#endif

#endif 
