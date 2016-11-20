
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>

#include <Json_common.h>
#include <json-c/json_object.h>
#include <json-c/json_tokener.h>

#include <CAPLinkedList.h>
#include <CAPThread.h>
#include <CAPStack.h>
#include <CAPThreadEvent.h>
#include <CAPQueue.h>
#include <CAPThreadLock.h>
#include <CAPLogger.h>

#include "AppValueCache.h"
#include "DBHandler.h"
#include "AppRunner.h"
#include "MQTTMessageHandler.h"

#define MQTT_CLIENT_ID_PREFIX "cap_iot_middleware_apprunner_"
#define TOPIC_SEPARATOR "/"
#define RESULT_FUNCTION_STR "TM/RESULT/FUNCTION/"
#define MT_STR "MT/"

#define JSON_KEY_ARG_VALUE "argValue"
#define JSON_KEY_SCENARIO_NAME "scenario"

#define SECOND 1000

#define JSON_KEY_ERROR "error"
#define JSON_KEY_SCENARIO "scenario"
#define JSON_KEY_VALUE "value"

CAPSTRING_CONST(CAPSTR_RESULT_FUNCTION, RESULT_FUNCTION_STR);
CAPSTRING_CONST(CAPSTR_TOPIC_SEPERATOR, TOPIC_SEPARATOR);
CAPSTRING_CONST(CAPSTR_TM, "TM/");
CAPSTRING_CONST(CAPSTR_MT, MT_STR);

CAPSTRING_CONST(CAPSTR_CATEGORY_RESULT, "RESULT");

#define TIME_MAX 10000
#define EPSILON 0.000001


typedef struct _SHashCallback {
    cap_string strRealThingId;
    SAppRunner *pstAppRunner;
    cap_string strTemp;
} SHashCallback;




typedef struct _SAppRunningThreadData {
    SAppRunner *pstAppRunner;
    cap_handle hCacheIndexHash;
} SAppRunningThreadData;

typedef struct _SAppActionResult {
    cap_bool bError;
} SAppActionResult;


typedef long cap_big_bool; //set bool as 8 bytes on 64-bit Linux machine to use stack


static CALLBACK cap_result destroyIndexHashData(IN cap_string strKey, IN void *pData, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;

    SAFEMEMFREE(pData);

    result = ERR_CAP_NOERROR;

    return result;
}


