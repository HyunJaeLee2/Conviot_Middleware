/*
 * CAPLinkedList.c
 *
 *  Created on: 2015. 5. 21.
 *      Author: chjej202
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <CAPLinkedList.h>

#define CURRENT_NOT_SET (-1)

typedef struct _SNode {
    void *pData;
    struct _SNode *pNext;
} SNode;

typedef struct _SCAPLinkedList {
    EHandleId enId;
    struct _SNode *pFirst;
    struct _SNode *pLast;
    struct _SNode *pCurrent;
    int nLinkSize;
    int nCurrent;
} SCAPLinkedList;

cap_result CAPLinkedList_Create(OUT cap_handle *phLinkedList)
{
    SCAPLinkedList* pstLinkedList = NULL;
    cap_result result = ERR_CAP_UNKNOWN;

    IFVARERRASSIGNGOTO(phLinkedList, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstLinkedList = (SCAPLinkedList *) malloc(sizeof(SCAPLinkedList));
    ERRMEMGOTO(pstLinkedList, result, _EXIT);

    pstLinkedList->enId = HANDLEID_CAP_LINKED_LIST;
    pstLinkedList->pFirst = NULL;
    pstLinkedList->pLast = NULL;
    pstLinkedList->pCurrent = NULL;
    pstLinkedList->nLinkSize = 0;
    pstLinkedList->nCurrent = CURRENT_NOT_SET;

    *phLinkedList = (cap_handle) pstLinkedList;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result CAPLinkedList_Add(cap_handle hLinkedList, IN ELinkedListOffset enOffset, IN int nIndex, IN void *pData)
{
    SNode *pstParentNode = NULL;
    SNode *pstNewNode = NULL;
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPLinkedList* pstLinkedList = NULL;
    int nOffset = 0;

    if (IS_VALID_HANDLE(hLinkedList, HANDLEID_CAP_LINKED_LIST) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstLinkedList = (SCAPLinkedList*) hLinkedList;

    if (enOffset != LINKED_LIST_OFFSET_FIRST
            && enOffset != LINKED_LIST_OFFSET_LAST
            && enOffset != LINKED_LIST_OFFSET_CURRENT)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_PARAM, _EXIT);
    }

    if (enOffset == LINKED_LIST_OFFSET_FIRST) {
        nOffset = 0;
    } else if (enOffset == LINKED_LIST_OFFSET_CURRENT) {
        nOffset = pstLinkedList->nCurrent;
    } else { // enOffset == LINKED_LIST_OFFSET_LAST || LINKED_LIST_OFFSET_DEFAULT
        nOffset = pstLinkedList->nLinkSize;
    }

    nOffset = nOffset + nIndex;

    if (nOffset < 0 || nOffset > pstLinkedList->nLinkSize) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_PARAM, _EXIT);
    }

    pstNewNode = (SNode *) malloc(sizeof(SNode));
    ERRMEMGOTO(pstNewNode, result, _EXIT);

    if (pstLinkedList->nLinkSize == 0) { /* If a linked list is empty */
        pstNewNode->pData = pData;
        pstNewNode->pNext = NULL;
        pstLinkedList->pFirst = pstNewNode;
        pstLinkedList->pLast = pstLinkedList->pFirst;
        pstLinkedList->pCurrent = pstLinkedList->pFirst;
        pstLinkedList->nLinkSize++;
        pstLinkedList->nCurrent = 0;
    } else {
        if (nOffset == 0) {
            /* add first */
            pstNewNode->pData = pData;
            pstNewNode->pNext = pstLinkedList->pFirst;

            pstLinkedList->pFirst = pstNewNode;
        } else {
            result = CAPLinkedList_Seek(hLinkedList, LINKED_LIST_OFFSET_FIRST,
                    nOffset - 1);
            ERRIFGOTO(result, _EXIT);

            if (nOffset == pstLinkedList->nLinkSize) {
                /* add last */
                pstParentNode = pstLinkedList->pCurrent;
                pstParentNode->pNext = pstNewNode;

                pstNewNode->pData = pData;
                pstNewNode->pNext = NULL;

                pstLinkedList->pLast = pstNewNode;
            } else {
                /* add in middle */
                pstParentNode = pstLinkedList->pCurrent;

                pstNewNode->pData = pData;
                pstNewNode->pNext = pstParentNode->pNext;

                pstParentNode->pNext = pstNewNode;
            }
        }

        pstLinkedList->pCurrent = pstNewNode;
        pstLinkedList->nCurrent = nOffset;
        pstLinkedList->nLinkSize++;
    }
    result = ERR_CAP_NOERROR;
