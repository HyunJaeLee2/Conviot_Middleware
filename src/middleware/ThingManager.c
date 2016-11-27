#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <json-c/json_object.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>

#include <CAPString.h>
#include <CAPThread.h>
#include <CAPQueue.h>
#include <CAPLogger.h>
#include <CAPLinkedList.h>
#include <CAPThreadLock.h>
#include <CAPThreadEvent.h>
#include <CAPTime.h>

#include <MQTT_common.h>
#include <Json_common.h>

#include "ThingManager.h"
#include "MQTTMessageHandler.h"
/*
#include "DBHandler.h"
*/
#define SECOND 1000

static char* paszThingManagerSubcriptionList[] = {
    "TM/REGISTER/#",
    "TM/UNREGISTER/#",
    "TM/ALIVE/#",
    "EM/SET_THING_ID/#",
};


#define MQTT_SUBSCRIPTION_NUM (sizeof(paszThingManagerSubcriptionList) / sizeof(char*))
#define MQTT_CLIENT_ID "Conviot_ThingManager"

#define TOPIC_SEPERATOR "/"

CAPSTRING_CONST(CAPSTR_MQTT_CLIENT_ID, MQTT_CLIENT_ID);
CAPSTRING_CONST(CAPSTR_CATEGORY_REGISTER, "REGISTER");
CAPSTRING_CONST(CAPSTR_CATEGORY_UNREGISTER, "UNREGISTER");
CAPSTRING_CONST(CAPSTR_CATEGORY_SET_THING_ID, "SET_THING_ID");
CAPSTRING_CONST(CAPSTR_CATEGORY_ALIVE, "ALIVE");
CAPSTRING_CONST(CAPSTR_TOPIC_SEPERATOR, TOPIC_SEPERATOR);

