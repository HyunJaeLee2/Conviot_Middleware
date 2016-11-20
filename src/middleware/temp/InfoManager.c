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
#include "DBHandler.h"
#include "MQTTMessageHandler.h"

#define SOCKET_SEND_TIMEOUT 10
#define SOCKET_ACCEPT_TIMEOUT 10
#define HEADER_LENGTH 30

#define MQTT_CLIENT_ID "cap_iot_middleware_info"
#define MQTT_CLIENT_DISCONNECT_TIMEOUT (10000)
#define MQTT_RECEIVE_TIMEOUT (3000)
#define MQTT_SUBSCRIPTION_NUM (sizeof(paszInfoManagerSubcriptionList) / sizeof(char*))

//TODO
//Check destroy callback of queue

static char* paszInfoManagerSubcriptionList[] = {
    "TM/#",
    "MT/#",
    "ME/#",
    "EM/#",
};


CAPSTRING_CONST(CAPSTR_MQTT_CLIENT_ID, MQTT_CLIENT_ID);
CAPSTRING_CONST(CAPSTR_CATEGORY_REFRESH, "REFRESH");
CAPSTRING_CONST(CAPSTR_THING_LIST, "ME/RESULT/THING_LIST/");
CAPSTRING_CONST(CAPSTR_SCENARIO_LIST, "ME/RESULT/SCENARIO_LIST/");
CAPSTRING_CONST(CAPSTR_CATEGORY_REGISTER, "REGISTER");
CAPSTRING_CONST(CAPSTR_CATEGORY_UNREGISTER, "UNREGISTER");
CAPSTRING_CONST(CAPSTR_CATEGORY_SET_THING_ID, "SET_THING_ID");
CAPSTRING_CONST(CAPSTR_IDENTIFIER_TM, "TM");
CAPSTRING_CONST(CAPSTR_IDENTIFIER_MT, "MT");
CAPSTRING_CONST(CAPSTR_IDENTIFIER_EM, "EM");
CAPSTRING_CONST(CAPSTR_IDENTIFIER_ME, "ME");

typedef struct _SSocketThreadData{
    cap_handle hMessageQueue;
    cap_handle hClientSocket;
} SSocketThreadData;

