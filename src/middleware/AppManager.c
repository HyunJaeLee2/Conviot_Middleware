
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <json-c/json_object.h>

#include <CAPString.h>
#include <CAPLinkedList.h>
#include <CAPThreadEvent.h>
#include <CAPQueue.h>
#include <CAPLogger.h>
#include <Json_common.h>

#include "AppManager.h"
#include "DBHandler.h"
#include "MQTTMessageHandler.h"

#define MQTT_CLIENT_ID "Conviot_AppManager"
#define MQTT_CLIENT_DISCONNECT_TIMEOUT (10000)
#define MQTT_RECEIVE_TIMEOUT (3000)
#define MQTT_SUBSCRIPTION_NUM (sizeof(paszAppManagerSubcriptionList) / sizeof(char*))

static char* paszAppManagerSubcriptionList[] = {
   // "TM/SEND_VARIABLE/#",
    "TM/FUNRTION_RESULT/#",
};


CAPSTRING_CONST(CAPSTR_MQTT_CLIENT_ID, MQTT_CLIENT_ID);

CAPSTRING_CONST(CAPSTR_CATEGORY_FUNCTION_RESULT, "FUNCTION_RESULT");
CAPSTRING_CONST(CAPSTR_CATEGORY_SEND_VARIABLE, "SEND_VARIABLE");

CAPSTRING_CONST(CAPSTR_SCENARIO_RESULT, "MT/REQUEST_FUNCTION/");

CAPSTRING_CONST(CAPSTR_TOPIC_SEPERATOR, "/");
CAPSTRING_CONST(CAPSTR_MT, "MT/");


