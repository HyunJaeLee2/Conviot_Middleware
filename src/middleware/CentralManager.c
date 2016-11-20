#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <unistd.h>

#include <CAPThreadEvent.h>
#include <CAPQueue.h>

#include "CentralManager.h"

/*
#include "ThingManager.h"
#include "JobManager.h"
#include "InfoManager.h"
#include "ClientManager.h"
#include "DBHandler.h"
*/

static cap_handle hEvent;
cap_bool g_bExit = FALSE;

void intHandler(int dummy) {
    CAPThreadEvent_SetEvent(hEvent);
    g_bExit = TRUE;
}

cap_result CentralManager_Create(OUT cap_handle *phCentralManager, IN char *pszBrokerURI, IN char *pszDBFileName, IN int nSocketListeningPort){
    cap_result result = ERR_CAP_UNKNOWN;
    SCentralManager *pstCentralManager = NULL;
    IFVARERRASSIGNGOTO(phCentralManager, NULL, result, ERR_CAP_INVALID_PARAM,
            _EXIT);

    signal(SIGINT, intHandler);
	signal(SIGTERM, intHandler);

    pstCentralManager = (SCentralManager *)malloc(sizeof(SCentralManager));
    ERRMEMGOTO(pstCentralManager, result, _EXIT);

    pstCentralManager->enID = HANDLEID_CENTRAL_MANAGER;
    pstCentralManager->hClientManager = NULL;
    pstCentralManager->hJobManager = NULL;
    pstCentralManager->hThingManager = NULL;
    pstCentralManager->hInfoManager = NULL;
    pstCentralManager->hValueTopicQueue = NULL;

    /*
    // create event to keep thread running
    CAPThreadEvent_Create(&hEvent);

    result = DBHandler_OpenDB(pszDBFileName);
    ERRIFGOTO(result, _EXIT);

    result = ClientManager_Create(&(pstCentralManager->hClientManager), pszBrokerURI);
    ERRIFGOTO(result, _EXIT);

    result = ThingManager_Create(&(pstCentralManager->hThingManager), pszBrokerURI);
    ERRIFGOTO(result, _EXIT);

    result = JobManager_Create(&(pstCentralManager->hJobManager), pszBrokerURI);
    ERRIFGOTO(result, _EXIT);

    result = InfoManager_Create(&(pstCentralManager->hInfoManager), pszBrokerURI, nSocketListeningPort);
    ERRIFGOTO(result, _EXIT);
    
    result = CAPQueue_Create(&(pstCentralManager->hValueTopicQueue));
    ERRIFGOTO(result, _EXIT);

    *phCentralManager = pstCentralManager;

    result = ERR_CAP_NOERROR;
    if (result != ERR_CAP_NOERROR && pstCentralManager != NULL) {
        CAPQueue_Destroy(&(pstCentralManager->hValueTopicQueue), NULL, NULL);
        JobManager_Destroy(&(pstCentralManager->hJobManager));
        ThingManager_Destroy(&(pstCentralManager->hThingManager));
        InfoManager_Destroy(&(pstCentralManager->hInfoManager));
        ClientManager_Destroy(&(pstCentralManager->hClientManager));
        SAFEMEMFREE(pstCentralManager);
    }
    */
_EXIT:
    return result;
}

cap_result CentralManager_Execute(IN cap_handle hCentralManager, IN char *pszBrokerURI) {
    cap_result result = ERR_CAP_UNKNOWN;
    SCentralManager *pstMgr = NULL;

    if (IS_VALID_HANDLE(hCentralManager, HANDLEID_CENTRAL_MANAGER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstMgr = (SCentralManager *)hCentralManager;

    /*
    result = ThingManager_Run(pstMgr->hThingManager, pstMgr->hValueTopicQueue);
    ERRIFGOTO(result, _WRAPUP_EXIT);

    result = InfoManager_Run(pstMgr->hInfoManager, pstMgr->hValueTopicQueue);
    ERRIFGOTO(result, _WRAPUP_EXIT);
    
    result = JobManager_Run(pstMgr->hJobManager);
    ERRIFGOTO(result, _WRAPUP_EXIT);

    result = ClientManager_Run(pstMgr->hClientManager);
    ERRIFGOTO(result, _WRAPUP_EXIT);

    result = DBHandler_InitJobHandler(pstMgr->hJobManager, pszBrokerURI);
    ERRIFGOTO(result, _EXIT);

    // wait to be connected to broker
    CAPThreadEvent_WaitEvent(hEvent);

    result = ERR_CAP_NOERROR;

_WRAPUP_EXIT:
    CAPQueue_SetExit(pstMgr->hValueTopicQueue);
    // ignore error to reserve the result value
    ClientManager_Join(pstMgr->hClientManager);
    JobManager_Join(pstMgr->hJobManager);
    ThingManager_Join(pstMgr->hThingManager);
    InfoManager_Join(pstMgr->hInfoManager);
*/
_EXIT:
    return result;
}

cap_result CentralManager_Destroy(IN OUT cap_handle *phCentralManager) {
    cap_result result = ERR_CAP_UNKNOWN;
    SCentralManager *pstCentralManager = NULL;

    IFVARERRASSIGNGOTO(phCentralManager, NULL, result, ERR_CAP_INVALID_PARAM,
            _EXIT);

    if (IS_VALID_HANDLE(*phCentralManager, HANDLEID_CENTRAL_MANAGER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstCentralManager = (SCentralManager *)*phCentralManager;
/*
    JobManager_Destroy(&(pstCentralManager->hJobManager));
    ThingManager_Destroy(&(pstCentralManager->hThingManager));
    InfoManager_Destroy(&(pstCentralManager->hInfoManager));
    ClientManager_Destroy(&(pstCentralManager->hClientManager));

    CAPQueue_Destroy(&(pstCentralManager->hValueTopicQueue), NULL, NULL);
    
    CAPThreadEvent_Destroy(&hEvent);

    DBHandler_CloseDB();

    SAFEMEMFREE(pstCentralManager);

    *phCentralManager = NULL;
*/
    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}
