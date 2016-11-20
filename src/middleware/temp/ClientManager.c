#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <CAPString.h>
#include <CAPLinkedList.h>

#include "ClientManager.h"
#include "DBHandler.h"

#define MQTT_CLIENT_ID "cap_iot_middleware_client"
#define MQTT_CLIENT_DISCONNECT_TIMEOUT (10000)
#define MQTT_RECEIVE_TIMEOUT (3000)
#define MQTT_SUBSCRIPTION_NUM (sizeof(paszClientManagerSubcriptionList) / sizeof(char*))


static cap_result ClientManager_MQTTCallBack(void* pstClientManager, char* pszTopic, int nTopicLen, MQTTClient_message* pstMessage)
{
    return 1;
}

cap_result ClientManager_Create(OUT cap_handle* phClientManager, IN char *pszBrokerURI)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SClientManager* pstClientMgmr = NULL;
    int nMqttRet = MQTTCLIENT_SUCCESS;

    IFVARERRASSIGNGOTO(phClientManager, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstClientMgmr = (SClientManager*)malloc(sizeof(SClientManager));
    ERRMEMGOTO(pstClientMgmr, result, _EXIT);

    pstClientMgmr->enID = HANDLEID_THING_MANAGER;
    pstClientMgmr->bCreated = FALSE;

    //Create MQTT Client
    nMqttRet = MQTTClient_create(&(pstClientMgmr->hClient), pszBrokerURI, MQTT_CLIENT_ID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    if (nMqttRet != MQTTCLIENT_SUCCESS) {
        ERRASSIGNGOTO(result, ERR_CAP_INTERNAL_FAIL, _EXIT);
    }

    *phClientManager = pstClientMgmr;

    result = ERR_CAP_NOERROR;
_EXIT:
    if (result != ERR_CAP_NOERROR) {
        SAFEMEMFREE(pstClientMgmr);
    }
    return result;
}

cap_result ClientManager_Run(IN cap_handle hClientManager)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SClientManager* pstClientManager = NULL;

    /*
    MQTTClient_connectOptions stConnectOptions = MQTTClient_connectOptions_initializer;
    int nMqttRet = MQTTCLIENT_SUCCESS;
    */
    if (IS_VALID_HANDLE(hClientManager, HANDLEID_THING_MANAGER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }
    pstClientManager = (SClientManager*)hClientManager;

    // Already created
    if (pstClientManager->bCreated == TRUE) {
        CAPASSIGNGOTO(result, ERR_CAP_NOERROR, _EXIT);
    }

    /*
    //set MQTT callback
    nMqttRet = MQTTClient_setCallbacks(pstClientManager->hClient, pstClientManager, NULL, ClientManager_MQTTCallBack, NULL);
    if (nMqttRet != MQTTCLIENT_SUCCESS) {
        ERRASSIGNGOTO(result, ERR_CAP_CALLBACK_ERROR, _EXIT);
    }
    //connect Client
    nMqttRet = MQTTClient_connect(pstClientManager->hClient, &stConnectOptions);
    if (nMqttRet != MQTTCLIENT_SUCCESS) {
        ERRASSIGNGOTO(result, ERR_CAP_CONNECT_ERROR, _EXIT);
    }

    nMqttRet = MQTTClient_subscribeMany(pstClientManager->hClient, MQTT_SUBSCRIPTION_NUM, paszClientManagerSubcriptionList, paszClientManagerSubscriptionQosList);
    if (nMqttRet != MQTTCLIENT_SUCCESS) {
        ERRASSIGNGOTO(result, ERR_CAP_SUBSCRIBE_ERROR, _EXIT);
    }
    */

    pstClientManager->bCreated = TRUE;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result ClientManager_Join(IN cap_handle hClientManager)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SClientManager* pstClientManager = NULL;

    if (IS_VALID_HANDLE(hClientManager, HANDLEID_THING_MANAGER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstClientManager = (SClientManager*)hClientManager;

    if (pstClientManager->bCreated == TRUE) {
        pstClientManager->bCreated = FALSE;

        /*
        nMqttRet = MQTTClient_unsubscribeMany(pstClientManager->hClient, MQTT_SUBSCRIPTION_NUM, paszClientManagerSubcriptionList);
        if (nMqttRet != MQTTCLIENT_SUCCESS) {
            ERRASSIGNGOTO(result, ERR_CAP_UNSUBSCRIBE_ERROR, _EXIT);
        }

        nMqttRet = MQTTClient_disconnect(pstClientManager->hClient, MQTT_CLIENT_DISCONNECT_TIMEOUT);
        if (nMqttRet != MQTTCLIENT_SUCCESS) {
            ERRASSIGNGOTO(result, ERR_CAP_CONNECT_ERROR, _EXIT);
        }
        */
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result ClientManager_Destroy(IN OUT cap_handle* phClientManager)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SClientManager* pstClientMgmr = NULL;

    IFVARERRASSIGNGOTO(phClientManager, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if (IS_VALID_HANDLE(*phClientManager, HANDLEID_THING_MANAGER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstClientMgmr = (SClientManager*)*phClientManager;

    MQTTClient_destroy(&pstClientMgmr->hClient);

    SAFEMEMFREE(pstClientMgmr);

    *phClientManager = NULL;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}
