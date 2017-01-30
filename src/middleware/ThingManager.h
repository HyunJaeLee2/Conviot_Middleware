#ifndef THINGMANAGER_H_
#define THINGMANAGER_H_

#include <MQTTClient.h>
#include <MQTT_common.h>
#include <mysql.h>

#include "capiot_common.h"
#include "CAPThread.h"
#include "CentralManager.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MQTT_CLIENT_DISCONNECT_TIMEOUT (10000)
#define MQTT_RECEIVE_TIMEOUT (3000)


typedef struct _SThingAliveInfo {
    cap_string strDeviceId;
    long long llAliveCycle;
    long long llLatestTime;
} SThingAliveInfo;

typedef struct _SThingManager {
    EIoTHandleId enID;
    cap_bool bCreated;
    cap_handle hAliveHandlingThread;
    cap_handle hMQTTHandler;
    cap_handle hEvent;
    SThingAliveInfo *pstThingAliveInfoArray;
    int nAliveCheckingPeriod;
    MYSQL *pDBconn;
} SThingManager;

cap_result ThingManager_Create(OUT cap_handle* phThingManager, IN cap_string strBrokerURI, IN SDBInfo *pstDBInfo);
cap_result ThingManager_Run(IN cap_handle hThingManager, IN int nAliveCheckingPeriod);
cap_result ThingManager_Join(IN cap_handle hThingManager);
cap_result ThingManager_Destroy(IN OUT cap_handle* phThingManager);

#ifdef __cplusplus
}
#endif

#endif /* THINGMANAGER_H_ */
