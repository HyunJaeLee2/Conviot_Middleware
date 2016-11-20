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
#include "ThingHandler.h"
#include "DBHandler.h"
#include "MQTTMessageHandler.h"
#include "AppEngine.h"

#define SECOND 1000
/*
#define PING_CYCLE 400
#define PING_WAIT_THING 7
*/

#define DEFAULT_ALIVE_CHECK_TIME (60)

static char* paszThingManagerSubcriptionList[] = {
    "TM/REGISTER/#",
    "TM/UNREGISTER/#",
    "TM/ALIVE/#",
    "EM/SET_THING_ID/#",
};


#define MQTT_SUBSCRIPTION_NUM (sizeof(paszThingManagerSubcriptionList) / sizeof(char*))
#define MQTT_CLIENT_ID "cap_iot_middleware_thing"

#define TOPIC_SEPERATOR "/"

CAPSTRING_CONST(CAPSTR_MQTT_CLIENT_ID, MQTT_CLIENT_ID);
CAPSTRING_CONST(CAPSTR_CATEGORY_REGISTER, "REGISTER");
CAPSTRING_CONST(CAPSTR_CATEGORY_UNREGISTER, "UNREGISTER");
CAPSTRING_CONST(CAPSTR_CATEGORY_SET_THING_ID, "SET_THING_ID");
CAPSTRING_CONST(CAPSTR_CATEGORY_ALIVE, "ALIVE");
CAPSTRING_CONST(CAPSTR_TOPIC_SEPERATOR, TOPIC_SEPERATOR);

CAPSTRING_CONST(CAPSTR_REGACK, "MT/REGACK/");
CAPSTRING_CONST(CAPSTR_SET_THING_ID_RESULT, "ME/RESULT/SET_THING_ID/");
CAPSTRING_CONST(CAPSTR_PINGREQ, "MT/PINGREQ/");


/*
static CALLBACK cap_result destroyValueTopic(int nOffset, void* pData, void* pUsrData)
{
    cap_result result = ERR_CAP_UNKNOWN;

    SAFE_CAPSTRING_DELETE(pData);

    result = ERR_CAP_NOERROR;
    return result;
}
*/
static cap_result makeUpdateNameObject(json_object *pJsonReceivedObject, json_object *pJsonToSendObject){
    cap_result result = ERR_CAP_UNKNOWN;

    json_object* pJsonOldId = NULL;
    json_object* pJsonNewId = NULL;
    char *pszOldId = NULL;
    char *pszNewId = NULL;
    const char* pszConstOldId = "old_id";
    const char* pszConstNewId = "new_id";

    if (!json_object_object_get_ex(pJsonReceivedObject, pszConstOldId, &pJsonOldId)) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }

    if (!json_object_object_get_ex(pJsonReceivedObject, pszConstNewId, &pJsonNewId)) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }

    pszOldId = strdup(json_object_get_string(pJsonOldId));
    pszNewId = strdup(json_object_get_string(pJsonNewId));

    json_object_object_add(pJsonToSendObject, "msg_type", json_object_new_string((const char*)"update_virtual_thing_name"));
    json_object_object_add(pJsonToSendObject, "old_name", json_object_new_string((const char*)pszOldId));
    json_object_object_add(pJsonToSendObject, "new_name", json_object_new_string((const char*)pszNewId));

    SAFEMEMFREE(pszOldId);
    SAFEMEMFREE(pszNewId);
    
    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

