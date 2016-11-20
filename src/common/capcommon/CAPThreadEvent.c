/*
 * CAPThreadEvent.c
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
	//error
#endif

#include <CAPThreadEvent.h>

typedef struct _SThreadEvent {
	EHandleId enId;
	cap_bool bIsSet;
    int nTimeOut;
#ifdef HAVE_PTHREAD
	pthread_cond_t hCond;
	pthread_mutex_t hMutex;
#else
	//error
#endif
} SThreadEvent;

cap_result CAPThreadEvent_Create(OUT cap_handle *phEvent) {
	cap_result result = ERR_CAP_UNKNOWN;
	SThreadEvent *pstEvent = NULL;

	IFVARERRASSIGNGOTO(phEvent, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

	pstEvent = (SThreadEvent *) malloc(sizeof(SThreadEvent));
	ERRMEMGOTO(pstEvent, result, _EXIT);

	pstEvent->enId = HANDLEID_CAP_THREAD_EVENT;
	pstEvent->bIsSet = FALSE;
	pstEvent->nTimeOut = 0;
#ifdef HAVE_PTHREAD
	if (pthread_mutex_init(&(pstEvent->hMutex), NULL) != 0) {
		ERRASSIGNGOTO(result, ERR_CAP_INTERNAL_FAIL, _EXIT);
	}

	if (pthread_cond_init(&(pstEvent->hCond), NULL) != 0) {
		ERRASSIGNGOTO(result, ERR_CAP_INTERNAL_FAIL, _EXIT);
	}
#else
	//error
#endif

	*phEvent = (cap_handle) pstEvent;

	result = ERR_CAP_NOERROR;
_EXIT:
	if (result != ERR_CAP_NOERROR) {
		SAFEMEMFREE(pstEvent);
	}
	return result;
}

cap_result ThreadEvent_SetEventTimeOut(cap_handle hEvent) {
    cap_result result = ERR_CAP_UNKNOWN;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result CAPThreadEvent_SetEvent(cap_handle hEvent) {
	cap_result result = ERR_CAP_UNKNOWN;
	SThreadEvent *pstEvent = NULL;
	cap_bool bMutexFailed = FALSE;

	if (IS_VALID_HANDLE(hEvent, HANDLEID_CAP_THREAD_EVENT) == FALSE) {
		ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
	}

	pstEvent = (SThreadEvent *) hEvent;
#ifdef HAVE_PTHREAD
	if (pthread_mutex_lock(&(pstEvent->hMutex)) != 0) {
		bMutexFailed = TRUE;
	}

	pstEvent->bIsSet = TRUE; // event is set
	// send a signal
	pthread_cond_signal(&(pstEvent->hCond));

	if (bMutexFailed == FALSE && pthread_mutex_unlock(&(pstEvent->hMutex)) != 0) {
		// ignore error
	}
#else
	//error
#endif

	result = ERR_CAP_NOERROR;
_EXIT:
	return result;
}

cap_result CAPThreadEvent_WaitTimeEvent(cap_handle hEvent, long long llSleepTimeMs){
	cap_result result = ERR_CAP_UNKNOWN;
	SThreadEvent *pstEvent = NULL;
	struct timespec sTimeOut;
	struct timespec sNow;
	int ret = 0;

	if (IS_VALID_HANDLE(hEvent, HANDLEID_CAP_THREAD_EVENT) == FALSE) {
		if(hEvent != NULL)
		{
			pstEvent = (SThreadEvent *) hEvent;
		}
		ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
	}

	pstEvent = (SThreadEvent *) hEvent;

	//gettimeofday(&sNow, NULL);
	clock_gettime(CLOCK_REALTIME, &sNow);

	sTimeOut.tv_sec = sNow.tv_sec + llSleepTimeMs/1000;
	sTimeOut.tv_nsec = sNow.tv_nsec + (llSleepTimeMs % 1000)*1000000;

#ifdef HAVE_PTHREAD
	if (pthread_mutex_lock(&(pstEvent->hMutex)) != 0) {
		ERRASSIGNGOTO(result, ERR_CAP_MUTEX_ERROR, _EXIT);
	}

	ret = pthread_cond_timedwait(&(pstEvent->hCond), &(pstEvent->hMutex), &sTimeOut);
	
	if(ret == 0)
	{
	    // do nothing
	}
	else if(ret == ETIMEDOUT)
	{
	    CAPASSIGNGOTO(result, ERR_CAP_TIME_EXPIRED, _EXIT_LOCK);
	}
	else
	{
		ERRASSIGNGOTO(result, ERR_CAP_MUTEX_ERROR, _EXIT_LOCK);
	}

	pstEvent->bIsSet = FALSE;

	result = ERR_CAP_NOERROR;
_EXIT_LOCK:
	if (pthread_mutex_unlock(&(pstEvent->hMutex)) != 0) {
		ERRASSIGNGOTO(result, ERR_CAP_MUTEX_ERROR, _EXIT);
	}
#else
	//error
#endif
_EXIT:
	return result;
}

cap_result CAPThreadEvent_WaitEvent(cap_handle hEvent) {
	cap_result result = ERR_CAP_UNKNOWN;
	SThreadEvent *pstEvent = NULL;

	if (IS_VALID_HANDLE(hEvent, HANDLEID_CAP_THREAD_EVENT) == FALSE) {
		ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
	}

	pstEvent = (SThreadEvent *) hEvent;
#ifdef HAVE_PTHREAD
	if (pthread_mutex_lock(&(pstEvent->hMutex)) != 0) {
		ERRASSIGNGOTO(result, ERR_CAP_MUTEX_ERROR, _EXIT);
	}

	while(pstEvent->bIsSet == FALSE) {
		if (pthread_cond_wait(&(pstEvent->hCond), &(pstEvent->hMutex)) != 0) {
			ERRASSIGNGOTO(result, ERR_CAP_MUTEX_ERROR, _EXIT_LOCK);
		}
	}

	pstEvent->bIsSet = FALSE;
_EXIT_LOCK:
	if (pthread_mutex_unlock(&(pstEvent->hMutex)) != 0) {
		ERRASSIGNGOTO(result, ERR_CAP_MUTEX_ERROR, _EXIT);
	}
#else
	//error
#endif
	result = ERR_CAP_NOERROR;
_EXIT:
	return result;
}


cap_result CAPThreadEvent_Destroy(IN OUT cap_handle *phEvent) {
	cap_result result = ERR_CAP_UNKNOWN;
	SThreadEvent *pstEvent = NULL;

	IFVARERRASSIGNGOTO(phEvent, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

	if (IS_VALID_HANDLE(*phEvent, HANDLEID_CAP_THREAD_EVENT) == FALSE) {
		ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
	}

	pstEvent = (SThreadEvent *) *phEvent;

	// send an event to release waiting tasks (ignore error)
	CAPThreadEvent_SetEvent(*phEvent);

#ifdef HAVE_PTHREAD
	// ignore error
	if (pthread_cond_destroy(&(pstEvent->hCond)) != 0) {
		ERRASSIGNGOTO(result, ERR_CAP_MUTEX_ERROR, _EXIT);
	}

	// ignore error
	if (pthread_mutex_destroy(&(pstEvent->hMutex)) != 0) {
		ERRASSIGNGOTO(result, ERR_CAP_MUTEX_ERROR, _EXIT);
	}
#else
	//error
#endif
	*phEvent = NULL;

	result = ERR_CAP_NOERROR;
_EXIT:
	SAFEMEMFREE(pstEvent);
	return result;
}

