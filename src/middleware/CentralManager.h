#ifndef CENTRALMANAGER_H_
#define CENTRALMANAGER_H_

#include "capiot_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _SCentralManager {
    EIoTHandleId enID;
    cap_handle hJobManager;
    cap_handle hThingManager;
    cap_handle hClientManager;
    cap_handle hInfoManager;
    cap_handle hValueTopicQueue;
} SCentralManager;

cap_result CentralManager_Create(OUT cap_handle *phCentralManager, IN char *pszBrokerURI);
cap_result CentralManager_Execute(IN cap_handle hCentralManager, IN char *pszBrokerURI);
cap_result CentralManager_Destroy(IN OUT cap_handle* phCentralManager);

#ifdef __cplusplus
}
#endif

#endif /* CENTRALMANAGER_H_ */
