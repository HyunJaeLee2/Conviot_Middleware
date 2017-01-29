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
#include "DBHandler.h"

#define SECOND 1000

static char* paszThingManagerSubcriptionList[] = {
    "TM/REGISTER/#",
    "TM/UNREGISTER/#",
    "TM/ALIVE/#",
    "TM/SEND_VARIABLE/#",
};


#define MQTT_SUBSCRIPTION_NUM (sizeof(paszThingManagerSubcriptionList) / sizeof(char*))
#define MQTT_CLIENT_ID "Conviot_ThingManager"

#define TOPIC_SEPERATOR "/"

CAPSTRING_CONST(CAPSTR_MQTT_CLIENT_ID, MQTT_CLIENT_ID);
CAPSTRING_CONST(CAPSTR_CATEGORY_REGISTER, "REGISTER");
CAPSTRING_CONST(CAPSTR_CATEGORY_UNREGISTER, "UNREGISTER");
CAPSTRING_CONST(CAPSTR_CATEGORY_SEND_VARIABLE, "SEND_VARIABLE");
CAPSTRING_CONST(CAPSTR_CATEGORY_ALIVE, "ALIVE");
CAPSTRING_CONST(CAPSTR_TOPIC_SEPERATOR, TOPIC_SEPERATOR);

CAPSTRING_CONST(CAPSTR_REGISTER_RESULT, "MT/REGISTER_RESULT/");
CAPSTRING_CONST(CAPSTR_UNREGISTER_RESULT, "MT/UNREGISTER_RESULT/");

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

        result = DBHandler_MakeThingAliveInfoArray(pstThingManager->pDBconn, &pstThingManager->pstThingAliveInfoArray, &nArrayLength);
        ERRIFGOTO(result, _EXIT);

        //If there is no thing in database, continue
        if(pstThingManager->pstThingAliveInfoArray == NULL)
            continue;
        
        result = CAPTime_GetCurTimeInMilliSeconds(&llCurrTime);
        ERRIFGOTO(result, _EXIT);
           
        for(nLoop = 0; nLoop < nArrayLength; nLoop++){
            llLatestTime = pstThingManager->pstThingAliveInfoArray[nLoop].llLatestTime;
            //Minor Adjustment to alive cycle considering network overhead
            llAliveCycle = pstThingManager->pstThingAliveInfoArray[nLoop].nAliveCycle * 1000 + 1 * SECOND;
                
            if(llCurrTime - llLatestTime > llAliveCycle){
                result = DBHandler_DisableDeviceAndEca(pstThingManager->pDBconn, pstThingManager->pstThingAliveInfoArray[nLoop].strDeviceId);  
                ERRIFGOTO(result, _EXIT);

                CAPLogger_Write(g_hLogger, MSG_INFO, "ThingManager has disabled %s and removed related ECAs for not receiving alive message.",\
                        CAPString_LowPtr(pstThingManager->pstThingAliveInfoArray[nLoop].strDeviceId, NULL));
            }
        }
        //Free all the memory of ThingAliveInfoArray 
        for (nLoop = 0; nLoop < nArrayLength; nLoop++) {
            SAFE_CAPSTRING_DELETE(pstThingManager->pstThingAliveInfoArray[nLoop].strDeviceId);
        }
        SAFEMEMFREE(pstThingManager->pstThingAliveInfoArray);
    }

    result = ERR_CAP_NOERROR;

