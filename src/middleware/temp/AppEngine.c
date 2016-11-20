/*
 * AppEngine.c
 *
 *  Created on: 2016. 8. 3.
 *      Author: chjej202
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <capiot_common.h>
#include <CAPString.h>
#include <CAPHash.h>
#include <CAPThreadLock.h>
#include <CAPLinkedList.h>

#include "AppScriptModeler.h"
#include "AppValueCache.h"
#include "AppRunner.h"
#include "AppEngine.h"
#include "DBHandler.h"

typedef struct _SAppEngine {
    EIoTHandleId enID;
    cap_handle hAppRunnerHash;
    cap_string strBrokerURI;
    cap_handle hLock;
} SAppEngine;

#define APP_RUNNER_HASH_BUCKET_NUM (17)


typedef struct _SAppActionUserData {
    SFunction *pstFunction;
    cap_string strErrorString;
} SAppActionUserData;

#define ERROR_SEPARATOR "\n"


static CALLBACK cap_result destroyAppRunner(IN cap_string strKey, IN void *pData, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    cap_handle hAppRunner = pData;

    AppRunner_Join(hAppRunner);
    AppRunner_Destroy(&hAppRunner);

    result = ERR_CAP_NOERROR;

    return result;
}


cap_result AppEngine_Create(cap_string strBrokerURI, OUT cap_handle *phEngine)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SAppEngine *pstAppEngine = NULL;

    IFVARERRASSIGNGOTO(phEngine, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstAppEngine = malloc(sizeof(SAppEngine));
    ERRMEMGOTO(pstAppEngine, result, _EXIT);

    pstAppEngine->enID = HANDLEID_APP_ENGINE;
    pstAppEngine->hAppRunnerHash = NULL;
    pstAppEngine->strBrokerURI = NULL;
    pstAppEngine->hLock = NULL;

    pstAppEngine->strBrokerURI = CAPString_New();
    ERRMEMGOTO(pstAppEngine->strBrokerURI, result, _EXIT);

    result = CAPString_Set(pstAppEngine->strBrokerURI, strBrokerURI);
    ERRIFGOTO(result, _EXIT);

    result = CAPThreadLock_Create(&(pstAppEngine->hLock));
    ERRIFGOTO(result, _EXIT);

    result = CAPHash_Create(APP_RUNNER_HASH_BUCKET_NUM, &(pstAppEngine->hAppRunnerHash));
    ERRIFGOTO(result, _EXIT);

    *phEngine = pstAppEngine;

    result = ERR_CAP_NOERROR;
_EXIT:
    if(result != ERR_CAP_NOERROR && pstAppEngine != NULL)
    {
        CAPHash_Destroy(&(pstAppEngine->hAppRunnerHash), NULL, NULL);
        CAPThreadLock_Destroy(&(pstAppEngine->hLock));
        SAFE_CAPSTRING_DELETE(pstAppEngine->strBrokerURI);
        SAFEMEMFREE(pstAppEngine);
    }
    return result;
}


static CALLBACK cap_result traverseAndFillRealThingName(IN cap_string strKey, IN void *pData, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SThingKeywords *pstKeywords = NULL;
    cap_handle hList = NULL;
    cap_string strRealThingId = NULL;

    IFVARERRASSIGNGOTO(pData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstKeywords = (SThingKeywords *) pData;
    hList = pUserData;

    result = DBHandler_GetRealThingName(strKey, pstKeywords->strThingRealId);
    ERRIFGOTO(result, _EXIT);

    if(hList != NULL)
    {
        strRealThingId = CAPString_New();
        ERRMEMGOTO(strRealThingId, result, _EXIT);

        result = CAPString_Set(strRealThingId, pstKeywords->strThingRealId);
        ERRIFGOTO(result, _EXIT);

        result = CAPLinkedList_Add(hList, LINKED_LIST_OFFSET_LAST, 0, strRealThingId);
        ERRIFGOTO(result, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    if(result != ERR_CAP_NOERROR)
    {
        SAFE_CAPSTRING_DELETE(strRealThingId);
    }
    return result;
}

static cap_result getExpressionType(SExpression *pstExpType, IN OUT cap_string strErrorString, OUT EValueType *penType)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SValue *pstValue = NULL;
    cap_string strRealThingId = NULL;

    switch(pstExpType->enType)
    {
    case EXP_TYPE_MEMBER_VARIABLE:
        strRealThingId = CAPString_New();
        ERRMEMGOTO(strRealThingId, result, _EXIT);
        result = DBHandler_GetRealThingName(pstExpType->strPrimaryIdentifier, strRealThingId);
        if(result == ERR_CAP_NOT_FOUND)
        {
            CAPString_AppendFormat(strErrorString, "Thing %s cannot be found.%s",
                    CAPString_LowPtr(pstExpType->strPrimaryIdentifier, NULL), ERROR_SEPARATOR);
        }
        ERRIFGOTO(result, _EXIT);
        result = DBHandler_GetThingValue(strRealThingId,
                pstExpType->strSubIdentifier, &pstValue, NULL);
        if(result == ERR_CAP_NOT_FOUND)
        {
            CAPString_AppendFormat(strErrorString, "Unknown thing.value %s.%s.%s",
                    CAPString_LowPtr(pstExpType->strPrimaryIdentifier, NULL),
                    CAPString_LowPtr(pstExpType->strSubIdentifier, NULL), ERROR_SEPARATOR);
        }
        ERRIFGOTO(result, _EXIT);
        if(pstValue->enType == VALUE_TYPE_BOOL || pstValue->enType == VALUE_TYPE_INT ||
           pstValue->enType == VALUE_TYPE_DOUBLE)
        {
            *penType = pstValue->enType;
        }
        else
        {
            ERRASSIGNGOTO(result, ERR_CAP_NOT_SUPPORTED, _EXIT);
        }
        break;
    case EXP_TYPE_INTEGER:
        *penType = VALUE_TYPE_INT;
        break;
    case EXP_TYPE_DOUBLE:
        *penType = VALUE_TYPE_DOUBLE;
        break;
    case EXP_TYPE_STRING:
    case EXP_TYPE_VARIABLE:
        ERRASSIGNGOTO(result, ERR_CAP_NOT_SUPPORTED, _EXIT);
        break;
    default:
        // error
        break;
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFE_CAPSTRING_DELETE(strRealThingId);
    if(pstValue != NULL)
    {
        SAFE_CAPSTRING_DELETE(pstValue->strValueName);
        SAFEMEMFREE(pstValue);
    }
    return result;
}

cap_result traverseConditionList(IN int nOffset, IN void *pData, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCondition *pstCondition = NULL;
    EValueType enLeftType, enRightType;
    cap_string strErrorString = NULL;

    pstCondition = (SCondition *) pData;
    strErrorString = (cap_string) pUserData;

    switch(pstCondition->enOperator)
    {
    case OPERATOR_EQUAL:
    case OPERATOR_GREATER:
    case OPERATOR_LESS:
    case OPERATOR_GREATER_EQUAL:
    case OPERATOR_LESS_EQUAL:
    case OPERATOR_NOT_EQUAL:
        result = getExpressionType(pstCondition->pstLeftOperand, strErrorString, &enLeftType);
        if(result == ERR_CAP_NOT_SUPPORTED)
        {
            CAPString_AppendFormat(strErrorString, "Unsupported type is used on the left operand.%s", ERROR_SEPARATOR);
        }
        ERRIFGOTO(result, _EXIT);
        result = getExpressionType(pstCondition->pstRightOperand, strErrorString, &enRightType);
        if(result == ERR_CAP_NOT_SUPPORTED)
        {
            CAPString_AppendFormat(strErrorString, "Unsupported type is used on the right operand.%s", ERROR_SEPARATOR);
        }
        ERRIFGOTO(result, _EXIT);

        if((enLeftType == VALUE_TYPE_BOOL && enRightType == VALUE_TYPE_DOUBLE) ||
           (enLeftType == VALUE_TYPE_DOUBLE && enRightType == VALUE_TYPE_BOOL))
        {
            CAPString_AppendFormat(strErrorString, "Boolean type and double type cannot be compared. %s", ERROR_SEPARATOR);
        }
        ERRIFGOTO(result, _EXIT);

        break;
    case OPERATOR_AND:
    case OPERATOR_OR:
    case OPERATOR_LEFT_PARENTHESIS:
    case OPERATOR_RIGHT_PARENTHESIS:
        // do nothing
        break;
    case OPERATOR_NOT:
    case OPERATOR_NONE:
    default:
        CAPString_AppendFormat(strErrorString, "Irregular input is inserted.%s", ERROR_SEPARATOR);
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
        // error
        break;
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


static cap_result verifyConditionalStatement(EStatementType enType, SConditionalStatement *pstCondStmt,
        cap_string strErrorString)
{
    cap_result result = ERR_CAP_UNKNOWN;

    if(enType == LOOP_STATEMENT && pstCondStmt->hConditionList == NULL)
    {
        // skip if the loop statement does not have condition
    }
    else
    {
        result = CAPLinkedList_Traverse(pstCondStmt->hConditionList, traverseConditionList, strErrorString);
        ERRIFGOTO(result, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


#define THINGARG_STRING_TYPE "string"
#define THINGARG_INT_TYPE "int"
#define THINGARG_BOOL_TYPE "bool"
#define THINGARG_DOUBLE_TYPE "double"
#define THINGARG_BINARY_TYPE "binary"

static const char * valueTypeToString(EValueType enType)
{
    const char *pszValueString = NULL;
    switch(enType)
    {
    case VALUE_TYPE_INT:
        pszValueString = THINGARG_INT_TYPE;
        break;
    case VALUE_TYPE_DOUBLE:
        pszValueString = THINGARG_DOUBLE_TYPE;
        break;
    case VALUE_TYPE_BOOL:
        pszValueString = THINGARG_BOOL_TYPE;
        break;
    case VALUE_TYPE_STRING:
        pszValueString = THINGARG_STRING_TYPE;
        break;
    case VALUE_TYPE_BINARY:
        pszValueString = THINGARG_BINARY_TYPE;
        break;
    default:
        // no other default type
        break;
    }

    return pszValueString;
}


static void printArgTypeMismatchErrorString(cap_string strFunctionName, cap_string strError,
        int nOffset, EValueType enExpectedType, EValueType enType)
{
    CAPString_AppendFormat(strError,
            "Function %s's Argument %d type mismatch: expected '%s' but argument is of type '%s'.%s",
            CAPString_LowPtr(strFunctionName, NULL), nOffset+1, valueTypeToString(enExpectedType),
            valueTypeToString(enType), ERROR_SEPARATOR);
}


static EExpressionType convertValueTypeToExpType(EValueType enValueType)
{
    EExpressionType enType = EXP_TYPE_MEMBER_VARIABLE;
    switch(enValueType)
    {
    case VALUE_TYPE_STRING:
        enType = EXP_TYPE_STRING;
        break;
    case VALUE_TYPE_INT:
        enType = EXP_TYPE_INTEGER;
        break;
    case VALUE_TYPE_BOOL:
        enType = EXP_TYPE_INTEGER;
        break;
    case VALUE_TYPE_DOUBLE:
        enType = EXP_TYPE_DOUBLE;
        break;
    case VALUE_TYPE_BINARY:
        // This will cause an error
        enType = EXP_TYPE_VARIABLE;
        break;
    }

    return enType;
}

static cap_result traverseInputList(IN int nOffset, IN void *pData, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SExpression *pstExp = NULL;
    SFunction *pstFunction = NULL;
    SAppActionUserData *pstActionData = NULL;
    SValue *pstValue = NULL;
    EExpressionType enExpType;

    pstActionData = (SAppActionUserData *) pUserData;

    pstExp = (SExpression *) pData;
    pstFunction = (SFunction *) pstActionData->pstFunction;

    if(pstExp->enType == EXP_TYPE_MEMBER_VARIABLE)
    {
        result = DBHandler_GetThingValue(pstExp->strPrimaryIdentifier, pstExp->strSubIdentifier, &pstValue, NULL);
        ERRIFGOTO(result, _EXIT);
        enExpType = convertValueTypeToExpType(pstValue->enType);
    }
    else if(pstExp->enType == EXP_TYPE_VARIABLE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_NOT_SUPPORTED, _EXIT);
    }
    else
    {
        enExpType = pstExp->enType;
    }

    switch(enExpType)
    {
    case EXP_TYPE_STRING:
        if(pstFunction->enArgumentType != VALUE_TYPE_STRING)
        {
            printArgTypeMismatchErrorString(pstFunction->strFunctionName, pstActionData->strErrorString, nOffset,
                    VALUE_TYPE_STRING, pstFunction->enArgumentType);
            ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
        }
        break;
    case EXP_TYPE_INTEGER:
        if(pstFunction->enArgumentType != VALUE_TYPE_DOUBLE && pstFunction->enArgumentType != VALUE_TYPE_INT &&
           pstFunction->enArgumentType == VALUE_TYPE_BOOL && TRUE != pstExp->dbValue && FALSE != pstExp->dbValue)
        {
            printArgTypeMismatchErrorString(pstFunction->strFunctionName, pstActionData->strErrorString, nOffset,
                    VALUE_TYPE_INT, pstFunction->enArgumentType);
            ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
        }
        break;
    case EXP_TYPE_DOUBLE:
        if(pstFunction->enArgumentType != VALUE_TYPE_DOUBLE)
        {
            printArgTypeMismatchErrorString(pstFunction->strFunctionName, pstActionData->strErrorString, nOffset,
                    VALUE_TYPE_DOUBLE, pstFunction->enArgumentType);
            ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
        }
        break;
    case EXP_TYPE_MEMBER_VARIABLE:
    case EXP_TYPE_VARIABLE:
    default:
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
        break;
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


static cap_result verifyActionStatement(SExecutionStatement *pstExecStmt, cap_string strErrorString)
{
    cap_result result = ERR_CAP_UNKNOWN;
    int nLen = 0;
    cap_string strRealThingId = NULL;
    SFunction *pstFunction = NULL;
    SAppActionUserData stActionUserData;

    nLen = CAPString_Length(pstExecStmt->strSubIdentifier);
    if(nLen <= 0)
    {
        CAPString_AppendFormat(strErrorString, "Function name is required for execution.%s", ERROR_SEPARATOR);
        ERRASSIGNGOTO(result, ERR_CAP_NOT_SUPPORTED, _EXIT);
    }

    strRealThingId = CAPString_New();
    ERRMEMGOTO(strRealThingId, result, _EXIT);

    result = DBHandler_GetRealThingName(pstExecStmt->strPrimaryIdentifier, strRealThingId);
    if(result == ERR_CAP_NOT_FOUND)
    {
        CAPString_AppendFormat(strErrorString, "Thing %s cannot be found.%s",
                CAPString_LowPtr(pstExecStmt->strPrimaryIdentifier, NULL), ERROR_SEPARATOR);
    }
    ERRIFGOTO(result, _EXIT);
    result = DBHandler_GetThingFunction(strRealThingId, pstExecStmt->strSubIdentifier, &pstFunction);
    if(result == ERR_CAP_NOT_FOUND)
    {
        CAPString_AppendFormat(strErrorString, "Unknown thing.function %s.%s.%s",
                CAPString_LowPtr(pstExecStmt->strPrimaryIdentifier, NULL),
                CAPString_LowPtr(pstExecStmt->strSubIdentifier, NULL), ERROR_SEPARATOR);
    }
    ERRIFGOTO(result, _EXIT);

    nLen = 0;
    result = CAPLinkedList_GetLength(pstExecStmt->hInputList, &nLen);
    if(pstFunction->bUseArgument == TRUE && nLen != 1)
    {
        CAPString_AppendFormat(strErrorString, "%s.%s does not support multiple arguments.%s",
                CAPString_LowPtr(pstExecStmt->strPrimaryIdentifier, NULL),
                CAPString_LowPtr(pstExecStmt->strSubIdentifier, NULL), ERROR_SEPARATOR);
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }
    else if(pstFunction->bUseArgument == FALSE && nLen != 0)
    {
        CAPString_AppendFormat(strErrorString, "%s.%s does not support any argument.%s",
                CAPString_LowPtr(pstExecStmt->strPrimaryIdentifier, NULL),
                CAPString_LowPtr(pstExecStmt->strSubIdentifier, NULL), ERROR_SEPARATOR);
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }
    // ignore other cases because pstExecStmt->hInputList is NULL when there is no argument.

    if(nLen > 0)
    {
        stActionUserData.pstFunction = pstFunction;
        stActionUserData.strErrorString = strErrorString;
        result = CAPLinkedList_Traverse(pstExecStmt->hInputList, traverseInputList, &stActionUserData);
        ERRIFGOTO(result, _EXIT);
    }

    nLen = 0;
    result = CAPLinkedList_GetLength(pstExecStmt->hOutputList, &nLen);
    // ignore error because pstExecStmt->hOutputList is NULL when there is no output.
    if(nLen > 0)
    {
        CAPString_AppendFormat(strErrorString, "%s.%s Output retrieval is not supported yet.%s",
                CAPString_LowPtr(pstExecStmt->strPrimaryIdentifier, NULL),
                CAPString_LowPtr(pstExecStmt->strSubIdentifier, NULL), ERROR_SEPARATOR);
        ERRASSIGNGOTO(result, ERR_CAP_NOT_SUPPORTED, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    if(pstFunction != NULL)
    {
        SAFE_CAPSTRING_DELETE(pstFunction->strFunctionName);
        SAFEMEMFREE(pstFunction);
    }
    SAFE_CAPSTRING_DELETE(strRealThingId);
    return result;
}


static CALLBACK cap_result traverseExecutionNodeHash(int nIndex, SExecutionNode *pstNode, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SConditionalStatement *pstCondStmt = NULL;
    SExecutionStatement *pstExecStmt = NULL;
    cap_string strErrorString = NULL;

    IFVARERRASSIGNGOTO(pstNode, NULL, result, ERR_CAP_INVALID_DATA, _EXIT);

    strErrorString = (cap_string) pUserData;

    switch(pstNode->enType)
    {
    case IF_STATEMENT:
    case LOOP_STATEMENT:
    case WAIT_UNTIL_STATEMENT:
        pstCondStmt = (SConditionalStatement *) pstNode->pstStatementData;
        result = verifyConditionalStatement(pstNode->enType, pstCondStmt, strErrorString);
        ERRIFGOTO(result, _EXIT);
        break;
    case ACTION_STATEMENT:
        pstExecStmt = (SExecutionStatement *) pstNode->pstStatementData;
        result = verifyActionStatement(pstExecStmt, strErrorString);
        ERRIFGOTO(result, _EXIT);
        break;
    case FINISH_STATEMENT:
    case SEND_STATEMENT:
    case RECEIVE_STATEMENT:
        // do nothing
        break;
    default:
        break;
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

static cap_result verifyScenarioStructure(IN cap_string strScenarioName, IN cap_handle hAppModel, IN OUT cap_string strError)
{
    cap_result result = ERR_CAP_UNKNOWN;

    result = DBHandler_CheckScenarioName(strScenarioName);
    if(result == ERR_CAP_DUPLICATED)
    {
        // ignore error to preserve previous error code.
        CAPString_AppendFormat(strError, "The same scenario name is already existed.%s", ERROR_SEPARATOR);
    }
    ERRIFGOTO(result, _EXIT);

    result = AppScriptModeler_TraverseExecutionNodeList(hAppModel, traverseExecutionNodeHash, strError);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result AppEngine_RunScenario(cap_handle hEngine, IN cap_string strScenarioName)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SAppEngine *pstAppEngine = NULL;
    cap_handle hAppRunner = NULL;

    if (IS_VALID_HANDLE(hEngine, HANDLEID_APP_ENGINE) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(strScenarioName, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstAppEngine = (SAppEngine *) hEngine;

    result = CAPThreadLock_Lock(pstAppEngine->hLock);
    ERRIFGOTO(result, _EXIT);

    result = CAPHash_GetDataByKey(pstAppEngine->hAppRunnerHash, strScenarioName, &hAppRunner);
    ERRIFGOTO(result, _EXIT_LOCK);

    result = AppRunner_Run(hAppRunner);
    ERRIFGOTO(result, _EXIT_LOCK);
    
    //Changing state of Scenario should be done in a function which called this function.

    result = ERR_CAP_NOERROR;
_EXIT_LOCK:
    CAPThreadLock_Unlock(pstAppEngine->hLock);
_EXIT:
    return result;
}


cap_result AppEngine_StopScenario(cap_handle hEngine, IN cap_string strScenarioName)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SAppEngine *pstAppEngine = NULL;
    cap_handle hAppRunner = NULL;

    if (IS_VALID_HANDLE(hEngine, HANDLEID_APP_ENGINE) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(strScenarioName, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstAppEngine = (SAppEngine *) hEngine;

    result = CAPThreadLock_Lock(pstAppEngine->hLock);
    ERRIFGOTO(result, _EXIT);

    result = CAPHash_GetDataByKey(pstAppEngine->hAppRunnerHash, strScenarioName, &hAppRunner);
    ERRIFGOTO(result, _EXIT_LOCK);

    result = AppRunner_Stop(hAppRunner);
    if(result == ERR_CAP_ALREADY_DONE)
    {// Because the scenario is already finished, just consider this case as no error.
        result = ERR_CAP_NOERROR;
    }
    ERRIFGOTO(result, _EXIT_LOCK);
    
    //Changing state of Scenario should be done in a function which called this function.

    result = ERR_CAP_NOERROR;
_EXIT_LOCK:
    CAPThreadLock_Unlock(pstAppEngine->hLock);
_EXIT:
    return result;
}


cap_result AppEngine_GetScenarioState(cap_handle hEngine, IN cap_string strScenarioName, OUT ERunnerState *penState)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SAppEngine *pstAppEngine = NULL;
    cap_handle hAppRunner = NULL;

    if (IS_VALID_HANDLE(hEngine, HANDLEID_APP_ENGINE) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(strScenarioName, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(penState, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstAppEngine = (SAppEngine *) hEngine;

    result = CAPThreadLock_Lock(pstAppEngine->hLock);
    ERRIFGOTO(result, _EXIT);

    result = CAPHash_GetDataByKey(pstAppEngine->hAppRunnerHash, strScenarioName, &hAppRunner);
    ERRIFGOTO(result, _EXIT_LOCK);

    result = AppRunner_GetState(hAppRunner, penState);
    ERRIFGOTO(result, _EXIT_LOCK);

    result = ERR_CAP_NOERROR;
_EXIT_LOCK:
    CAPThreadLock_Unlock(pstAppEngine->hLock);
_EXIT:
    return result;
}


cap_result AppEngine_VerifyScenario(cap_handle hEngine, IN cap_string strScenarioName,
        IN cap_string strText, IN OUT cap_string strError)
{
    cap_result result = ERR_CAP_UNKNOWN;
    cap_handle hAppModel = NULL;

    if (IS_VALID_HANDLE(hEngine, HANDLEID_APP_ENGINE) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    if(CAPString_Length(strScenarioName) <= 0 || CAPString_Length(strText) <= 0) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_PARAM, _EXIT);
    }

    IFVARERRASSIGNGOTO(strError, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    result = CAPString_SetLength(strError, 0);
    ERRIFGOTO(result, _EXIT);

    result = AppScriptModeler_Create(&hAppModel);
    ERRIFGOTO(result, _EXIT);

    result = AppScriptModeler_Load(hAppModel, strScenarioName, strText);
    if(result  != ERR_CAP_NOERROR)
    {
        // ignore error to preserve previous result error code
        result = AppScriptModeler_GetErrorInfo(hAppModel, strError);
        ERRIFGOTO(result, _EXIT);
    }
    ERRIFGOTO(result, _EXIT);

    result = verifyScenarioStructure(strScenarioName, hAppModel, strError);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    if(result != ERR_CAP_NOERROR && CAPString_Length(strError) == 0)
    {
        // ignore error to preserve previous error code.
        CAPString_PrintFormat(strError, "Internal error is happened during scenario verification(%d)%s",
                result, ERROR_SEPARATOR);
    }
    if(hAppModel != NULL)
    {
        AppScriptModeler_Destroy(&hAppModel);
    }
    return result;
}

cap_result AppEngine_Destroy(OUT cap_handle *phEngine)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SAppEngine *pstAppEngine = NULL;

    IFVARERRASSIGNGOTO(phEngine, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if (IS_VALID_HANDLE(*phEngine, HANDLEID_APP_ENGINE) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstAppEngine = (SAppEngine *) *phEngine;

    CAPHash_Destroy(&(pstAppEngine->hAppRunnerHash), destroyAppRunner, NULL);

    CAPThreadLock_Destroy(&(pstAppEngine->hLock));
    SAFE_CAPSTRING_DELETE(pstAppEngine->strBrokerURI);

    SAFEMEMFREE(pstAppEngine);

    *phEngine = NULL;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result AppEngine_AddScenario(cap_handle hEngine, IN cap_string strScenarioName, IN cap_string strText,
        IN OUT cap_string strError, IN OUT cap_handle hThingList)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SAppEngine *pstAppEngine = NULL;
    cap_handle hAppModel = NULL;
    cap_handle hHash = NULL;
    cap_handle hAppRunner = NULL;
    int nLen = 0;

    if (IS_VALID_HANDLE(hEngine, HANDLEID_APP_ENGINE) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    if(CAPString_Length(strScenarioName) <= 0 || CAPString_Length(strText) <= 0) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_PARAM, _EXIT);
    }

    IFVARERRASSIGNGOTO(strError, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if(hThingList != NULL)
    {
        if (IS_VALID_HANDLE(hThingList, HANDLEID_CAP_LINKED_LIST) == FALSE) {
                ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
        }

        result = CAPLinkedList_GetLength(hThingList, &nLen);
        ERRIFGOTO(result, _EXIT);

        if(nLen != 0) {
            ERRASSIGNGOTO(result, ERR_CAP_NOT_EMTPY, _EXIT);
        }
    }

    pstAppEngine = (SAppEngine *) hEngine;

    result = CAPString_SetLength(strError, 0);
    ERRIFGOTO(result, _EXIT);

    result = AppScriptModeler_Create(&hAppModel);
    ERRIFGOTO(result, _EXIT);

    result = AppScriptModeler_Load(hAppModel, strScenarioName, strText);
    if(result  != ERR_CAP_NOERROR)
    {
        // ignore error to preserve previous result error code
        result = AppScriptModeler_GetErrorInfo(hAppModel, strError);
        ERRIFGOTO(result, _EXIT);
    }
    ERRIFGOTO(result, _EXIT);

    result = verifyScenarioStructure(strScenarioName, hAppModel, strError);
    ERRIFGOTO(result, _EXIT);

    result = AppScriptModeler_GetThingKeywordHash(hAppModel, &hHash);
    ERRIFGOTO(result, _EXIT);

    result = CAPHash_Traverse(hHash, traverseAndFillRealThingName, hThingList);
    ERRIFGOTO(result, _EXIT);

    result = AppRunner_Create(hAppModel, pstAppEngine->strBrokerURI, &hAppRunner);
    ERRIFGOTO(result, _EXIT);

    hAppModel = NULL; // AppModel is assigned to AppRunner, so do not free this handle

    result = AppRunner_Run(hAppRunner);
    ERRIFGOTO(result, _EXIT);

    result = CAPThreadLock_Lock(pstAppEngine->hLock);
    ERRIFGOTO(result, _EXIT);

    result = CAPHash_AddKey(pstAppEngine->hAppRunnerHash, strScenarioName, hAppRunner);
    ERRIFGOTO(result, _EXIT_LOCK);

    result = ERR_CAP_NOERROR;
_EXIT_LOCK:
    CAPThreadLock_Unlock(pstAppEngine->hLock);
_EXIT:
    if(result != ERR_CAP_NOERROR)
    {
        // Because AppModel is assigned to AppRunner, remove AppRunner only after creating AppRunner.
        // (AppModel will be automatically removed from AppRunner)
        if(hAppRunner != NULL) {
            AppRunner_Join(hAppRunner);
            AppRunner_Destroy(&hAppRunner);
        }
        else if(hAppModel != NULL) {
            AppScriptModeler_Destroy(&hAppModel);
        }

        if(CAPString_Length(strError) == 0)
        {
            // ignore error result setting to preserve previous error code.
            CAPString_PrintFormat(strError, "Internal error is happened during scenario add(%d)%s",
                    result, ERROR_SEPARATOR);
        }
    }
    return result;
}


cap_result AppEngine_DeleteScenario(cap_handle hEngine, IN cap_string strScenarioName)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SAppEngine *pstAppEngine = NULL;

    if (IS_VALID_HANDLE(hEngine, HANDLEID_APP_ENGINE) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    if(CAPString_Length(strScenarioName) <= 0) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_PARAM, _EXIT);
    }

    pstAppEngine = (SAppEngine *) hEngine;

    result = CAPThreadLock_Lock(pstAppEngine->hLock);
    ERRIFGOTO(result, _EXIT);

    result = CAPHash_DeleteKey(pstAppEngine->hAppRunnerHash, strScenarioName, destroyAppRunner, NULL);
    ERRIFGOTO(result, _EXIT_LOCK);

    result = ERR_CAP_NOERROR;
_EXIT_LOCK:
    CAPThreadLock_Unlock(pstAppEngine->hLock);
_EXIT:
    return result;
}


