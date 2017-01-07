
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
#define TOPIC_SEPERATOR "/"

static char* paszAppManagerSubcriptionList[] = {
    "TM/SEND_VARIABLE/#",
    "TM/FUNCTION_RESULT/#",
};


CAPSTRING_CONST(CAPSTR_MQTT_CLIENT_ID, MQTT_CLIENT_ID);

CAPSTRING_CONST(CAPSTR_CATEGORY_FUNCTION_RESULT, "FUNCTION_RESULT");
CAPSTRING_CONST(CAPSTR_CATEGORY_SEND_VARIABLE, "SEND_VARIABLE");

CAPSTRING_CONST(CAPSTR_REQUEST_FUNCTION, "MT/REQUEST_FUNCTION/");
CAPSTRING_CONST(CAPSTR_TOPIC_SEPERATOR, TOPIC_SEPERATOR);

CAPSTRING_CONST(CAPSTR_MT, "MT/");

static CALLBACK cap_result destroyRelatedCondtion(int nOffset, void* pData, void* pUsrData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SConditionContext* pstConditionContext = NULL;

    IFVARERRASSIGNGOTO(pData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstConditionContext = (SConditionContext*)pData;

	SAFE_CAPSTRING_DELETE(pstConditionContext->strExpression);
    SAFEMEMFREE(pstConditionContext);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}
static CALLBACK cap_result destroySatisfiedEca(int nOffset, void* pData, void* pUsrData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SConditionContext* pstConditionContext = NULL;

    IFVARERRASSIGNGOTO(pData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    SAFEMEMFREE(pData);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

static CALLBACK cap_result destroyAction(int nOffset, void* pData, void* pUsrData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SActionContext* pstActionContext = NULL;

    IFVARERRASSIGNGOTO(pData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstActionContext = (SActionContext*)pData;

	SAFE_CAPSTRING_DELETE(pstActionContext->strFunctionName);
	SAFE_CAPSTRING_DELETE(pstActionContext->strArgumentPayload);
    SAFEMEMFREE(pstActionContext);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

static EOperator convertStringOperatorToEnum(char *pszOperator, int nOperatorIndex){
    if(strncmp(pszOperator, "<=", 2) == 0){
        if(nOperatorIndex == 1){
            return OPERATOR_GREATER_EQUAL;
        }
        else {
            return OPERATOR_LESS_EQUAL;
        }
    }
    else if(strncmp(pszOperator, "<", 1) == 0){
        if(nOperatorIndex == 1){
            return OPERATOR_GREATER;
        }
        else {
            return OPERATOR_LESS;
        }
    }
    else if(strncmp(pszOperator, ">=", 2) == 0){
        if(nOperatorIndex == 1){
            return OPERATOR_LESS_EQUAL;
        }
        else {
            //Not supported
            return OPERATOR_NONE;
        }
    }
    else if(strncmp(pszOperator, ">", 1) == 0){
        if(nOperatorIndex == 1){
            return OPERATOR_LESS;
        }
        else {
            //Not supported
            return OPERATOR_NONE;
        }
    }
    else if(strncmp(pszOperator, "isEqual", 7) == 0){
        return OPERATOR_STRING_IS_EQUAL;
    }
    else if(strncmp(pszOperator, "contains", 8) == 0){
        return OPERATOR_STRING_CONTAINS;
    }
    else {
        return OPERATOR_NONE;
    }
}

static cap_result checkDoubleCondition(double dbVariable, EOperator enOperator, double dbOperand, cap_bool *pbIsSatisfied) {
    cap_result result = ERR_CAP_UNKNOWN;
    cap_bool bIsSatisfied = FALSE;

    switch (enOperator) {
        case OPERATOR_GREATER:
            bIsSatisfied = DOUBLE_IS_GREATER(dbVariable, dbOperand);
            break;
        case OPERATOR_LESS:
            bIsSatisfied = DOUBLE_IS_LESS(dbVariable, dbOperand);
            break;
        case OPERATOR_GREATER_EQUAL:
            bIsSatisfied = (DOUBLE_IS_APPROX_EQUAL(dbVariable, dbOperand)) || (DOUBLE_IS_GREATER(dbVariable, dbOperand));
            break;
        case OPERATOR_LESS_EQUAL:
            bIsSatisfied = (DOUBLE_IS_APPROX_EQUAL(dbVariable, dbOperand)) || (DOUBLE_IS_LESS(dbVariable, dbOperand));
            break;
        default:
            dlp("not supported operator for double condition!\n");
            bIsSatisfied = FALSE;
    } 

    *pbIsSatisfied = bIsSatisfied;
    
    result = ERR_CAP_NOERROR;
    return result;
}

static cap_result computeSingleCondition(SConditionContext* pstConditionContext, char *pszVariable) {
    cap_result result = ERR_CAP_UNKNOWN;
    cap_bool bIsSatisfied = FALSE;
    char *pszExpression = NULL;
    char *pszToken, *pszPtr = NULL;
    int nTokenCount = 0;

    IFVARERRASSIGNGOTO(pstConditionContext, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    
    pszExpression = CAPString_LowPtr(pstConditionContext->strExpression, NULL); 

    if(pstConditionContext->enType == TYPE_INTEGER || pstConditionContext->enType == TYPE_DOUBLE) {
        //case 1 : [operand1][space][operator1][space]"value"
        //case 2 : [operand1][space][operator1][space]"value"[space][operator2][space][operand2]
        double dbOperand1, dbOperand2;
        EOperator enOperator1, enOperator2;

        double dbVariable = atof(pszVariable);

        //First Token -> operand1
        if( (pszToken = strtok_r(pszExpression, " ", &pszPtr)) ) {
            dbOperand1 = atof(pszToken);
            nTokenCount++;
        }

        //Second Token -> operator1
        if( (pszToken = strtok_r(NULL, " ", &pszPtr)) ) {
            enOperator1 = convertStringOperatorToEnum(pszToken, 1);
            nTokenCount++;
        }

        //Third Token -> dummy
        if( (pszToken = strtok_r(NULL, " ", &pszPtr)) ) {
            nTokenCount++;
        }

        //Fourth Token -> operator2
        if( (pszToken = strtok_r(NULL, " ", &pszPtr)) ) {
            enOperator2 = convertStringOperatorToEnum(pszToken, 2);
            nTokenCount++;
        }

        //Fifth Token -> operand2
        if( (pszToken = strtok_r(NULL, " ", &pszPtr)) ) {
            dbOperand2 = atof(pszToken);
            nTokenCount++;
        }

        //case 1
        if(nTokenCount == 3) {
            result = checkDoubleCondition(dbVariable, enOperator1, dbOperand1, &bIsSatisfied);
            ERRIFGOTO(result, _EXIT);
        }
        //case 2
        else if(nTokenCount == 5){
            cap_bool bIsSatisfied1 = FALSE;
            cap_bool bIsSatisfied2 = FALSE;
            
            result = checkDoubleCondition(dbVariable, enOperator1, dbOperand1, &bIsSatisfied1);
            ERRIFGOTO(result, _EXIT);
            
            result = checkDoubleCondition(dbVariable, enOperator2, dbOperand2, &bIsSatisfied2);
            ERRIFGOTO(result, _EXIT);

            bIsSatisfied = (bIsSatisfied1 && bIsSatisfied2);
        } 
        else {
            dlp("nTokenCount error!\n");
        }
    }
    else if(pstConditionContext->enType == TYPE_STRING || pstConditionContext->enType == TYPE_SELECT) {
        //case 1 : "value"[space][operator][space][operand]
        char *pszOperand = NULL;
        EOperator enOperator;

        //First Token -> dummy 
        if( (pszToken = strtok_r(pszExpression, " ", &pszPtr)) ) {
            nTokenCount++;
        }

        //Second Token -> operator
        if( (pszToken = strtok_r(NULL, " ", &pszPtr)) ) {
            enOperator = convertStringOperatorToEnum(pszToken, 1);
            nTokenCount++;
        }

        //Third Token -> operand 
        if( (pszToken = strtok_r(NULL, " ", &pszPtr)) ) {
            pszOperand = strdup(pszToken);
            nTokenCount++;
        }

        if(enOperator == OPERATOR_STRING_IS_EQUAL){
            //strcmp returns 0 when two strings are equal
            if(strcmp(pszOperand, pszVariable) == 0) {
                bIsSatisfied = TRUE;
            }
            else {
                bIsSatisfied = FALSE;
            }

        }
        else if(enOperator == OPERATOR_STRING_CONTAINS) {
            char *pszResult = strstr(pszVariable, pszOperand);
            if(pszResult != NULL) {
                bIsSatisfied = TRUE;
            }
            else {
                bIsSatisfied = FALSE;
            }
        }
        SAFEMEMFREE(pszOperand);
    }
    else {
        //binary type is not available for condition
        dlp("not supported type for condition!\n");
    }

    dlp("is satisfied : %d\n", bIsSatisfied);
    pstConditionContext->bIsSatisfied = bIsSatisfied;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

static cap_result requestAction(int nEcaId, IN cap_string strDeviceId, cap_handle hAppManager)
{
    cap_result result = ERR_CAP_UNKNOWN;
    json_object* pJsonObject = NULL, *pJsonArgumentArray = NULL;
    cap_string strTopic = NULL;
    SAppManager *pstAppManager = NULL;
    const char* pszConstEcaId = "eca_id", *pszConstArguments = "arguments";
    //const char *pszConstName = "name", *pszConstValue = "value";
    char *pszPayload = NULL;
    int nPayloadLen;
    int nLength = 0, nLoop = 0;
    cap_handle hActionList = NULL; 
    SActionContext *pstActionContext;

    IFVARERRASSIGNGOTO(hAppManager, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstAppManager = (SAppManager *)hAppManager;
    
    result = CAPLinkedList_Create(&hActionList);
    ERRIFGOTO(result, _EXIT);

    result = DBHandler_RetrieveActionList(pstAppManager->pDBconn, nEcaId, hActionList);
    ERRIFGOTO(result, _EXIT);
    
    //set topic
    //Format : MT/REQUEST_FUNCTION/[Thing ID]/[Function name]
    strTopic = CAPString_New();
    ERRMEMGOTO(strTopic, result, _EXIT);

    result = CAPLinkedList_GetLength(hActionList, &nLength);
    ERRIFGOTO(result, _EXIT);

    for(nLoop = 0; nLoop < nLength; nLoop++){
        result = CAPLinkedList_Get(hActionList, LINKED_LIST_OFFSET_FIRST, nLoop, (void**)&pstActionContext);
        ERRIFGOTO(result, _EXIT);

        result = CAPString_Set(strTopic, CAPSTR_REQUEST_FUNCTION);
        ERRIFGOTO(result, _EXIT);

        result = CAPString_AppendString(strTopic, strDeviceId);
        ERRIFGOTO(result, _EXIT);

        result = CAPString_AppendString(strTopic, CAPSTR_TOPIC_SEPERATOR);
        ERRIFGOTO(result, _EXIT);
    
        result = CAPString_AppendString(strTopic, pstActionContext->strFunctionName);
        ERRIFGOTO(result, _EXIT);

        pJsonObject = json_object_new_object();
        ERRMEMGOTO(pJsonObject, result, _EXIT);

        //add eca id
        json_object_object_add(pJsonObject, pszConstEcaId, json_object_new_int(nEcaId));

        //parse argument payload string to json object to add it to json
        result = ParsingJson(&pJsonArgumentArray, CAPString_LowPtr(pstActionContext->strArgumentPayload, NULL),\
                CAPString_Length(pstActionContext->strArgumentPayload));
        ERRIFGOTO(result, _EXIT);

        json_object_object_add(pJsonObject, pszConstArguments, pJsonArgumentArray); 

        pszPayload = strdup(json_object_to_json_string(pJsonObject));
        nPayloadLen = strlen(pszPayload);

        result = MQTTMessageHandler_Publish(pstAppManager->hMQTTHandler, strTopic, pszPayload, nPayloadLen);
        ERRIFGOTO(result, _EXIT);

        SAFEMEMFREE(pszPayload);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEJSONFREE(pJsonObject);
    SAFEMEMFREE(pszPayload);
    SAFE_CAPSTRING_DELETE(strTopic);
    CAPLinkedList_Traverse(hActionList, destroyAction, NULL);
    CAPLinkedList_Destroy(&hActionList);
    return result;
}

static cap_result computeConditionsThenActuateIfPossible(cap_handle hRelatedConditionList, char *pszVariable, IN cap_string strDeviceId, cap_handle hAppManager)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SConditionContext* pstConditionContext = NULL;
    SAppManager *pstAppManager = NULL;
    int nLength = 0;
    int nLoop = 0;

    IFVARERRASSIGNGOTO(hRelatedConditionList, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(hAppManager, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstAppManager = (SAppManager *)hAppManager;

    result = CAPLinkedList_GetLength(hRelatedConditionList, &nLength);
    ERRIFGOTO(result, _EXIT);

    for(nLoop = 0; nLoop < nLength; nLoop++){
        result = CAPLinkedList_Get(hRelatedConditionList, LINKED_LIST_OFFSET_FIRST, nLoop, (void**)&pstConditionContext);
        ERRIFGOTO(result, _EXIT);

        result = computeSingleCondition(pstConditionContext, pszVariable);
        ERRIFGOTO(result, _EXIT);
        
        //if condition is satisfied and there is only one condition or operator 'any' condition -> publish action right away
        if(pstConditionContext->bIsSatisfied) {
            if(pstConditionContext->bIsSingleCondition || pstConditionContext->enEcaOp == OPERATOR_OR) {
                result = requestAction(pstConditionContext->nEcaId, strDeviceId, hAppManager);
                ERRIFGOTO(result, _EXIT);
            }
        }
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;

}

static cap_result actuateSatisfiedEcaList(cap_handle hSatisfiedEcaList, IN cap_string strDeviceId, cap_handle hAppManager) 
{
    cap_result result = ERR_CAP_UNKNOWN;
    SAppManager *pstAppManager = NULL;
    int nLength = 0, nLoop = 0;
    int *pnEcaId = NULL;

    IFVARERRASSIGNGOTO(hSatisfiedEcaList, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(hAppManager, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstAppManager = (SAppManager *)hAppManager;

    result = CAPLinkedList_GetLength(hSatisfiedEcaList, &nLength);
    ERRIFGOTO(result, _EXIT);

    for(nLoop = 0; nLoop < nLength; nLoop++){
        int nEcaId = 0;

        result = CAPLinkedList_Get(hSatisfiedEcaList, LINKED_LIST_OFFSET_FIRST, nLoop, (void**)&pnEcaId);
        ERRIFGOTO(result, _EXIT);

        nEcaId = *pnEcaId;

        result = requestAction(nEcaId, strDeviceId, hAppManager);
        ERRIFGOTO(result, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

static cap_result AppManager_PublishErrorCode(IN int errorCode, cap_handle hAppManager, cap_string strDeviceId,
        cap_string strTopicCategory, cap_string strErrorString)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SAppManager* pstAppManager = NULL;
    cap_string strTopic = NULL;
    char* pszPayload = NULL;
    int nPayloadLen = 0;
    EMqttErrorCode enError;
    char *pszApiKey = NULL;
    json_object* pJsonObject;

    IFVARERRASSIGNGOTO(hAppManager, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(strDeviceId, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(strErrorString, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstAppManager = (SAppManager*)hAppManager;

    //set topic
    strTopic = CAPString_New();
    ERRMEMGOTO(strTopic, result, _EXIT);

    result = CAPString_Set(strTopic, strTopicCategory);
    ERRIFGOTO(result, _EXIT);

    result = CAPString_AppendString(strTopic, strDeviceId);
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
    
    result = DBHandler_RetrieveApiKey(pstAppManager->pDBconn, strDeviceId, &pszApiKey);
    ERRIFGOTO(result, _EXIT);

    if(pszApiKey == NULL) {
        //do nothing
    } 
    else {
        json_object_object_add(pJsonObject, "apikey", json_object_new_string(pszApiKey));
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

static cap_result handleUserApplication(IN SAppManager *pstAppManager, IN cap_string strDeviceId, IN cap_string strVariableName, IN char *pszVariable) {
    cap_result result = ERR_CAP_UNKNOWN;
    cap_handle hRelatedConditionList = NULL; 
    cap_handle hSatisfiedEcaList = NULL; 

    result = CAPLinkedList_Create(&hRelatedConditionList);
    ERRIFGOTO(result, _EXIT);
    
    result = CAPLinkedList_Create(&hSatisfiedEcaList);
    ERRIFGOTO(result, _EXIT);

    //Get Condition List
    result = DBHandler_MakeConditionList(pstAppManager->pDBconn, strDeviceId, strVariableName, hRelatedConditionList);
    ERRIFGOTO(result, _EXIT);

    //Compute Each condition -> if condition is satisfied and there is only one condition or operator 'any' condition -> publish action right away
    result = computeConditionsThenActuateIfPossible(hRelatedConditionList, pszVariable, strDeviceId, (cap_handle)pstAppManager );
    ERRIFGOTO(result, _EXIT);

    //push is_satisfied of each condition into db(if there is only one condition or operator 'any' condition, this step will be ignored)
    result = DBHandler_InsertSatisfiedCondition(pstAppManager->pDBconn, hRelatedConditionList);
    ERRIFGOTO(result, _EXIT);

    // Compute each eca if condition is met(only if there is more than one condition)
    result = DBHandler_RetrieveSatisfiedEcaList(pstAppManager->pDBconn, hSatisfiedEcaList);
    ERRIFGOTO(result, _EXIT);

    // Actuate function where eca condition is met
    result = actuateSatisfiedEcaList(hSatisfiedEcaList, strDeviceId, (cap_handle)pstAppManager);
    ERRIFGOTO(result, _EXIT);

_EXIT:
    if(result != ERR_CAP_NOERROR){
        //nothing 
    }

    CAPLinkedList_Traverse(hRelatedConditionList, destroyRelatedCondtion, NULL);
    CAPLinkedList_Traverse(hSatisfiedEcaList, destroySatisfiedEca, NULL);
    CAPLinkedList_Destroy(&hRelatedConditionList);
    CAPLinkedList_Destroy(&hSatisfiedEcaList);
    return result;

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
    result_save = DBHandler_VerifyApiKey(pstAppManager->pDBconn, strDeviceId, (char *)json_object_get_string(pJsonApiKey));

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

        result = handleUserApplication(pstAppManager, strDeviceId, strVariableName, (char *)json_object_get_string(pJsonVariable));
        ERRIFGOTO(result, _EXIT);

    }
    else if (CAPString_IsEqual(strCategory, CAPSTR_CATEGORY_FUNCTION_RESULT) == TRUE) {
        json_object* pJsonTemp;
        cap_string strFunctionName = NULL;
        const char* pszConstEcaId = "eca_id", *pszConstError = "error";
        int nErrorCode = 0, nEcaId = 0;

        /*
        //If api key error has occured, goto exit 
        if(result_save != ERR_CAP_NOERROR){
        goto _EXIT;
        }
        */

        if (!json_object_object_get_ex(pJsonObject, pszConstEcaId, &pJsonTemp)) {
            ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
        }

        nEcaId = json_object_get_int(pJsonTemp);

        if (!json_object_object_get_ex(pJsonObject, pszConstError, &pJsonTemp)) {
            ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
        }

        nErrorCode = json_object_get_int(pJsonTemp);

        //Get variable name 
        result = CAPLinkedList_Get(hTopicItemList, LINKED_LIST_OFFSET_FIRST, TOPIC_LEVEL_FOURTH, (void**)&strFunctionName);
        ERRIFGOTO(result, _EXIT);

        result = DBHandler_InsertApplicationHistory(pstAppManager->pDBconn,strDeviceId, strFunctionName, nEcaId, nErrorCode);

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

cap_result AppManager_Create(OUT cap_handle* phAppManager, cap_string strBrokerURI, IN SDBInfo *pstDBInfo)
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

    result = DBHandler_OpenDB(pstDBInfo, &pstAppManager->pDBconn);
    ERRIFGOTO(result, _EXIT);

    result = CAPString_Set(pstAppManager->strBrokerURI, strBrokerURI);
    ERRIFGOTO(result, _EXIT);

    result = MQTTMessageHandler_Create(pstAppManager->strBrokerURI, CAPSTR_MQTT_CLIENT_ID, 0,
            &(pstAppManager->hMQTTHandler));
    ERRIFGOTO(result, _EXIT);

    *phAppManager = pstAppManager;

    result = ERR_CAP_NOERROR;
_EXIT:
    if (result != ERR_CAP_NOERROR) {
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

        result = DBHandler_CloseDB(pstAppManager->pDBconn);
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
