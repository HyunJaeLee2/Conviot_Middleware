/*
 * CAPLogger.c
 *
 *  Created on: 2015. 8. 20.
 *      Author: chjej202
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define DEFAULT_MAX_LOG_SIZE 2*1024
#define DEFAULT_MAX_BACKUP_NUM 1

#include <CAPFile.h>
#include <CAPLogger.h>
#include <CAPThread.h>
#include <CAPThreadLock.h>
#include <sys/syscall.h>
#include <time.h>
#include <stdarg.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct _SCAPLogger
{
    EHandleId enId;
    ELogLevel enLogLevel;
    cap_string strName;
	cap_string strLogPath;	//
    int nNameLength;
	int nMaxLogSize;
	int nMaxBackupNum;
	int nBackupNum;
    cap_handle hThreadLock;
    cap_handle hFile;
} SCAPLogger;

static char* pszLoggerStr[] = { 
    "LOG_NONE",
    "LOG_INFO",
    "LOG_ERROR",
    "LOG_WARN",
    "LOG_DETAIL",
    "LOG_DEBUG"
};

static ELogLevel convertLogMsgLevelToLogLevel(ELogMsgLevel enMsgLevel)
{
    ELogLevel enLogLevel;
    switch(enMsgLevel)
    {
    case MSG_NONE:
        enLogLevel = LOG_NONE;
        break;
    case MSG_INFO:
        enLogLevel = LOG_INFO;
        break;
    case MSG_ERROR:
        enLogLevel = LOG_ERROR;
        break;
    case MSG_WARN:
        enLogLevel = LOG_WARN;
        break;
    case MSG_DETAIL:
        enLogLevel = LOG_DETAIL;
        break;
    case MSG_DEBUG:
        enLogLevel = LOG_DEBUG;
        break;
    default:
        enLogLevel = LOG_NONE;
        break;
    }

    return enLogLevel;
}

cap_result CAPLogger_Create(IN cap_string strLogPath, IN cap_string strPrefix, IN ELogLevel enLogLevel, IN int nMaxLogSize, IN int nMaxBackupNum, OUT cap_handle *phLog)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPLogger *pstLogger = NULL;
	int nLoop = 0;
	cap_string strLogBackup = NULL;

    IFVARERRASSIGNGOTO(strLogPath, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(strPrefix, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(phLog, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
	
    pstLogger = (SCAPLogger *) malloc (sizeof(SCAPLogger));
    ERRMEMGOTO(pstLogger, result, _EXIT);

    pstLogger->enId = HANDLEID_CAP_LOGGER;
    pstLogger->enLogLevel = enLogLevel;

    pstLogger->strName = CAPString_New();
    ERRMEMGOTO(pstLogger->strName, result, _EXIT);

    result = CAPString_Set(pstLogger->strName, strPrefix);
    ERRIFGOTO(result, _EXIT);
	
	pstLogger->strLogPath = CAPString_New();
	ERRMEMGOTO(pstLogger->strLogPath, result, _EXIT);

	result = CAPString_Set(pstLogger->strLogPath, strLogPath);
	ERRIFGOTO(result, _EXIT);

    pstLogger->nNameLength = CAPString_Length(strPrefix);

    result = CAPThreadLock_Create(&(pstLogger->hThreadLock));
    ERRIFGOTO(result, _EXIT);
	
    result = CAPFile_Create(&pstLogger->hFile);
    ERRIFGOTO(result, _EXIT);
    result = CAPFile_Open(pstLogger->hFile, pstLogger->strLogPath, FILE_MODE_APPEND_PLUS);
    ERRIFGOTO(result, _EXIT);

	//Default value
	if( nMaxLogSize == -1 ){
		pstLogger->nMaxLogSize = DEFAULT_MAX_LOG_SIZE;
	}
	else{
		pstLogger->nMaxLogSize = nMaxLogSize;
	}
	
	if( nMaxBackupNum == -1 ){
		pstLogger->nMaxBackupNum = DEFAULT_MAX_BACKUP_NUM;
	}
	else{
		pstLogger->nMaxBackupNum = nMaxBackupNum;
	}
	
	strLogBackup = CAPString_New();
	ERRMEMGOTO(strLogBackup, result, _EXIT);
	
	//In order to count log files
	while( nLoop < pstLogger->nMaxBackupNum ){
		result = CAPString_Set(strLogBackup, pstLogger->strLogPath);
		ERRIFGOTO(result, _EXIT);
		
		CAPString_AppendFormat(strLogBackup, ".%d", nLoop);	//Each loop makes a path of a log file like /usr/local/capital/middleware/log.0 when nLoop is 0

		if(access(CAPString_LowPtr(strLogBackup,NULL), F_OK) != 0){	//if access is failed, break the loop 
			break;
		}
		nLoop++;
	}
	pstLogger->nBackupNum = nLoop;
    *phLog = pstLogger;
    pstLogger = NULL;

    result = ERR_CAP_NOERROR;
_EXIT:
	SAFE_CAPSTRING_DELETE(strLogBackup);
    SAFEMEMFREE(pstLogger);
    return result;
}


cap_result CAPLogger_Destroy(IN OUT cap_handle *phLog)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPLogger *pstLogger = NULL;

    IFVARERRASSIGNGOTO(phLog, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

	if(IS_VALID_HANDLE(*phLog, HANDLEID_CAP_LOGGER) == FALSE)
	{
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstLogger = (SCAPLogger *) *phLog;

    SAFE_CAPSTRING_DELETE(pstLogger->strName);
    SAFE_CAPSTRING_DELETE(pstLogger->strLogPath);

    if(pstLogger->hThreadLock != NULL)
    {
        result = CAPThreadLock_Lock(pstLogger->hThreadLock);
        if(result == ERR_CAP_NOERROR)
        {
            // ignore error
            CAPFile_Destroy(&(pstLogger->hFile));

            // ignore error
            CAPThreadLock_Unlock(pstLogger->hThreadLock);
        }

        // ignore error
        CAPThreadLock_Destroy(&(pstLogger->hThreadLock));
    }

    SAFEMEMFREE(pstLogger);

    *phLog = NULL;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result CAPLogger_Write(IN cap_handle hLog, ELogMsgLevel enMsgLogLevel, char *pszFormat, ...)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPLogger *pstLogger = NULL;
    va_list a_list;
    time_t stTime = NULL;
    struct tm stTm;
    struct tm *pstTm;
    cap_string strTemp = NULL;
    cap_string strPrint = NULL;
    int nDataWritten = 0;
    int nDataToWrite = 0;
    ELogLevel enLogLevel;
	int nSize = 0;
	int nFileNum;
	cap_string strOldBackupPath = NULL;
	cap_string strNewBackupPath = NULL;

	if(IS_VALID_HANDLE(hLog, HANDLEID_CAP_LOGGER) == FALSE)
	{
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstLogger = hLog;

    enLogLevel = convertLogMsgLevelToLogLevel(enMsgLogLevel);

    if (enLogLevel <= pstLogger->enLogLevel)
    {
        time (&stTime);
        pstTm = localtime_r(&stTime, &stTm);
        ERRMEMGOTO(pstTm, result, _EXIT);

        strTemp = CAPString_New();
        ERRMEMGOTO(strTemp, result, _EXIT);

        va_start(a_list, pszFormat);
        result = CAPString_VPrintFormat(strTemp, pszFormat, a_list);
        va_end(a_list);
        ERRIFGOTO(result, _EXIT);

        strPrint = CAPString_New();
        ERRMEMGOTO(strPrint, result, _EXIT);

        result = CAPString_PrintFormat (strPrint, "[%4d/%02d/%02d %02d:%02d:%02d][%d][0x%x][%s][%s] ",
                stTm.tm_year + 1900, stTm.tm_mon + 1, stTm.tm_mday, stTm.tm_hour, stTm.tm_min,
                stTm.tm_sec, getpid(), CAPThread_GetCurThreadID(), pszLoggerStr[enMsgLogLevel], CAPString_LowPtr(pstLogger->strName, NULL));
        ERRIFGOTO(result, _EXIT);

        result = CAPString_AppendString(strPrint, strTemp);
        ERRIFGOTO(result, _EXIT);

        result = CAPString_AppendChar(strPrint, LINE_SEPARATOR);
        ERRIFGOTO(result, _EXIT);

        nDataToWrite = sizeof(char) * CAPString_Length(strPrint);
		/**************************************************************************************/
        CAPThreadLock_Lock(pstLogger->hThreadLock);
		
		result = CAPFile_GetFileSize(pstLogger->hFile, &nSize);
		ERRIFGOTO(result, _EXIT);

		nSize = sizeof(char) * nSize + nDataToWrite;
		if(nSize > pstLogger->nMaxLogSize){	//If the log file size exceeds nMaxLogSize, make a backup file
			result = CAPFile_Close(pstLogger->hFile);	//close the current log file
			ERRIFGOTO(result, _EXIT);
		
			if( 0 != pstLogger-> nMaxBackupNum ) {
				//Forloop rename backup files
				//If there are 3 backup files,there are [backup_file_name].0 [backup_file_name].1 [backup_file_name].2
				//Each loop renames a backupfile, starting at 2
				//strOldBackupPath      ->   strNewBackupPath
				//[backup_file_name].2  ->   [backup_file_name].3
				//[backup_file_name].1  ->   [backup_file_name].2
				//[backup_file_name].0  ->   [backup_file_name].1
				//[backup_file_name]    ->    [backup_file_name].0
				for(nFileNum = pstLogger->nBackupNum-1 ; nFileNum >= -1 ; nFileNum--){
					if(nFileNum < pstLogger->nMaxBackupNum-1){
						strOldBackupPath = CAPString_New();
						ERRMEMGOTO(strOldBackupPath, result, _EXIT);
						strNewBackupPath = CAPString_New();
						ERRMEMGOTO(strNewBackupPath, result, _EXIT);

						result = CAPString_Set(strOldBackupPath,pstLogger->strLogPath);
						ERRIFGOTO(result, _EXIT);
						result = CAPString_Set(strNewBackupPath,pstLogger->strLogPath);
						ERRIFGOTO(result, _EXIT);

						if(nFileNum >= 0){	
							result = CAPString_AppendFormat(strOldBackupPath,".%d", nFileNum);
							ERRIFGOTO(result, _EXIT);
							result = CAPString_AppendFormat(strNewBackupPath,".%d", nFileNum+1);
							ERRIFGOTO(result, _EXIT);
						}
						else{	//[backup_file_name] -> [backup_file_name].0
							result = CAPString_AppendFormat(strNewBackupPath,".%d", 0);
							ERRIFGOTO(result, _EXIT);
							pstLogger->nBackupNum++;
						}

						rename(CAPString_LowPtr(strOldBackupPath,NULL), CAPString_LowPtr(strNewBackupPath,NULL));

						SAFE_CAPSTRING_DELETE(strOldBackupPath);
						SAFE_CAPSTRING_DELETE(strNewBackupPath);
					}
				}
			}
			result = CAPFile_Open(pstLogger->hFile, pstLogger->strLogPath, FILE_MODE_WRITE_PLUS); //create a new log file
			ERRIFGOTO(result, _EXIT);
		}
		
		result = CAPFile_Write(pstLogger->hFile, CAPString_LowPtr(strPrint, NULL), nDataToWrite, &nDataWritten); //write a log on log file
		CAPFile_Flush(pstLogger->hFile);
		
		CAPThreadLock_Unlock(pstLogger->hThreadLock);
		/**************************************************************************************/
        ERRIFGOTO(result, _EXIT);

        if(nDataWritten != nDataToWrite)
        {
            ERRASSIGNGOTO(result, ERR_CAP_NOT_FINISHED, _EXIT);
        }
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFE_CAPSTRING_DELETE(strTemp);
    SAFE_CAPSTRING_DELETE(strPrint);
    return result;
}
