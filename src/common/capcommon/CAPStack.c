/*
 * CAPStack.c
 *
 *  Created on: 2016. 5. 10.
 *      Author: chjej202
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cap_common.h>

#include <CAPLinkedList.h>
#include <CAPStack.h>

typedef struct _SCAPStack {
    EHandleId enId;
    cap_handle hLinkedList;
} SCAPStack;

typedef struct _SCAPStackUserData {
    CbFnCAPStack fnDestroyCallback;
    void *pUserData;
} SCAPStackUserData;

cap_result removeStackData(IN int nOffset, IN void *pData, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPStackUserData *pstUserData = NULL;

    pstUserData = (SCAPStackUserData *) pUserData;

    pstUserData->fnDestroyCallback(pData, pstUserData->pUserData);

    result = ERR_CAP_NOERROR;

    return result;
}


cap_result CAPStack_Create(OUT cap_handle *phStack)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPStack * pstStack = NULL;

    IFVARERRASSIGNGOTO(phStack, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstStack = (SCAPStack * ) malloc(sizeof(SCAPStack));
    ERRMEMGOTO(pstStack, result, _EXIT);

    pstStack->enId = HANDLEID_CAP_STACK;
    pstStack->hLinkedList = NULL;

    result = CAPLinkedList_Create(&(pstStack->hLinkedList));
    ERRIFGOTO(result, _EXIT);

    *phStack = pstStack;

    result = ERR_CAP_NOERROR;
_EXIT:
    if(result != ERR_CAP_NOERROR && pstStack != NULL)
    {
        CAPLinkedList_Destroy(&(pstStack->hLinkedList));
        SAFEMEMFREE(pstStack);
    }
    return result;
}


cap_result CAPStack_Push(cap_handle hStack, IN void *pData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPStack * pstStack = NULL;

    if (IS_VALID_HANDLE(hStack, HANDLEID_CAP_STACK) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstStack = (SCAPStack *) hStack;

    result = CAPLinkedList_Add(pstStack->hLinkedList, LINKED_LIST_OFFSET_FIRST, 0, pData);
    ERRIFGOTO(result, _EXIT);

    result = CAPLinkedList_Get(pstStack->hLinkedList, LINKED_LIST_OFFSET_FIRST, 0, &pData);
    ERRIFGOTO(result, _EXIT);
   
    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result CAPStack_Top(cap_handle hStack, OUT void **ppData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPStack * pstStack = NULL;
    void *pData = NULL;
    int nLen = 0;

    if (IS_VALID_HANDLE(hStack, HANDLEID_CAP_STACK) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(ppData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstStack = (SCAPStack *) hStack;

    result = CAPLinkedList_GetLength(pstStack->hLinkedList, &nLen);
    ERRIFGOTO(result, _EXIT);

    if(nLen == 0)
    {
        ERRASSIGNGOTO(result, ERR_CAP_NO_DATA, _EXIT);
    }

    result = CAPLinkedList_Get(pstStack->hLinkedList, LINKED_LIST_OFFSET_FIRST, 0, &pData);
    ERRIFGOTO(result, _EXIT);

    *ppData = pData;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result CAPStack_Pop(cap_handle hStack, OUT void **ppData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPStack * pstStack = NULL;
    void *pData = NULL;
    int nLen = 0;

    if (IS_VALID_HANDLE(hStack, HANDLEID_CAP_STACK) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(ppData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstStack = (SCAPStack *) hStack;

    result = CAPLinkedList_GetLength(pstStack->hLinkedList, &nLen);
    ERRIFGOTO(result, _EXIT);

    if(nLen == 0)
    {
        ERRASSIGNGOTO(result, ERR_CAP_NO_DATA, _EXIT);
    }

    result = CAPLinkedList_Get(pstStack->hLinkedList, LINKED_LIST_OFFSET_FIRST, 0, &pData);
    ERRIFGOTO(result, _EXIT);

    result = CAPLinkedList_Remove(pstStack->hLinkedList, LINKED_LIST_OFFSET_FIRST, 0);
    ERRIFGOTO(result, _EXIT);

    *ppData = pData;
    
    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result CAPStack_Duplicate(cap_handle hStack, IN cap_handle hSrcStack, IN CbFnCAPStackDup fnCallback, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;

    if (IS_VALID_HANDLE(hStack, HANDLEID_CAP_STACK) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    if (IS_VALID_HANDLE(hSrcStack, HANDLEID_CAP_STACK) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(fnCallback, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    // CAPLinkedList_Duplicate will retrieve appropriate error codes
    // such as ERR_CAP_NOT_EMPTY ,or ERR_CAP_NO_DATA.
    result = CAPLinkedList_Duplicate(hStack, hSrcStack, fnCallback, pUserData);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result CAPStack_Length(cap_handle hStack, OUT int *pnLength)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPStack * pstStack = NULL;
    int nLen = 0;

    if (IS_VALID_HANDLE(hStack, HANDLEID_CAP_STACK) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(pnLength, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstStack = (SCAPStack *) hStack;

    result = CAPLinkedList_GetLength(pstStack->hLinkedList, &nLen);
    ERRIFGOTO(result, _EXIT);

    *pnLength = nLen;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result CAPStack_Destroy(IN OUT cap_handle *phStack, IN CbFnCAPStack fnDestroyCallback, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPStack * pstStack = NULL;
    SCAPStackUserData stCbData;

    IFVARERRASSIGNGOTO(phStack, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if (IS_VALID_HANDLE(*phStack, HANDLEID_CAP_STACK) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstStack = (SCAPStack *) *phStack;

    // ignore errors in destroy function
    if(fnDestroyCallback != NULL)
    {
        stCbData.fnDestroyCallback = fnDestroyCallback;
        stCbData.pUserData = pUserData;
        CAPLinkedList_Traverse(pstStack->hLinkedList, removeStackData, &stCbData);
    }

    CAPLinkedList_Destroy(&(pstStack->hLinkedList));
    SAFEMEMFREE(pstStack);

    *phStack = NULL;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}



