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
//
cap_result DBHandler_VerifyApiKey(IN MYSQL *pDBconn, IN cap_string strDeviceId, IN OUT char *pszApiKey);
cap_result DBHandler_VerifyServiceApiKey(IN MYSQL *pDBconn, IN cap_string strProductName, IN OUT char *pszApiKey);

cap_result DBHandler_RetrieveApiKey(IN MYSQL *pDBconn, IN cap_string strDeviceId, IN char **ppszApiKey);

cap_result DBHandler_InsertVariableHistory(IN MYSQL *pDBconn,IN cap_string strDeviceId, IN cap_string strVariableName, IN char * pszVariable);
cap_result DBHandler_InsertServiceVariableHistory(IN MYSQL *pDBconn, IN cap_string strProductName, IN int nUserId, IN cap_string strVariableName, IN char * pszVariable);

cap_result DBHandler_InsertApplicationHistory(IN MYSQL *pDBconn,IN cap_string strDeviceId, IN cap_string strFunctionName, IN int nEcaId, IN int nErrorCode);

cap_result DBHandler_MakeConditionList(IN MYSQL *pDBconn, IN cap_string strDeviceId, IN cap_string strVariableName, IN OUT cap_handle hRelatedConditionList);
//
cap_result DBHandler_RegisterDevice(IN MYSQL *pDBconn,IN cap_string strDeviceId, IN char *pszPinCode);  
cap_result DBHandler_UnregisterDeviceAndEca(IN MYSQL *pDBconn,IN cap_string strDeviceId, IN char *pszPinCode);  
cap_result DBHandler_UpdateLatestTime(IN MYSQL *pDBconn,IN cap_string strDeviceId);

cap_result DBHandler_RetrieveActionList(IN MYSQL *pDBconn, IN int nEcaId, IN OUT cap_handle hActionList);
cap_result DBHandler_RetrieveSatisfiedEcaList(IN MYSQL *pDBconn, IN OUT cap_handle hSatisfiedEcaList);
cap_result DBHandler_InsertSatisfiedCondition(IN MYSQL *pDBconn, IN cap_handle hRelatedConditionList);

cap_result DBHandler_MakeThingAliveInfoArray(IN MYSQL *pDBconn, IN OUT SThingAliveInfo **ppstThingAliveInfoArray, IN int *pnArrayLength);
cap_result DBHandler_DisableDeviceAndEca(IN MYSQL *pDBconn,IN cap_string strDeviceId); 

#ifdef __cplusplus
}
#endif

#endif 
