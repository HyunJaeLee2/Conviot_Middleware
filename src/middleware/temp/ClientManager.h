#ifndef CLIENTREQUESTManager_H_
#define CLIENTREQUESTManager_H_

#include <MQTTClient.h>
#include <MQTT_common.h>

#include "capiot_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _SClientManager {
    EIoTHandleId enID;
    cap_bool bCreated;
    cap_handle hClient;
} SClientManager;

/*
static char* paszClientManagerSubcriptionList[] = {
};

static int paszClientManagerSubscriptionQosList[] = {
};
*/

cap_result ClientManager_Create(OUT cap_handle* phClientReqManager, IN char *pszBrokerURI);
cap_result ClientManager_Run(cap_handle hClientReqManager);
cap_result ClientManager_Join(cap_handle hClientReqManager);
cap_result ClientManager_Destroy(IN OUT cap_handle* phClientReqManager);

#ifdef __cplusplus
}
#endif

#endif /* CLIENTREQUESTManager_H_ */