_EXIT:
    if (result != ERR_CAP_NOERROR) {
        SAFEMEMFREE(pstNewNode);
    }
    return result;
}

cap_result CAPLinkedList_Seek(cap_handle hLinkedList, IN ELinkedListOffset enOffset, IN int nIndex) {
    SCAPLinkedList *pstLinkedList = NULL;
    cap_result result = ERR_CAP_UNKNOWN;
    int nOffset = 0;
    int nLoop = 0;

    if (IS_VALID_HANDLE(hLinkedList, HANDLEID_CAP_LINKED_LIST) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstLinkedList = (SCAPLinkedList *) hLinkedList;

    if (enOffset == LINKED_LIST_OFFSET_FIRST) {
        nOffset = 0;
    } else if (enOffset == LINKED_LIST_OFFSET_CURRENT) {
        nOffset = pstLinkedList->nCurrent;
    } else { // enOffset == LINKED_LIST_OFFSET_LAST || LINKED_LIST_OFFSET_DEFAULT
        nOffset = pstLinkedList->nLinkSize;
    }

    nOffset = nOffset + nIndex;

    if (nOffset < 0 || nOffset > pstLinkedList->nLinkSize) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_PARAM, _EXIT);
    }

    if (pstLinkedList->nLinkSize == 0) {
        ERRASSIGNGOTO(result, ERR_CAP_NO_DATA, _EXIT);
    }

    if (nOffset >= pstLinkedList->nCurrent) {
        for (nLoop = 0; nLoop < nOffset - pstLinkedList->nCurrent; nLoop++) {
            pstLinkedList->pCurrent = pstLinkedList->pCurrent->pNext;
        }
    } else {
        pstLinkedList->pCurrent = pstLinkedList->pFirst;

        for (nLoop = 0; nLoop < nOffset; nLoop++) {
            pstLinkedList->pCurrent = pstLinkedList->pCurrent->pNext;
        }
    }

    pstLinkedList->nCurrent = nOffset;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result CAPLinkedList_Get(cap_handle hLinkedList,
        IN ELinkedListOffset enOffset, IN int nIndex, OUT void **ppData)
{
    SCAPLinkedList* pstLinkedList = NULL;
    cap_result result = ERR_CAP_UNKNOWN;

    if (IS_VALID_HANDLE(hLinkedList, HANDLEID_CAP_LINKED_LIST) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(ppData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstLinkedList = (SCAPLinkedList *) hLinkedList;

    result = CAPLinkedList_Seek(pstLinkedList, enOffset, nIndex);
    ERRIFGOTO(result, _EXIT);

    // nIndex is not located in the linked list
    if(pstLinkedList->pCurrent == NULL) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_PARAM, _EXIT);
    }

    *ppData = (void *) (pstLinkedList->pCurrent->pData);

    result = ERR_CAP_NOERROR;
_EXIT: return result;
}

// set the data to existing node
// before setting the new data, use CAPLinkedList_Get to get the internal data and free the data first
cap_result CAPLinkedList_Set(cap_handle hLinkedList,
        IN ELinkedListOffset enOffset, IN int nIndex, IN void *pData)
{
    SCAPLinkedList* pstLinkedList = NULL;
    cap_result result = ERR_CAP_UNKNOWN;

    if (IS_VALID_HANDLE(hLinkedList, HANDLEID_CAP_LINKED_LIST) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(pData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstLinkedList = (SCAPLinkedList *) hLinkedList;

    result = CAPLinkedList_Seek(pstLinkedList, enOffset, nIndex);
    ERRIFGOTO(result, _EXIT);

    // nIndex is not located in the linked list
    if(pstLinkedList->pCurrent == NULL)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_PARAM, _EXIT);
    }

    pstLinkedList->pCurrent->pData = pData;

    result = ERR_CAP_NOERROR;
_EXIT: return result;
}

