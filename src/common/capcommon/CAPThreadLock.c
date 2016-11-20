/*
 * CAPThreadLock.c
 *
 *  Created on: 2015. 5. 21.
 *      Author: chjej202
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_PTHREAD
#include <pthread.h>
#else
	error
#endif

#include <CAPThreadLock.h>

typedef struct _SThreadLock {
	EHandleId enId;
	cap_bool bLocked;
#ifdef HAVE_PTHREAD
	pthread_mutex_t hSystemLock;
#else
	//error
#endif
} SThreadLock;

cap_result CAPThreadLock_Create(OUT cap_handle *phLock)
{
	cap_result result = ERR_CAP_UNKNOWN;
	SThreadLock *pstLock = NULL;

	IFVARERRASSIGNGOTO(phLock, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

	pstLock = (SThreadLock *) malloc(sizeof(SThreadLock));
	ERRMEMGOTO(pstLock, result, _EXIT);

	pstLock->enId = HANDLEID_CAP_THREAD_LOCK;
	pstLock->bLocked = FALSE;
#ifdef HAVE_PTHREAD
	if(pthread_mutex_init(&(pstLock->hSystemLock), NULL) != 0)
	{
		ERRASSIGNGOTO(result, ERR_CAP_MUTEX_ERROR, _EXIT);
	}
#else
	//error
#endif
	*phLock = (cap_handle) pstLock;

	result = ERR_CAP_NOERROR;
_EXIT:
	if(result != ERR_CAP_NOERROR)
	{
		SAFEMEMFREE(pstLock);
	}
	return result;
}


cap_result CAPThreadLock_Lock(cap_handle hLock)
{
	cap_result result = ERR_CAP_UNKNOWN;
	SThreadLock *pstLock = NULL;

	if(IS_VALID_HANDLE(hLock, HANDLEID_CAP_THREAD_LOCK) == FALSE)
	{
		ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
	}

	pstLock = (SThreadLock *) hLock;

#ifdef HAVE_PTHREAD
	if(pthread_mutex_lock(&(pstLock->hSystemLock)) != 0)
	{
		ERRASSIGNGOTO(result, ERR_CAP_MUTEX_ERROR, _EXIT);
	}
#else
	//error
#endif

	pstLock->bLocked = TRUE;

	result = ERR_CAP_NOERROR;
_EXIT:
	return result;
}


cap_result CAPThreadLock_Unlock(cap_handle hLock)
{
	cap_result result = ERR_CAP_UNKNOWN;
	SThreadLock *pstLock = NULL;

	if(IS_VALID_HANDLE(hLock, HANDLEID_CAP_THREAD_LOCK) == FALSE)
	{
		ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
	}

	pstLock = (SThreadLock *) hLock;

	if(pstLock->bLocked == FALSE)
	{
		ERRASSIGNGOTO(result, ERR_CAP_MUTEX_ERROR, _EXIT);
	}

	// Change the lock state before unlock
	pstLock->bLocked = FALSE;
#ifdef HAVE_PTHREAD
	if(pthread_mutex_unlock(&(pstLock->hSystemLock)) != 0)
	{
		pstLock->bLocked = TRUE; // restore the lock state because of unlock fail
		ERRASSIGNGOTO(result, ERR_CAP_MUTEX_ERROR, _EXIT);
	}
#else
	//error
#endif


	result = ERR_CAP_NOERROR;
_EXIT:
	return result;
}


cap_result CAPThreadLock_Destroy(IN OUT cap_handle *phLock)
{
	cap_result result = ERR_CAP_UNKNOWN;
	SThreadLock *pstLock = NULL;

	IFVARERRASSIGNGOTO(phLock, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

	if(IS_VALID_HANDLE(*phLock, HANDLEID_CAP_THREAD_LOCK) == FALSE)
	{
		ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
	}

	pstLock = (SThreadLock *) *phLock;
#ifdef HAVE_PTHREAD
	if(pstLock->bLocked == TRUE) // if the mutex is locked, unlock the mutex
	{
		// ignore error
		pthread_mutex_unlock(&(pstLock->hSystemLock));
	}

	if(pthread_mutex_destroy(&(pstLock->hSystemLock)) != 0)
	{
		ERRASSIGNGOTO(result, ERR_CAP_MUTEX_ERROR, _EXIT);
	}
#else
	//error
#endif
	*phLock = NULL;

	result = ERR_CAP_NOERROR;
_EXIT:
	// free is performed here
	SAFEMEMFREE(pstLock);
	return result;
}

