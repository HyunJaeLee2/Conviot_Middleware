/*
 * AppEngine.h
 *
 *  Created on: 2016. 8. 12.
 *      Author: chjej202
 */

#ifndef APPENGINE_H_
#define APPENGINE_H_

#include <capiot_common.h>

#include <CAPString.h>

#include "AppRunner.h"

#ifdef __cplusplus
extern "C" {
#endif

cap_result AppEngine_Create(cap_string strBrokerURI, OUT cap_handle *phEngine);
cap_result AppEngine_VerifyScenario(cap_handle hEngine, IN cap_string strScenarioName, IN cap_string strText, IN OUT cap_string strError);
cap_result AppEngine_AddScenario(cap_handle hEngine, IN cap_string strScenarioName, IN cap_string strText,
        IN OUT cap_string strError, IN OUT cap_handle hThingList);
cap_result AppEngine_DeleteScenario(cap_handle hEngine, IN cap_string strScenarioName);
cap_result AppEngine_RunScenario(cap_handle hEngine, IN cap_string strScenarioName);
cap_result AppEngine_StopScenario(cap_handle hEngine, IN cap_string strScenarioName);
cap_result AppEngine_GetScenarioState(cap_handle hEngine, IN cap_string strScenarioName, OUT ERunnerState *penState);
cap_result AppEngine_Destroy(IN OUT cap_handle *phEngine);

#ifdef __cplusplus
}
#endif

#endif /* APPENGINE_H_ */
