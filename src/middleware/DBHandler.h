#ifndef DBHandler_H_
#define DBHandler_H_

#include "capiot_common.h"

#include <CAPString.h>

#include "ThingManager.h"
#include "CentralManager.h"

#ifdef __cplusplus
extern "C" {
#endif


cap_result DBHandler_OpenDB(SDBInfo *pstDBInfo);
cap_result DBHandler_CloseDB();

cap_result DBHandler_VerifyApiKey(IN cap_string strDeviceId, IN char *pszApiKey);
cap_result DBHandler_RegisterDevice(IN cap_string strDeviceId, IN char *pszPinCode);  
cap_result DBHandler_UnregisterDevice(IN cap_string strDeviceId, IN char *pszPinCode);  
cap_result DBHandler_UpdateLatestTime(IN cap_string strDeviceId);
cap_result DBHandler_InsertVariableHistory(IN cap_string strDeviceId, IN cap_string strVariableName, IN char * pszVariable);
cap_result DBHandler_InsertApplicationHistory(IN cap_string strDeviceId, IN cap_string strFunctionName, IN int nEcdId, IN int nErrorCode);
#ifdef __cplusplus
}
#endif

#endif 
