#ifndef CENTRALMANAGER_H_
#define CENTRALMANAGER_H_

#include "capiot_common.h"
#include "CAPString.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _SDBInfo{
    char *pszDBHost;
    char *pszDBUser;
    char *pszDBPassword;
    char *pszDBName;
    int nDBPort;
} SDBInfo;

typedef struct _SConfigData{
    char *pszBrokerURI;
	int nLogLevel;
	cap_string strLogFilePath;
	int nLogMaxSize;
	int nLogBackupNum;
    int nSocketListeningPort;
    int nAliveCheckingPeriod;
    SDBInfo *pstDBInfo; 
} SConfigData;


typedef struct _SCentralManager {
    EIoTHandleId enID;
    cap_handle hAppManager;
    cap_handle hThingManager;
    cap_handle hInfoManager;
} SCentralManager;

cap_result CentralManager_Create(OUT cap_handle *phCentralManager, IN SConfigData *pstConfigData);
cap_result CentralManager_Execute(IN cap_handle hCentralManager, IN SConfigData *pstConfigData);
cap_result CentralManager_Destroy(IN OUT cap_handle* phCentralManager);

#ifdef __cplusplus
}
#endif

#endif /* CENTRALMANAGER_H_ */
