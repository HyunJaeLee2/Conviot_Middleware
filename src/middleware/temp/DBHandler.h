#ifndef DBHandler_H_
#define DBHandler_H_

#include "capiot_common.h"

#include <CAPString.h>

#include "AppScriptModeler.h"
#include "ThingHandler.h"
#include "ThingManager.h"
#include "AppRunner.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef cap_result (*CbFnTraverseScenario)(int nId, cap_string strScenarioName, cap_string strScenarioText, void *pUserData);


cap_result DBHandler_OpenDB(IN char *pszMainDBFileName, IN char *pszValueLogDBFileName);

cap_result DBHandler_SetSensorValuePayload(IN char *pszThingId, IN char *pszValueName, IN char *pszPayload, OUT char **ppszMessage);
cap_result DBHandler_GetScenarioNamebyId(IN int nScenarioId, IN OUT cap_string strScenarioName);
cap_result DBHandler_TraverseAllScenarios(CbFnTraverseScenario fnScenarioTraverse, void *pUserData);
cap_result DBHandler_MakeThingListPayload(OUT char** ppszPayload, OUT int* nPayloadLen);
cap_result DBHandler_MakeScenarioListPayload(IN cap_handle hAppEngine, OUT char** ppszPayload, OUT int* nPayloadLen);
cap_result DBHandler_IsScenarioDisabled(IN cap_string strScenarioName, cap_bool *pbDisabled);

cap_result DBHandler_GetDBTime(OUT char **ppszTimeStr);
cap_result DBHandler_FillThingList(IN cap_handle hThingManager);
cap_result DBHandler_MakeThingAliveInfoArray(IN OUT SThingAliveInfo **ppstThingAliveInfoArray, IN int *pnArrayLength);
cap_result DBHandler_UpdateLatestTime(IN char *pszThingId);
cap_result DBHandler_ChangeScenarioState(IN cap_string strScenarioName, cap_bool bDisabled);

cap_result DBHandler_CheckIsReadyScenario(IN cap_string strThingId, IN OUT cap_handle hScenarioList);
cap_result DBHandler_CheckScenarioToDisable(IN cap_string strThingId, IN OUT cap_handle hScenarioList);

cap_result DBHandler_UpdateValue(IN char *pszThingId, IN char *pszValueName, IN char *pszPayload);
cap_result DBHandler_AddScenario(cap_string strScenarioName, cap_string strScenarioText, cap_handle hDependentThingList);
cap_result DBHandler_AddThing(IN SThingInfo* pstThing);
cap_result DBHandler_DeleteScenario(IN int nScenarioId);
cap_result DBHandler_DeleteThing(cap_string strDeviceName);
cap_result DBHandler_SetVirtualThingId(IN char *pszPayload, IN int nPayloadLen);

cap_result DBHandler_GetThingValue(IN cap_string strThingId, IN cap_string strValueName, OUT SValue **ppstValue, OUT double *pdblatestVal);
cap_result DBHandler_GetThingFunction(IN cap_string strThingId, IN cap_string strFunctionName, OUT SFunction **ppstFunction);

cap_result DBHandler_CheckValueName(cap_string strThingName, cap_string strValueName);
cap_result DBHandler_CheckFunctionName(cap_string strThingName, cap_string strFunctionName);
cap_result DBHandler_GetRealThingName(cap_string strVirtualThingName, IN OUT cap_string strRealThingName);
cap_result DBHandler_CheckScenarioName(cap_string strScenarioName);
cap_result DBHandler_CloseDB();

#ifdef __cplusplus
}
#endif

#endif /* CENTRALMANAGER_H_ */
