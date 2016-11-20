/*
 * CAPThread.c
 *
 *  Created on: 2015. 5. 21.
 *      Author: chjej202
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sched.h>

#ifdef HAVE_PTHREAD
#include <pthread.h>
#else
#endif

#include <CAPThread.h>

typedef struct _SCAPThread {
	EHandleId enId;
#ifdef HAVE_PTHREAD
	pthread_t hNativeThread;
#else
	error
#endif
} SCAPThread;

cap_result CAPThread_Create(IN FnNativeThread fnThreadRoutine, IN void *pUserData, OUT cap_handle *phThread) {
	cap_result result = ERR_CAP_UNKNOWN;
	SCAPThread *pstThread = NULL;
#ifdef HAVE_PTHREAD
	pthread_attr_t threadAttr;

	pthread_attr_init(&threadAttr);
#else
#endif

	IFVARERRASSIGNGOTO(phThread, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
	IFVARERRASSIGNGOTO(fnThreadRoutine, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

	// pUserData is an optional argument
	pstThread = (SCAPThread *) malloc(sizeof(SCAPThread));
	ERRMEMGOTO(pstThread, result, _EXIT);

	pstThread->enId = HANDLEID_CAP_THREAD;

#ifdef HAVE_PTHREAD
	if(pthread_create(&(pstThread->hNativeThread), &threadAttr, fnThreadRoutine, (void *)pUserData) != 0) {
		ERRASSIGNGOTO(result, ERR_CAP_INTERNAL_FAIL, _EXIT);
	}
#else
#endif

	*phThread = (cap_handle) pstThread;

	result = ERR_CAP_NOERROR;
_EXIT:
	if(result != ERR_CAP_NOERROR) {
		SAFEMEMFREE(pstThread);
	}
#ifdef HAVE_PTHREAD
	pthread_attr_destroy( &threadAttr );
#else
#endif
	return result;
}

cap_result CAPThread_Destroy(IN OUT cap_handle *phThread) {
	cap_result result = ERR_CAP_UNKNOWN;
	SCAPThread *pstThread = NULL;

	IFVARERRASSIGNGOTO(phThread, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

	if(IS_VALID_HANDLE(*phThread, HANDLEID_CAP_THREAD) == FALSE) {
		ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
	}

	pstThread = (SCAPThread *) *phThread;

#ifdef HAVE_PTHREAD
	if(pthread_join(pstThread->hNativeThread, NULL) != 0) {
		// free the memory even though pthread_join is failed.
		ERRASSIGNGOTO(result, ERR_CAP_INTERNAL_FAIL, _EXIT);
	}
#else
#endif

	*phThread = NULL;

	result = ERR_CAP_NOERROR;
_EXIT:
	SAFEMEMFREE(pstThread);
	return result;
}

cap_tid CAPThread_GetCurThreadID() {
#ifdef HAVE_PTHREAD
	return pthread_self();
#else
    return -1;
#endif
}

cap_result CAPThread_Yield(){
	cap_result result = ERR_CAP_UNKNOWN;
    int ret = 0;
    ret = sched_yield();
    if(ret != 0){
        result = ERR_CAP_INTERNAL_FAIL;
    } else{
        result = ERR_CAP_NOERROR;
    }
    return result;
}



