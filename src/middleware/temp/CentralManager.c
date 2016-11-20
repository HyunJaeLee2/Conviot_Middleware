
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

#include "ThingManager.h"
#include "AppManager.h"
#include "InfoManager.h"
#include "ClientManager.h"
#include "DBHandler.h"

static cap_handle hEvent;
cap_bool g_bExit = FALSE;

void intHandler(int dummy) {
    CAPThreadEvent_SetEvent(hEvent);
    g_bExit = TRUE;
}

cap_result CentralManager_Create(OUT cap_handle *phCentralManager, IN SConfigData *pstConfigData){
    cap_result result = ERR_CAP_UNKNOWN;
    SCentralManager *pstCentralManager = NULL;
    cap_string strBrokerURI = NULL;

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
    pstCentralManager->hMessageToCloudQueue = NULL;

    // create event to keep thread running
    result = CAPThreadEvent_Create(&hEvent);
    ERRIFGOTO(result, _EXIT);

    strBrokerURI = CAPString_New();
    ERRMEMGOTO(strBrokerURI, result, _EXIT);

    result = CAPString_SetLow(strBrokerURI, pstConfigData->pszBrokerURI, CAPSTRING_MAX);
    ERRIFGOTO(result, _EXIT);

    result = DBHandler_OpenDB(pstConfigData->pszMainDBFilePath, pstConfigData->pszValueLogDBFilePath);
    ERRIFGOTO(result, _EXIT);

    result = AppManager_Create(&(pstCentralManager->hJobManager), strBrokerURI);
    ERRIFGOTO(result, _EXIT);

    result = ThingManager_Create(&(pstCentralManager->hThingManager), strBrokerURI);
    ERRIFGOTO(result, _EXIT);

    result = InfoManager_Create(&(pstCentralManager->hInfoManager), strBrokerURI, pstConfigData->nSocketListeningPort);
    ERRIFGOTO(result, _EXIT);
    
    result = ClientManager_Create(&(pstCentralManager->hClientManager), pstConfigData->pszBrokerURI);
    ERRIFGOTO(result, _EXIT);
    
    result = CAPQueue_Create(&(pstCentralManager->hValueTopicQueue));
    ERRIFGOTO(result, _EXIT);
    
    result = CAPQueue_Create(&(pstCentralManager->hMessageToCloudQueue));
    ERRIFGOTO(result, _EXIT);

    *phCentralManager = pstCentralManager;

    result = ERR_CAP_NOERROR;
_EXIT:
    if (result != ERR_CAP_NOERROR && pstCentralManager != NULL) {
        CAPQueue_Destroy(&(pstCentralManager->hValueTopicQueue), NULL, NULL);
        CAPQueue_Destroy(&(pstCentralManager->hMessageToCloudQueue), NULL, NULL);
        AppManager_Destroy(&(pstCentralManager->hJobManager));
        ThingManager_Destroy(&(pstCentralManager->hThingManager));
        InfoManager_Destroy(&(pstCentralManager->hInfoManager));
        ClientManager_Destroy(&(pstCentralManager->hClientManager));
        SAFEMEMFREE(pstCentralManager);
    }
    SAFE_CAPSTRING_DELETE(strBrokerURI);
    return result;
}

cap_result CentralManager_Execute(IN cap_handle hCentralManager, IN SConfigData *pstConfigData){
    cap_result result = ERR_CAP_UNKNOWN;
    SCentralManager *pstMgr = NULL;
    cap_handle hAppEngine = NULL;

    if (IS_VALID_HANDLE(hCentralManager, HANDLEID_CENTRAL_MANAGER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstMgr = (SCentralManager *)hCentralManager;

    //hAppEngine is retrieved then passed to ThingManager.
    //It is used when ThingManager tries to run or stop scenario.
    result = AppManager_Run(pstMgr->hJobManager, &hAppEngine);
    ERRIFGOTO(result, _WRAPUP_EXIT);

    result = ThingManager_Run(pstMgr->hThingManager, pstMgr->hValueTopicQueue, pstMgr->hMessageToCloudQueue,\
            pstConfigData->nAliveCheckingPeriod, hAppEngine);
    ERRIFGOTO(result, _WRAPUP_EXIT);

    result = InfoManager_Run(pstMgr->hInfoManager, pstMgr->hValueTopicQueue, pstMgr->hMessageToCloudQueue, hAppEngine);
    ERRIFGOTO(result, _WRAPUP_EXIT);
    
    result = ClientManager_Run(pstMgr->hClientManager);
    ERRIFGOTO(result, _WRAPUP_EXIT);

    // wait to be connected to broker
    CAPThreadEvent_WaitEvent(hEvent);

    result = ERR_CAP_NOERROR;

_WRAPUP_EXIT:
    CAPQueue_SetExit(pstMgr->hValueTopicQueue);
    CAPQueue_SetExit(pstMgr->hMessageToCloudQueue);
    // ignore error to reserve the result value
    ClientManager_Join(pstMgr->hClientManager);
    InfoManager_Join(pstMgr->hInfoManager);
    ThingManager_Join(pstMgr->hThingManager);
    AppManager_Join(pstMgr->hJobManager);

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

    ClientManager_Destroy(&(pstCentralManager->hClientManager));
    InfoManager_Destroy(&(pstCentralManager->hInfoManager));
    ThingManager_Destroy(&(pstCentralManager->hThingManager));
    AppManager_Destroy(&(pstCentralManager->hJobManager));

    CAPQueue_Destroy(&(pstCentralManager->hValueTopicQueue), NULL, NULL);
    CAPQueue_Destroy(&(pstCentralManager->hMessageToCloudQueue), NULL, NULL);
    
    CAPThreadEvent_Destroy(&hEvent);

    DBHandler_CloseDB();

    SAFEMEMFREE(pstCentralManager);

    *phCentralManager = NULL;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}