static cap_result AppManager_PublishErrorCode(IN int errorCode, cap_handle hAppManager, cap_string strClientId,
        cap_string strTopicCategory, cap_string strErrorString)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SAppManager* pstAppManager = NULL;
    cap_string strTopic = NULL;
    char* pszPayload = NULL;
    int nPayloadLen = 0;
    EMqttErrorCode enError;
    json_object* pJsonObject;
    
    IFVARERRASSIGNGOTO(hAppManager, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(strClientId, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(strErrorString, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstAppManager = (SAppManager*)hAppManager;

    //set topic
    strTopic = CAPString_New();
    ERRMEMGOTO(strTopic, result, _EXIT);

    result = CAPString_Set(strTopic, strTopicCategory);
    ERRIFGOTO(result, _EXIT);

    result = CAPString_AppendString(strTopic, strClientId);
    ERRIFGOTO(result, _EXIT);

    //make error code depend on result value
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

    if( CAPString_Length(strErrorString) > 0)
    {
        json_object_object_add(pJsonObject, "error_string", json_object_new_string(CAPString_LowPtr(strErrorString, NULL)));
    }

    pszPayload = strdup(json_object_to_json_string(pJsonObject));
    nPayloadLen = strlen(pszPayload);

    result = MQTTMessageHandler_Publish(pstAppManager->hMQTTHandler, strTopic, pszPayload, nPayloadLen);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEJSONFREE(pJsonObject);
    SAFEMEMFREE(pszPayload);
    SAFE_CAPSTRING_DELETE(strTopic);
    return result;
}

static cap_result handleUserApplication(IN cap_string strDeviceId, IN cap_string strVariableName, IN char *pszVariable) {
        //TODO
        //1. Get Condition List and ECA List
        //2. Compute Each condition then push it into db
        //3. Compute each eca if condition is met
        //4. Actuate function where eca condition is met
        
}

static CALLBACK cap_result mqttMessageHandlingCallback(cap_string strTopic, cap_handle hTopicItemList,
        char *pszPayload, int nPayloadLen, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    cap_result result_save = ERR_CAP_UNKNOWN;
    json_object* pJsonObject, *pJsonApiKey;
    const char* pszConstApiKey = "apikey";
    cap_string strCategory = NULL;
    cap_string strDeviceId = NULL;

    SAppManager* pstAppManager = NULL;

    IFVARERRASSIGNGOTO(pUserData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstAppManager = (SAppManager *) pUserData;


    /*Topics are set as follow
     *[TM]/[TOPIC CATEGORY]/[THING ID] and functio name of value name could be set at last topic level
     */

    //Get Category
    result = CAPLinkedList_Get(hTopicItemList, LINKED_LIST_OFFSET_FIRST, TOPIC_LEVEL_SECOND, (void**)&strCategory);
    ERRIFGOTO(result, _EXIT);
   
    //Get Thing ID
    result = CAPLinkedList_Get(hTopicItemList, LINKED_LIST_OFFSET_FIRST, TOPIC_LEVEL_THIRD, (void**)&strDeviceId);
    ERRIFGOTO(result, _EXIT);
    
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
    result_save = DBHandler_VerifyApiKey(strDeviceId, (char *)json_object_get_string(pJsonApiKey));

    if (CAPString_IsEqual(strCategory, CAPSTR_CATEGORY_SEND_VARIABLE) == TRUE) {
        json_object* pJsonVariable;
        cap_string strVariableName = NULL;
        const char* pszConstVariable = "variable";
        
        //If api key error has occured, goto exit 
        if(result_save != ERR_CAP_NOERROR){
            goto _EXIT;
        }
        
        if (!json_object_object_get_ex(pJsonObject, pszConstVariable, &pJsonVariable)) {
            ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
        }
        
        //Get variable name 
        result = CAPLinkedList_Get(hTopicItemList, LINKED_LIST_OFFSET_FIRST, TOPIC_LEVEL_FOURTH, (void**)&strVariableName);
        ERRIFGOTO(result, _EXIT);

        /*
        result = handleUserApplication(strDeviceId, strVariableName, (char *)json_object_get_string(pJsonVariable));
        ERRIFGOTO(result, _EXIT);
        */

    }
    else if (CAPString_IsEqual(strCategory, CAPSTR_CATEGORY_FUNCTION_RESULT) == TRUE) {
        json_object* pJsonTemp;
        cap_string strFunctionName = NULL;
        const char* pszConstEcaId = "eca_id", *pszConstError = "error";
        int nErrorCode = 0, nEcdId;
       
        /*
        //If api key error has occured, goto exit 
        if(result_save != ERR_CAP_NOERROR){
            goto _EXIT;
        }
        */
        
        if (!json_object_object_get_ex(pJsonObject, pszConstEcaId, &pJsonTemp)) {
            ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
        }

        nEcdId = json_object_get_int(pJsonTemp);
        
        if (!json_object_object_get_ex(pJsonObject, pszConstError, &pJsonTemp)) {
            ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
        }
        
        nErrorCode = json_object_get_int(pJsonTemp);

        //Get variable name 
        result = CAPLinkedList_Get(hTopicItemList, LINKED_LIST_OFFSET_FIRST, TOPIC_LEVEL_FOURTH, (void**)&strFunctionName);
        ERRIFGOTO(result, _EXIT);

        //TODO
        //1.Insert into applicationhistory of its result
        
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

cap_result AppManager_Create(OUT cap_handle* phAppManager, cap_string strBrokerURI)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SAppManager* pstAppManager = NULL;

    IFVARERRASSIGNGOTO(phAppManager, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstAppManager = (SAppManager*)malloc(sizeof(SAppManager));
    ERRMEMGOTO(pstAppManager, result, _EXIT);

    pstAppManager->enID = HANDLEID_APP_MANAGER;
    pstAppManager->bCreated = FALSE;
    pstAppManager->strBrokerURI = NULL;
    pstAppManager->hMQTTHandler = NULL;

    pstAppManager->strBrokerURI = CAPString_New();
    ERRMEMGOTO(pstAppManager->strBrokerURI, result, _EXIT);

    result = CAPString_Set(pstAppManager->strBrokerURI, strBrokerURI);
    ERRIFGOTO(result, _EXIT);

    result = MQTTMessageHandler_Create(pstAppManager->strBrokerURI, CAPSTR_MQTT_CLIENT_ID, 0,
            &(pstAppManager->hMQTTHandler));
    ERRIFGOTO(result, _EXIT);

    *phAppManager = pstAppManager;

    result = ERR_CAP_NOERROR;
_EXIT:
    if (result != ERR_CAP_NOERROR) {
        //Destroy lock before deallocating pstThingMgmr
        if(pstAppManager != NULL){
            SAFE_CAPSTRING_DELETE(pstAppManager->strBrokerURI);
        }
        SAFEMEMFREE(pstAppManager);
    }
    return result;
}

cap_result AppManager_Run(cap_handle hAppManager)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SAppManager* pstAppManager = NULL;

    if (IS_VALID_HANDLE(hAppManager, HANDLEID_APP_MANAGER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }
    pstAppManager = (SAppManager*)hAppManager;

    // Already created
    if (pstAppManager->bCreated == TRUE) {
        CAPASSIGNGOTO(result, ERR_CAP_NOERROR, _EXIT);
    }

    result = MQTTMessageHandler_SetReceiveCallback(pstAppManager->hMQTTHandler,
            mqttMessageHandlingCallback, pstAppManager);
    ERRIFGOTO(result, _EXIT);

    result = MQTTMessageHandler_Connect(pstAppManager->hMQTTHandler);
    ERRIFGOTO(result, _EXIT);

    result = MQTTMessageHandler_SubscribeMany(pstAppManager->hMQTTHandler, paszAppManagerSubcriptionList, MQTT_SUBSCRIPTION_NUM);
    ERRIFGOTO(result, _EXIT);

    pstAppManager->bCreated = TRUE;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result AppManager_Join(IN cap_handle hAppManager)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SAppManager* pstAppManager = NULL;

    if (IS_VALID_HANDLE(hAppManager, HANDLEID_APP_MANAGER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstAppManager = (SAppManager*)hAppManager;

    if (pstAppManager->bCreated == TRUE) {
        result = MQTTMessageHandler_Disconnect(pstAppManager->hMQTTHandler);
        ERRIFGOTO(result, _EXIT);
    }

    pstAppManager->bCreated = FALSE;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result AppManager_Destroy(IN OUT cap_handle* phAppManager)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SAppManager* pstAppManager = NULL;

    IFVARERRASSIGNGOTO(phAppManager, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if (IS_VALID_HANDLE(*phAppManager, HANDLEID_APP_MANAGER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstAppManager = (SAppManager*)*phAppManager;

    MQTTMessageHandler_Destroy(&(pstAppManager->hMQTTHandler));

    SAFE_CAPSTRING_DELETE(pstAppManager->strBrokerURI);

    SAFEMEMFREE(pstAppManager);

    *phAppManager = NULL;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}