static cap_result ThingManager_PublishErrorCode(IN int errorCode, cap_handle hThingManager, cap_string strMessageReceiverId, cap_string strTopicCategory)
{

    cap_result result = ERR_CAP_UNKNOWN;
    SThingManager* pstThingManager = NULL;
    cap_string strTopic = NULL;
    char* pszPayload;
    int nPayloadLen = 0;
    EMqttErrorCode enError;
    json_object* pJsonObject;

    IFVARERRASSIGNGOTO(hThingManager, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(strMessageReceiverId, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstThingManager = (SThingManager*)hThingManager;

    //set topic
    strTopic = CAPString_New();
    ERRMEMGOTO(strTopic, result, _EXIT);

    result = CAPString_Set(strTopic, strTopicCategory);
    ERRIFGOTO(result, _EXIT);

    result = CAPString_AppendString(strTopic, strMessageReceiverId);
    ERRIFGOTO(result, _EXIT);

    //make error code depend on errorcode value
    if (errorCode == ERR_CAP_NOERROR) {
        enError = ERR_MQTT_NOERROR;
    }
    else if (errorCode == ERR_CAP_DUPLICATED) {
        enError = ERR_MQTT_DUPLICATED;
    }
    else if (errorCode == ERR_CAP_INVALID_DATA) {
        enError = ERR_MQTT_INVALID_REQUEST;
    }
    else {
        enError = ERR_MQTT_FAIL;
    }

    //set payload
    pJsonObject = json_object_new_object();
    json_object_object_add(pJsonObject, "error", json_object_new_int(enError));

    pszPayload = strdup(json_object_to_json_string(pJsonObject));
    nPayloadLen = strlen(pszPayload);

    result = MQTTMessageHandler_Publish(pstThingManager->hMQTTHandler, strTopic, pszPayload, nPayloadLen);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEJSONFREE(pJsonObject);
    SAFEMEMFREE(pszPayload);
    SAFE_CAPSTRING_DELETE(strTopic);

    return result;
}
static CALLBACK cap_result mqttMessageHandlingCallback(cap_string strTopic, cap_handle hTopicItemList,
        char *pszPayload, int nPayloadLen, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    cap_result result_save = ERR_CAP_UNKNOWN;
    SThingManager* pstThingManager = NULL;
    cap_string strCategory = NULL;
    //SThingInfo* pstThing = NULL;
    cap_string strThingId = NULL;

    IFVARERRASSIGNGOTO(pUserData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstThingManager = (SThingManager*)pUserData;

    result = CAPLinkedList_Get(hTopicItemList, LINKED_LIST_OFFSET_FIRST, TOPIC_LEVEL_SECOND, (void**)&strCategory);
    ERRIFGOTO(result, _EXIT);

    /*
    //assign thing id from linked_list (topic_level_third)
    result = getLastElementFromTopicList(hTopicItemList, &strThingId);
    ERRIFGOTO(result, _EXIT);

    if (CAPString_IsEqual(strCategory, CAPSTR_CATEGORY_REGISTER) == TRUE) {
        //create thing_Handler
        result = ThingHandler_Create(CAPString_LowPtr(strThingId, NULL), CAPString_Length(strThingId), &pstThing);
        ERRIFGOTO(result, _EXIT);

        //insert thing_handler
        result = ThingHandler_Insert(pstThing, pszPayload, nPayloadLen);
        ERRIFGOTO(result, _EXIT);

        //add thing into DB
        result = DBHandler_AddThing(pstThing);

        //Save result to check if an error occured
        result_save = result;
        
        result = ThingManager_PublishErrorCode(result, pstThingManager, strThingId, CAPSTR_REGACK);
        ERRIFGOTO(result, _EXIT);
       
        if(result_save == ERR_CAP_NOERROR){
            result = handleDisabledScenario(strThingId, pstThingManager->hAppEngine);
            ERRIFGOTO(result, _EXIT);
            
            //update latest time when register
            result = DBHandler_UpdateLatestTime(CAPString_LowPtr(strThingId, NULL));
            ERRIFGOTO(result, _EXIT);

            //subscribe sensor topic
            result = ThingManager_PutValueTopicToQueue(pstThingManager, pstThing);
            ERRIFGOTO(result, _EXIT);

            //make payload to Cloud
            result = makeMessageToCloud(pstThingManager, strCategory, strThingId, pszPayload, nPayloadLen);
            ERRIFGOTO(result, _EXIT);
        }
        result = ThingHandler_Destroy(&pstThing);
        ERRIFGOTO(result, _EXIT);
    }
    else if (CAPString_IsEqual(strCategory, CAPSTR_CATEGORY_UNREGISTER) == TRUE) {
        result = disableDependentScenario(strThingId, pstThingManager->hAppEngine);
        ERRIFGOTO(result, _EXIT);
            
        //delete thing from DB
        result = DBHandler_DeleteThing(strThingId);

        //Save result to check if an error occured
        result_save = result;
        
        result = ThingManager_PublishErrorCode(result, pstThingManager, strThingId, CAPSTR_REGACK);
        ERRIFGOTO(result, _EXIT);
        
        if(result_save == ERR_CAP_NOERROR){
            //make payload to Cloud
            result = makeMessageToCloud(pstThingManager, strCategory, strThingId, pszPayload, nPayloadLen);
            ERRIFGOTO(result, _EXIT);
        }
    }
    else if (CAPString_IsEqual(strCategory, CAPSTR_CATEGORY_ALIVE) == TRUE) {
        result = DBHandler_UpdateLatestTime(CAPString_LowPtr(strThingId, NULL));
        ERRIFGOTO(result, _EXIT);
    }
    else if (CAPString_IsEqual(strCategory, CAPSTR_CATEGORY_SET_THING_ID) == TRUE) {
        result = DBHandler_SetVirtualThingId(pszPayload, nPayloadLen);

        //Save result to check if an error occured
        result_save = result;
        
        //TODO
        //Third argument(strThingID) should be named as strClientId.
        //However, since middleware retrieves ID from the end of the topic element, it works fine functionally.
        result = ThingManager_PublishErrorCode(result, pstThingManager, strThingId, CAPSTR_SET_THING_ID_RESULT);
        ERRIFGOTO(result, _EXIT);
        
        if(result_save == ERR_CAP_NOERROR){
            //make payload to Cloud
            result = makeMessageToCloud(pstThingManager, strCategory, strThingId, pszPayload, nPayloadLen);
            ERRIFGOTO(result, _EXIT);
        }
    }
    else {

        //ERRASSIGNGOTO(result, ERR_CAP_NOT_SUPPORTED, _EXIT);
    }
    */
    result = ERR_CAP_NOERROR;

_EXIT:
    if(result != ERR_CAP_NOERROR){
        //Added if clause for a case where a thread is terminated without accepting any data at all
    }

    return result;
}


cap_result ThingManager_Create(OUT cap_handle* phThingManager, IN cap_string strBrokerURI)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SThingManager* pstThingManager = NULL;
    IFVARERRASSIGNGOTO(phThingManager, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstThingManager = (SThingManager*)malloc(sizeof(SThingManager));
    ERRMEMGOTO(pstThingManager, result, _EXIT);
/*
    pstThingManager->enID = HANDLEID_THING_MANAGER;
    pstThingManager->bCreated = FALSE;
    pstThingManager->hAliveHandlingThread = NULL;
    pstThingManager->pstThingAliveInfoArray = NULL;
    pstThingManager->hMessageToCloudQueue = NULL;
    pstThingManager->hEvent = NULL;
    pstThingManager->hMQTTHandler = NULL;
    pstThingManager->hAppEngine = NULL;
    pstThingManager->nAliveCheckingPeriod = DEFAULT_ALIVE_CHECK_TIME;
*/
    //create event for timedwait
    result = CAPThreadEvent_Create(&pstThingManager->hEvent);
    ERRIFGOTO(result, _EXIT);

    //Create MQTT Client
    result = MQTTMessageHandler_Create(strBrokerURI, CAPSTR_MQTT_CLIENT_ID, 0, &(pstThingManager->hMQTTHandler));
    ERRIFGOTO(result, _EXIT);

    *phThingManager = pstThingManager;

    result = ERR_CAP_NOERROR;
_EXIT:
    if (result != ERR_CAP_NOERROR && pstThingManager != NULL) {
        //Destroy lock before deallocating pstThingManager
        CAPThreadEvent_Destroy(&(pstThingManager->hEvent));
        SAFEMEMFREE(pstThingManager);
    }
    return result;
}

cap_result ThingManager_Run(IN cap_handle hThingManager)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SThingManager* pstThingManager = NULL;

    if (IS_VALID_HANDLE(hThingManager, HANDLEID_THING_MANAGER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }
    pstThingManager = (SThingManager*)hThingManager;

    // Already created
    if (pstThingManager->bCreated == TRUE) {
        CAPASSIGNGOTO(result, ERR_CAP_NOERROR, _EXIT);
    }

    /*
    pstThingManager->hValueTopicQueue = hValueTopicQueue;
    pstThingManager->hMessageToCloudQueue = hMessageToCloudQueue;
    pstThingManager->hAppEngine = hAppEngine;

    result = MQTTMessageHandler_SetReceiveCallback(pstThingManager->hMQTTHandler,
            mqttMessageHandlingCallback, pstThingManager);
    ERRIFGOTO(result, _EXIT);

    result = MQTTMessageHandler_Connect(pstThingManager->hMQTTHandler);
    ERRIFGOTO(result, _EXIT);

    result = MQTTMessageHandler_SubscribeMany(pstThingManager->hMQTTHandler, paszThingManagerSubcriptionList, MQTT_SUBSCRIPTION_NUM);
    ERRIFGOTO(result, _EXIT);

    pstThingManager->nAliveCheckingPeriod = nAliveCheckingPeriod;

    //pstAliveHandlingThreadData will be freed in a thread
    result = CAPThread_Create(aliveHandlingThread, pstThingManager, &(pstThingManager->hAliveHandlingThread));
    ERRIFGOTO(result, _EXIT);
    */

    pstThingManager->bCreated = TRUE;

    result = ERR_CAP_NOERROR;
_EXIT:
    if(result != ERR_CAP_NOERROR){
        //deallocate threadData if it fails to create a thread
    }
    return result;
}

cap_result ThingManager_Join(IN cap_handle hThingManager)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SThingManager* pstThingManager = NULL;

    if (IS_VALID_HANDLE(hThingManager, HANDLEID_THING_MANAGER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstThingManager = (SThingManager*)hThingManager;

    /*
    if (pstThingManager->bCreated == TRUE) {
        result = CAPThreadEvent_SetEvent(pstThingManager->hEvent);
        ERRIFGOTO(result, _EXIT);
        
        result = CAPThread_Destroy(&(pstThingManager->hAliveHandlingThread));
        ERRIFGOTO(result, _EXIT);

        result = MQTTMessageHandler_Disconnect(pstThingManager->hMQTTHandler);
        ERRIFGOTO(result, _EXIT);

        pstThingManager->bCreated = FALSE;
    }
    */

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result ThingManager_Destroy(IN OUT cap_handle* phThingManager)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SThingManager* pstThingManager = NULL;

    IFVARERRASSIGNGOTO(phThingManager, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if (IS_VALID_HANDLE(*phThingManager, HANDLEID_THING_MANAGER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstThingManager = (SThingManager*)*phThingManager;

    /*
    MQTTMessageHandler_Destroy(&(pstThingManager->hMQTTHandler));

    CAPThreadEvent_Destroy(&(pstThingManager->hEvent));

    */

    SAFEMEMFREE(pstThingManager);

    *phThingManager = NULL;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}