cap_result CAPLinkedList_GetLength(cap_handle hLinkedList, OUT int *pnLength)
{
    SCAPLinkedList* pstLinkedList = NULL;
    cap_result result = ERR_CAP_UNKNOWN;

    if (IS_VALID_HANDLE(hLinkedList, HANDLEID_CAP_LINKED_LIST) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(pnLength, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstLinkedList = (SCAPLinkedList *) hLinkedList;

    *pnLength = pstLinkedList->nLinkSize;

    result = ERR_CAP_NOERROR;
_EXIT: return result;
}

cap_result CAPLinkedList_Duplicate(cap_handle hLinkedListDst, cap_handle hLinkedListSrc, IN CbFnCAPLinkedListDup fnCallback, IN void *pUserData) {
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPLinkedList* pstLinkedListDst = NULL;
    SCAPLinkedList* pstLinkedListSrc = NULL;
    SNode *pstNode = NULL;
    int nOffset = 0;
    void *pDataDst = NULL;

    if (IS_VALID_HANDLE(hLinkedListDst, HANDLEID_CAP_LINKED_LIST) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    if (IS_VALID_HANDLE(hLinkedListSrc, HANDLEID_CAP_LINKED_LIST) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(fnCallback, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstLinkedListDst = (SCAPLinkedList *) hLinkedListDst;
    pstLinkedListSrc = (SCAPLinkedList *) hLinkedListSrc;

    if(pstLinkedListDst->nLinkSize > 0) {
        ERRASSIGNGOTO(result, ERR_CAP_NOT_EMTPY, _EXIT);
    }

    if(pstLinkedListSrc->nLinkSize == 0) {
        ERRASSIGNGOTO(result, ERR_CAP_NO_DATA, _EXIT);
    }

    for(pstNode = pstLinkedListSrc->pFirst; pstNode != NULL ; pstNode = pstNode->pNext) {
        result = fnCallback(nOffset, pstNode->pData, pUserData, &pDataDst);
        ERRIFGOTO(result, _EXIT);

        result = CAPLinkedList_Add(hLinkedListDst, LINKED_LIST_OFFSET_LAST, 0, pDataDst);
        ERRIFGOTO(result, _EXIT);
        nOffset++;
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result CAPLinkedList_Remove(cap_handle hLinkedList,
        IN ELinkedListOffset enOffset, IN int nIndex)
{
    SNode *pstCurNode = NULL;
    SNode *pstNextNode = NULL;
    SCAPLinkedList* pstLinkedList = NULL;
    cap_result result = ERR_CAP_UNKNOWN;
    int nOffset = 0;

    if (IS_VALID_HANDLE(hLinkedList, HANDLEID_CAP_LINKED_LIST) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstLinkedList = (SCAPLinkedList *) hLinkedList;

    if (enOffset == LINKED_LIST_OFFSET_FIRST)
    {
        nOffset = 0;
    }
    else if (enOffset == LINKED_LIST_OFFSET_CURRENT)
    {
        nOffset = pstLinkedList->nCurrent;
    }
    else // enOffset == LINKED_LIST_OFFSET_LAST || LINKED_LIST_OFFSET_DEFAULT
    {
        nOffset = pstLinkedList->nLinkSize;
    }

    nOffset = nOffset + nIndex;

    // To remove the last node, enOffset = LINKED_LIST_OFFSET_LAST, nIndex = -1
    if (nOffset < 0 || nOffset >= pstLinkedList->nLinkSize)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_PARAM, _EXIT);
    }

    if (pstLinkedList->nLinkSize == 0)
    {
        ERRASSIGNGOTO(result, ERR_CAP_NO_DATA, _EXIT);
    }

    if (nOffset == 0)
    { /* remove first */
        pstCurNode = pstLinkedList->pFirst;
        pstLinkedList->pFirst = pstCurNode->pNext;
        pstLinkedList->pCurrent = pstLinkedList->pFirst;
        pstLinkedList->nCurrent = nOffset;
    }
    else
    {
        result = CAPLinkedList_Seek(hLinkedList, enOffset, nIndex - 1);
        ERRIFGOTO(result, _EXIT);

        if (nOffset == pstLinkedList->nLinkSize - 1)
        { /* remove last */
            pstCurNode = pstLinkedList->pCurrent->pNext;

            pstLinkedList->pLast = pstLinkedList->pCurrent;
            pstLinkedList->pLast->pNext = NULL;
            pstLinkedList->pCurrent = pstLinkedList->pLast;
            pstLinkedList->nCurrent = nOffset - 1;
        }
        else
        { /* remove in middle */
            pstCurNode = pstLinkedList->pCurrent->pNext;
            pstNextNode = pstCurNode->pNext;
            pstLinkedList->pCurrent->pNext = pstNextNode;
            pstLinkedList->nCurrent = nOffset - 1;
        }
    }
    SAFEMEMFREE(pstCurNode);
    pstLinkedList->nLinkSize--;

    result = ERR_CAP_NOERROR;
_EXIT: return result;
}

cap_result CAPLinkedList_Traverse(cap_handle hLinkedList, IN CbFnCAPLinkedList fnCallback, IN void *pUserData) {
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPLinkedList* pstLinkedList = NULL;
    SNode *pstNode = NULL;
    int nOffset = 0;

    if (IS_VALID_HANDLE(hLinkedList, HANDLEID_CAP_LINKED_LIST) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstLinkedList = (SCAPLinkedList*) hLinkedList;

    for(pstNode = pstLinkedList->pFirst; pstNode != NULL ; pstNode = pstNode->pNext)
    {
        result = fnCallback(nOffset, pstNode->pData, pUserData);
        if(result == ERR_CAP_USER_CANCELED) // if a user canceled the work, just return no error
        {
            result = ERR_CAP_NOERROR;
            break;
        }
        else if(result == ERR_CAP_FOUND_DATA)
        {
            CAPASSIGNGOTO(result, ERR_CAP_FOUND_DATA, _EXIT);
        }
        ERRIFGOTO(result, _EXIT);
        nOffset++;
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result CAPLinkedList_Attach(IN OUT cap_handle hLinkedListDst, IN OUT cap_handle *phLinkedListSrc) {
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPLinkedList* pstLinkedListDst = NULL;
    SCAPLinkedList* pstLinkedListSrc = NULL;

    if (IS_VALID_HANDLE(hLinkedListDst, HANDLEID_CAP_LINKED_LIST) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(phLinkedListSrc, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if (IS_VALID_HANDLE(*phLinkedListSrc, HANDLEID_CAP_LINKED_LIST) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstLinkedListDst = (SCAPLinkedList *) hLinkedListDst;
    pstLinkedListSrc = (SCAPLinkedList *) *phLinkedListSrc;

    // The destination linked list is empty.
    if(pstLinkedListDst->nLinkSize == 0)
    {
        // attach at the first node
        pstLinkedListDst->pFirst = pstLinkedListSrc->pFirst;

        // set new current pointer
        pstLinkedListDst->pCurrent = pstLinkedListDst->pFirst;
        pstLinkedListDst->nCurrent = 0;
    }
    else // The destination linked list already has own list.
    {
        // attach at the next to the last node
        pstLinkedListDst->pLast->pNext = pstLinkedListSrc->pFirst;
    }

    pstLinkedListDst->pLast = pstLinkedListSrc->pLast;

    pstLinkedListDst->nLinkSize = pstLinkedListDst->nLinkSize
        + pstLinkedListSrc->nLinkSize;

    // Remove source linked list structure
    SAFEMEMFREE(pstLinkedListSrc);
    *phLinkedListSrc = NULL;

    result = ERR_CAP_NOERROR;
_EXIT: return result;
}

cap_result CAPLinkedList_Split(IN OUT cap_handle phLinkedList_front,
        IN OUT cap_handle phLinkedList_end)
{
    return ERR_CAP_NOERROR;
}


// CAPLinkedList_Destroy does not free the internal void pointer which is inserted by CAPLinkedList_Add function.
// To destroy all the elements in the linked list,
// API user should use CAPLinkedList_Traverse to traverse all the linked list element and destroy the internal contents.
cap_result CAPLinkedList_Destroy(IN OUT cap_handle *phLinkedList)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPLinkedList* pstLinkedList = NULL;
    int nLoop = 0;
    int nLinkedListSize = 0;

    IFVARERRASSIGNGOTO(phLinkedList, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if (IS_VALID_HANDLE(*phLinkedList, HANDLEID_CAP_LINKED_LIST) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstLinkedList = (SCAPLinkedList*) *phLinkedList;

    nLinkedListSize = pstLinkedList->nLinkSize;
    for(nLoop = 0 ; nLoop < nLinkedListSize ; nLoop++)
        //for(pstNode = pstLinkedList->pFirst; pstNode != NULL ; pstNode = pstNode->pNext)
    {
        result = CAPLinkedList_Remove((cap_handle) pstLinkedList, LINKED_LIST_OFFSET_FIRST, 0);
        // ignore error;
    }

    SAFEMEMFREE(pstLinkedList);
    *phLinkedList = NULL;

    result = ERR_CAP_NOERROR;
_EXIT: return result;
}
