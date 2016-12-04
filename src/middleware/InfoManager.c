#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <json-c/json_object.h>
#include <unistd.h>
#include <signal.h>

#include <CAPString.h>
#include <CAPThread.h>
#include <CAPLinkedList.h>
#include <CAPThreadEvent.h>
#include <CAPThreadLock.h>
#include <CAPSocket.h>
#include <CAPQueue.h> 
#include <CAPLogger.h>

#include <MQTT_common.h>

#include "InfoManager.h"
#include "MQTTMessageHandler.h"

#define SOCKET_SEND_TIMEOUT 10
#define SOCKET_ACCEPT_TIMEOUT 10
#define HEADER_LENGTH 30

#define MQTT_CLIENT_ID "Conviot_InfoManager"
#define MQTT_CLIENT_DISCONNECT_TIMEOUT (10000)
#define MQTT_RECEIVE_TIMEOUT (3000)
#define MQTT_SUBSCRIPTION_NUM (sizeof(paszInfoManagerSubcriptionList) / sizeof(char*))

static char* paszInfoManagerSubcriptionList[] = {
    "TM/TEMP/#"
};


CAPSTRING_CONST(CAPSTR_MQTT_CLIENT_ID, MQTT_CLIENT_ID);

static CALLBACK cap_result mqttMessageHandlingCallback(cap_string strTopic, cap_handle hTopicItemList, char *pszPayload,
        int nPayloadLen, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    cap_string strIdentifier = NULL;
    char* pszValueName = NULL;
    int nValueNameLength = 0;
    char *pszThingId = NULL;
    char *pszMessage = NULL;
    SInfoManager* pstInfoManager = NULL;

    IFVARERRASSIGNGOTO(pUserData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstInfoManager = (SInfoManager*)pUserData;

    //Check if it is sensor value or not
    result = CAPLinkedList_Get(hTopicItemList, LINKED_LIST_OFFSET_FIRST, TOPIC_LEVEL_FIRST, (void**)&strIdentifier);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    //Added if clause for a case where a thread is terminated without accepting any data at all

    SAFEMEMFREE(pszMessage);
    SAFEMEMFREE(pszThingId);
    SAFEMEMFREE(pszValueName);

    return result;
}

cap_result InfoManager_Create(OUT cap_handle* phInfoManager, IN cap_string strBrokerURI)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SInfoManager* pstInfoManager = NULL;
    SSocketInfo* pstSocketInfo = NULL;
    cap_handle hServerSocket;
    
    IFVARERRASSIGNGOTO(phInfoManager, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    
    pstInfoManager = (SInfoManager*)malloc(sizeof(SInfoManager));
    ERRMEMGOTO(pstInfoManager, result, _EXIT);
    
    pstInfoManager->enID = HANDLEID_INFO_MANAGER;
    pstInfoManager->bCreated = FALSE;
    pstInfoManager->hMQTTHandler = NULL;
    
    result = MQTTMessageHandler_Create(strBrokerURI, CAPSTR_MQTT_CLIENT_ID, 0, &(pstInfoManager->hMQTTHandler));
    ERRIFGOTO(result, _EXIT);

    *phInfoManager = pstInfoManager;

    result = ERR_CAP_NOERROR;
_EXIT:
    if (result != ERR_CAP_NOERROR) {
        //Destroy lock before deallocating pstInfoManager
        if(pstInfoManager != NULL){
        }
        SAFEMEMFREE(pstInfoManager);
    }
    return result;
}

cap_result InfoManager_Run(IN cap_handle hInfoManager)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SInfoManager* pstInfoManager = NULL;

    if (IS_VALID_HANDLE(hInfoManager, HANDLEID_INFO_MANAGER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }
    pstInfoManager = (SInfoManager*)hInfoManager;

    // Already created
    if (pstInfoManager->bCreated == TRUE) {
        CAPASSIGNGOTO(result, ERR_CAP_NOERROR, _EXIT);
    }

    result = MQTTMessageHandler_SetReceiveCallback(pstInfoManager->hMQTTHandler,
            mqttMessageHandlingCallback, pstInfoManager);
    ERRIFGOTO(result, _EXIT);

    result = MQTTMessageHandler_Connect(pstInfoManager->hMQTTHandler);
    ERRIFGOTO(result, _EXIT);

    result = MQTTMessageHandler_SubscribeMany(pstInfoManager->hMQTTHandler, paszInfoManagerSubcriptionList, MQTT_SUBSCRIPTION_NUM);
    ERRIFGOTO(result, _EXIT);
    
    pstInfoManager->bCreated = TRUE;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result InfoManager_Join(IN cap_handle hInfoManager)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SInfoManager* pstInfoManager = NULL;

    if (IS_VALID_HANDLE(hInfoManager, HANDLEID_INFO_MANAGER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstInfoManager = (SInfoManager*)hInfoManager;

    if (pstInfoManager->bCreated == TRUE) {
        result = MQTTMessageHandler_Disconnect(pstInfoManager->hMQTTHandler);
        ERRIFGOTO(result, _EXIT);

        pstInfoManager->bCreated = FALSE;
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result InfoManager_Destroy(IN OUT cap_handle* phInfoManager)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SInfoManager* pstInfoManager = NULL;

    IFVARERRASSIGNGOTO(phInfoManager, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if (IS_VALID_HANDLE(*phInfoManager, HANDLEID_INFO_MANAGER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstInfoManager = (SInfoManager*)*phInfoManager;
    
    MQTTMessageHandler_Destroy(&(pstInfoManager->hMQTTHandler));

    SAFEMEMFREE(pstInfoManager);

    *phInfoManager = NULL;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}
