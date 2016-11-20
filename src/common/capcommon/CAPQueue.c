/*
 * CAPQueue.c
 *
 *  Created on: 2015. 5. 21.
 *      Author: chjej202
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <CAPLinkedList.h>
#include <CAPThreadLock.h>
#include <CAPThreadEvent.h>
#include <CAPQueue.h>


typedef struct _SQueueCallback {
	CbFnQueueTraverse fnCallback;
	void *pUserData;
} SQueueCallback;

typedef struct _SQueueData {
	void *pData;
} SQueueData;

typedef struct _SCAPQueue {
	EHandleId enID;
	cap_handle hEvent; // This might be used for request queue
	cap_handle hLock;
	cap_handle hDataLinkedList;
	cap_bool bDestroy;
} SCAPQueue;

#define DATA_OFFSET_NOT_SET (-1)


static CALLBACK cap_result CAPQueueCB_DataDestroy(int nOffset, void *pData, void *pUsrData)
{
	cap_result result = ERR_CAP_UNKNOWN;
	SQueueData *pstQueueData = NULL;
	SQueueCallback *pstQueueCallback = NULL;

	IFVARERRASSIGNGOTO(pData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

	pstQueueData = (SQueueData *) pData;

	if (pUsrData != NULL)
	{
		pstQueueCallback = (SQueueCallback *) pUsrData;
		// callback for pstQueue is needed here
		result = (pstQueueCallback->fnCallback)(pstQueueData->pData, pstQueueCallback->pUserData);
		// skip an error check at least to destroy the upper structures
	}

	SAFEMEMFREE(pstQueueData);


	result = ERR_CAP_NOERROR;
_EXIT:
	return result;
}


cap_result CAPQueue_Create(OUT cap_handle *phQueue)
{
	cap_result result = ERR_CAP_UNKNOWN;
	SCAPQueue *pstQueue = NULL;

	IFVARERRASSIGNGOTO(phQueue, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

	pstQueue = (SCAPQueue *) malloc(sizeof(SCAPQueue));
	ERRMEMGOTO(pstQueue, result, _EXIT);

	pstQueue->enID = HANDLEID_CAP_THREAD_QUEUE;
	pstQueue->hDataLinkedList = NULL;
	pstQueue->hEvent = NULL;
	pstQueue->hLock = NULL;
	pstQueue->bDestroy = FALSE;

	result = CAPLinkedList_Create(&(pstQueue->hDataLinkedList));
	ERRIFGOTO(result, _EXIT);

	result = CAPThreadEvent_Create(&(pstQueue->hEvent));
	ERRIFGOTO(result, _EXIT);

	result = CAPThreadLock_Create(&(pstQueue->hLock));
	ERRIFGOTO(result, _EXIT);

	*phQueue = pstQueue;

	result = ERR_CAP_NOERROR;
_EXIT:
	if (result != ERR_CAP_NOERROR)
	{
		SAFEMEMFREE(pstQueue);
	}
	return result;
}

cap_result CAPQueue_SetExit(cap_handle hQueue) {
	cap_result result = ERR_CAP_UNKNOWN;
	SCAPQueue *pstQueue = NULL;

	if (IS_VALID_HANDLE(hQueue, HANDLEID_CAP_THREAD_QUEUE) == FALSE) {
		ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
	}

	pstQueue = (SCAPQueue *) hQueue;

	result = CAPThreadLock_Lock(pstQueue->hLock);
	ERRIFGOTO(result, _EXIT);

	pstQueue->bDestroy = TRUE;

	// send a signal to default signal event
	result = CAPThreadEvent_SetEvent(pstQueue->hEvent);
	ERRIFGOTO(result, _EXIT_LOCK);

	result = ERR_CAP_NOERROR;
_EXIT_LOCK:
	CAPThreadLock_Unlock(pstQueue->hLock);
_EXIT:
	return result;
}


cap_result CAPQueue_RemoveAll(cap_handle hQueue, CbFnQueueTraverse fnCallbackDestroy, void *pUsrData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPQueue *pstQueue = NULL;
    SQueueCallback stCallback;
    int nLoop = 0;
    int nLinkedListSize = 0;

    IFVARERRASSIGNGOTO(fnCallbackDestroy, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if (IS_VALID_HANDLE(hQueue, HANDLEID_CAP_THREAD_QUEUE) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstQueue = (SCAPQueue *) hQueue;

    result = CAPThreadLock_Lock(pstQueue->hLock);
    ERRIFGOTO(result, _EXIT);

    stCallback.fnCallback = fnCallbackDestroy;
    stCallback.pUserData = pUsrData;
    result = CAPLinkedList_Traverse(pstQueue->hDataLinkedList, CAPQueueCB_DataDestroy, &stCallback);
    ERRIFGOTO(result, _EXIT_LOCK);

    result = CAPLinkedList_GetLength(pstQueue->hDataLinkedList,  &nLinkedListSize);
    ERRIFGOTO(result, _EXIT_LOCK);

    for(nLoop = 0 ; nLoop < nLinkedListSize ; nLoop++)
        //for(pstNode = pstLinkedList->pFirst; pstNode != NULL ; pstNode = pstNode->pNext)
    {
        result = CAPLinkedList_Remove(pstQueue->hDataLinkedList, LINKED_LIST_OFFSET_FIRST, 0);
        ERRIFGOTO(result, _EXIT_LOCK);
    }

    pstQueue->bDestroy = FALSE;

    result = ERR_CAP_NOERROR;
_EXIT_LOCK:
    CAPThreadLock_Unlock(pstQueue->hLock);
_EXIT:
    return result;
}


cap_result CAPQueue_Destroy(IN OUT cap_handle *phQueue, CbFnQueueTraverse fnCallbackDestroy, void *pUsrData)
{
	cap_result result = ERR_CAP_UNKNOWN;
	SCAPQueue *pstQueue = NULL;
	SQueueCallback stCallback;

	IFVARERRASSIGNGOTO(phQueue, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

	if (IS_VALID_HANDLE(*phQueue, HANDLEID_CAP_THREAD_QUEUE) == FALSE) {
		ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
	}

	pstQueue = (SCAPQueue *) *phQueue;

	pstQueue->bDestroy = TRUE;

	if (fnCallbackDestroy != NULL) {
		stCallback.fnCallback = fnCallbackDestroy;
		stCallback.pUserData = pUsrData;
		result = CAPLinkedList_Traverse(pstQueue->hDataLinkedList, CAPQueueCB_DataDestroy, &stCallback);
		// ignore error
	}

	// ignore error returns to destroy all the elements of ThreadQueue
	CAPThreadLock_Destroy(&(pstQueue->hLock));

	// CAPThreadEvent_Destroy internally send a signal for waking up the tasks
	CAPThreadEvent_Destroy(&(pstQueue->hEvent));

	CAPLinkedList_Destroy(&(pstQueue->hDataLinkedList));

	SAFEMEMFREE(pstQueue);
	*phQueue = NULL;

	result = ERR_CAP_NOERROR;
_EXIT:
	return result;
}

cap_result CAPQueue_Put(cap_handle hQueue, IN void *pData) {
	cap_result result = ERR_CAP_UNKNOWN;
	SCAPQueue *pstQueue = NULL;
	SQueueData *pstQueueData = NULL;

	if (IS_VALID_HANDLE(hQueue, HANDLEID_CAP_THREAD_QUEUE) == FALSE) {
		ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
	}

	pstQueue = (SCAPQueue *) hQueue;

	pstQueueData = (SQueueData *) malloc(sizeof(SQueueData));
	ERRMEMGOTO(pstQueueData, result, _EXIT);

	pstQueueData->pData = pData;

	result = CAPThreadLock_Lock(pstQueue->hLock);
	ERRIFGOTO(result, _EXIT);

	result = CAPLinkedList_Add(pstQueue->hDataLinkedList, LINKED_LIST_OFFSET_LAST, 0, pstQueueData);
	ERRIFGOTO(result, _EXIT_LOCK);

	// send a signal to default signal event;
	result = CAPThreadEvent_SetEvent(pstQueue->hEvent);
	ERRIFGOTO(result, _EXIT_LOCK);

	result = ERR_CAP_NOERROR;
_EXIT_LOCK:
	CAPThreadLock_Unlock(pstQueue->hLock);
_EXIT:
	if (result != ERR_CAP_NOERROR)
	{
		if (pstQueueData != NULL)
		{
			SAFEMEMFREE(pstQueueData);
		}
	}
	return result;
}

// The caller who calls ThreadQueue_Get function needs to free *ppszItemId, *ppData
cap_result CAPQueue_Get(cap_handle hQueue, IN cap_bool bBlocked, OUT void **ppData)
{
	cap_result result = ERR_CAP_UNKNOWN;
	SCAPQueue *pstQueue = NULL;
	SQueueData * pstQueueData = NULL;
	int nDataLength = 0;

	if (IS_VALID_HANDLE(hQueue, HANDLEID_CAP_THREAD_QUEUE) == FALSE) {
		ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
	}

	IFVARERRASSIGNGOTO(ppData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

	pstQueue = (SCAPQueue *) hQueue;

	result = CAPThreadLock_Lock(pstQueue->hLock);
	ERRIFGOTO(result, _EXIT);

	result = CAPLinkedList_GetLength(pstQueue->hDataLinkedList, &nDataLength);
	ERRIFGOTO(result, _EXIT_LOCK);

	while(nDataLength == 0 && bBlocked == TRUE && pstQueue->bDestroy == FALSE) {
		result = CAPThreadLock_Unlock(pstQueue->hLock);
		ERRIFGOTO(result, _EXIT_LOCK);

		result = CAPThreadEvent_WaitEvent(pstQueue->hEvent);
		ERRIFGOTO(result, _EXIT);

		result = CAPThreadLock_Lock(pstQueue->hLock);
		ERRIFGOTO(result, _EXIT);

		result = CAPLinkedList_GetLength(pstQueue->hDataLinkedList, &nDataLength);
		ERRIFGOTO(result, _EXIT_LOCK);
	}

	if (pstQueue->bDestroy == TRUE) {
		ERRASSIGNGOTO(result, ERR_CAP_SUSPEND, _EXIT_LOCK);
	}

	if (nDataLength == 0) {
	    if(bBlocked == FALSE)
	    {
	        CAPASSIGNGOTO(result, ERR_CAP_NO_DATA, _EXIT_LOCK);
	    }
	    else
	    {
	        ERRASSIGNGOTO(result, ERR_CAP_NO_DATA, _EXIT_LOCK);
	    }
	}

	// nDataLength > 0
	result = CAPLinkedList_Get(pstQueue->hDataLinkedList, LINKED_LIST_OFFSET_FIRST, 0, (void **) &pstQueueData);
	ERRIFGOTO(result, _EXIT_LOCK);

	// erase the popped data in linked list
	result = CAPLinkedList_Remove(pstQueue->hDataLinkedList, LINKED_LIST_OFFSET_FIRST, 0);
	ERRIFGOTO(result, _EXIT_LOCK);

	*ppData = pstQueueData->pData;

	result = ERR_CAP_NOERROR;
_EXIT_LOCK:
	CAPThreadLock_Unlock(pstQueue->hLock);
	SAFEMEMFREE(pstQueueData);
_EXIT:
	return result;
}

cap_result CAPQueue_GetLength(cap_handle hQueue, OUT int *pnLength)
{
	cap_result result = ERR_CAP_UNKNOWN;
	SCAPQueue *pstQueue = NULL;

	if (IS_VALID_HANDLE(hQueue, HANDLEID_CAP_THREAD_QUEUE) == FALSE)
	{
		ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
	}

	IFVARERRASSIGNGOTO(pnLength, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

	pstQueue = (SCAPQueue *) hQueue;

	result = CAPThreadLock_Lock(pstQueue->hLock);
	ERRIFGOTO(result, _EXIT);

	result = CAPLinkedList_GetLength(pstQueue->hDataLinkedList, pnLength);
	ERRIFGOTO(result, _EXIT_LOCK);

	result = ERR_CAP_NOERROR;
_EXIT_LOCK:
	CAPThreadLock_Unlock(pstQueue->hLock);
_EXIT:
	return result;
}

/*
cap_result CAPQueue_Traverse(cap_handle hQueue, CbFnQueueTraverse fnTraverse, void *pUserData)
{
	cap_result result = ERR_CAP_UNKNOWN;
	SThreadQueue *pstQueue = NULL;

	if (IS_VALID_HANDLE(hQueue, HANDLEID_CAP_THREAD_QUEUE) == FALSE)
	{
		ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
	}

	pstQueue = (SThreadQueue *) hQueue;

	result = ERR_CAP_NOERROR;
_EXIT:
	return result;
}
*/


