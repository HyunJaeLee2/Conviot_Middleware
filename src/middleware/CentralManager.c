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
#include "InfoManager.h"
#include "AppManager.h"

/*
#include "DBHandler.h"
*/

static cap_handle hEvent;
cap_bool g_bExit = FALSE;

void intHandler(int dummy) {
    CAPThreadEvent_SetEvent(hEvent);
    g_bExit = TRUE;
}

cap_result CentralManager_Create(OUT cap_handle *phCentralManager, IN SConfigData *pstConfigData)
{
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
    pstCentralManager->hAppManager = NULL;
    pstCentralManager->hThingManager = NULL;
    pstCentralManager->hInfoManager = NULL;

    // create event to keep thread running
    result = CAPThreadEvent_Create(&hEvent);
    ERRIFGOTO(result, _EXIT);

    strBrokerURI = CAPString_New();
    ERRMEMGOTO(strBrokerURI, result, _EXIT);

    result = CAPString_SetLow(strBrokerURI, pstConfigData->pszBrokerURI, CAPSTRING_MAX);
    ERRIFGOTO(result, _EXIT);

    /*
    result = DBHandler_OpenDB(pszDBFileName);
    ERRIFGOTO(result, _EXIT);
    */

    result = ThingManager_Create(&(pstCentralManager->hThingManager), strBrokerURI);
    ERRIFGOTO(result, _EXIT);
   
    result = AppManager_Create(&(pstCentralManager->hAppManager), strBrokerURI);
    ERRIFGOTO(result, _EXIT);

    result = InfoManager_Create(&(pstCentralManager->hInfoManager), strBrokerURI, pstConfigData->nSocketListeningPort);
    ERRIFGOTO(result, _EXIT);
    
    *phCentralManager = pstCentralManager;

    result = ERR_CAP_NOERROR;
    if (result != ERR_CAP_NOERROR && pstCentralManager != NULL) {
        ThingManager_Destroy(&(pstCentralManager->hThingManager));
        AppManager_Destroy(&(pstCentralManager->hAppManager));
        InfoManager_Destroy(&(pstCentralManager->hInfoManager));
        SAFEMEMFREE(pstCentralManager);
    }
_EXIT:
    return result;
}

cap_result CentralManager_Execute(IN cap_handle hCentralManager, IN SConfigData *pstConfigData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCentralManager *pstCentralManager = NULL;

    if (IS_VALID_HANDLE(hCentralManager, HANDLEID_CENTRAL_MANAGER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstCentralManager = (SCentralManager *)hCentralManager;

    result = ThingManager_Run(pstCentralManager->hThingManager, pstConfigData->nAliveCheckingPeriod);
    ERRIFGOTO(result, _WRAPUP_EXIT);
    
    result = InfoManager_Run(pstCentralManager->hInfoManager);
    ERRIFGOTO(result, _WRAPUP_EXIT);

    result = AppManager_Run(pstCentralManager->hAppManager);
    ERRIFGOTO(result, _WRAPUP_EXIT);

    // wait to be connected to broker
    CAPThreadEvent_WaitEvent(hEvent);

    result = ERR_CAP_NOERROR;

_WRAPUP_EXIT:
    ThingManager_Join(pstCentralManager->hThingManager);
    AppManager_Join(pstCentralManager->hAppManager);
    InfoManager_Join(pstCentralManager->hInfoManager);
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
    ThingManager_Destroy(&(pstCentralManager->hThingManager));
    InfoManager_Destroy(&(pstCentralManager->hInfoManager));
    AppManager_Destroy(&(pstCentralManager->hAppManager));
    CAPThreadEvent_Destroy(&hEvent);

    //DBHandler_CloseDB();

    SAFEMEMFREE(pstCentralManager);

    *phCentralManager = NULL;
    
    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}
