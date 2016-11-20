#ifndef APPMANAGER_H_
#define APPMANAGER_H_

#include <CAPString.h>
#include <CAPThread.h>

#include <capiot_common.h>

#include <MQTT_common.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef struct _SAppManagerCallbackData {
    cap_string strPayload;
    cap_string strScenarioName;
    cap_string strScenarioText;
    cap_string strError;
} SAppManagerCallbackData;

typedef struct _SAppManager {
    EIoTHandleId enID;
    cap_string strBrokerURI;
    cap_bool bCreated;
    cap_handle hMQTTHandler;
    cap_handle hAppRunnerList;
    cap_handle hAppEngine;
    SAppManagerCallbackData *pstCallback;
} SAppManager;


cap_result AppManager_Create(OUT cap_handle* phJobManager, cap_string strBrokerURI);
cap_result AppManager_Run(cap_handle hJobManager, OUT cap_handle *phAppEngine);
cap_result AppManager_Join(cap_handle hJobManager);
cap_result AppManager_Destroy(OUT cap_handle* phJobManager);

#ifdef __cplusplus
}
#endif

#endif
