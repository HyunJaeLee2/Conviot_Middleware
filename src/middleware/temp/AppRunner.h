#ifndef APPRUNNER_H_
#define APPRUNNER_H_

#include <MQTTClient.h>
#include <MQTT_common.h>

#include "capiot_common.h"
#include "CAPThread.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MQTT_CLIENT_DISCONNECT_TIMEOUT (10000)
#define MQTT_RECEIVE_TIMEOUT (3000)

typedef enum _ERunnerState {
    RUNNER_STATE_INIT,
    RUNNER_STATE_RUNNING,
    RUNNER_STATE_PAUSE, // Not used
    RUNNER_STATE_STOP,
    RUNNER_STATE_COMPLETED,
} ERunnerState;

typedef struct _SAppRunnerMQTTCallback SAppRunnerMQTTCallback;

typedef struct _SAppRunner {
    EIoTHandleId enID;
    cap_bool bCreated;
    cap_bool bExit;
    cap_handle hMQTTHandler;
    cap_handle hEvent;
    cap_handle hScenario;
    cap_handle hAppRunnerThread;
    cap_handle hLock;
    ERunnerState enState;
    cap_handle hAppModel;
    cap_handle hConditionQueue;
    cap_handle hAppValueCache;
    cap_string strScenarioName;
    cap_handle hActionQueue;
    SAppRunnerMQTTCallback *pstCallback;
} SAppRunner;

typedef struct _SAppRunnerMQTTCallback {
    SAppRunner *pstAppRunner;
    cap_handle hCacheIndexHash;
    cap_handle hConditionList;
} SAppRunnerMQTTCallback;


cap_result AppRunner_Create(IN cap_handle hAppModel, IN cap_string strBrokerURI, OUT cap_handle* phAppRunner);
cap_result AppRunner_Run(cap_handle hAppRunner);
cap_result AppRunner_Stop(cap_handle hAppRunner);
cap_result AppRunner_GetState(cap_handle hAppRunner, OUT ERunnerState *penState);

cap_result AppRunner_Join(cap_handle hAppRunner);
cap_result AppRunner_Destroy(OUT cap_handle* phAppRunner);

#ifdef __cplusplus
}
#endif

#endif /* APPRUNNER_H_ */
