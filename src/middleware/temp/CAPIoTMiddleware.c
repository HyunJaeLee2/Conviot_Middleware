/*
 ============================================================================
 Name        : CAPIoTMiddleware.c
 Author      : 
 Version     :
 Copyright   : 
 Description : 
 ============================================================================
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define BROKER_URI "broker_uri"
#define MAIN_DB_FILE_PATH "main_db_file_path"
#define VALUE_LOG_DB_FILE_PATH "value_log_db_file_path"
#define SOCKET_LISTENING_PORT "socket_listening_port"
#define LOG_LEVEL "log_level"
#define LOG_FILE_PATH "log_file_path"
#define LOG_MAX_SIZE "log_max_size"
#define LOG_BACKUP_NUM "log_backup_num"
#define ALIVE_CHECKING_PERIOD "alive_checking_period"
#define MB 1024*1024

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <libconfig.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <libgen.h>   //in order to use dirname()
#include "CentralManager.h"
#include "CAPLogger.h"
#include "cap_common.h"

cap_handle g_hLogger = NULL;

static cap_result getConfigData(SConfigData *pstConfigData, char *pszConfigPath)
{
	cap_result result = ERR_CAP_UNKNOWN;
    const char *pszBrokerURI = NULL;
    const char *pszMainDBFilePath = NULL;
    const char *pszValueLogDBFilePath = NULL;
    int nSocketListeningPort; 
	int nLogLevel;
	const char *pszLogFilePath = NULL;
	int nLogMaxSize;
	int nLogBackupNum;
    int nAliveCheckingPeriod = 0;

	config_t cfg, *cf;
	char *pszMainDBPath = NULL;
	char *pszValueLogDBPath = NULL;
	char *pszLogPath = NULL;
	cf = &cfg;
    config_init(cf);

	char *pszConfigDirPath = NULL;

    if (!config_read_file(cf, pszConfigPath)) {
        fprintf(stderr, "%s:%d - %s %s\n",
            config_error_file(cf),
            config_error_line(cf),
            config_error_text(cf), pszConfigPath);
        config_destroy(cf);
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }

    if (!config_lookup_string(cf, BROKER_URI, &pszBrokerURI)){
        fprintf(stderr, "broker_uri error\n");
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }

    if (!config_lookup_string(cf, MAIN_DB_FILE_PATH, &pszMainDBFilePath)){
        fprintf(stderr, "main_db_file_path error\n");
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }
	
    if (!config_lookup_string(cf, VALUE_LOG_DB_FILE_PATH, &pszValueLogDBFilePath)){
        fprintf(stderr, "value_log_db_file_path error\n");
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }
	
    if (!config_lookup_int(cf, SOCKET_LISTENING_PORT, &nSocketListeningPort)){
        fprintf(stderr, "socket_listening_port error\n");
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }

	if (!config_lookup_int(cf, LOG_LEVEL, &nLogLevel)){
		fprintf(stderr, "log_level error\n");
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
	}

	if (!config_lookup_string(cf, LOG_FILE_PATH, &pszLogFilePath)){
		fprintf(stderr, "log_file_path error\n");
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
	}

	if (!config_lookup_int(cf, LOG_MAX_SIZE, &nLogMaxSize)){
		fprintf(stderr, "lof_max_size error\n");
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
	}

	if (!config_lookup_int(cf, LOG_BACKUP_NUM, &nLogBackupNum)){
		fprintf(stderr, "log_backup_num error\n");
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
	}
	
    if (!config_lookup_int(cf, ALIVE_CHECKING_PERIOD, &nAliveCheckingPeriod)){
		fprintf(stderr, "AliveCheckingPeriod error\n");
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
	}

	pszConfigDirPath = (char *)malloc(PATH_MAX);
	ERRMEMGOTO(pszConfigDirPath, result, _EXIT);

	memcpy(pszConfigDirPath, pszConfigPath, strlen(pszConfigPath)+1);
	dirname(pszConfigDirPath);

	pszMainDBPath = (char *)malloc(PATH_MAX);
	ERRMEMGOTO(pszMainDBPath, result, _EXIT);

	pszValueLogDBPath = (char *)malloc(PATH_MAX);
	ERRMEMGOTO(pszValueLogDBPath, result, _EXIT);
	
    //make the full path of Main DB file
	if((pszMainDBFilePath[0] != '/') && (pszMainDBFilePath[0] != '.') && (pszMainDBFilePath[0] != '~')){
		memcpy(pszMainDBPath, pszConfigDirPath, strlen(pszConfigDirPath)+1);
		strncat(pszMainDBPath, "/", 1);
		strncat(pszMainDBPath, pszMainDBFilePath, strlen(pszMainDBFilePath)+1);
	}
	else{
		realpath(pszMainDBFilePath, pszMainDBPath);
	}

	//make the full path of value log DB file
	if((pszValueLogDBFilePath[0] != '/') && (pszValueLogDBFilePath[0] != '.') && (pszValueLogDBFilePath[0] != '~')){
		memcpy(pszValueLogDBPath, pszConfigDirPath, strlen(pszConfigDirPath)+1);
		strncat(pszValueLogDBPath, "/", 1);
		strncat(pszValueLogDBPath, pszValueLogDBFilePath, strlen(pszValueLogDBFilePath)+1);
	}
	else{
		realpath(pszValueLogDBFilePath, pszValueLogDBPath);
	}

	pszLogPath = (char *)malloc(PATH_MAX);
	ERRMEMGOTO(pszLogPath, result, _EXIT);

	//make the full path of log file
	if((pszLogFilePath[0] != '/') && (pszLogFilePath[0] != '.') && (pszLogFilePath[0] != '~')){
		memcpy(pszLogPath, pszConfigDirPath, strlen(pszConfigDirPath)+1);
		strncat(pszLogPath, "/", 1);
		strncat(pszLogPath,pszLogFilePath, strlen(pszLogFilePath)+1);
	}
	else{
		realpath(pszLogFilePath, pszLogPath);
	}

	pstConfigData->pszBrokerURI = strdup(pszBrokerURI);
	pstConfigData->pszMainDBFilePath = strdup(pszMainDBPath);
	pstConfigData->pszValueLogDBFilePath = strdup(pszValueLogDBPath);
    pstConfigData->nAliveCheckingPeriod = nAliveCheckingPeriod;
	pstConfigData->nSocketListeningPort = nSocketListeningPort;
	pstConfigData->nLogLevel = nLogLevel;

	pstConfigData->strLogFilePath = CAPString_New();
	ERRMEMGOTO(pstConfigData->strLogFilePath, result, _EXIT);

	result = CAPString_SetLow(pstConfigData->strLogFilePath, pszLogPath,strlen(pszLogPath)+1);
	ERRIFGOTO(result, _EXIT);

	pstConfigData->nLogMaxSize = nLogMaxSize*MB;
	pstConfigData->nLogBackupNum = nLogBackupNum;
	
	SAFEMEMFREE(pszConfigDirPath);
	SAFEMEMFREE(pszMainDBPath);
	SAFEMEMFREE(pszValueLogDBPath);
	SAFEMEMFREE(pszLogPath);

	config_destroy(cf);

	result = ERR_CAP_NOERROR;
_EXIT:
	return result;
}

cap_result getExePath(char* pszArgv0, char* pszRealPath, int pszRealPathSize)
{
    cap_result result = ERR_CAP_UNKNOWN;
	char *pszEnvPath = NULL;
	char *pszToken = NULL;
	char *ptr = NULL;
	char *pszSymLinkPath = NULL;

	if((pszArgv0[0] != '/') && (pszArgv0[0] != '.') && (pszArgv0[0] != '~')){	//if the program is executed by symbolic link on /usr/local/bin
		pszEnvPath = getenv("PATH");	//every path of environment variable /usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games
		pszToken = strtok_r(pszEnvPath, ":", &ptr); 
		
		pszSymLinkPath = (char*)malloc(PATH_MAX);
		ERRMEMGOTO(pszSymLinkPath, result, _EXIT);
		
		while(pszToken != NULL){

			memcpy(pszSymLinkPath, pszToken, strlen(pszToken)+1);
			strncat(pszSymLinkPath, "/cap_iot_middleware", strlen("cap_iot_middleware")+1);

			if( access(pszSymLinkPath,F_OK) == 0 ){		//it will be ok when pszSymLinkPath is /usr/local/bin/cap_iot_middleware
				realpath(pszSymLinkPath, pszRealPath);	//get real path of symbolic link
				break;
			}

			pszToken = strtok_r(NULL, ":", &ptr);
		}
		SAFEMEMFREE(pszSymLinkPath);
	}
	else {		//if the program is executed with absolute path or relative path
		realpath(pszArgv0, pszRealPath);
	}
	
	result = ERR_CAP_NOERROR;
_EXIT:
	return result;
}

cap_result getConfigPath(char *pszExeFilePath, char *config_file, char *pszConfigPath, int pszConfigPathSize)
{
    cap_result result = ERR_CAP_UNKNOWN;
	char *pszExeDirPath = NULL;

	if((config_file[0] != '/') && (config_file[0] != '.') && (config_file[0] != '~')){	//if the configuration file hasn't a path, the file should be in a directory of executable file
		pszExeDirPath = (char *)malloc(strlen(pszExeFilePath)+1);
		ERRMEMGOTO(pszExeDirPath, result, _EXIT);

		memcpy(pszExeDirPath, pszExeFilePath, strlen(pszExeFilePath)+1);	// /usr/local/capital/middleware/cap_iot_middleware
		dirname(pszExeDirPath);		// /usr/local/capital/middleware
		memcpy(pszConfigPath, pszExeDirPath, strlen(pszExeDirPath)+1);
		strncat(pszConfigPath, "/",1);	// /usr/local/capital/middleare/
		strncat(pszConfigPath, config_file, strlen(config_file)+1);	// /usr/local/capital/middleware/[the configuration file name]

		SAFEMEMFREE(pszExeDirPath);
	}
	else{
		realpath(config_file,pszConfigPath);
	}

	result = ERR_CAP_NOERROR;
_EXIT:
	return result;
}

int main(int argc, char* argv[])
{
    cap_handle hCentralManager = NULL;
    SConfigData *pstConfigData = NULL;
    cap_result result = ERR_CAP_UNKNOWN;
	char* pszConfigPath = NULL;
	char* pszExeRealPath = NULL;	
	char* config_file = NULL;
	cap_bool d_opt = FALSE;
	cap_bool f_opt = FALSE;
	int opt;
	int optnum = 0;
	cap_string strLogPrefix = NULL;

	opterr = 0;
	while((opt = getopt(argc, argv, "df:")) != -1) 
	{
		optnum++;
		switch(opt) 
		{ 
			case 'd':
				d_opt = TRUE;
				break; 
			case 'f':
				f_opt = TRUE;
				config_file = optarg;
				optnum++;
				break;
			case '?':		//in the case of unknown options
				printf("Usage: cap_iot_middleware [-d] [-f configure_file_path]\n");
				return -1;
		} 
	}
	if( (argc > 1) && (optnum == 0)){	//with argvs without '-'  ex)./cap_iot_middleware argv
		printf("Usage: cap_iot_middleware [-d] [-f configure_file]\n");
		return -1;
	}

	if( f_opt == FALSE ){
		config_file = "middleware_config.cfg";
	}

	signal(SIGINT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);

	pszExeRealPath = (char *)malloc(PATH_MAX);
	ERRMEMGOTO(pszExeRealPath, result, _EXIT);
	
	pszConfigPath = (char *)malloc(PATH_MAX);
	ERRMEMGOTO(pszConfigPath, result, _EXIT);
	
	result = getExePath(argv[0], pszExeRealPath, PATH_MAX);	//get a full path of the executable file
	ERRIFGOTO(result, _EXIT);

	result = getConfigPath(pszExeRealPath, config_file, pszConfigPath, PATH_MAX);	//get a full path of the configuration file
	ERRIFGOTO(result, _EXIT);

    pstConfigData = (SConfigData*)malloc(sizeof(SConfigData));
	ERRMEMGOTO(pstConfigData, result, _EXIT);

    result = getConfigData(pstConfigData, pszConfigPath);
    ERRIFGOTO(result, _EXIT);
    
	SAFEMEMFREE(pszExeRealPath);
	SAFEMEMFREE(pszConfigPath);
 //   printf("getConfigData result : %x\n", result);


	print_id("test");
	/*****************************************************/
	if(d_opt == TRUE){
		if(daemon(0,0) == -1){	// demonize a program and is included in unistd.h
			return -1;
		}
	}
	/*****************************************************/
	strLogPrefix = CAPString_New();
	ERRMEMGOTO(strLogPrefix, result, _EXIT);

	result = CAPString_SetLow(strLogPrefix, "cap_iot_middleware", CAPSTRING_MAX);
	ERRIFGOTO(result, _EXIT);
	
	result = CAPLogger_Create(pstConfigData->strLogFilePath, strLogPrefix, pstConfigData->nLogLevel, pstConfigData->nLogMaxSize, pstConfigData->nLogBackupNum, &g_hLogger);
	if( result == ERR_CAP_OPEN_FAIL ){
		printf("Can not open a log file. %s \n", CAPString_LowPtr(pstConfigData->strLogFilePath, NULL));
	}
	ERRIFGOTO(result, _EXIT);
		
	result = CAPLogger_Write(g_hLogger, MSG_INFO, "cap_iot_middleware start");
	ERRIFGOTO(result, _EXIT);
    
	result = CentralManager_Create(&hCentralManager, pstConfigData);
    ERRIFGOTO(result, _EXIT);

    result = CentralManager_Execute(hCentralManager, pstConfigData);
    ERRIFGOTO(result, _EXIT);

	result = CentralManager_Destroy(&hCentralManager);
    ERRIFGOTO(result, _EXIT);

    SAFEMEMFREE(pstConfigData->pszBrokerURI);
    SAFEMEMFREE(pstConfigData->pszMainDBFilePath);
    SAFEMEMFREE(pstConfigData->pszValueLogDBFilePath);
	SAFE_CAPSTRING_DELETE(pstConfigData->strLogFilePath);
	SAFE_CAPSTRING_DELETE(strLogPrefix);
    SAFEMEMFREE(pstConfigData);
 
_EXIT:
	CAPLogger_Write(g_hLogger, MSG_INFO, "cap_iot_middlware stop");
	result = CAPLogger_Destroy(&g_hLogger);    
	return 0;
}
