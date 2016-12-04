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
#ifdef __cplusplus
}
#endif

#endif 