_EXIT:
    if(result != ERR_CAP_NOERROR && pstThingManager != NULL && pstThingManager->pstThingAliveInfoArray != NULL){
        //Free all the memory of ThingAliveInfoArray 
        for (nLoop = 0; nLoop < nArrayLength; nLoop++) {
            SAFE_CAPSTRING_DELETE(pstThingManager->pstThingAliveInfoArray[nLoop].strDeviceId);
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
    char *pszApiKey = NULL;
    int nPayloadLen = 0;
    EConviotErrorCode enError;
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
        enError = ERR_CONVIOT_NOERROR;
    }
    else if (errorCode == ERR_CAP_DUPLICATED) {
        enError = ERR_CONVIOT_DUPLICATED;
    }
    else if (errorCode == ERR_CAP_INVALID_DATA) {
        enError = ERR_CONVIOT_INVALID_REQUEST;
    }
    else {
        enError = ERR_CONVIOT_FAIL;
    }
    
    //set payload
    pJsonObject = json_object_new_object();
    json_object_object_add(pJsonObject, "error", json_object_new_int(enError));

    result = DBHandler_RetrieveApiKey(pstThingManager->pDBconn, strMessageReceiverId, &pszApiKey);
    ERRIFGOTO(result, _EXIT);

    if(pszApiKey == NULL) {
        //do nothing
    } 
    else {
        json_object_object_add(pJsonObject, "apikey", json_object_new_string(pszApiKey));
    }

    pszPayload = strdup(json_object_to_json_string(pJsonObject));
    nPayloadLen = strlen(pszPayload);

    result = MQTTMessageHandler_Publish(pstThingManager->hMQTTHandler, strTopic, pszPayload, nPayloadLen);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEJSONFREE(pJsonObject);
    SAFEMEMFREE(pszPayload);
    SAFEMEMFREE(pszApiKey);
    SAFE_CAPSTRING_DELETE(strTopic);

    return result;
}
static CALLBACK cap_result mqttMessageHandlingCallback(cap_string strTopic, cap_handle hTopicItemList,
        char *pszPayload, int nPayloadLen, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    cap_result result_save = ERR_CAP_UNKNOWN;
    SThingManager* pstThingManager = NULL;
    json_object* pJsonObject, *pJsonApiKey;
    const char* pszConstApiKey = "apikey";
    cap_string strCategory = NULL;
    cap_string strDeviceId = NULL;

    IFVARERRASSIGNGOTO(pUserData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstThingManager = (SThingManager*)pUserData;

    /*Topics are set as follow
     *[TM]/[TOPIC CATEGORY]/[THING ID] and functio name of value name could be set at last topic level
     */

    //Get Category
    result = CAPLinkedList_Get(hTopicItemList, LINKED_LIST_OFFSET_FIRST, TOPIC_LEVEL_SECOND, (void**)&strCategory);
    ERRIFGOTO(result, _EXIT);
   
    //Get Thing ID
    result = CAPLinkedList_Get(hTopicItemList, LINKED_LIST_OFFSET_FIRST, TOPIC_LEVEL_THIRD, (void**)&strDeviceId);
    ERRIFGOTO(result, _EXIT);
   
   
    dlp("ThingManager received message! topic : %s, payload : %s\n", CAPString_LowPtr(strTopic, NULL),pszPayload);
    //Parse Payload to check its api key
    result = ParsingJson(&pJsonObject, pszPayload, nPayloadLen);
    ERRIFGOTO(result, _EXIT);
   
    if (!json_object_object_get_ex(pJsonObject, pszConstApiKey, &pJsonApiKey)) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }

    //TODO
    //Add Error code to each specific situation

    //save result to report error later
    //ERR_CAP_NO_DATA : no matching device
    //ERR_CAP_INVALID_DATA : api key doesn't match
    result_save = DBHandler_VerifyApiKey(pstThingManager->pDBconn, strDeviceId, (char *)json_object_get_string(pJsonApiKey));

    if (CAPString_IsEqual(strCategory, CAPSTR_CATEGORY_REGISTER) == TRUE) {
        json_object* pJsonPinCode;
        const char* pszConstPinCode = "pincode";

        //If api key error has occured, publish error code then return 
        if(result_save != ERR_CAP_NOERROR){
            result = ThingManager_PublishErrorCode(result_save, pstThingManager, strDeviceId, CAPSTR_REGISTER_RESULT);
            ERRIFGOTO(result, _EXIT);

            goto _EXIT;
        }
        
        if (!json_object_object_get_ex(pJsonObject, pszConstPinCode, &pJsonPinCode)) {
            ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
        }

        //publish error code of function result
        result_save = DBHandler_RegisterDevice(pstThingManager->pDBconn, strDeviceId, (char *)json_object_get_string(pJsonPinCode));  
        
        result = ThingManager_PublishErrorCode(result_save, pstThingManager, strDeviceId, CAPSTR_REGISTER_RESULT);
        ERRIFGOTO(result, _EXIT);

        
        /*
        if(result_save == ERR_CAP_NOERROR){
            result = handleDisabledScenario(strDeviceId, pstThingManager->hAppEngine);
            ERRIFGOTO(result, _EXIT);
            
            //update latest time when register
            result = DBHandler_UpdateLatestTime(CAPString_LowPtr(strDeviceId, NULL));
            ERRIFGOTO(result, _EXIT);
        }
        */
    }
    else if (CAPString_IsEqual(strCategory, CAPSTR_CATEGORY_UNREGISTER) == TRUE) {
        json_object* pJsonPinCode;
        const char* pszConstPinCode = "pincode";
        
        //If api key error has occured, publish error code then return 
        if(result_save != ERR_CAP_NOERROR){
            result = ThingManager_PublishErrorCode(result_save, pstThingManager, strDeviceId, CAPSTR_UNREGISTER_RESULT);
            ERRIFGOTO(result, _EXIT);

            goto _EXIT;
        }
        
        if (!json_object_object_get_ex(pJsonObject, pszConstPinCode, &pJsonPinCode)) {
            ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
        }

        //publish error code of function result
        result_save = DBHandler_UnregisterDevice(pstThingManager->pDBconn, strDeviceId, (char *)json_object_get_string(pJsonPinCode));  
        
        result = ThingManager_PublishErrorCode(result_save, pstThingManager, strDeviceId, CAPSTR_UNREGISTER_RESULT);
        ERRIFGOTO(result, _EXIT);

        
        /*
        result = disableDependentScenario(strDeviceId, pstThingManager->hAppEngine);
        ERRIFGOTO(result, _EXIT);
            
        //delete thing from DB
        result = DBHandler_DeleteThing(strDeviceId);

        //Save result to check if an error occured
        result_save = result;
        
        result = ThingManager_PublishErrorCode(result, pstThingManager, strDeviceId, CAPSTR_REGACK);
        ERRIFGOTO(result, _EXIT);
        
        if(result_save == ERR_CAP_NOERROR){
            //make payload to Cloud
            result = makeMessageToCloud(pstThingManager, strCategory, strDeviceId, pszPayload, nPayloadLen);
            ERRIFGOTO(result, _EXIT);
        }
        */
    }
    else if (CAPString_IsEqual(strCategory, CAPSTR_CATEGORY_ALIVE) == TRUE) {
        //If api key error has occured, goto exit 
        if(result_save != ERR_CAP_NOERROR){
            goto _EXIT;
        }

        //ignore error because alive message does not have a return type
        result = DBHandler_UpdateLatestTime(pstThingManager->pDBconn, strDeviceId);
    }
    else if (CAPString_IsEqual(strCategory, CAPSTR_CATEGORY_SEND_VARIABLE) == TRUE) {
        json_object* pJsonVariable;
        cap_string strVariableName = NULL;
        const char* pszConstVariable = "variable";
        
        //If api key error has occured, goto exit 
        if(result_save != ERR_CAP_NOERROR){
            goto _EXIT;
        }
        
        //Get variable name 
        result = CAPLinkedList_Get(hTopicItemList, LINKED_LIST_OFFSET_FIRST, TOPIC_LEVEL_FOURTH, (void**)&strVariableName);
        ERRIFGOTO(result, _EXIT);
        
        
        if (!json_object_object_get_ex(pJsonObject, pszConstVariable, &pJsonVariable)) {
            ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
        }
        
        //ignore error because send variable does not have a return type
        result = DBHandler_InsertVariableHistory(pstThingManager->pDBconn, strDeviceId, strVariableName, (char *)json_object_get_string(pJsonVariable));
    }
    else {
        ERRASSIGNGOTO(result, ERR_CAP_NOT_SUPPORTED, _EXIT);
    }
    result = ERR_CAP_NOERROR;

_EXIT:
    if(result != ERR_CAP_NOERROR){
        //Added if clause for a case where a thread is terminated without accepting any data at all
    }

    return result;
}