static CALLBACK cap_result MessageQueueList_Destroy(int nOffset, void* pData, void* pUsrData)
{
    cap_result result = ERR_CAP_UNKNOWN;

    IFVARERRASSIGNGOTO(pData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    result = CAPQueue_Destroy((cap_handle*)&pData, NULL, NULL);
    ERRIFGOTO(result, _EXIT);
    
    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

static CALLBACK cap_result MessageQueueList_SendMessage(int nOffset, void* pData, void* pUsrData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    cap_handle hMessageQueue = NULL;
    char *pszMessage = NULL;

    IFVARERRASSIGNGOTO(pData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    hMessageQueue = pData;
    
    pszMessage = strdup(pUsrData);
    result = CAPQueue_Put(hMessageQueue, pszMessage);
    ERRIFGOTO(result, _EXIT);
    
    result = ERR_CAP_NOERROR;

_EXIT:
    if(result != ERR_CAP_NOERROR){
        SAFEMEMFREE(pszMessage);
    }
    return result;
}
static CALLBACK cap_result MessageQueueList_SetExit(int nOffset, void* pData, void* pUsrData)
{
    cap_result result = ERR_CAP_UNKNOWN;

    IFVARERRASSIGNGOTO(pData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    result = CAPQueue_SetExit((cap_handle)pData);
    ERRIFGOTO(result, _EXIT);
    
    result = ERR_CAP_NOERROR;

_EXIT:
    return result;
}
static CALLBACK cap_result SocketThreadList_Destroy(int nOffset, void* pData, void* pUsrData)
{
    cap_result result = ERR_CAP_UNKNOWN;

    IFVARERRASSIGNGOTO(pData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    result = CAPThread_Destroy((cap_handle*) &pData);
    ERRIFGOTO(result, _EXIT);
    
    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

static cap_result InfoManager_HandleRefreshRequest(cap_handle hInfoManager, cap_handle hTopicLinkedList){
    cap_result result = ERR_CAP_UNKNOWN;
    cap_string strTopic = NULL;
    cap_string strCategory = NULL;
    char* pszClientId = NULL;
    int nClientIdLength = 0;
    char* pszPayload;
    int nPayloadLen = 0;
    SInfoManager* pstInfoManager = NULL;
    
    IFVARERRASSIGNGOTO(hInfoManager, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(hTopicLinkedList, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstInfoManager = (SInfoManager*)hInfoManager;
    
    strTopic = CAPString_New();
    ERRMEMGOTO(strTopic, result, _EXIT);
    
    result = CAPLinkedList_Get(hTopicLinkedList, LINKED_LIST_OFFSET_FIRST, TOPIC_LEVEL_SECOND, (void**)&strCategory);
    ERRIFGOTO(result, _EXIT);

    result = assignLastElementFromTopicList(hTopicLinkedList, &pszClientId, &nClientIdLength);
    ERRIFGOTO(result, _EXIT);

    //Check if it is refresh request from client
    if (CAPString_IsEqual(strCategory, CAPSTR_CATEGORY_REFRESH) == TRUE) {
        //do refresh
        
        //make thing list directly from DB
        result = CAPString_Set(strTopic, CAPSTR_THING_LIST);
        ERRIFGOTO(result, _EXIT);

        result = CAPString_AppendLow(strTopic, pszClientId, nClientIdLength);
        ERRIFGOTO(result, _EXIT);

        result = DBHandler_MakeThingListPayload(&pszPayload, &nPayloadLen);
        ERRIFGOTO(result, _EXIT);

        result = MQTTMessageHandler_Publish(pstInfoManager->hMQTTHandler, strTopic, pszPayload, nPayloadLen);
        ERRIFGOTO(result, _EXIT);

        SAFEMEMFREE(pszPayload);

        // Scenario list
        result = CAPString_Set(strTopic, CAPSTR_SCENARIO_LIST);
        ERRIFGOTO(result, _EXIT);

        result = CAPString_AppendLow(strTopic, pszClientId, nClientIdLength);
        ERRIFGOTO(result, _EXIT);

        result = DBHandler_MakeScenarioListPayload(pstInfoManager->hAppEngine, &pszPayload, &nPayloadLen);
        ERRIFGOTO(result, _EXIT);

        result = MQTTMessageHandler_Publish(pstInfoManager->hMQTTHandler, strTopic, pszPayload, nPayloadLen);
        ERRIFGOTO(result, _EXIT);

        SAFEMEMFREE(pszPayload);
    }
    
    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEMEMFREE(pszClientId);
    SAFE_CAPSTRING_DELETE(strTopic);
    return result;
}

static cap_result makePacketThenSend(cap_handle hInfoManager, char *pszMessage){
    cap_result result = ERR_CAP_UNKNOWN;
    int nMessageLen = 0;
    char pszHeader[HEADER_LENGTH + 1];
    char *pszPacket = NULL;
    SInfoManager* pstInfoManager = NULL;
    
    IFVARERRASSIGNGOTO(hInfoManager, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstInfoManager = (SInfoManager*)hInfoManager;
    
    nMessageLen = strlen(pszMessage);

    //set packet
    sprintf(pszHeader, "%-30d", nMessageLen);

    pszPacket = (char *)malloc(HEADER_LENGTH + nMessageLen + 1);
    ERRMEMGOTO(pszPacket, result, _EXIT);

    memcpy(pszPacket, pszHeader, HEADER_LENGTH);
    memcpy(pszPacket + HEADER_LENGTH, pszMessage, nMessageLen);
    pszPacket[HEADER_LENGTH + nMessageLen] = '\0';

    result = CAPThreadLock_Lock(pstInfoManager->hLock);
    ERRIFGOTO(result, _EXIT);

    result = CAPLinkedList_Traverse(pstInfoManager->hMessageQueueList, MessageQueueList_SendMessage, pszPacket);
    ERRIFGOTO(result, _EXIT_ERROR);

    result = ERR_CAP_NOERROR; 
_EXIT_ERROR:
    CAPThreadLock_Unlock(pstInfoManager->hLock);
_EXIT:
    SAFEMEMFREE(pszPacket);
    return result;
}

static cap_result makeBrokerLogPayload(cap_string strTopic, char *pszPayload, OUT char **ppszMessage){
    cap_result result = ERR_CAP_UNKNOWN;
    char *pszTimeStr = NULL;
    json_object* pJsonObject = NULL;
    
    pJsonObject = json_object_new_object();
    ERRMEMGOTO(pJsonObject, result, _EXIT);

    result = DBHandler_GetDBTime(&pszTimeStr);
    ERRIFGOTO(result, _EXIT);

    json_object_object_add(pJsonObject, "msg_type", json_object_new_string((const char*)"broker_log"));
    json_object_object_add(pJsonObject, "time", json_object_new_string((const char*)pszTimeStr));
    json_object_object_add(pJsonObject, "topic", json_object_new_string((const char*)CAPString_LowPtr(strTopic, NULL)));
    json_object_object_add(pJsonObject, "payload", json_object_new_string((const char*)pszPayload));

    *ppszMessage = strdup(json_object_to_json_string(pJsonObject));
    SAFEMEMFREE(pszTimeStr);
    
    result = ERR_CAP_NOERROR; 
_EXIT:
    SAFEJSONFREE(pJsonObject);
    return result;

}
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
    ////////////////////For Experiment, value count 
    long long llCurrTime = 0;
    static long long llLatestTime = 0;
    long long llCycle = 10 * 1000;
    static int nValueCount = 0; 
    ///////////////////////////

    IFVARERRASSIGNGOTO(pUserData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstInfoManager = (SInfoManager*)pUserData;

    //Handle if it is REFRESH request from external client.
    result = InfoManager_HandleRefreshRequest((cap_handle) pstInfoManager, hTopicItemList);
    ERRIFGOTO(result, _EXIT);

    //Check if it is sensor value or not
    result = CAPLinkedList_Get(hTopicItemList, LINKED_LIST_OFFSET_FIRST, TOPIC_LEVEL_FIRST, (void**)&strIdentifier);
    ERRIFGOTO(result, _EXIT);

    if (CAPString_IsEqual(strIdentifier, CAPSTR_IDENTIFIER_TM) == TRUE || \
            CAPString_IsEqual(strIdentifier, CAPSTR_IDENTIFIER_MT) == TRUE || \
            CAPString_IsEqual(strIdentifier, CAPSTR_IDENTIFIER_EM) == TRUE || \
            CAPString_IsEqual(strIdentifier, CAPSTR_IDENTIFIER_ME) == TRUE ) {

        ////////////////////For Experiment, ignore broker log
        result = ERR_CAP_NOERROR;
        return result;
        ///////////////////////////
        
        //Make broker log payload for cloud
        result = makeBrokerLogPayload(strTopic, pszPayload, &pszMessage);
        ERRIFGOTO(result, _EXIT);
   
    }
    else {
        //since strIdentifier is first element of TopicLinkedList, it is thing id in case of value payload.
        pszThingId = strdup(CAPString_LowPtr(strIdentifier, NULL));

        result = assignLastElementFromTopicList(hTopicItemList, &pszValueName, &nValueNameLength);
        ERRIFGOTO(result, _EXIT);

        result = DBHandler_UpdateValue(pszThingId, pszValueName, pszPayload);
        ERRIFGOTO(result, _EXIT);

        result = DBHandler_SetSensorValuePayload(pszThingId, pszValueName, pszPayload, &pszMessage);
        ERRIFGOTO(result, _EXIT);

        ////////////////////For Experiment, value count 
        nValueCount++;
        
        result = CAPTime_GetCurTimeInMilliSeconds(&llCurrTime);
        ERRIFGOTO(result, _EXIT);
        
        if(llCurrTime - llLatestTime > llCycle){
            dlp("Handled value count : %d\n", nValueCount);
            llLatestTime = llCurrTime;
            nValueCount = 0;
        }
        ///////////////////////////
    }

    result = makePacketThenSend(pstInfoManager, pszMessage);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    //Added if clause for a case where a thread is terminated without accepting any data at all

    SAFEMEMFREE(pszMessage);
    SAFEMEMFREE(pszThingId);
    SAFEMEMFREE(pszValueName);

    return result;
}


CAP_THREAD_HEAD SocketThread(IN void* pSocketThreadData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    cap_handle hClientSocket = NULL;
    SSocketThreadData* pstSocketThreadData = NULL;
    char *pszContent = NULL;
    cap_handle hMessageQueue = NULL;
    ////////////////////For Experiment, value count 
    long long llCurrTime = 0;
    static long long llLatestTime = 0;
    long long llCycle = 10 * 1000;
    static int nValueCount = 0; 
    ///////////////////////////

    IFVARERRASSIGNGOTO(pSocketThreadData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstSocketThreadData = (SSocketThreadData*)pSocketThreadData;

    hMessageQueue = pstSocketThreadData->hMessageQueue;
    hClientSocket = pstSocketThreadData->hClientSocket;

    while (g_bExit == FALSE) {
        int nContentLen = 0;
        char *pszOrigContent = NULL;

        result = CAPQueue_Get(hMessageQueue, TRUE, (void **) &pszContent);  
        ERRIFGOTO(result, _EXIT);

        //copy to free at the end
        pszOrigContent = pszContent;

        //dlp("Socket Thread received msg : %s\n", pszContent);
        nContentLen = strlen(pszContent);

        while(nContentLen >0)
        {
            int nSentSize = 0;

            result = CAPSocket_Send(hClientSocket, SOCKET_SEND_TIMEOUT, pszContent, nContentLen, &nSentSize);
            ERRIFGOTO(result, _EXIT);

            if(nSentSize <= 0)
                break;

            pszContent += nSentSize;
            nContentLen -= nSentSize;   
        }

        SAFEMEMFREE(pszOrigContent);
        ////////////////////For Experiment, value count 
        nValueCount++;
        
        result = CAPTime_GetCurTimeInMilliSeconds(&llCurrTime);
        ERRIFGOTO(result, _EXIT);
        
        if(llCurrTime - llLatestTime > llCycle){
            dlp("Handled Socket value count : %d\n", nValueCount);
            llLatestTime = llCurrTime;
            nValueCount = 0;
        }
        ///////////////////////////
    }
    result = ERR_CAP_NOERROR;

    //TODO
    //When connection is lost, its thread must be joined
_EXIT:
    CAPSocket_Destroy(&pstSocketThreadData->hClientSocket);
    SAFEMEMFREE(pstSocketThreadData);
    CAP_THREAD_END;
}

CAP_THREAD_HEAD SocketAcceptingThread(IN void* pInfoManager)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SInfoManager* pstInfoManager = NULL;
    cap_handle hClientSocket = NULL;
    SSocketInfo* pstSocketInfo = NULL;
    SSocketThreadData* pstSocketThreadData = NULL;
    cap_handle hSocketThread = NULL;
    cap_handle hMessageQueue = NULL;

    IFVARERRASSIGNGOTO(pInfoManager, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstInfoManager = (SInfoManager*)pInfoManager;

    pstSocketInfo = (SSocketInfo*)malloc(sizeof(SSocketInfo));
    ERRMEMGOTO(pstSocketInfo, result, _EXIT);

    pstSocketInfo->enSocketType = SOCKET_TYPE_TCP;
    pstSocketInfo->nPort = pstInfoManager->nSocketListeningPort;
    pstSocketInfo->strSocketPath = NULL;

    while (g_bExit == FALSE) {
        if(hClientSocket == NULL){    
            result = CAPSocket_Create(pstSocketInfo, FALSE, &hClientSocket);
            ERRIFGOTO(result, _EXIT);
        }

        //TODO
        //Handle case of Timeout(when getting SIGINT)
        result = CAPSocket_Accept(pstInfoManager->hServerSocket, SOCKET_ACCEPT_TIMEOUT, hClientSocket);
        if(result != ERR_CAP_NOERROR)
        {
            if(result == ERR_CAP_NET_TIMEOUT && g_bExit == TRUE)
                goto _EXIT;
            else if(result == ERR_CAP_NET_TIMEOUT && g_bExit == FALSE){
				CAPLogger_Write(g_hLogger, MSG_DETAIL, "Socket Accept Timeout");
                continue;
            }
            else
                ERRIFGOTO(result, _EXIT);
        } else {
            //when there is no error
            //dlp("Accepted new socket from cloud!!\n");
        }

        result = CAPQueue_Create(&hMessageQueue);
        ERRIFGOTO(result, _EXIT);

        result = CAPThreadLock_Lock(pstInfoManager->hLock);
        ERRIFGOTO(result, _EXIT);

        result = CAPLinkedList_Add(pstInfoManager->hMessageQueueList, LINKED_LIST_OFFSET_LAST, 0, hMessageQueue);
        ERRIFGOTO(result, _EXIT_ERROR);

        result = CAPThreadLock_Unlock(pstInfoManager->hLock);
        ERRIFGOTO(result, _EXIT);

        pstSocketThreadData = (SSocketThreadData*)malloc(sizeof(SSocketThreadData));
        ERRMEMGOTO(pstSocketThreadData, result, _EXIT);

        pstSocketThreadData->hMessageQueue = hMessageQueue;
        pstSocketThreadData->hClientSocket = hClientSocket;

        //pstSocketThreadData and hClientSocket will be freed in a thread
        result = CAPThread_Create(SocketThread, pstSocketThreadData, &hSocketThread);
        ERRIFGOTO(result, _EXIT);

        pstSocketThreadData = NULL;
        hClientSocket = NULL;

        result = CAPLinkedList_Add(pstInfoManager->hSocketThreadList, LINKED_LIST_OFFSET_LAST, 0, hSocketThread);
        ERRIFGOTO(result, _EXIT);

    }
    result = ERR_CAP_NOERROR;

_EXIT:
    CAPSocket_Destroy(&hClientSocket);
    SAFEMEMFREE(pstSocketInfo);
    //deallocate threadData if it fails to create socket thread
    SAFEMEMFREE(pstSocketThreadData);
    CAP_THREAD_END;
_EXIT_ERROR:
    CAPThreadLock_Unlock(pstInfoManager->hLock);
    CAPSocket_Destroy(&hClientSocket);
    SAFEMEMFREE(pstSocketInfo);
    //deallocate threadData if it fails to create socket thread
    SAFEMEMFREE(pstSocketThreadData);
    CAP_THREAD_END;
}

CAP_THREAD_HEAD ReceiveMessageToCloudThread(IN void* pInfoManager)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SInfoManager* pstInfoManager = NULL;
    char *pszMessage = NULL;

    IFVARERRASSIGNGOTO(pInfoManager, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

	pstInfoManager = (SInfoManager*)pInfoManager;

	while (g_bExit == FALSE) {
        result = CAPQueue_Get(pstInfoManager->hMessageToCloudQueue, TRUE, (void **) &pszMessage);
        ERRIFGOTO(result, _EXIT);

        result = makePacketThenSend(pstInfoManager, pszMessage);
        ERRIFGOTO(result, _EXIT);

        SAFEMEMFREE(pszMessage);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    //deallocate memory of pszMessage in case of error
    SAFEMEMFREE(pszMessage);

    CAP_THREAD_END;

}

CAP_THREAD_HEAD ValueTopicSubscribingThread(IN void* pInfoManager)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SInfoManager* pstInfoManager = NULL;
    cap_string strSubscribeTopic = NULL;

    IFVARERRASSIGNGOTO(pInfoManager, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

	pstInfoManager = (SInfoManager*)pInfoManager;

	while (g_bExit == FALSE) {
        //TODO
        //Deal with case when result is ERR_CAP_SUSPEND which happens when setexit is called
        result = CAPQueue_Get(pstInfoManager->hValueTopicQueue, TRUE, (void **) &strSubscribeTopic);
        ERRIFGOTO(result, _EXIT);

        result = MQTTMessageHandler_Subscribe(pstInfoManager->hMQTTHandler, strSubscribeTopic);
        ERRIFGOTO(result, _EXIT);
        
        SAFE_CAPSTRING_DELETE(strSubscribeTopic);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    //deallocate memory of pszContent in case of error
    SAFE_CAPSTRING_DELETE(strSubscribeTopic);

    CAP_THREAD_END;

}


cap_result InfoManager_Create(OUT cap_handle* phInfoManager, IN cap_string strBrokerURI, IN int nSocketListeningPort)
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
    pstInfoManager->nSocketListeningPort = nSocketListeningPort;
    pstInfoManager->hValueTopicSubscribingThread = NULL;
    pstInfoManager->hReceiveMessageToCloudThread = NULL;
    pstInfoManager->hSocketAcceptingThread = NULL;
    pstInfoManager->hValueTopicQueue = NULL;
    pstInfoManager->hMessageToCloudQueue = NULL;
    pstInfoManager->hServerSocket = NULL;
    pstInfoManager->hMessageQueueList = NULL; 
    pstInfoManager->hAppEngine = NULL; 
    pstInfoManager->hMQTTHandler = NULL;
    
    //Create Socket
    pstSocketInfo = (SSocketInfo*)malloc(sizeof(SSocketInfo));
    ERRMEMGOTO(pstSocketInfo, result, _EXIT);

    pstSocketInfo->enSocketType = SOCKET_TYPE_TCP;
    pstSocketInfo->nPort = nSocketListeningPort;
    pstSocketInfo->strSocketPath = NULL;

    result = CAPSocket_Create(pstSocketInfo, TRUE, &hServerSocket);
    ERRIFGOTO(result, _EXIT);
    pstInfoManager->hServerSocket = hServerSocket;

    //create lock queue list
    result = CAPThreadLock_Create(&(pstInfoManager->hLock));
    ERRIFGOTO(result, _EXIT);
    
    result = CAPLinkedList_Create(&(pstInfoManager->hMessageQueueList));
    ERRIFGOTO(result, _EXIT);

    result = CAPLinkedList_Create(&pstInfoManager->hSocketThreadList);
    ERRIFGOTO(result, _EXIT);

    result = MQTTMessageHandler_Create(strBrokerURI, CAPSTR_MQTT_CLIENT_ID, 0, &(pstInfoManager->hMQTTHandler));
    ERRIFGOTO(result, _EXIT);

    *phInfoManager = pstInfoManager;

    result = ERR_CAP_NOERROR;
_EXIT:
    if (result != ERR_CAP_NOERROR) {
        //Destroy lock before deallocating pstInfoManager
        if(pstInfoManager != NULL){
            CAPLinkedList_Destroy(&(pstInfoManager->hSocketThreadList));
            CAPLinkedList_Destroy(&(pstInfoManager->hMessageQueueList));
            CAPThreadLock_Destroy(&(pstInfoManager->hLock));
            CAPSocket_Destroy(&(pstInfoManager->hServerSocket));
        }
        SAFEMEMFREE(pstInfoManager);
    }
    SAFEMEMFREE(pstSocketInfo);
    return result;
}

cap_result InfoManager_Run(IN cap_handle hInfoManager, IN cap_handle hValueTopicQueue, IN cap_handle hMessageToCloudQueue, IN cap_handle hAppEngine)
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

    pstInfoManager->hValueTopicQueue = hValueTopicQueue;
    pstInfoManager->hMessageToCloudQueue = hMessageToCloudQueue; 
    pstInfoManager->hAppEngine = hAppEngine;

    result = MQTTMessageHandler_SetReceiveCallback(pstInfoManager->hMQTTHandler,
            mqttMessageHandlingCallback, pstInfoManager);
    ERRIFGOTO(result, _EXIT);

    result = MQTTMessageHandler_Connect(pstInfoManager->hMQTTHandler);
    ERRIFGOTO(result, _EXIT);

    result = MQTTMessageHandler_SubscribeMany(pstInfoManager->hMQTTHandler, paszInfoManagerSubcriptionList, MQTT_SUBSCRIPTION_NUM);
    ERRIFGOTO(result, _EXIT);
    
    result = CAPThread_Create(ValueTopicSubscribingThread, pstInfoManager, &(pstInfoManager->hValueTopicSubscribingThread));
    ERRIFGOTO(result, _EXIT);
    
    result = CAPThread_Create(ReceiveMessageToCloudThread, pstInfoManager, &(pstInfoManager->hReceiveMessageToCloudThread));
    ERRIFGOTO(result, _EXIT);

    //Socket bind then listen
    result = CAPSocket_Bind(pstInfoManager->hServerSocket);
    ERRIFGOTO(result, _EXIT);

    result = CAPSocket_Listen(pstInfoManager->hServerSocket);
    ERRIFGOTO(result, _EXIT);
    
    result = CAPThread_Create(SocketAcceptingThread, pstInfoManager, &(pstInfoManager->hSocketAcceptingThread));
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
        //hold lock
        result = CAPThreadLock_Lock(pstInfoManager->hLock);
        ERRIFGOTO(result, _EXIT);

        //SetExit for all MessageQueueList to enable to destroy all the socket thread which is busy waiting for message queue
        result = CAPLinkedList_Traverse(pstInfoManager->hMessageQueueList, MessageQueueList_SetExit, NULL);
        ERRIFGOTO(result, _EXIT_ERROR);

        //destory SocketAcceptingThread while holding lock because it could add new messagequeue to the list
        result = CAPThread_Destroy(&(pstInfoManager->hSocketAcceptingThread));
        ERRIFGOTO(result, _EXIT_ERROR);
       
        //unlock
        result = CAPThreadLock_Unlock(pstInfoManager->hLock);
        ERRIFGOTO(result, _EXIT);
        
        result = CAPThread_Destroy(&(pstInfoManager->hValueTopicSubscribingThread));
        ERRIFGOTO(result, _EXIT);

        result = CAPThread_Destroy(&(pstInfoManager->hReceiveMessageToCloudThread));
        ERRIFGOTO(result, _EXIT);
        
        //Destroy all the SocketThread since messagequeue has been set exited
        result = CAPLinkedList_Traverse(pstInfoManager->hSocketThreadList, SocketThreadList_Destroy, NULL);
        ERRIFGOTO(result, _EXIT);
        
        //Destroy message queue
        result = CAPLinkedList_Traverse(pstInfoManager->hMessageQueueList, MessageQueueList_Destroy, NULL);
        ERRIFGOTO(result, _EXIT);
        
        result = MQTTMessageHandler_Disconnect(pstInfoManager->hMQTTHandler);
        ERRIFGOTO(result, _EXIT);

        pstInfoManager->bCreated = FALSE;
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
_EXIT_ERROR:
    CAPThreadLock_Unlock(pstInfoManager->hLock);
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
    
    CAPLinkedList_Destroy(&(pstInfoManager->hMessageQueueList));

    CAPLinkedList_Destroy(&(pstInfoManager->hSocketThreadList));

    CAPSocket_Destroy(&pstInfoManager->hServerSocket);

    CAPThreadLock_Destroy(&pstInfoManager->hLock);

    MQTTMessageHandler_Destroy(&(pstInfoManager->hMQTTHandler));

    SAFEMEMFREE(pstInfoManager);

    *phInfoManager = NULL;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}
