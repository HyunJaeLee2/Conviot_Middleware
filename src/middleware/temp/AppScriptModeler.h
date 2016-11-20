/*
 * AppScriptModeler.h
 *
 *  Created on: 2016. 8. 2.
 *      Author: chjej202
 */

#ifndef APP_SCRIPTMODELER_H_
#define APP_SCRIPTMODELER_H_

#include <CAPString.h>
#include <CAPHash.h>
#include <CAPKeyValue.h>
#include <CAPStack.h>

#include <capiot_common.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _EStatementType
{
    IF_STATEMENT,
    LOOP_STATEMENT,
    SEND_STATEMENT,
    RECEIVE_STATEMENT,
    ACTION_STATEMENT,
    FINISH_STATEMENT,
    WAIT_UNTIL_STATEMENT,
} EStatementType;

typedef enum _ETimeUnit
{
    TIME_UNIT_SEC,
    TIME_UNIT_MINUTE,
    TIME_UNIT_HOUR,
    TIME_UNIT_DAY,
} ETimeUnit;

typedef enum _EOperator
{
    OPERATOR_NONE,
    OPERATOR_EQUAL,
    OPERATOR_GREATER,
    OPERATOR_LESS,
    OPERATOR_GREATER_EQUAL,
    OPERATOR_LESS_EQUAL,
    OPERATOR_NOT_EQUAL,
    OPERATOR_AND,
    OPERATOR_OR,
    OPERATOR_NOT,
    OPERATOR_LEFT_PARENTHESIS,
    OPERATOR_RIGHT_PARENTHESIS,
} EOperator;

typedef enum _EExpressionType
{
    EXP_TYPE_VARIABLE,
    EXP_TYPE_STRING,
    EXP_TYPE_INTEGER,
    EXP_TYPE_DOUBLE,
    EXP_TYPE_MEMBER_VARIABLE,
} EExpressionType;

typedef struct _SExpression
{
    EExpressionType enType;
    cap_string strPrimaryIdentifier;
    cap_string strSubIdentifier;
    double dbValue;
    cap_string strStringValue;
} SExpression;

typedef struct _SCondition
{
    SExpression *pstLeftOperand;
    EOperator enOperator;
    SExpression *pstRightOperand;
    cap_bool bConditionTrue; // initially set to zero
    cap_bool bInit; // this is for checking the condition data is dirty or not
} SCondition;

typedef struct _SPeriod {
    ETimeUnit enTimeUnit;
    double dbTimeVal;
} SPeriod;

// use this from loop, wait until, if
typedef struct _SConditionalStatement
{
    SPeriod stPeriod;
    cap_handle hConditionList;
} SConditionalStatement;

// use this from send, receive, Action, execute
typedef struct _SExecutionStatement
{
    cap_handle hInputList;
    cap_handle hOutputList;
    cap_string strPrimaryIdentifier; // General/First IDENTIFIER
    cap_string strSubIdentifier; // second part of IDENTIFIER.IDENTIFIER
} SExecutionStatement;

typedef struct _SExecutionNode SExecutionNode;

typedef struct _SExecutionNode
{
    EStatementType enType;
    void *pstStatementData; // SExecutionStatement or SConditionalStatement
    SExecutionNode *pstNextTrue;
    SExecutionNode *pstNextFalse;
    int nLineNo;
} SExecutionNode;

typedef struct _SExecutionGraph
{
  void *pGraphData;
  SExecutionNode *pstRoot;
  SExecutionNode *pstCurrent;
} SExecutionGraph;

typedef struct _SThingKeywords
{
    cap_handle hValueHash;
    cap_handle hFunctionHash;
    cap_string strThingRealId;
} SThingKeywords;

typedef struct _SThingVariable
{
    cap_string strVarName;
    cap_string strClass;
    int nArrayNum;
} SThingVariable;

typedef struct _SGroupBehavior
{
    cap_string strTeamName;
    cap_handle hThingNameHash;
    SExecutionGraph *pstAction;
    SExecutionGraph *pstListen;
    SExecutionGraph *pstReport;
} STeamBehavior;

typedef struct _SScenario
{
    cap_string strScenarioText;
    cap_string strScenarioName;
    SExecutionGraph *pstGlobalExecModel;
    cap_handle hGroupExecModelHash;
    cap_handle hThingHash;
    cap_bool bNotLoaded; // initialize to FALSE
    cap_handle hExecutionNodeList;
    SExecutionNode *pstFinish;
    cap_handle hLock;
    cap_string strErrorString;
} SScenario;

typedef cap_result (*CbFnExecutionNodeTraverse)(int nIndex, SExecutionNode *pstNode, IN void *pUserData);

cap_result AppScriptModeler_Create(OUT cap_handle *phModel);
cap_result AppScriptModeler_Load(cap_handle hModel, cap_string strScenarioName, cap_string strText);
cap_result AppScriptModeler_GetScenarioName(cap_handle hModel, cap_string strScenarioName);
cap_result AppScriptModeler_GetCurrent(cap_handle hModel, OUT SExecutionNode **ppstExecNode);
cap_result AppScriptModeler_MoveToNext(cap_handle hModel, cap_bool bDirection, OUT SExecutionNode **ppstExecNode);
cap_result AppScriptModeler_ClearExecution(cap_handle hModel);
cap_result AppScriptModeler_GetThingKeywordHash(cap_handle hModel, OUT cap_handle *phThingHash);
cap_result AppScriptModeler_GetThingRealId(cap_handle hModel, IN cap_string strVirtuaThinglId, OUT cap_string strReaThinglId);
cap_result AppScriptModeler_TraverseExecutionNodeList(cap_handle hModel, IN CbFnExecutionNodeTraverse fnCallback, IN void *pUserData);
cap_result AppScriptModeler_GetErrorInfo(cap_handle hModel, IN OUT cap_string strErrorInfo);
cap_result AppScriptModeler_Destroy(IN OUT cap_handle *phModel);


#ifdef __cplusplus
}
#endif

#endif /* APP_SCRIPTMODELER_H_ */
