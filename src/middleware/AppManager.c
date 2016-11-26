
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
//#include "DBHandler.h"
#include "MQTTMessageHandler.h"

#define MQTT_CLIENT_ID "cap_iot_middleware_app_runner"
#define MQTT_CLIENT_DISCONNECT_TIMEOUT (10000)
#define MQTT_RECEIVE_TIMEOUT (3000)
#define MQTT_SUBSCRIPTION_NUM (sizeof(paszAppManagerSubcriptionList) / sizeof(char*))

static char* paszAppManagerSubcriptionList[] = {
    "EM/ADD_SCENARIO/#",
    "EM/DELETE_SCENARIO/#",
    "EM/RUN_SCENARIO/#",
    "EM/STOP_SCENARIO/#",
    "EM/ACTUATE/#",
    "EM/VERIFY_SCENARIO/#"
};


CAPSTRING_CONST(CAPSTR_MQTT_CLIENT_ID, MQTT_CLIENT_ID);
CAPSTRING_CONST(CAPSTR_SCENARIO_RESULT, "ME/RESULT/SCENARIO/");
CAPSTRING_CONST(CAPSTR_SCENARIO_ADD_RESULT, "ME/RESULT/ADD_SCENARIO/");
CAPSTRING_CONST(CAPSTR_SCENARIO_VERIFY_RESULT, "ME/RESULT/VERIFY_SCENARIO/");
CAPSTRING_CONST(CAPSTR_SCENARIO_DELETE_RESULT, "ME/RESULT/DELETE_SCENARIO/");
CAPSTRING_CONST(CAPSTR_ACTUATE_RESULT, "ME/RESULT/ACTUATE/");
CAPSTRING_CONST(CAPSTR_SCENARIO_RUN_RESULT, "ME/RESULT/RUN_SCENARIO/");
CAPSTRING_CONST(CAPSTR_SCENARIO_STOP_RESULT, "ME/RESULT/STOP_SCENARIO/");

CAPSTRING_CONST(CAPSTR_CATEGORY_ADD_SCENARIO, "ADD_SCENARIO");
CAPSTRING_CONST(CAPSTR_CATEGORY_DELETE_SCENARIO, "DELETE_SCENARIO");
CAPSTRING_CONST(CAPSTR_CATEGORY_RUN_SCENARIO, "RUN_SCENARIO");
CAPSTRING_CONST(CAPSTR_CATEGORY_STOP_SCENARIO, "STOP_SCENARIO");
CAPSTRING_CONST(CAPSTR_CATEGORY_VERIFY_SCENARIO, "VERIFY_SCENARIO");

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

static CALLBACK cap_result mqttMessageHandlingCallback(cap_string strTopic, cap_handle hTopicItemList,
        char *pszPayload, int nPayloadLen, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    cap_string strCategory = NULL;
    cap_string strClientId = NULL;

    SAppManager* pstAppManager = NULL;

    IFVARERRASSIGNGOTO(pUserData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstAppManager = (SAppManager *) pUserData;

    result = CAPLinkedList_Get(hTopicItemList, LINKED_LIST_OFFSET_FIRST, TOPIC_LEVEL_SECOND, (void**)&strCategory);
    ERRIFGOTO(result, _EXIT);
/*

    //assign ID for result publishing purpose
    result = getLastElementFromTopicList(hTopicItemList, &strClientId);
    ERRIFGOTO(result, _EXIT);

    if (CAPString_IsEqual(strCategory, CAPSTR_CATEGORY_ADD_SCENARIO) == TRUE) {
        result = addScenario(pstAppManager, strClientId, pszPayload, nPayloadLen);
        ERRIFGOTO(result, _EXIT);
    }
    else if (CAPString_IsEqual(strCategory, CAPSTR_CATEGORY_DELETE_SCENARIO) == TRUE) {
        int nScenarioId = atoi(pszPayload);

        result = deleteScenarioById(pstAppManager, strClientId, nScenarioId);
        ERRIFGOTO(result, _EXIT);
    }
    else if (CAPString_IsEqual(strCategory, CAPSTR_CATEGORY_RUN_SCENARIO) == TRUE) {
        result = runScenario(pstAppManager, strClientId, pszPayload, nPayloadLen);
        ERRIFGOTO(result, _EXIT);
    }
    else if (CAPString_IsEqual(strCategory, CAPSTR_CATEGORY_STOP_SCENARIO) == TRUE) {
        result = stopScenario(pstAppManager, strClientId, pszPayload, nPayloadLen);
        ERRIFGOTO(result, _EXIT);
    }
    else if (CAPString_IsEqual(strCategory, CAPSTR_CATEGORY_VERIFY_SCENARIO) == TRUE) {
        result = verifyScenario(pstAppManager, strClientId, pszPayload, nPayloadLen);
        ERRIFGOTO(result, _EXIT);
    }
    else {
        ERRASSIGNGOTO(result, ERR_CAP_NOT_SUPPORTED, _EXIT);
    }
*/
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
    SAppManagerCallbackData *pstCallback = NULL;

    IFVARERRASSIGNGOTO(phAppManager, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstAppManager = (SAppManager*)malloc(sizeof(SAppManager));
    ERRMEMGOTO(pstAppManager, result, _EXIT);

    pstAppManager->enID = HANDLEID_APP_MANAGER;
    pstAppManager->bCreated = FALSE;
    pstAppManager->hAppRunnerList = NULL;
    pstAppManager->strBrokerURI = NULL;
    pstAppManager->hMQTTHandler = NULL;
    pstAppManager->pstCallback = NULL;

    pstCallback = (SAppManagerCallbackData *) malloc(sizeof(SAppManagerCallbackData));
    ERRMEMGOTO(pstCallback, result, _EXIT);

    pstCallback->strPayload = NULL;
    pstCallback->strScenarioName = NULL;
    pstCallback->strScenarioText = NULL;
    pstCallback->strError = NULL;

    pstCallback->strPayload = CAPString_New();
    ERRMEMGOTO(pstCallback->strPayload, result, _EXIT);
    pstCallback->strScenarioName = CAPString_New();
    ERRMEMGOTO(pstCallback->strScenarioName, result, _EXIT);
    pstCallback->strScenarioText = CAPString_New();
    ERRMEMGOTO(pstCallback->strScenarioText, result, _EXIT);
    pstCallback->strError = CAPString_New();
    ERRMEMGOTO(pstCallback->strError, result, _EXIT);

    pstAppManager->pstCallback = pstCallback;

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
            if(pstCallback != NULL) {
                SAFE_CAPSTRING_DELETE(pstCallback->strError);
                SAFE_CAPSTRING_DELETE(pstCallback->strPayload);
                SAFE_CAPSTRING_DELETE(pstCallback->strScenarioName);
                SAFE_CAPSTRING_DELETE(pstCallback->strScenarioText);
                SAFEMEMFREE(pstCallback);
            }
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

    CAPLinkedList_Destroy(&(pstAppManager->hAppRunnerList));

    SAFE_CAPSTRING_DELETE(pstAppManager->strBrokerURI);

    if(pstAppManager->pstCallback != NULL)
    {
        SAFE_CAPSTRING_DELETE(pstAppManager->pstCallback->strError);
        SAFE_CAPSTRING_DELETE(pstAppManager->pstCallback->strPayload);
        SAFE_CAPSTRING_DELETE(pstAppManager->pstCallback->strScenarioName);
        SAFE_CAPSTRING_DELETE(pstAppManager->pstCallback->strScenarioText);
        SAFEMEMFREE(pstAppManager->pstCallback);
    }

    SAFEMEMFREE(pstAppManager);

    *phAppManager = NULL;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}
