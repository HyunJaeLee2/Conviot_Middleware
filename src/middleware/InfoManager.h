#ifndef INFOMANAGER_H_
#define INFOMANAGER_H_

#include <MQTTClient.h>
#include <MQTT_common.h>
#include <mysql.h>

#include "capiot_common.h"
#include "CAPThread.h"
#include "CentralManager.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _SInfoManager {
    EIoTHandleId enID;
    cap_bool bCreated;
    cap_handle hMQTTHandler;
    MYSQL *pDBconn;
} SInfoManager;



cap_result InfoManager_Create(OUT cap_handle* phInfoManager, IN cap_string strBrokerURI, IN SDBInfo *pstDBInfo);
cap_result InfoManager_Run(IN cap_handle hInfoManager);
cap_result InfoManager_Join(IN cap_handle hInfoManager);
cap_result InfoManager_Destroy(IN OUT cap_handle* phInfoManager);

#ifdef __cplusplus
}
#endif

#endif /* InfoMANAGER_H_ */