static cap_result updateValueToCache(cap_handle hAppValueCache, cap_handle hIndexHash, cap_string strKey, double dbValue)
{
    cap_result result = ERR_CAP_UNKNOWN;
    int *pnIndex = 0;

    result = CAPHash_GetDataByKey(hIndexHash, strKey, (void **)&pnIndex);
    ERRIFGOTO(result, _EXIT);

    result = AppValueCache_UpdateByIndex(hAppValueCache, *pnIndex, dbValue);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


static cap_result getValueFromCache(cap_handle hAppValueCache, cap_handle hIndexHash, cap_string strKey, EValueType *penValueType, double *pdbValue)
{
    cap_result result = ERR_CAP_UNKNOWN;
    int *pnIndex = 0;

    result = CAPHash_GetDataByKey(hIndexHash, strKey, (void **)&pnIndex);
    ERRIFGOTO(result, _EXIT);

    result = AppValueCache_GetByIndex(hAppValueCache, *pnIndex, penValueType, pdbValue);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}



static cap_result getValueFromExpression(cap_handle hAppModel, cap_handle hAppValueCache, cap_handle hIndexHash,
                                SExpression *pstExpression, double *pdbValue)
{
    cap_result result = ERR_CAP_UNKNOWN;
    EValueType enType;
    cap_string strKey = NULL;
    cap_string strRealThingId = NULL;

    switch(pstExpression->enType)
    {
    case EXP_TYPE_INTEGER:
        *pdbValue = (int) pstExpression->dbValue;
        break;
    case EXP_TYPE_DOUBLE:
        *pdbValue = pstExpression->dbValue;
        break;
    case EXP_TYPE_MEMBER_VARIABLE:
        strKey = CAPString_New();
        ERRMEMGOTO(strKey, result, _EXIT);

        strRealThingId = CAPString_New();
        ERRMEMGOTO(strRealThingId, result, _EXIT);

        result = AppScriptModeler_GetThingRealId(hAppModel, pstExpression->strPrimaryIdentifier, strRealThingId);
        ERRIFGOTO(result, _EXIT);

        result = CAPString_PrintFormat(strKey, "%s.%s",
                CAPString_LowPtr(strRealThingId, NULL),
                CAPString_LowPtr(pstExpression->strSubIdentifier, NULL));
        ERRIFGOTO(result, _EXIT);

        result = getValueFromCache(hAppValueCache, hIndexHash, strKey, &enType, pdbValue);
        ERRIFGOTO(result, _EXIT);
        if(enType == VALUE_TYPE_INT)
        {
            *pdbValue = (int) *pdbValue;
        }
        else
        {
            // do nothing use *pdbValue without modification
        }
        break;
    case EXP_TYPE_STRING:
        ERRASSIGNGOTO(result, ERR_CAP_NOT_SUPPORTED, _EXIT);
        break;
    case EXP_TYPE_VARIABLE:
    default:
        ERRASSIGNGOTO(result, ERR_CAP_NOT_SUPPORTED, _EXIT);
        break;
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFE_CAPSTRING_DELETE(strKey);
    SAFE_CAPSTRING_DELETE(strRealThingId);
    return result;
}


static cap_result checkSingleCondition(cap_handle hAppModel, cap_handle hAppValueCache, cap_handle hIndexHash, SCondition *pstCondition, cap_bool *pbConditionTrue)
{
   cap_result result = ERR_CAP_UNKNOWN;
    cap_bool bCheck = FALSE;
    double dbLeftValue = 0;
    double dbRightValue = 0;

    IFVARERRASSIGNGOTO(hAppValueCache, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(hIndexHash, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(pbConditionTrue, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    result = getValueFromExpression(hAppModel, hAppValueCache, hIndexHash, pstCondition->pstLeftOperand, &dbLeftValue);
    ERRIFGOTO(result, _EXIT);

    result = getValueFromExpression(hAppModel, hAppValueCache, hIndexHash, pstCondition->pstRightOperand, &dbRightValue);
    ERRIFGOTO(result, _EXIT);

    switch (pstCondition->enOperator) {
        case OPERATOR_NONE:
            ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
            break;
        case OPERATOR_EQUAL:
            bCheck = DOUBLE_IS_APPROX_EQUAL(dbLeftValue, dbRightValue);
            break;
        case OPERATOR_GREATER:
            bCheck = DOUBLE_IS_GREATER(dbLeftValue, dbRightValue);
            break;
        case OPERATOR_LESS:
            bCheck = DOUBLE_IS_LESS(dbLeftValue, dbRightValue);
            break;
        case OPERATOR_GREATER_EQUAL:
            bCheck = (DOUBLE_IS_APPROX_EQUAL(dbLeftValue, dbRightValue)) || (DOUBLE_IS_GREATER(dbLeftValue, dbRightValue));
            break;
        case OPERATOR_LESS_EQUAL:
            bCheck = (DOUBLE_IS_APPROX_EQUAL(dbLeftValue, dbRightValue)) || (DOUBLE_IS_LESS(dbLeftValue, dbRightValue));
            break;
        case OPERATOR_NOT_EQUAL:
            bCheck = !DOUBLE_IS_APPROX_EQUAL(dbLeftValue, dbRightValue);
            break;
        case OPERATOR_AND:
        case OPERATOR_OR:
        case OPERATOR_NOT:
            ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
            break;
        default:
            CAPLogger_Write(g_hLogger, MSG_ERROR, "App Runner : condition type error!");
            break;
    }

    *pbConditionTrue = bCheck;

    result = ERR_CAP_NOERROR;
_EXIT:
    if(result == ERR_CAP_NO_DATA)
    {
        *pbConditionTrue = FALSE;
        result = ERR_CAP_NOERROR;
    }
    return result;
}


static cap_result checkConditionListMatch(cap_handle hAppModel, cap_handle hAppValueCache, cap_handle hIndexHash, cap_handle hConditionList, OUT cap_bool *pbIsTrue)
{
    void* pData;
    int nConditionSize;
    cap_result result = ERR_CAP_UNKNOWN;
    cap_big_bool bFinalCheck = FALSE;   //set type as cap_big_bool to use stack
    cap_handle hStack = NULL;
    cap_big_bool bbStackVal = FALSE;
    cap_bool bConditionTrue = FALSE; //set type as cap_big_bool to use stack

    IFVARERRASSIGNGOTO(hConditionList, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    result = CAPStack_Create(&hStack);
    ERRIFGOTO(result, _EXIT);

    result = CAPLinkedList_GetLength(hConditionList, &nConditionSize);
    ERRIFGOTO(result, _EXIT_ERROR);

    int i = 0;
    for (i = 0; i < nConditionSize; i++) {
        SCondition* pstCondition = NULL;
        cap_bool bTempCheck = FALSE;
        cap_big_bool bOperand1 = FALSE, bOperand2 = FALSE;

        //get condition from the end of the linked list
        result = CAPLinkedList_Get(hConditionList, LINKED_LIST_OFFSET_FIRST, i, &pData);
        ERRIFGOTO(result, _EXIT_ERROR);

        pstCondition = (SCondition*)pData;

        switch(pstCondition->enOperator)
        {
        case OPERATOR_EQUAL:
        case OPERATOR_GREATER:
        case OPERATOR_LESS:
        case OPERATOR_GREATER_EQUAL:
        case OPERATOR_LESS_EQUAL:
        case OPERATOR_NOT_EQUAL:
            result = checkSingleCondition(hAppModel, hAppValueCache, hIndexHash, pstCondition, &bConditionTrue);
            ERRIFGOTO(result, _EXIT);

            bbStackVal = bConditionTrue;
            result = CAPStack_Push(hStack, (void *)bbStackVal);
            ERRIFGOTO(result, _EXIT);
            break;
        case OPERATOR_AND:
        case OPERATOR_OR:
            result = CAPStack_Pop(hStack, (void **)&bOperand2);
            ERRIFGOTO(result, _EXIT);
            result = CAPStack_Pop(hStack, (void **)&bOperand1);
            ERRIFGOTO(result, _EXIT);

            if(pstCondition->enOperator == OPERATOR_AND)
            {
                bTempCheck = (cap_bool)(bOperand1 && bOperand2);
                bbStackVal = bTempCheck;
                result = CAPStack_Push(hStack, (void *)bbStackVal);
                ERRIFGOTO(result, _EXIT);
            }
            else // pstCondition->enOperator == OPERATOR_OR
            {
                bTempCheck = (cap_bool)(bOperand1 || bOperand2);
                bbStackVal = bTempCheck;
                result = CAPStack_Push(hStack, (void *)bbStackVal);
                ERRIFGOTO(result, _EXIT);
            }
            break;
        case OPERATOR_NONE:
        case OPERATOR_NOT:
        case OPERATOR_LEFT_PARENTHESIS:
        case OPERATOR_RIGHT_PARENTHESIS:
        default:
            ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
            break;
        }
    }

    result = CAPStack_Pop(hStack, (void **)&bFinalCheck);
    ERRIFGOTO(result, _EXIT);

    *pbIsTrue = (cap_bool)bFinalCheck;

_EXIT_ERROR:
    CAPStack_Destroy(&hStack, NULL, NULL);
_EXIT:
    return result;
}


static cap_result acknowledgeFunctionResult(SAppRunner *pstAppRunner, cap_handle hTopicItemList, json_object* pJsonObject)
{
    cap_result result = ERR_CAP_UNKNOWN;
    json_object* pJsonError, *pJsonScenarioName;
    EMqttErrorCode enErrorCode = ERR_MQTT_NOERROR;
    cap_string strScenarioName = NULL;
    cap_string strFunctionName = NULL;
    cap_string strThingName = NULL;
    SAppActionResult *pstResultData = NULL;

    strScenarioName = CAPString_New();
    ERRMEMGOTO(strScenarioName, result, _EXIT);

    if (!json_object_object_get_ex(pJsonObject, JSON_KEY_ERROR, &pJsonError)) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }

    if (!json_object_object_get_ex(pJsonObject, JSON_KEY_SCENARIO, &pJsonScenarioName)) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }

    enErrorCode = (EMqttErrorCode) json_object_get_int(pJsonError);
    if(enErrorCode != ERR_MQTT_NOERROR)
    {
        result = CAPLinkedList_Get(hTopicItemList, LINKED_LIST_OFFSET_FIRST,
                TOPIC_LEVEL_FIFTH, (void**)&strThingName);
        ERRIFGOTO(result, _EXIT);

        result = CAPLinkedList_Get(hTopicItemList, LINKED_LIST_OFFSET_FIRST,
                TOPIC_LEVEL_FOURTH, (void**)&strFunctionName);
        ERRIFGOTO(result, _EXIT);

        CAPLogger_Write(g_hLogger, MSG_WARN, "%s.%s actuation is failed with error code %d.",
                CAPString_LowPtr(strThingName, NULL), CAPString_LowPtr(strFunctionName, NULL), enErrorCode);
    }

    result = CAPString_SetLow(strScenarioName, json_object_get_string(pJsonScenarioName), CAPSTRING_MAX);
    ERRIFGOTO(result, _EXIT);

    if(CAPString_IsEqual(strScenarioName, pstAppRunner->strScenarioName) == TRUE)
    {
        pstResultData = malloc(sizeof(SAppActionResult));
        ERRMEMGOTO(pstResultData, result, _EXIT);

        if(enErrorCode != ERR_MQTT_NOERROR)
        {
            pstResultData->bError = TRUE;
        }
        else
        {
            pstResultData->bError = FALSE;
        }

        result = CAPQueue_Put(pstAppRunner->hActionQueue, pstResultData);
        ERRIFGOTO(result, _EXIT);

        pstResultData = NULL;
    }
    else
    {
        // do nothing
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFE_CAPSTRING_DELETE(strScenarioName);
    SAFEMEMFREE(pstResultData);
    return result;
}


static CALLBACK cap_result conditionListDestroy(IN int nOffset, IN void *pData, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCondition *pstCondition = NULL;

    pstCondition = (SCondition *) pData;

    if(pstCondition->pstLeftOperand != NULL)
    {
        SAFE_CAPSTRING_DELETE(pstCondition->pstLeftOperand->strStringValue);
        SAFE_CAPSTRING_DELETE(pstCondition->pstLeftOperand->strSubIdentifier);
        SAFE_CAPSTRING_DELETE(pstCondition->pstLeftOperand->strPrimaryIdentifier);
    }

    if(pstCondition->pstRightOperand != NULL)
    {
        SAFE_CAPSTRING_DELETE(pstCondition->pstRightOperand->strStringValue);
        SAFE_CAPSTRING_DELETE(pstCondition->pstRightOperand->strSubIdentifier);
        SAFE_CAPSTRING_DELETE(pstCondition->pstRightOperand->strPrimaryIdentifier);
    }

    SAFEMEMFREE(pstCondition->pstRightOperand);
    SAFEMEMFREE(pstCondition->pstLeftOperand);

    SAFEMEMFREE(pstCondition);

    result = ERR_CAP_NOERROR;

    return result;
}


static cap_result updateCacheValue(cap_handle hAppValueCache, cap_handle hIndexHash, cap_handle hTopicItemList, json_object* pJsonObject)
{
    cap_result result = ERR_CAP_UNKNOWN;
    json_object* pJsonValue = NULL;
    double dbValue = 0;
    cap_string strKey = NULL;
    cap_string strThingId = NULL;
    cap_string strValueName = NULL;

    result = CAPLinkedList_Get(hTopicItemList, LINKED_LIST_OFFSET_FIRST, TOPIC_LEVEL_FIRST, (void**)&strThingId);
    ERRIFGOTO(result, _EXIT);

    result = CAPLinkedList_Get(hTopicItemList, LINKED_LIST_OFFSET_FIRST, TOPIC_LEVEL_SECOND, (void**)&strValueName);
    ERRIFGOTO(result, _EXIT);

    if (!json_object_object_get_ex(pJsonObject, JSON_KEY_VALUE, &pJsonValue)) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }

    dbValue = json_object_get_double(pJsonValue);

    strKey = CAPString_New();
    ERRMEMGOTO(strKey, result, _EXIT);

    result = CAPString_PrintFormat(strKey, "%s.%s",
            CAPString_LowPtr(strThingId, NULL),
            CAPString_LowPtr(strValueName, NULL));
    ERRIFGOTO(result, _EXIT);

    result = updateValueToCache(hAppValueCache, hIndexHash, strKey, dbValue);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFE_CAPSTRING_DELETE(strKey);
    return result;
}


static CALLBACK cap_result mqttMessageHandlingCallback(cap_string strTopic, cap_handle hTopicItemList, char *pszPayload,
        int nPayloadLen, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    cap_string strCategory = NULL;
    SAppRunner* pstAppRunner = NULL;
    cap_handle hConditionList = NULL;
    json_object* pJsonObject = NULL;
    cap_bool bConditionTrue = FALSE;
    SAppRunnerMQTTCallback *pstCallbackData = NULL;

    IFVARERRASSIGNGOTO(pUserData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstCallbackData = (SAppRunnerMQTTCallback *) pUserData;
    pstAppRunner = pstCallbackData->pstAppRunner;

    result = CAPLinkedList_Get(hTopicItemList, LINKED_LIST_OFFSET_FIRST, TOPIC_LEVEL_SECOND, (void**)&strCategory);
    ERRIFGOTO(result, _EXIT);

    if (CAPString_IsEqual(strCategory, CAPSTR_CATEGORY_RESULT) == TRUE)
    {
        CAPLogger_Write(g_hLogger, MSG_DETAIL, "App Runner : Actuator results has been arrived");

        //parse payload
        result = ParsingJson(&pJsonObject, pszPayload, nPayloadLen);
        ERRIFGOTO(result, _EXIT);

        result = acknowledgeFunctionResult(pstAppRunner, hTopicItemList, pJsonObject);
        ERRIFGOTO(result, _EXIT);
    }
    else
    {
        //parse payload
        result = ParsingJson(&pJsonObject, pszPayload, nPayloadLen);
        ERRIFGOTO(result, _EXIT);

        // Update App Value Cache
        result = updateCacheValue(pstAppRunner->hAppValueCache, pstCallbackData->hCacheIndexHash, hTopicItemList, pJsonObject);
        ERRIFGOTO(result, _EXIT);

        result = CAPQueue_Get(pstAppRunner->hConditionQueue, FALSE, &hConditionList);
        if(result == ERR_CAP_NOERROR)
        {
            pstCallbackData->hConditionList = hConditionList;
        }
        else if(result == ERR_CAP_NO_DATA)
        {
            result = ERR_CAP_NOERROR;
        }
        ERRIFGOTO(result, _EXIT);

        if(pstCallbackData->hConditionList != NULL)
        {
            result = checkConditionListMatch(pstAppRunner->hAppModel, pstAppRunner->hAppValueCache,
                    pstCallbackData->hCacheIndexHash, pstCallbackData->hConditionList, &bConditionTrue);
            ERRIFGOTO(result, _EXIT);

            if(bConditionTrue == TRUE)
            {
                result = CAPThreadEvent_SetEvent(pstAppRunner->hEvent);
                ERRIFGOTO(result, _EXIT);

                result = CAPLinkedList_Traverse(pstCallbackData->hConditionList, conditionListDestroy, NULL);
                ERRIFGOTO(result, _EXIT);
                // CAPLinkedList_Destroy will set pstCallbackData->hConditionList to NULL
                result = CAPLinkedList_Destroy(&(pstCallbackData->hConditionList));
                ERRIFGOTO(result, _EXIT);

                CAPLogger_Write(g_hLogger, MSG_DETAIL, "App Runner : wait until condition satisfied.");
            }
        }
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    if(result != ERR_CAP_NOERROR){
        if(result != ERR_CAP_SUSPEND)
        {
            CAPLogger_Write(g_hLogger, MSG_ERROR, "App Runner thread is exited with an error code: %d.", result);
        }
        else
        {
            CAPLogger_Write(g_hLogger, MSG_DEBUG, "App Runner thread is exited with an error code: %d.", result);
        }
    }
    // if the pstCallbackDAta->hConditionList is not freed, the structure will be freed in AppRunnerStop
    SAFEJSONFREE(pJsonObject);
    return result;
}

cap_result traverseValueNameHash(IN cap_string strKey, IN void *pData, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SHashCallback *pstCallback = NULL;
    cap_handle hAppValueCache = NULL;
    SValue *pstValue = NULL;
    double dbLatestVal = 0;

    IFVARERRASSIGNGOTO(pUserData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstCallback = (SHashCallback *) pUserData;
    hAppValueCache = pstCallback->pstAppRunner->hAppValueCache;

    result = CAPString_PrintFormat(pstCallback->strTemp, "%s.%s",
            CAPString_LowPtr(pstCallback->strRealThingId, NULL),
            CAPString_LowPtr(strKey, NULL));
    ERRIFGOTO(result, _EXIT);

    result = DBHandler_GetThingValue(pstCallback->strRealThingId, strKey, &pstValue, &dbLatestVal);
    if(result == ERR_CAP_NO_DATA)
    {
        result = AppValueCache_Add(hAppValueCache, pstCallback->strTemp, pstValue, NULL);
    }
    else if(result == ERR_CAP_NOERROR)
    {
        result = AppValueCache_Add(hAppValueCache, pstCallback->strTemp, pstValue, &dbLatestVal);
    }
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    if(pstValue != NULL)
    {
        SAFE_CAPSTRING_DELETE(pstValue->strValueName);
        SAFEMEMFREE(pstValue);
    }
    return result;
}


cap_result traverseValueNameToSubscribe(IN cap_string strKey, IN void *pData, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SHashCallback *pstCallback = NULL;
    cap_handle hMQTTHandler = NULL;

    IFVARERRASSIGNGOTO(pUserData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstCallback = (SHashCallback *) pUserData;
    hMQTTHandler = pstCallback->pstAppRunner->hMQTTHandler;

    result = CAPString_PrintFormat(pstCallback->strTemp, "%s%s%s", CAPString_LowPtr(pstCallback->strRealThingId, NULL),
            TOPIC_SEPARATOR, CAPString_LowPtr(strKey, NULL));
    ERRIFGOTO(result, _EXIT);

    result = MQTTMessageHandler_Subscribe(hMQTTHandler, pstCallback->strTemp);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


static cap_result traverseFunctionNameToSubscribe(IN cap_string strKey, IN void *pData, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    cap_handle hMQTTHandler = NULL;
    SHashCallback *pstCallback = NULL;

    IFVARERRASSIGNGOTO(pUserData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstCallback = (SHashCallback *) pUserData;

    hMQTTHandler = pstCallback->pstAppRunner->hMQTTHandler;

    result = CAPString_PrintFormat(pstCallback->strTemp, "%s%s%s%s", RESULT_FUNCTION_STR,
            CAPString_LowPtr(strKey, NULL), TOPIC_SEPARATOR,
            CAPString_LowPtr(pstCallback->strRealThingId, NULL));
    ERRIFGOTO(result, _EXIT);

    result = MQTTMessageHandler_Subscribe(hMQTTHandler, pstCallback->strTemp);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


static cap_result traverseThingNameHash(IN cap_string strKey, IN void *pData, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SThingKeywords *pstKeywords = NULL;
    SHashCallback stUserData;

    stUserData.strRealThingId = NULL;
    stUserData.pstAppRunner = NULL;
    stUserData.strTemp = NULL;

    IFVARERRASSIGNGOTO(pData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(pUserData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstKeywords = (SThingKeywords *) pData;

    stUserData.strRealThingId = pstKeywords->strThingRealId;
    stUserData.pstAppRunner = (cap_handle) pUserData;
    stUserData.strTemp = CAPString_New();
    ERRMEMGOTO(stUserData.strTemp, result, _EXIT);

    result = CAPHash_Traverse(pstKeywords->hValueHash, traverseValueNameHash, &stUserData);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFE_CAPSTRING_DELETE(stUserData.strTemp);
    return result;
}


static cap_result traverseThingNameToSubscribe(IN cap_string strKey, IN void *pData, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SThingKeywords *pstKeywords = NULL;
    SHashCallback stUserData;

    stUserData.strRealThingId = NULL;
    stUserData.pstAppRunner = NULL;
    stUserData.strTemp = NULL;

    IFVARERRASSIGNGOTO(pData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(pUserData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstKeywords = (SThingKeywords *) pData;

    stUserData.strRealThingId = pstKeywords->strThingRealId;
    stUserData.pstAppRunner = (cap_handle) pUserData;
    stUserData.strTemp = CAPString_New();
    ERRMEMGOTO(stUserData.strTemp, result, _EXIT);

    result = CAPHash_Traverse(pstKeywords->hValueHash, traverseValueNameToSubscribe, &stUserData);
    ERRIFGOTO(result, _EXIT);

    result = CAPHash_Traverse(pstKeywords->hFunctionHash, traverseFunctionNameToSubscribe, &stUserData);
    ERRIFGOTO(result, _EXIT);


    result = ERR_CAP_NOERROR;
_EXIT:
    SAFE_CAPSTRING_DELETE(stUserData.strTemp);
    return result;
}


static cap_result convertTimeToMilliseconds(double dbTimeValue, ETimeUnit enTimeUnit, long long *pllMilliseconds)
{
    long long llTimeValueMs = 0;
    cap_result result = ERR_CAP_UNKNOWN;

    if(dbTimeValue < 0)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }

    switch(enTimeUnit)
    {
    case TIME_UNIT_SEC:
        llTimeValueMs = (long long) (dbTimeValue * 1000);
        break;
    case TIME_UNIT_MINUTE:
        llTimeValueMs = (long long) (dbTimeValue * 1000 * 60);
        break;
    case TIME_UNIT_HOUR:
        llTimeValueMs = (long long) (dbTimeValue * 1000 * 60 * 60);
        break;
    case TIME_UNIT_DAY:
        llTimeValueMs = (long long) (dbTimeValue * 1000 * 60 * 60 * 24);
        break;
    default:
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
        break;
    }

    *pllMilliseconds = llTimeValueMs;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

static cap_result handleLoopStatement(SAppRunningThreadData *pstData, SConditionalStatement *pstCondStmt,
        cap_bool *pbNextDirection)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SAppRunner* pstAppRunner = NULL;
    long long  llWaitTimeMs = 0;
    cap_bool bConditionTrue = FALSE;

    pstAppRunner = pstData->pstAppRunner;

    result = convertTimeToMilliseconds(pstCondStmt->stPeriod.dbTimeVal,
            pstCondStmt->stPeriod.enTimeUnit, &llWaitTimeMs);
    ERRIFGOTO(result, _EXIT);

    if(llWaitTimeMs > 0)
    {
        result = CAPThreadEvent_WaitTimeEvent(pstAppRunner->hEvent, llWaitTimeMs);
        if(result == ERR_CAP_TIME_EXPIRED)
        {
            result = ERR_CAP_NOERROR;
        }
        else if(result == ERR_CAP_NOERROR)
        {
            // event is received when the AppRunner is going to be terminated.
            result = ERR_CAP_SUSPEND;
        }
        ERRIFGOTO(result, _EXIT);
    }

    if(pstCondStmt->hConditionList != NULL)
    {
        result = checkConditionListMatch(pstAppRunner->hAppModel, pstAppRunner->hAppValueCache, pstData->hCacheIndexHash,
                pstCondStmt->hConditionList, &bConditionTrue);
        ERRIFGOTO(result, _EXIT);
    }
    else
    {
        bConditionTrue = TRUE;
    }

    *pbNextDirection = bConditionTrue;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


static cap_result handleIfStatement(SAppRunningThreadData *pstData, SConditionalStatement *pstCondStmt,
        cap_bool *pbNextDirection)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SAppRunner* pstAppRunner = NULL;
    cap_bool bConditionTrue = FALSE;

    pstAppRunner = pstData->pstAppRunner;

    result = checkConditionListMatch(pstAppRunner->hAppModel, pstAppRunner->hAppValueCache, pstData->hCacheIndexHash,
                    pstCondStmt->hConditionList, &bConditionTrue);
    ERRIFGOTO(result, _EXIT);

    *pbNextDirection = bConditionTrue;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


static cap_result copyOperand(SExpression *pstDstOperand, SExpression *pstSrcOperand)
{
    cap_result result = ERR_CAP_UNKNOWN;

    pstDstOperand->dbValue = pstSrcOperand->dbValue;
    pstDstOperand->enType = pstSrcOperand->enType;
    pstDstOperand->strPrimaryIdentifier = NULL;
    pstDstOperand->strSubIdentifier = NULL;
    pstDstOperand->strStringValue = NULL;

    if(pstSrcOperand->strPrimaryIdentifier != NULL)
    {
        pstDstOperand->strPrimaryIdentifier = CAPString_New();
        ERRMEMGOTO(pstDstOperand->strPrimaryIdentifier, result, _EXIT);

        result = CAPString_Set(pstDstOperand->strPrimaryIdentifier, pstSrcOperand->strPrimaryIdentifier);
        ERRIFGOTO(result, _EXIT);
    }

    if(pstSrcOperand->strSubIdentifier != NULL)
    {
        pstDstOperand->strSubIdentifier = CAPString_New();
        ERRMEMGOTO(pstDstOperand->strSubIdentifier, result, _EXIT);

        result = CAPString_Set(pstDstOperand->strSubIdentifier, pstSrcOperand->strSubIdentifier);
        ERRIFGOTO(result, _EXIT);
    }

    if(pstSrcOperand->strStringValue != NULL)
    {
        pstDstOperand->strStringValue = CAPString_New();
        ERRMEMGOTO(pstDstOperand->strStringValue, result, _EXIT);

        result = CAPString_Set(pstDstOperand->strStringValue, pstSrcOperand->strStringValue);
        ERRIFGOTO(result, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    if(result != ERR_CAP_NOERROR && pstDstOperand != NULL)
    {
        SAFE_CAPSTRING_DELETE(pstDstOperand->strStringValue);
        SAFE_CAPSTRING_DELETE(pstDstOperand->strSubIdentifier);
        SAFE_CAPSTRING_DELETE(pstDstOperand->strPrimaryIdentifier);
    }
    return result;
}


static CALLBACK cap_result conditionListDuplicate(IN int nOffset, IN void *pDataSrc, IN void *pUserData, OUT void **ppDataDst)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCondition *pstSrcCondition = NULL;
    SCondition *pstDstCondition = NULL;

    pstSrcCondition = (SCondition *) pDataSrc;

    pstDstCondition = malloc(sizeof(SCondition));
    ERRMEMGOTO(pstDstCondition, result, _EXIT);

    pstDstCondition->bConditionTrue = pstSrcCondition->bConditionTrue;
    pstDstCondition->bInit = pstSrcCondition->bInit;
    pstDstCondition->enOperator = pstSrcCondition->enOperator;
    pstDstCondition->pstLeftOperand = NULL;
    pstDstCondition->pstRightOperand = NULL;

    if(pstSrcCondition->pstLeftOperand != NULL)
    {
        pstDstCondition->pstLeftOperand = malloc(sizeof(SExpression));
        ERRMEMGOTO(pstDstCondition->pstLeftOperand, result, _EXIT);

        result = copyOperand(pstDstCondition->pstLeftOperand, pstSrcCondition->pstLeftOperand);
        ERRIFGOTO(result, _EXIT);
    }

    if(pstSrcCondition->pstRightOperand != NULL)
    {
        pstDstCondition->pstRightOperand = malloc(sizeof(SExpression));
        ERRMEMGOTO(pstDstCondition->pstRightOperand, result, _EXIT);

        result = copyOperand(pstDstCondition->pstRightOperand, pstSrcCondition->pstRightOperand);
        ERRIFGOTO(result, _EXIT);
    }

    *ppDataDst = pstDstCondition;

    result = ERR_CAP_NOERROR;
_EXIT:
    if(result != ERR_CAP_NOERROR && pstDstCondition != NULL)
    {
        SAFEMEMFREE(pstDstCondition->pstRightOperand);
        SAFEMEMFREE(pstDstCondition->pstLeftOperand);
        SAFEMEMFREE(pstDstCondition);
    }
    return result;
}


static cap_result handleWaitUntilStatement(SAppRunningThreadData *pstData, SConditionalStatement *pstCondStmt,
        cap_bool *pbNextDirection)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SAppRunner* pstAppRunner = NULL;
    cap_bool bConditionTrue = FALSE;
    cap_handle hPassedConditionList = NULL;

    pstAppRunner = pstData->pstAppRunner;

    result = checkConditionListMatch(pstAppRunner->hAppModel, pstAppRunner->hAppValueCache, pstData->hCacheIndexHash,
                    pstCondStmt->hConditionList, &bConditionTrue);
    ERRIFGOTO(result, _EXIT);

    if(bConditionTrue == FALSE)
    {
        result = CAPLinkedList_Create(&hPassedConditionList);
        ERRIFGOTO(result, _EXIT);
        result = CAPLinkedList_Duplicate(hPassedConditionList, pstCondStmt->hConditionList,
                conditionListDuplicate, NULL);
        ERRIFGOTO(result, _EXIT);

        result = CAPQueue_Put(pstAppRunner->hConditionQueue, hPassedConditionList);
        ERRIFGOTO(result, _EXIT);

        // hPassedConditionList is passed to MQTT Message handling callback
        hPassedConditionList = NULL;

        result = CAPThreadEvent_WaitEvent(pstAppRunner->hEvent);
        ERRIFGOTO(result, _EXIT);
    }

    *pbNextDirection = TRUE;

    result = ERR_CAP_NOERROR;
_EXIT:
    if(hPassedConditionList != NULL)
    {
        CAPLinkedList_Traverse(hPassedConditionList, conditionListDestroy, NULL);
        CAPLinkedList_Destroy(&hPassedConditionList);
    }
    return result;
}

static cap_result handleActionStatement(SAppRunningThreadData *pstData, SExecutionStatement *pstExecStmt,
        cap_bool *pbNextDirection)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SAppRunner* pstAppRunner = NULL;
    cap_bool bConditionTrue = FALSE;
    cap_string strTopic = NULL;
    int nLen = 0;
    SExpression *pstArgument = NULL;
    double dbValue = 0;
    json_object* pJsonObject = NULL;
    cap_string strTemp = NULL;
    SAppActionResult *pstActionResult = NULL;

    pstAppRunner = pstData->pstAppRunner;

    strTopic = CAPString_New();
    ERRMEMGOTO(strTopic, result, _EXIT);

    strTemp = CAPString_New();
    ERRMEMGOTO(strTemp, result, _EXIT);

    if(pstExecStmt->strPrimaryIdentifier == NULL || pstExecStmt->strSubIdentifier == NULL)
    {
        ERRASSIGNGOTO(result, ERR_CAP_NOT_SUPPORTED, _EXIT);
    }

    pJsonObject = json_object_new_object();
    ERRMEMGOTO(pJsonObject, result, _EXIT);

    result = AppScriptModeler_GetThingRealId(pstAppRunner->hAppModel, pstExecStmt->strPrimaryIdentifier, strTemp);
    ERRIFGOTO(result, _EXIT);

    result = CAPString_PrintFormat(strTopic, "%s%s/%s", MT_STR, CAPString_LowPtr(pstExecStmt->strSubIdentifier, NULL),
            CAPString_LowPtr(strTemp, NULL));
    ERRIFGOTO(result, _EXIT);

    // Add argument information
    if(pstExecStmt->hInputList != NULL)
    {
        result = CAPLinkedList_GetLength(pstExecStmt->hInputList, &nLen);
        ERRIFGOTO(result, _EXIT);

        if(nLen != 1) // Currently, only one argument is allowed for function.
        {
            ERRASSIGNGOTO(result, ERR_CAP_NOT_SUPPORTED, _EXIT);
        }

        result = CAPLinkedList_Get(pstExecStmt->hInputList, LINKED_LIST_OFFSET_FIRST, 0, (void **) &pstArgument);
        ERRIFGOTO(result, _EXIT);

        if(pstArgument->enType == EXP_TYPE_STRING)
        {
            // just copy string
            json_object_object_add(pJsonObject, JSON_KEY_ARG_VALUE,
                    json_object_new_string(CAPString_LowPtr(pstArgument->strStringValue, NULL)));
        }
        else
        {
            result = getValueFromExpression(pstAppRunner->hAppModel, pstAppRunner->hAppValueCache, pstData->hCacheIndexHash,
                    pstArgument, &dbValue);
            ERRIFGOTO(result, _EXIT);

            if(pstArgument->enType == EXP_TYPE_INTEGER)
            {
                json_object_object_add(pJsonObject, JSON_KEY_ARG_VALUE, json_object_new_int((int)dbValue));
            }
            else
            {
                result = CAPString_PrintFormat(strTemp, "%lf", dbValue);
                ERRIFGOTO(result, _EXIT);
                json_object_object_add(pJsonObject, JSON_KEY_ARG_VALUE,
                        json_object_new_string(CAPString_LowPtr(strTemp, NULL)));
            }
        }
    }
    else // Function with no argument
    {
        json_object_object_add(pJsonObject, JSON_KEY_ARG_VALUE, json_object_new_string(" "));
    }

    // Add scenario name
    json_object_object_add(pJsonObject, JSON_KEY_SCENARIO_NAME,
            json_object_new_string(CAPString_LowPtr(pstAppRunner->strScenarioName, NULL)));

    // Make payload
    result = CAPString_SetLow(strTemp, json_object_to_json_string(pJsonObject), CAPSTRING_MAX);
    ERRIFGOTO(result, _EXIT);
    nLen = CAPString_Length(strTemp);

    result = MQTTMessageHandler_Publish(pstAppRunner->hMQTTHandler, strTopic,
            CAPString_LowPtr(strTemp, NULL), nLen);
    ERRIFGOTO(result, _EXIT);

    result = CAPQueue_Get(pstAppRunner->hActionQueue, TRUE, (void **) &pstActionResult);
    ERRIFGOTO(result, _EXIT);

    if(pstActionResult->bError == TRUE)
    {
        bConditionTrue = FALSE;
    }
    else
    {
        bConditionTrue = TRUE;
    }

    *pbNextDirection = bConditionTrue;

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEMEMFREE(pstActionResult);
    SAFE_CAPSTRING_DELETE(strTemp);
    SAFE_CAPSTRING_DELETE(strTopic);
    SAFEJSONFREE(pJsonObject);

    return result;
}


CAP_THREAD_HEAD AppRunningThread(IN void* pData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SAppRunningThreadData *pstData = NULL;
    SAppRunner* pstAppRunner = NULL;
    SExecutionNode *pstNode = NULL;
    SConditionalStatement *pstCondStmt = NULL;
    SExecutionStatement *pstExecStmt = NULL;
    cap_bool bDirection = TRUE;

    IFVARERRASSIGNGOTO(pData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstData = (SAppRunningThreadData*)pData;
    pstAppRunner = pstData->pstAppRunner;

    result = AppScriptModeler_ClearExecution(pstAppRunner->hAppModel);
    ERRIFGOTO(result, _EXIT);

    result = AppScriptModeler_GetCurrent(pstAppRunner->hAppModel, &pstNode);
    ERRIFGOTO(result, _EXIT);

    pstAppRunner->enState = RUNNER_STATE_RUNNING;

    while(pstNode->enType != FINISH_STATEMENT && pstAppRunner->bExit == FALSE)
    {
        switch(pstNode->enType)
        {
        case IF_STATEMENT:
            pstCondStmt = (SConditionalStatement *) pstNode->pstStatementData;
            result = handleIfStatement(pstData, pstCondStmt, &bDirection);
            ERRIFGOTO(result, _EXIT);
            break;
        case LOOP_STATEMENT:
            pstCondStmt = (SConditionalStatement *) pstNode->pstStatementData;
            result = handleLoopStatement(pstData, pstCondStmt, &bDirection);
            ERRIFGOTO(result, _EXIT);
            break;
        case ACTION_STATEMENT:
            pstExecStmt = (SExecutionStatement *) pstNode->pstStatementData;
            result = handleActionStatement(pstData, pstExecStmt, &bDirection);
            ERRIFGOTO(result, _EXIT);
            break;
        case WAIT_UNTIL_STATEMENT:
            pstCondStmt = (SConditionalStatement *) pstNode->pstStatementData;
            result = handleWaitUntilStatement(pstData, pstCondStmt, &bDirection);
            ERRIFGOTO(result, _EXIT);
            break;
        case SEND_STATEMENT:
        case RECEIVE_STATEMENT:
            ERRASSIGNGOTO(result, ERR_CAP_NOT_SUPPORTED, _EXIT);
            break;
        case FINISH_STATEMENT:
        default:
            ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
            // Error
            break;
        }

        printf("statement proceed: %d\n", pstNode->enType);

        result = AppScriptModeler_MoveToNext(pstAppRunner->hAppModel, bDirection, &pstNode);
        ERRIFGOTO(result, _EXIT);
    }

    pstAppRunner->enState = RUNNER_STATE_COMPLETED;

    result = ERR_CAP_NOERROR;
_EXIT:
    if(pstAppRunner->bExit == TRUE || (result != ERR_CAP_NOERROR && pstAppRunner != NULL))
    {
        pstAppRunner->enState = RUNNER_STATE_STOP;
    }
    if(pstData != NULL)
    {
        CAPHash_Destroy(&(pstData->hCacheIndexHash), destroyIndexHashData, NULL);
        SAFEMEMFREE(pstData);
    }

    CAP_THREAD_END;
}


static cap_result executeAppRunnerThread(SAppRunner *pstAppRunner)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SAppRunningThreadData *pstThreadData = NULL;
    int nBucketNum = 0;

    pstThreadData = malloc(sizeof(SAppRunningThreadData));
    ERRMEMGOTO(pstThreadData, result, _EXIT);

    pstThreadData->hCacheIndexHash = NULL;
    pstThreadData->pstAppRunner = pstAppRunner;

    result = AppValueCache_GetBucketNumber(pstAppRunner->hAppValueCache, &nBucketNum);
    ERRIFGOTO(result, _EXIT);

    if(nBucketNum > 0)
    {
        result = CAPHash_Create(nBucketNum, &(pstThreadData->hCacheIndexHash));
        ERRIFGOTO(result, _EXIT);

        result = AppValueCache_DuplicateIndexHash(pstAppRunner->hAppValueCache, pstThreadData->hCacheIndexHash);
        ERRIFGOTO(result, _EXIT);
    }

    // pstThreadData is deleted inside at AppRunningThread
    result = CAPThread_Create(AppRunningThread, pstThreadData, &(pstAppRunner->hAppRunnerThread));
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    if(result != ERR_CAP_NOERROR && pstThreadData != NULL)
    {
        CAPHash_Destroy(&(pstThreadData->hCacheIndexHash), destroyIndexHashData, NULL);
        SAFEMEMFREE(pstThreadData);
    }
    return result;
}


static cap_result initAndExecuteAppRunner(SAppRunner * pstAppRunner)
{
    cap_result result = ERR_CAP_UNKNOWN;
    cap_handle hThingHash = NULL;
    SAppRunnerMQTTCallback *pstMQTTCallback = NULL;
    int nBucketNum = 0;

    IFVARERRASSIGNGOTO(pstAppRunner, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    result = MQTTMessageHandler_Connect(pstAppRunner->hMQTTHandler);
    ERRIFGOTO(result, _EXIT);

    result = AppScriptModeler_GetThingKeywordHash(pstAppRunner->hAppModel, &hThingHash);
    ERRIFGOTO(result, _EXIT);

    result = CAPHash_Traverse(hThingHash, traverseThingNameHash, pstAppRunner);
    ERRIFGOTO(result, _EXIT);

    result = AppValueCache_GetBucketNumber(pstAppRunner->hAppValueCache, &nBucketNum);
    ERRIFGOTO(result, _EXIT);

    pstMQTTCallback = malloc(sizeof(SAppRunnerMQTTCallback));
    ERRMEMGOTO(pstMQTTCallback, result, _EXIT);

    pstMQTTCallback->hCacheIndexHash = NULL;
    pstMQTTCallback->hConditionList = NULL;
    pstMQTTCallback->pstAppRunner = pstAppRunner;

    if(nBucketNum > 0)
    {
        result = CAPHash_Create(nBucketNum, &(pstMQTTCallback->hCacheIndexHash));
        ERRIFGOTO(result, _EXIT);

        result = AppValueCache_DuplicateIndexHash(pstAppRunner->hAppValueCache, pstMQTTCallback->hCacheIndexHash);
        ERRIFGOTO(result, _EXIT);
    }

    // Store a callback user data structure to remove structure during scenario destruction
    pstAppRunner->pstCallback = pstMQTTCallback;

    result = MQTTMessageHandler_SetReceiveCallback(pstAppRunner->hMQTTHandler,
            mqttMessageHandlingCallback, pstMQTTCallback);
    ERRIFGOTO(result, _EXIT);

    result = CAPHash_Traverse(hThingHash, traverseThingNameToSubscribe, pstAppRunner);
    ERRIFGOTO(result, _EXIT);


    result = executeAppRunnerThread(pstAppRunner);
    ERRIFGOTO(result, _EXIT);

_EXIT:
    if(result != ERR_CAP_NOERROR && pstMQTTCallback != NULL)
    {
        CAPHash_Destroy(&(pstMQTTCallback->hCacheIndexHash), destroyIndexHashData, NULL);
        SAFEMEMFREE(pstMQTTCallback);
    }
    return result;
}


cap_result traverseAndCountVariable(IN cap_string strKey, IN void *pData, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SThingKeywords *pstKeywords = NULL;
    int nNum = 0;
    int *pnTotalNum = 0;

    IFVARERRASSIGNGOTO(pData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(pUserData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstKeywords = (SThingKeywords *) pData;
    pnTotalNum = (int *) pUserData;

    result = CAPHash_GetNumberOfItems(pstKeywords->hValueHash, &nNum);
    ERRIFGOTO(result, _EXIT);

    *pnTotalNum += nNum;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result AppRunner_Create(IN cap_handle hAppModel, IN cap_string strBrokerURI, OUT cap_handle* phAppRunner)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SAppRunner* pstAppRunner = NULL;
    cap_string strClientId = NULL;
    cap_handle hThingHash = NULL;
    int nTotalValueNum = 0;

    // TODO
    // This will be changed to IS_VALID_HANDLE
    IFVARERRASSIGNGOTO(hAppModel, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    IFVARERRASSIGNGOTO(phAppRunner, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(strBrokerURI, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstAppRunner = (SAppRunner*)malloc(sizeof(SAppRunner));
    ERRMEMGOTO(pstAppRunner, result, _EXIT);

    pstAppRunner->enID = HANDLEID_APP_RUNNER;

    pstAppRunner->hAppRunnerThread = NULL;
    pstAppRunner->hScenario = NULL;
    pstAppRunner->bCreated = FALSE;
    pstAppRunner->hEvent = NULL;
    pstAppRunner->bExit = FALSE;
    pstAppRunner->hMQTTHandler = NULL;
    pstAppRunner->enState = RUNNER_STATE_INIT;
    pstAppRunner->hConditionQueue = NULL;
    pstAppRunner->hActionQueue = NULL;
    pstAppRunner->hAppValueCache = NULL;
    pstAppRunner->strScenarioName = NULL;
    pstAppRunner->pstCallback = NULL;
    pstAppRunner->hAppModel = NULL;

    strClientId = CAPString_New();
    ERRMEMGOTO(strClientId, result, _EXIT);

    pstAppRunner->strScenarioName = CAPString_New();
    ERRMEMGOTO(pstAppRunner->strScenarioName, result, _EXIT);

    result = CAPQueue_Create(&(pstAppRunner->hConditionQueue));
    ERRIFGOTO(result, _EXIT);

    result = CAPQueue_Create(&(pstAppRunner->hActionQueue));
        ERRIFGOTO(result, _EXIT);

    //create lock for ping
    result = CAPThreadLock_Create(&(pstAppRunner->hLock));
    ERRIFGOTO(result, _EXIT);

    //create event for timedwait
    result = CAPThreadEvent_Create(&(pstAppRunner->hEvent));
    ERRIFGOTO(result, _EXIT);

    result = AppScriptModeler_GetScenarioName(hAppModel, pstAppRunner->strScenarioName);
    ERRIFGOTO(result, _EXIT);

    //create client id for each app runner not to be repeated when connecting to broker
    result = CAPString_PrintFormat(strClientId, "%s%s", MQTT_CLIENT_ID_PREFIX,
            CAPString_LowPtr(pstAppRunner->strScenarioName, NULL));
    ERRIFGOTO(result, _EXIT);

    result = AppScriptModeler_GetThingKeywordHash(hAppModel, &hThingHash);
    ERRIFGOTO(result, _EXIT);

    result = CAPHash_Traverse(hThingHash, traverseAndCountVariable, &nTotalValueNum);
    ERRIFGOTO(result, _EXIT);

    result = AppValueCache_Create(nTotalValueNum, &(pstAppRunner->hAppValueCache));
    ERRIFGOTO(result, _EXIT);

    pstAppRunner->hAppModel = hAppModel;

    result = MQTTMessageHandler_Create(strBrokerURI, strClientId, 10, &(pstAppRunner->hMQTTHandler));
    ERRIFGOTO(result, _EXIT);

    *phAppRunner = pstAppRunner;

    result = ERR_CAP_NOERROR;
_EXIT:
    if (result != ERR_CAP_NOERROR) {
        if(pstAppRunner != NULL){
            AppValueCache_Destroy(&(pstAppRunner->hAppValueCache));
            CAPThreadEvent_Destroy(&(pstAppRunner->hEvent));
            CAPThreadLock_Destroy(&(pstAppRunner->hLock));
            CAPQueue_Destroy(&(pstAppRunner->hActionQueue), NULL, NULL);
            CAPQueue_Destroy(&(pstAppRunner->hConditionQueue), NULL, NULL);
            SAFE_CAPSTRING_DELETE(pstAppRunner->strScenarioName);
        }
        SAFEMEMFREE(pstAppRunner);
    }
    SAFE_CAPSTRING_DELETE(strClientId);
    return result;
}


cap_result AppRunner_Run(cap_handle hAppRunner)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SAppRunner* pstAppRunner = NULL;

    if (IS_VALID_HANDLE(hAppRunner, HANDLEID_APP_RUNNER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstAppRunner = (SAppRunner*)hAppRunner;

    pstAppRunner->bExit = FALSE;

    switch(pstAppRunner->enState)
    {
    case RUNNER_STATE_INIT:
        result = initAndExecuteAppRunner(pstAppRunner);
        ERRIFGOTO(result, _EXIT);
        break;
    case RUNNER_STATE_RUNNING:
        // do nothing
        CAPLogger_Write(g_hLogger, MSG_INFO, "App Runner : runner is still running now");
        break;
    case RUNNER_STATE_STOP:
        result = executeAppRunnerThread(pstAppRunner);
        ERRIFGOTO(result, _EXIT);
        break;
    case RUNNER_STATE_COMPLETED:
        result = AppRunner_Stop(hAppRunner);
        ERRIFGOTO(result, _EXIT);
        result = executeAppRunnerThread(pstAppRunner);
        ERRIFGOTO(result, _EXIT);
        break;
    case RUNNER_STATE_PAUSE:
    default:
        ERRASSIGNGOTO(result, ERR_CAP_NOT_SUPPORTED, _EXIT);
        break;
    }

    pstAppRunner->bCreated = TRUE;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


static cap_result queueActionDestroy(IN void *pData, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;

    SAFEMEMFREE(pData);

    result = ERR_CAP_NOERROR;

    return result;
}


static cap_result queueConditionDestroy(IN void *pData, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    cap_handle hConditionList = NULL;

    hConditionList = pData;

    result = CAPLinkedList_Traverse(hConditionList, conditionListDestroy, NULL);
    ERRIFGOTO(result, _EXIT);
    // CAPLinkedList_Destroy will set pstCallbackData->hConditionList to NULL
    result = CAPLinkedList_Destroy(&(hConditionList));
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}




cap_result AppRunner_Stop(cap_handle hAppRunner)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SAppRunner* pstAppRunner = NULL;
    SAppRunnerMQTTCallback *pstCallbackData = NULL;

    if (IS_VALID_HANDLE(hAppRunner, HANDLEID_APP_RUNNER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstAppRunner = (SAppRunner*)hAppRunner;
    pstCallbackData = pstAppRunner->pstCallback;

    pstAppRunner->bExit = TRUE;

    if(pstAppRunner->enState == RUNNER_STATE_RUNNING)
    {
        result = CAPThreadEvent_SetEvent(pstAppRunner->hEvent);
        ERRIFGOTO(result, _EXIT);

        result = CAPQueue_SetExit(pstAppRunner->hConditionQueue);
        ERRIFGOTO(result, _EXIT);

        result = CAPQueue_SetExit(pstAppRunner->hActionQueue);
        ERRIFGOTO(result, _EXIT);

        result = CAPThread_Destroy(&(pstAppRunner->hAppRunnerThread));
        ERRIFGOTO(result, _EXIT);

        result = CAPQueue_RemoveAll(pstAppRunner->hConditionQueue, queueConditionDestroy, NULL);
        ERRIFGOTO(result, _EXIT);

        result = CAPQueue_RemoveAll(pstAppRunner->hActionQueue, queueActionDestroy, NULL);
        ERRIFGOTO(result, _EXIT);
    }
    else if(pstAppRunner->enState == RUNNER_STATE_COMPLETED ||
            pstAppRunner->enState == RUNNER_STATE_STOP)
    {
        if(pstAppRunner->hAppRunnerThread != NULL)
        {
            result = CAPThread_Destroy(&(pstAppRunner->hAppRunnerThread));
            ERRIFGOTO(result, _EXIT);
        }

        result = CAPQueue_RemoveAll(pstAppRunner->hConditionQueue, queueConditionDestroy, NULL);
        ERRIFGOTO(result, _EXIT);
        result = CAPQueue_RemoveAll(pstAppRunner->hActionQueue, queueActionDestroy, NULL);
        ERRIFGOTO(result, _EXIT);

        CAPASSIGNGOTO(result, ERR_CAP_ALREADY_DONE, _EXIT);
    }
    else
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_REQUEST, _EXIT);
    }

    if(pstCallbackData != NULL)
    {
        CAPLinkedList_Traverse(pstCallbackData->hConditionList, conditionListDestroy, NULL);
        CAPLinkedList_Destroy(&(pstCallbackData->hConditionList));
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result AppRunner_GetState(cap_handle hAppRunner, OUT ERunnerState *penState)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SAppRunner* pstAppRunner = NULL;

    if (IS_VALID_HANDLE(hAppRunner, HANDLEID_APP_RUNNER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(penState, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstAppRunner = (SAppRunner*)hAppRunner;

    *penState = pstAppRunner->enState;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result AppRunner_Join(cap_handle hAppRunner)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SAppRunner* pstAppRunner = NULL;

    if (IS_VALID_HANDLE(hAppRunner, HANDLEID_APP_RUNNER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstAppRunner = (SAppRunner*)hAppRunner;

    result = MQTTMessageHandler_Disconnect(pstAppRunner->hMQTTHandler);
    ERRIFGOTO(result, _EXIT);
    
    result = AppRunner_Stop(hAppRunner);
    ERRIFGOTO(result, _EXIT);

	CAPLogger_Write(g_hLogger, MSG_INFO,
	        "App Runner : thread has been destroyed whose scenario name is : %s",
	        CAPString_LowPtr(pstAppRunner->strScenarioName, NULL));

    pstAppRunner->bCreated = FALSE;
    
    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result AppRunner_Destroy(OUT cap_handle* phAppRunner)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SAppRunner* pstAppRunner = NULL;

    IFVARERRASSIGNGOTO(phAppRunner, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if (IS_VALID_HANDLE(*phAppRunner, HANDLEID_APP_RUNNER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstAppRunner = (SAppRunner*)*phAppRunner;

    CAPQueue_Destroy(&(pstAppRunner->hConditionQueue), queueConditionDestroy, NULL);
    CAPQueue_Destroy(&(pstAppRunner->hActionQueue), queueActionDestroy, NULL);

    MQTTMessageHandler_Destroy(&(pstAppRunner->hMQTTHandler));

    AppValueCache_Destroy(&(pstAppRunner->hAppValueCache));

    CAPThreadLock_Destroy(&(pstAppRunner->hLock));

    CAPThreadEvent_Destroy(&(pstAppRunner->hEvent));

    AppScriptModeler_Destroy(&(pstAppRunner->hAppModel));

    SAFE_CAPSTRING_DELETE(pstAppRunner->strScenarioName);
    // pstAppRunner->hActionQueue;
    if(pstAppRunner->pstCallback != NULL)
    {
        CAPHash_Destroy(&(pstAppRunner->pstCallback->hCacheIndexHash), destroyIndexHashData, NULL);
    }
    SAFEMEMFREE(pstAppRunner->pstCallback);
    SAFEMEMFREE(pstAppRunner);

    *phAppRunner = NULL;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