static cap_result makeRegisterObject(json_object *pJsonReceivedObject, json_object *pJsonToSendObject, cap_string strThingId){
    cap_result result = ERR_CAP_UNKNOWN;

    json_object *pJsonValueArray, *pJsonDescription;
    const char* pszValue = "values", *pszDescription = "description";

//    pJsonDescription = json_object_new_object();
//    ERRMEMGOTO(pJsonDescription, result, _EXIT);
//    
//    pJsonValueArray = json_object_new_object();
//    ERRMEMGOTO(pJsonValueArray, result, _EXIT);

    if (!json_object_object_get_ex(pJsonReceivedObject, pszValue, &pJsonValueArray)) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }
    json_object_object_add(pJsonToSendObject, "msg_type", json_object_new_string((const char*)"register_thing"));
    json_object_object_add(pJsonToSendObject, "thing_name", json_object_new_string((const char*)CAPString_LowPtr(strThingId, NULL)));
  
    //description is optional
    if (json_object_object_get_ex(pJsonReceivedObject, pszDescription, &pJsonDescription)) {
        json_object_object_add(pJsonToSendObject, "description", json_object_get(pJsonDescription));
    }

    json_object_object_add(pJsonToSendObject, "values", json_object_get(pJsonValueArray));

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;

}

static cap_result makeMessageToCloud(IN cap_handle hThingManager, IN cap_string strCategory, IN cap_string strThingId, char *pszPayload, int nPayloadLen){
    cap_result result = ERR_CAP_UNKNOWN;
    SThingManager* pstThingManager = NULL;
    char *pszMessage = NULL;
    json_object* pJsonReceivedObject = NULL;
    json_object* pJsonToSendObject = NULL;
    
    //parse payload
    result = ParsingJson(&pJsonReceivedObject, pszPayload, nPayloadLen);
    ERRIFGOTO(result, _EXIT);
    
    pJsonToSendObject = json_object_new_object();
    ERRMEMGOTO(pJsonToSendObject, result, _EXIT);

    IFVARERRASSIGNGOTO(hThingManager, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstThingManager = (SThingManager*)hThingManager;   
    
    if (CAPString_IsEqual(strCategory, CAPSTR_CATEGORY_REGISTER) == TRUE) {
        result = makeRegisterObject(pJsonReceivedObject, pJsonToSendObject, strThingId);
        ERRIFGOTO(result, _EXIT);
    }
    else if (CAPString_IsEqual(strCategory, CAPSTR_CATEGORY_UNREGISTER) == TRUE) {
        json_object_object_add(pJsonToSendObject, "msg_type", json_object_new_string((const char*)"unregister_thing"));
        json_object_object_add(pJsonToSendObject, "thing_name", json_object_new_string((const char*)CAPString_LowPtr(strThingId, NULL)));
    }
    else if (CAPString_IsEqual(strCategory, CAPSTR_CATEGORY_SET_THING_ID) == TRUE) {
        result = makeUpdateNameObject(pJsonReceivedObject, pJsonToSendObject);
        ERRIFGOTO(result, _EXIT);
    }
    else {
        ERRASSIGNGOTO(result, ERR_CAP_NOT_SUPPORTED, _EXIT);
    }
    
    pszMessage = strdup(json_object_to_json_string(pJsonToSendObject));
   
    result = CAPQueue_Put(pstThingManager->hMessageToCloudQueue, pszMessage);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEJSONFREE(pJsonReceivedObject);
    SAFEJSONFREE(pJsonToSendObject);
    return result;

}

static cap_result traverseScenarioList(IN int nOffset, IN void *pData, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    cap_string strScenarioName = NULL;

    strScenarioName = pData;

    SAFE_CAPSTRING_DELETE(strScenarioName);

    result = ERR_CAP_NOERROR;

    return result;
}

static cap_result handleDisabledScenario(IN cap_string strThingId, IN cap_handle hAppEngine)
{
    cap_result result = ERR_CAP_UNKNOWN;
    cap_handle hScenarioList = NULL;
    cap_string strScenarioName = NULL;
    int nLength = 0;
    int nLoop = 0;
    
    result = CAPLinkedList_Create(&hScenarioList);
    ERRIFGOTO(result, _EXIT);
    
    //Check thing_left_to_enable table in database.
    //If all things are registered, change state of scenario
    //List of Scenarios those need to be enabled is returned from function
    result = DBHandler_CheckIsReadyScenario(strThingId, hScenarioList);
    ERRIFGOTO(result, _EXIT);
    
    result = CAPLinkedList_GetLength(hScenarioList, &nLength);
    ERRIFGOTO(result, _EXIT);
        
    for (nLoop = 0; nLoop < nLength; nLoop++) {
        result = CAPLinkedList_Get(hScenarioList, LINKED_LIST_OFFSET_FIRST, nLoop, (void**)&strScenarioName);
        ERRIFGOTO(result, _EXIT);
        
        //Change state of scenario
        result = DBHandler_ChangeScenarioState(strScenarioName, FALSE);
        ERRIFGOTO(result, _EXIT);
        
        result = AppEngine_RunScenario(hAppEngine, strScenarioName);
        ERRIFGOTO(result, _EXIT);
    }
            
    result = ERR_CAP_NOERROR;

_EXIT:
    if(result != ERR_CAP_NOERROR){
        //Added if clause for a case where a thread is terminated without accepting any data at all
    }
    if(hScenarioList != NULL)
    {
        CAPLinkedList_Traverse(hScenarioList, traverseScenarioList, NULL);
        CAPLinkedList_Destroy(&hScenarioList);
    }

    return result;
}

static cap_result disableDependentScenario(IN cap_string strThingId, IN cap_handle hAppEngine){
    cap_result result = ERR_CAP_UNKNOWN;
    cap_handle hScenarioList = NULL;
    cap_string strScenarioName = NULL;
    int nLength = 0;
    int nLoop = 0;

    result = CAPLinkedList_Create(&hScenarioList);
    ERRIFGOTO(result, _EXIT);
    
    //Check thing_left_to_enable table in database.
    //If there exists a scenario that needs unregistered thing, disable scenario.
    //List of Scenarios those need to be disabled is returned from function
    result = DBHandler_CheckScenarioToDisable(strThingId, hScenarioList);
    ERRIFGOTO(result, _EXIT);
        
    result = CAPLinkedList_GetLength(hScenarioList, &nLength);
    ERRIFGOTO(result, _EXIT);
        
    for (nLoop = 0; nLoop < nLength; nLoop++) {
        result = CAPLinkedList_Get(hScenarioList, LINKED_LIST_OFFSET_FIRST, nLoop, (void**)&strScenarioName);
        ERRIFGOTO(result, _EXIT);
      
        //Change state of scenario
        result = DBHandler_ChangeScenarioState(strScenarioName, TRUE);
        ERRIFGOTO(result, _EXIT);

        result = AppEngine_StopScenario(hAppEngine, strScenarioName);
        ERRIFGOTO(result, _EXIT);
    }
    
    result = ERR_CAP_NOERROR;

_EXIT:
    if(result != ERR_CAP_NOERROR){
        //Added if clause for a case where a thread is terminated without accepting any data at all
    }
    if(hScenarioList != NULL)
    {
        CAPLinkedList_Traverse(hScenarioList, traverseScenarioList, NULL);
        CAPLinkedList_Destroy(&hScenarioList);
    }

    return result;
}

static cap_result ThingManager_PutValueTopicToQueue(IN cap_handle hThingManager, IN cap_handle hThing)
{
    cap_result result = ERR_CAP_UNKNOWN;
    cap_string strTopic = NULL;
    SThingManager* pstThingManager = NULL;
    SThingInfo *pstThing = NULL;
    int nLoop = 0;

    IFVARERRASSIGNGOTO(hThing, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(hThingManager, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstThing = (SThingInfo*)hThing;
    pstThingManager = (SThingManager*)hThingManager;   
    
    // insert things values into values_table
    for (nLoop = 0; nLoop < pstThing->nValueNum; nLoop++) {
        SValue* pstValues = &(pstThing->pstValues[nLoop]);

        strTopic = CAPString_New();
        ERRMEMGOTO(strTopic, result, _EXIT);

        result = CAPString_PrintFormat(strTopic, "%s%s%s", CAPString_LowPtr(pstThing->strThingId, NULL),
                TOPIC_SEPERATOR, CAPString_LowPtr(pstValues->strValueName, NULL));
        ERRIFGOTO(result, _EXIT);
    
        result = CAPQueue_Put(pstThingManager->hValueTopicQueue, strTopic);
        ERRIFGOTO(result, _EXIT);

        strTopic = NULL;
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFE_CAPSTRING_DELETE(strTopic);
    return result;
}

CAP_THREAD_HEAD aliveHandlingThread(IN void* pUserData)
{
	cap_result result = ERR_CAP_UNKNOWN;
	SThingManager* pstThingManager = NULL;
    int nAliveCheckingPeriod = 0;
	int nArrayLength = 0;
    int nLoop = 0;
    long long llCurrTime = 0;
    long long llLatestTime = 0;
    long long llAliveCycle = 0;

	IFVARERRASSIGNGOTO(pUserData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

	pstThingManager = (SThingManager *)pUserData;

	nAliveCheckingPeriod = pstThingManager->nAliveCheckingPeriod;

	while (g_bExit == FALSE) {
		//timeout doesn't count as an error
		result = CAPThreadEvent_WaitTimeEvent(pstThingManager->hEvent, (long long) nAliveCheckingPeriod*SECOND);
		if(result == ERR_CAP_TIME_EXPIRED)
		{
		    result = ERR_CAP_NOERROR;
		}
		ERRIFGOTO(result, _EXIT);

        result = DBHandler_MakeThingAliveInfoArray(&pstThingManager->pstThingAliveInfoArray, &nArrayLength);
        ERRIFGOTO(result, _EXIT);

        //If there is no thing in database, continue
        if(pstThingManager->pstThingAliveInfoArray == NULL)
            continue;

        for(nLoop = 0; nLoop < nArrayLength; nLoop++){
            llLatestTime = pstThingManager->pstThingAliveInfoArray[nLoop].llLatestTime;
            
            //Minor Adjustment to alive cycle considering network overhead
            llAliveCycle = pstThingManager->pstThingAliveInfoArray[nLoop].nAliveCycle * 1000 + 1 * SECOND;

            result = CAPTime_GetCurTimeInMilliSeconds(&llCurrTime);
            ERRIFGOTO(result, _EXIT);
           
            if(llCurrTime - llLatestTime > llAliveCycle){
                result = disableDependentScenario(pstThingManager->pstThingAliveInfoArray[nLoop].strThingId, pstThingManager->hAppEngine);
                ERRIFGOTO(result, _EXIT);
                
                result = DBHandler_DeleteThing(pstThingManager->pstThingAliveInfoArray[nLoop].strThingId);
                ERRIFGOTO(result, _EXIT);
                
                result = makeMessageToCloud(pstThingManager, CAPSTR_CATEGORY_UNREGISTER, pstThingManager->pstThingAliveInfoArray[nLoop].strThingId, NULL, 0);
                ERRIFGOTO(result, _EXIT);

                CAPLogger_Write(g_hLogger, MSG_INFO, "ThingManager has unregistered %s for not receiving alive message.",\
                        CAPString_LowPtr(pstThingManager->pstThingAliveInfoArray[nLoop].strThingId, NULL));
            }
        }
        //Free all the memory of ThingAliveInfoArray 
        for (nLoop = 0; nLoop < nArrayLength; nLoop++) {
            SAFE_CAPSTRING_DELETE(pstThingManager->pstThingAliveInfoArray[nLoop].strThingId);
        }
        SAFEMEMFREE(pstThingManager->pstThingAliveInfoArray);
    }

    result = ERR_CAP_NOERROR;

_EXIT:
    if(result != ERR_CAP_NOERROR && pstThingManager != NULL && pstThingManager->pstThingAliveInfoArray != NULL){
        //Free all the memory of ThingAliveInfoArray 
        for (nLoop = 0; nLoop < nArrayLength; nLoop++) {
            SAFE_CAPSTRING_DELETE(pstThingManager->pstThingAliveInfoArray[nLoop].strThingId);
        }
        SAFEMEMFREE(pstThingManager->pstThingAliveInfoArray);
    }

    CAP_THREAD_END;
}

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
    SThingInfo* pstThing = NULL;
    cap_string strThingId = NULL;

    IFVARERRASSIGNGOTO(pUserData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstThingManager = (SThingManager*)pUserData;

    result = CAPLinkedList_Get(hTopicItemList, LINKED_LIST_OFFSET_FIRST, TOPIC_LEVEL_SECOND, (void**)&strCategory);
    ERRIFGOTO(result, _EXIT);
    
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
    SThingManager* pstThingMgmr = NULL;
    IFVARERRASSIGNGOTO(phThingManager, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstThingMgmr = (SThingManager*)malloc(sizeof(SThingManager));
    ERRMEMGOTO(pstThingMgmr, result, _EXIT);

    pstThingMgmr->enID = HANDLEID_THING_MANAGER;
    pstThingMgmr->bCreated = FALSE;
    pstThingMgmr->hAliveHandlingThread = NULL;
    pstThingMgmr->pstThingAliveInfoArray = NULL;
    pstThingMgmr->hValueTopicQueue = NULL;
    pstThingMgmr->hMessageToCloudQueue = NULL;
    pstThingMgmr->hEvent = NULL;
    pstThingMgmr->hMQTTHandler = NULL;
    pstThingMgmr->hAppEngine = NULL;
    pstThingMgmr->nAliveCheckingPeriod = DEFAULT_ALIVE_CHECK_TIME;

    //create event for timedwait
    result = CAPThreadEvent_Create(&pstThingMgmr->hEvent);
    ERRIFGOTO(result, _EXIT);

    /*
    //create lock for ping
    result = CAPThreadLock_Create(&pstThingMgmr->hPingLock);
    ERRIFGOTO(result, _EXIT);
    */

    //Create MQTT Client
    result = MQTTMessageHandler_Create(strBrokerURI, CAPSTR_MQTT_CLIENT_ID, 0, &(pstThingMgmr->hMQTTHandler));
    ERRIFGOTO(result, _EXIT);

    *phThingManager = pstThingMgmr;

    result = ERR_CAP_NOERROR;
_EXIT:
    if (result != ERR_CAP_NOERROR && pstThingMgmr != NULL) {
        //Destroy lock before deallocating pstThingMgmr
        CAPThreadEvent_Destroy(&(pstThingMgmr->hEvent));
        SAFEMEMFREE(pstThingMgmr);
    }
    return result;
}

cap_result ThingManager_Run(IN cap_handle hThingManager, IN cap_handle hValueTopicQueue, IN cap_handle hMessageToCloudQueue,\
        IN int nAliveCheckingPeriod, cap_handle hAppEngine)
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

    if (pstThingManager->bCreated == TRUE) {
        result = CAPThreadEvent_SetEvent(pstThingManager->hEvent);
        ERRIFGOTO(result, _EXIT);
        
        result = CAPThread_Destroy(&(pstThingManager->hAliveHandlingThread));
        ERRIFGOTO(result, _EXIT);

        result = MQTTMessageHandler_Disconnect(pstThingManager->hMQTTHandler);
        ERRIFGOTO(result, _EXIT);

        pstThingManager->bCreated = FALSE;
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result ThingManager_Destroy(IN OUT cap_handle* phThingManager)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SThingManager* pstThingMgmr = NULL;

    IFVARERRASSIGNGOTO(phThingManager, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if (IS_VALID_HANDLE(*phThingManager, HANDLEID_THING_MANAGER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstThingMgmr = (SThingManager*)*phThingManager;

    /*
    //destroy lock for ping
    CAPThreadLock_Destroy(&pstThingMgmr->hPingLock);
    */

    MQTTMessageHandler_Destroy(&(pstThingMgmr->hMQTTHandler));

    CAPThreadEvent_Destroy(&(pstThingMgmr->hEvent));

    SAFEMEMFREE(pstThingMgmr);

    *phThingManager = NULL;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}
