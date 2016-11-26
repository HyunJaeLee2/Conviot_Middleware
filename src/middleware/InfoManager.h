#ifndef INFOMANAGER_H_
#define INFOMANAGER_H_

#include <MQTTClient.h>
#include <MQTT_common.h>

#include "capiot_common.h"
#include "CAPThread.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _SInfoManager {
    EIoTHandleId enID;
    cap_bool bCreated;
    int nSocketListeningPort; 
    cap_handle hSocketAcceptingThread;
    cap_handle hMQTTHandler;
    cap_handle hServerSocket;
    cap_handle hSocketThreadList;
    cap_handle hMessageQueueList;
    cap_handle hLock;
} SInfoManager;



cap_result InfoManager_Create(OUT cap_handle* phInfoManager, IN cap_string strBrokerURI, IN int nSocketListeningPort);
cap_result InfoManager_Run(IN cap_handle hInfoManager);
cap_result InfoManager_Join(IN cap_handle hInfoManager);
cap_result InfoManager_Destroy(IN OUT cap_handle* phInfoManager);

#ifdef __cplusplus
}
#endif

#endif /* InfoMANAGER_H_ */
