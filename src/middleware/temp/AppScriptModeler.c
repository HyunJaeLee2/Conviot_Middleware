/*
 * AppScriptModeler.c
 *
 *  Created on: 2016. 8. 16.
 *      Author: chjej202
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include "AppScriptModeler.h"


cap_result AppScriptModeler_Create(OUT cap_handle *phModel)
{
    cap_result result = ERR_CAP_UNKNOWN;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result AppScriptModeler_Load(cap_handle hModel, cap_string strScenarioName, cap_string strText)
{
    cap_result result = ERR_CAP_UNKNOWN;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result AppScriptModeler_GetScenarioName(cap_handle hModel, cap_string strScenarioName)
{
    cap_result result = ERR_CAP_UNKNOWN;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result AppScriptModeler_GetCurrent(cap_handle hModel, OUT SExecutionNode **pstExecNode)
{
    cap_result result = ERR_CAP_UNKNOWN;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result AppScriptModeler_MoveToNext(cap_handle hModel, cap_bool bDirection, OUT SExecutionNode **pstExecNode)
{
    cap_result result = ERR_CAP_UNKNOWN;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result AppScriptModeler_GetThingKeywordHash(cap_handle hModel, OUT cap_handle *phThingHash)
{
    cap_result result = ERR_CAP_UNKNOWN;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result AppScriptModeler_GetThingRealId(cap_handle hModel, IN cap_string strVirtuaThinglId, OUT cap_string strReaThinglId)
{
    cap_result result = ERR_CAP_UNKNOWN;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result AppScriptModeler_TraverseExecutionNodeList(cap_handle hModel, IN CbFnExecutionNodeTraverse fnCallback, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}



cap_result AppScriptModeler_GetErrorInfo(cap_handle hModel, IN OUT cap_string strErrorInfo)
{
    cap_result result = ERR_CAP_UNKNOWN;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result AppScriptModeler_Destroy(IN OUT cap_handle *phModel)
{
    cap_result result = ERR_CAP_UNKNOWN;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}




