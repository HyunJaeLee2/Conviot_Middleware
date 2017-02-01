/*
 * MQTTMessageHandler.c
 *
 *  Created on: 2016. 8. 8.
 *      Author: chjej202
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <capiot_common.h>

#include <CAPTime.h>
#include <CAPQueue.h>
#include <CAPLinkedList.h>
#include <CAPString.h>
#include <CAPThread.h>
#include <CAPThreadLock.h>
#include <CAPLogger.h>

#include <MQTT_common.h>

#include "MQTTMessageHandler.h"

#define MQTT_CLIENT_DISCONNECT_TIMEOUT (10000)

static CALLBACK cap_result mqttDataDestroy(void* pData, void* pUsrData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SMQTTData* pstMQTTData = NULL;

    IFVARERRASSIGNGOTO(pData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstMQTTData = (SMQTTData *)pData;

    SAFEMEMFREE(pstMQTTData->pszTopic);
    SAFEMEMFREE(pstMQTTData->pszPayload);
    SAFEMEMFREE(pstMQTTData);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

static CALLBACK cap_result topicNameDataDestroy(int nOffset, void *pData, void *pUsrData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    cap_string strData = NULL;

    IFVARERRASSIGNGOTO(pData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    strData = (cap_string) pData;

    SAFE_CAPSTRING_DELETE(strData);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

static CALLBACK cap_result topicUnsubscribeDestroy(int nOffset, void *pData, void *pUsrData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    int nMqttRet = MQTTCLIENT_SUCCESS;
    SMQTTMessageHandler *pstHandler = NULL;
    cap_string strTopic = NULL;

    IFVARERRASSIGNGOTO(pData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(pUsrData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstHandler = (SMQTTMessageHandler *) pUsrData;

    strTopic = (cap_string) pData;

    nMqttRet = MQTTClient_unsubscribe(pstHandler->hClient, CAPString_LowPtr(strTopic, NULL));
    if (nMqttRet != MQTTCLIENT_SUCCESS) {
        //ERRASSIGNGOTO(result, ERR_CAP_UNSUBSCRIBE_ERROR, _EXIT);
        // ignore error (unsubscribe as many as possible even though error is occurred.)
    }

    SAFE_CAPSTRING_DELETE(strTopic);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result MQTTMessageHandler_Create(cap_string strBrokerURI, cap_string strClientID, int nPublishWaitTime, OUT cap_handle *phHandler)
{
    cap_result result = ERR_CAP_UNKNOWN;
    int nMqttRet = MQTTCLIENT_SUCCESS;
    SMQTTMessageHandler *pstHandler = NULL;

    IFVARERRASSIGNGOTO(strBrokerURI, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(strClientID, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(phHandler, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstHandler = (SMQTTMessageHandler*)malloc(sizeof(SMQTTMessageHandler));
    ERRMEMGOTO(pstHandler, result, _EXIT);

    pstHandler->enID = HANDLEID_MQTT_MESSAGE_HANDLER;
    pstHandler->hClient = NULL;
    pstHandler->hMQTTMessageHandlingThread = NULL;
    pstHandler->hMQTTMessageQueue = NULL;
    pstHandler->nDeliveredToken = 0;
    pstHandler->fnCallback = NULL;
    pstHandler->pUserData = NULL;
    pstHandler->hSubscriptionList = NULL;
    pstHandler->bConnected = FALSE;
    pstHandler->hLock = NULL;
    pstHandler->nPublishWaitTime = nPublishWaitTime;

    //create queue for mqtt message
    result = CAPQueue_Create(&(pstHandler->hMQTTMessageQueue));
    ERRIFGOTO(result, _EXIT);

    result = CAPThreadLock_Create(&(pstHandler->hLock));
    ERRIFGOTO(result, _EXIT);

    result = CAPLinkedList_Create(&(pstHandler->hSubscriptionList));
    ERRIFGOTO(result, _EXIT);

    //Create MQTT Client
    nMqttRet = MQTTClient_create(&(pstHandler->hClient), CAPString_LowPtr(strBrokerURI, NULL),
            CAPString_LowPtr(strClientID, NULL), MQTTCLIENT_PERSISTENCE_NONE, NULL);
    if (nMqttRet != MQTTCLIENT_SUCCESS)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INTERNAL_FAIL, _EXIT);
    }

    *phHandler = pstHandler;

    result = ERR_CAP_NOERROR;
_EXIT:
    if(result != ERR_CAP_NOERROR && pstHandler != NULL)
    {
        CAPLinkedList_Destroy(&(pstHandler->hSubscriptionList));
        CAPThreadLock_Destroy(&(pstHandler->hLock));
        CAPQueue_Destroy(&(pstHandler->hMQTTMessageQueue), NULL, NULL);
        SAFEMEMFREE(pstHandler);
    }
    return result;
}


cap_result MQTTMessageHandler_SetReceiveCallback(cap_handle hHandler, CbFnMQTTHandler fnCbMessageArrive, void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SMQTTMessageHandler *pstHandler = NULL;

    if (IS_VALID_HANDLE(hHandler, HANDLEID_MQTT_MESSAGE_HANDLER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(fnCbMessageArrive, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstHandler = (SMQTTMessageHandler *) hHandler;

    pstHandler->fnCallback = fnCbMessageArrive;
    pstHandler->pUserData = pUserData;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


static CALLBACK int messageArrivedCallBack(void* pHandler, char* pszTopic, int nTopicLen, MQTTClient_message* pstMessage)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SMQTTData* pstMQTTData = NULL;
    SMQTTMessageHandler* pstHandler = NULL;
    int nActualTopicLen = 0;

    IFVARERRASSIGNGOTO(pHandler, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(pszTopic, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(pstMessage, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstHandler = (SMQTTMessageHandler*)pHandler;

    pstMQTTData = (SMQTTData*)malloc(sizeof(SMQTTData));
    ERRMEMGOTO(pstMQTTData, result, _EXIT);

    //Calculate actual topic length
    //MQTTClient returns nTopicLen as 0 if there is no null character in topic in between.
    if(nTopicLen == 0)
        nActualTopicLen = strlen(pszTopic);
    else
        nActualTopicLen = nTopicLen;

    //copy topic and topiclen
    pstMQTTData->pszTopic = (char *)malloc(sizeof(char) * (nActualTopicLen + 1));
    ERRMEMGOTO(pstMQTTData->pszTopic, result, _EXIT);

    strncpy(pstMQTTData->pszTopic, pszTopic, nActualTopicLen);
    pstMQTTData->pszTopic[nActualTopicLen] = '\0';

    pstMQTTData->nTopicLen = nActualTopicLen;

    //copy payload to add null character at the end
    pstMQTTData->pszPayload = (char *)malloc(sizeof(char) * (pstMessage->payloadlen + 1));
    ERRMEMGOTO(pstMQTTData->pszPayload, result, _EXIT);

    memcpy(pstMQTTData->pszPayload, pstMessage->payload, pstMessage->payloadlen);
    pstMQTTData->pszPayload[pstMessage->payloadlen] = '\0';

    pstMQTTData->nPayloadLen = pstMessage->payloadlen;

    result = CAPQueue_Put(pstHandler->hMQTTMessageQueue, pstMQTTData);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;

_EXIT:
    MQTTClient_freeMessage(&pstMessage);
    MQTTClient_free(pszTopic);
    if(result != ERR_CAP_NOERROR)
    {
        CAPLogger_Write(g_hLogger, MSG_ERROR, "MQTT message arrival callback is failed.");
        return FALSE;
    }
    //ought to return 1 due to mqtt library
    return TRUE;
}


static CALLBACK void deliveredCallBack(void *pHandler, MQTTClient_deliveryToken nConfirmedToken)
{
    SMQTTMessageHandler* pstHandler = NULL;

    if (IS_VALID_HANDLE(pHandler, HANDLEID_MQTT_MESSAGE_HANDLER) == FALSE)
    {
        CAPLogger_Write(g_hLogger, MSG_ERROR, "deliveredCallBack param error!");
    }
    else
    {

        pstHandler = (SMQTTMessageHandler*) pHandler;
        CAPLogger_Write(g_hLogger, MSG_DETAIL, "MQTTClient_publish with token %d delivery confirmed", nConfirmedToken);
        
        pstHandler->nDeliveredToken = nConfirmedToken;
    }
}

static cap_result restoreSubscription(SMQTTMessageHandler* pstHandler)
{
    cap_result result = ERR_CAP_UNKNOWN;
    int nMqttRet = MQTTCLIENT_SUCCESS;
    int nLoop = 0;
    int nTopicNum = 0;
    cap_string strTopic = NULL;

    IFVARERRASSIGNGOTO(pstHandler->bConnected, FALSE, result, ERR_CAP_NOT_CONNECTED, _EXIT);

    result = CAPThreadLock_Lock(pstHandler->hLock);
    ERRIFGOTO(result, _EXIT);

    result = CAPLinkedList_GetLength(pstHandler->hSubscriptionList, &nTopicNum);
    ERRIFGOTO(result, _EXIT_LOCK);

    for(nLoop = 0 ; nLoop < nTopicNum ; nLoop++)
    {
        result = CAPLinkedList_Get(pstHandler->hSubscriptionList, LINKED_LIST_OFFSET_FIRST, nLoop, (void **) &strTopic);
        ERRIFGOTO(result, _EXIT_LOCK);

        nMqttRet = MQTTClient_subscribe(pstHandler->hClient, CAPString_LowPtr(strTopic, NULL), QOS_LEVEL_2);
        if (nMqttRet != MQTTCLIENT_SUCCESS) {
            ERRASSIGNGOTO(result, ERR_CAP_SUBSCRIBE_ERROR, _EXIT_LOCK);
        }
    }

    result = ERR_CAP_NOERROR;
_EXIT_LOCK:
    CAPThreadLock_Unlock(pstHandler->hLock);
_EXIT:
    return result;
}


static CALLBACK void connectionLostCallback(void* pHandler, char *cause)
{
    cap_result result = ERR_CAP_NOERROR;
    SMQTTMessageHandler* pstHandler = NULL;
    MQTTClient_connectOptions stConnectOptions = MQTTClient_connectOptions_initializer;
    int nMqttRet = MQTTCLIENT_SUCCESS;

    IFVARERRASSIGNGOTO(pHandler, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    dlp("\nConnection lost\n");
    dlp("     cause: %s\n", cause);
    CAPLogger_Write(g_hLogger, MSG_ERROR, "MQTT connection lost!(strerror: %s)", strerror(errno));

    pstHandler = (SMQTTMessageHandler *) pHandler;

    stConnectOptions.cleansession = 1;

    nMqttRet = MQTTClient_connect(pstHandler->hClient, &stConnectOptions);
    if (nMqttRet != MQTTCLIENT_SUCCESS) {
        ERRASSIGNGOTO(result, ERR_CAP_CONNECT_ERROR, _EXIT);
    }

    CAPLogger_Write(g_hLogger, MSG_INFO, "MQTT Reconnection Succeeded!");

    result = restoreSubscription(pstHandler);
    ERRIFGOTO(result, _EXIT);

    CAPLogger_Write(g_hLogger, MSG_DETAIL, "MQTT Subscription restoration Succeeded!");
_EXIT:
    if(result != ERR_CAP_NOERROR) {
        CAPLogger_Write(g_hLogger, MSG_ERROR, "Error is happened during reconnection");
    }
    return;
}

static CAP_THREAD_HEAD MQTTMessageHandlingThread(IN void* pHandler)
{
    cap_result result = ERR_CAP_UNKNOWN;
    cap_string strTopic = NULL;
    cap_handle hTopicLinkedList = NULL;
    SMQTTData* pstMQTTData = NULL;
    SMQTTMessageHandler * pstHandler = NULL;

    IFVARERRASSIGNGOTO(pHandler, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstHandler = (SMQTTMessageHandler *)pHandler;

    strTopic = CAPString_New();
    ERRMEMGOTO(strTopic, result, _EXIT);

    while(g_bExit == FALSE && result != ERR_CAP_SUSPEND){
        result = CAPLinkedList_Create(&hTopicLinkedList);
        ERRIFGOTO(result, _EXIT);

        result = CAPQueue_Get(pstHandler->hMQTTMessageQueue, TRUE, (void **) &pstMQTTData);
        ERRIFGOTO(result, _EXIT);

        result = CAPString_SetLow(strTopic, pstMQTTData->pszTopic, pstMQTTData->nTopicLen);
        ERRIFGOTO(result, _EXIT);

        result = divideTopicNameToList(strTopic, hTopicLinkedList);
        ERRIFGOTO(result, _EXIT);
        
        CAPLogger_Write(g_hLogger, MSG_DETAIL, "MQTTMessageHandler Received message!");
        CAPLogger_Write(g_hLogger, MSG_DEBUG, "topic : %s\npayload : %s", pstMQTTData->pszTopic, pstMQTTData->pszPayload);

        if(pstHandler->fnCallback != NULL)
        {
            result = pstHandler->fnCallback(strTopic, hTopicLinkedList, pstMQTTData->pszPayload,
                    pstMQTTData->nPayloadLen, pstHandler->pUserData);
            if(result != ERR_CAP_NOERROR)
            {
                // ignore error?
                CAPLogger_Write(g_hLogger, MSG_ERROR, "MQTTMessageHandler callback error! %d", result);
            }
        }

        result = CAPLinkedList_Traverse(hTopicLinkedList, topicNameDataDestroy, NULL);
        ERRIFGOTO(result, _EXIT);
        result = CAPLinkedList_Destroy(&hTopicLinkedList);
        ERRIFGOTO(result, _EXIT);

        SAFEMEMFREE(pstMQTTData->pszTopic);
        SAFEMEMFREE(pstMQTTData->pszPayload);
        SAFEMEMFREE(pstMQTTData);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    if(pstMQTTData != NULL){
        SAFEMEMFREE(pstMQTTData->pszTopic);
        SAFEMEMFREE(pstMQTTData->pszPayload);
        SAFEMEMFREE(pstMQTTData);
    }

    CAPLinkedList_Traverse(hTopicLinkedList, topicNameDataDestroy, NULL);
    CAPLinkedList_Destroy(&hTopicLinkedList);
    SAFE_CAPSTRING_DELETE(strTopic);

    CAP_THREAD_END;
}



cap_result MQTTMessageHandler_Connect(cap_handle hHandler)
{
    cap_result result = ERR_CAP_UNKNOWN;
    MQTTClient_connectOptions stConnectOptions = MQTTClient_connectOptions_initializer;
    int nMqttRet = MQTTCLIENT_SUCCESS;
    SMQTTMessageHandler *pstHandler = NULL;

    if (IS_VALID_HANDLE(hHandler, HANDLEID_MQTT_MESSAGE_HANDLER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstHandler = (SMQTTMessageHandler *) hHandler;

    //set MQTT callback
    nMqttRet = MQTTClient_setCallbacks(pstHandler->hClient, pstHandler, connectionLostCallback, messageArrivedCallBack, deliveredCallBack);
    if (nMqttRet != MQTTCLIENT_SUCCESS) {
        ERRASSIGNGOTO(result, ERR_CAP_CALLBACK_ERROR, _EXIT);
    }

    stConnectOptions.cleansession = 1;

    //connect Client
    nMqttRet = MQTTClient_connect(pstHandler->hClient, &stConnectOptions);
    if (nMqttRet != MQTTCLIENT_SUCCESS) {
        ERRASSIGNGOTO(result, ERR_CAP_CONNECT_ERROR, _EXIT);
    }

    result = CAPThread_Create(MQTTMessageHandlingThread, pstHandler, &(pstHandler->hMQTTMessageHandlingThread));
    ERRIFGOTO(result, _EXIT);

    pstHandler->bConnected = TRUE;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result MQTTMessageHandler_Disconnect(cap_handle hHandler)
{
    cap_result result = ERR_CAP_UNKNOWN;
    int nMqttRet = MQTTCLIENT_SUCCESS;
    SMQTTMessageHandler *pstHandler = NULL;

    if (IS_VALID_HANDLE(hHandler, HANDLEID_MQTT_MESSAGE_HANDLER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstHandler = (SMQTTMessageHandler *) hHandler;

    IFVARERRASSIGNGOTO(pstHandler->bConnected, FALSE, result, ERR_CAP_NOT_CONNECTED, _EXIT);

    result = CAPQueue_SetExit(pstHandler->hMQTTMessageQueue);
    ERRIFGOTO(result, _EXIT);

    result = CAPThread_Destroy(&(pstHandler->hMQTTMessageHandlingThread));
    ERRIFGOTO(result, _EXIT);

    result = CAPThreadLock_Lock(pstHandler->hLock);
    ERRIFGOTO(result, _EXIT);

    result = CAPLinkedList_Traverse(pstHandler->hSubscriptionList, topicUnsubscribeDestroy, pstHandler);
    ERRIFGOTO(result, _EXIT_LOCK);

    nMqttRet = MQTTClient_disconnect(pstHandler->hClient, MQTT_CLIENT_DISCONNECT_TIMEOUT);
    if (nMqttRet != MQTTCLIENT_SUCCESS) {
        ERRASSIGNGOTO(result, ERR_CAP_DISCONNECT_ERROR, _EXIT_LOCK);
    }

    pstHandler->bConnected = FALSE;

    result = ERR_CAP_NOERROR;
_EXIT_LOCK:
    CAPThreadLock_Unlock(pstHandler->hLock);
_EXIT:
    return result;
}


cap_result MQTTMessageHandler_Publish(cap_handle hHandler, cap_string strTopic, char *pszPayload, int nPayloadLen)
{
    cap_result result = ERR_CAP_UNKNOWN;
    int nMqttRet = MQTTCLIENT_SUCCESS;
    SMQTTMessageHandler *pstHandler = NULL;
    MQTTClient_deliveryToken nToken = 0;

    if (IS_VALID_HANDLE(hHandler, HANDLEID_MQTT_MESSAGE_HANDLER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(strTopic, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(pszPayload, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if(nPayloadLen <= 0) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_PARAM, _EXIT);
    }

    pstHandler = (SMQTTMessageHandler *) hHandler;

    IFVARERRASSIGNGOTO(pstHandler->bConnected, FALSE, result, ERR_CAP_NOT_CONNECTED, _EXIT);

    pstHandler->nDeliveredToken = 0;
    
    nMqttRet = MQTTClient_publish(pstHandler->hClient, CAPString_LowPtr(strTopic, NULL), nPayloadLen, pszPayload, QOS_LEVEL_2, 0, &nToken);
    if (nMqttRet != MQTTCLIENT_SUCCESS) {
        CAPLogger_Write(g_hLogger, MSG_ERROR, "MQTTClient_publish error!\n Error msg: %d", nMqttRet);
        ERRASSIGNGOTO(result, ERR_CAP_NET_SEND_ERROR, _EXIT);
    }

    CAPLogger_Write(g_hLogger, MSG_DEBUG, "Waiting for message publication!");
    CAPLogger_Write(g_hLogger, MSG_DEBUG, "topic : %s\npayload : %s token : %d", CAPString_LowPtr(strTopic, NULL), pszPayload, nToken);
    
    //dlp("MQTT Pulish !! topic : %s\npayload : %s\n", CAPString_LowPtr(strTopic, NULL), pszPayload);

    while(pstHandler->nDeliveredToken != nToken) {
        if(pstHandler->nPublishWaitTime > 0) {
            CAPTime_Sleep(pstHandler->nPublishWaitTime);
        }
        else {
            result = CAPThread_Yield();
            ERRIFGOTO(result, _EXIT);
        }
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result MQTTMessageHandler_Subscribe(cap_handle hHandler, cap_string strTopic)
{
    cap_result result = ERR_CAP_UNKNOWN;
    int nMqttRet = MQTTCLIENT_SUCCESS;
    SMQTTMessageHandler *pstHandler = NULL;
    cap_string strTopicToList = NULL;

    if (IS_VALID_HANDLE(hHandler, HANDLEID_MQTT_MESSAGE_HANDLER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(strTopic, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstHandler = (SMQTTMessageHandler *) hHandler;

    IFVARERRASSIGNGOTO(pstHandler->bConnected, FALSE, result, ERR_CAP_NOT_CONNECTED, _EXIT);

    nMqttRet = MQTTClient_subscribe(pstHandler->hClient, CAPString_LowPtr(strTopic, NULL), QOS_LEVEL_2);
    if (nMqttRet != MQTTCLIENT_SUCCESS) {
        ERRASSIGNGOTO(result, ERR_CAP_SUBSCRIBE_ERROR, _EXIT);
    }

    strTopicToList = CAPString_New();
    ERRMEMGOTO(strTopicToList, result, _EXIT);

    result = CAPString_Set(strTopicToList, strTopic);
    ERRIFGOTO(result, _EXIT);

    result = CAPThreadLock_Lock(pstHandler->hLock);
    ERRIFGOTO(result, _EXIT);

    result = CAPLinkedList_Add(pstHandler->hSubscriptionList, LINKED_LIST_OFFSET_LAST, 0, strTopicToList);
    ERRIFGOTO(result, _EXIT_LOCK);

    // If the topic is included into the list, set NULL to avoid deletion.
    strTopicToList = NULL;

    result = ERR_CAP_NOERROR;
_EXIT_LOCK:
    CAPThreadLock_Unlock(pstHandler->hLock);
_EXIT:
    SAFE_CAPSTRING_DELETE(strTopicToList);
    return result;
}


cap_result MQTTMessageHandler_SubscribeMany(cap_handle hHandler, char * const *paszTopicArray, int nTopicNum)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SMQTTMessageHandler *pstHandler = NULL;
    int nLoop = 0;
    cap_string strTopic = NULL;

    if (IS_VALID_HANDLE(hHandler, HANDLEID_MQTT_MESSAGE_HANDLER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(paszTopicArray, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if(nTopicNum <= 0) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_PARAM, _EXIT);
    }

    pstHandler = (SMQTTMessageHandler *) hHandler;

    IFVARERRASSIGNGOTO(pstHandler->bConnected, FALSE, result, ERR_CAP_NOT_CONNECTED, _EXIT);

    strTopic = CAPString_New();
    ERRMEMGOTO(strTopic, result, _EXIT);

    for(nLoop = 0 ; nLoop < nTopicNum ; nLoop++)
    {
        result = CAPString_SetLow(strTopic, paszTopicArray[nLoop], CAPSTRING_MAX);
        ERRIFGOTO(result, _EXIT);

        result = MQTTMessageHandler_Subscribe(hHandler, strTopic);
        ERRIFGOTO(result, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFE_CAPSTRING_DELETE(strTopic);
    return result;
}


cap_result MQTTMessageHandler_Destroy(IN OUT cap_handle *phHandler)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SMQTTMessageHandler *pstHandler = NULL;

    IFVARERRASSIGNGOTO(phHandler, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if (IS_VALID_HANDLE(*phHandler, HANDLEID_MQTT_MESSAGE_HANDLER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstHandler = (SMQTTMessageHandler *) *phHandler;

    if(pstHandler->bConnected == TRUE)
    {
        // Ignore error
        MQTTMessageHandler_Disconnect((cap_handle) pstHandler);
    }

    MQTTClient_destroy(&(pstHandler->hClient));

    CAPLinkedList_Destroy(&(pstHandler->hSubscriptionList));
    CAPThreadLock_Destroy(&(pstHandler->hLock));
    CAPQueue_Destroy(&(pstHandler->hMQTTMessageQueue), mqttDataDestroy, NULL);

    SAFEMEMFREE(pstHandler);

    *phHandler = NULL;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