cap_result ThingManager_Create(OUT cap_handle* phThingManager, IN cap_string strBrokerURI, IN SDBInfo *pstDBInfo)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SThingManager* pstThingManager = NULL;
    IFVARERRASSIGNGOTO(phThingManager, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstThingManager = (SThingManager*)malloc(sizeof(SThingManager));
    ERRMEMGOTO(pstThingManager, result, _EXIT);
    
    pstThingManager->enID = HANDLEID_THING_MANAGER;
    pstThingManager->bCreated = FALSE;
    pstThingManager->hAliveHandlingThread = NULL;
    pstThingManager->pstThingAliveInfoArray = NULL;
    pstThingManager->hEvent = NULL;
    pstThingManager->hMQTTHandler = NULL;
    pstThingManager->nAliveCheckingPeriod = 0;
    
    result = DBHandler_OpenDB(pstDBInfo, &pstThingManager->pDBconn);
    ERRIFGOTO(result, _EXIT);

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
        CAPThreadEvent_Destroy(&(pstThingManager->hEvent));
        SAFEMEMFREE(pstThingManager);
    }
    return result;
}

cap_result ThingManager_Run(IN cap_handle hThingManager, IN int nAliveCheckingPeriod)
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

        result = DBHandler_CloseDB(pstThingManager->pDBconn);
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
    SThingManager* pstThingManager = NULL;

    IFVARERRASSIGNGOTO(phThingManager, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if (IS_VALID_HANDLE(*phThingManager, HANDLEID_THING_MANAGER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstThingManager = (SThingManager*)*phThingManager;

    MQTTMessageHandler_Destroy(&(pstThingManager->hMQTTHandler));

    CAPThreadEvent_Destroy(&(pstThingManager->hEvent));

    SAFEMEMFREE(pstThingManager);

    *phThingManager = NULL;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}
