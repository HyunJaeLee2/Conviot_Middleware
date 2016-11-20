/*
 * CAPFile.c
 *
 *  Created on: 2015. 8. 20.
 *      Author: chjej202
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>

#include <CAPFile.h>

typedef struct _SCAPFile {
    EHandleId enId;
    cap_string strFilePath;
    FILE *pFile;
    long lOffset;
} SCAPFile;

#define OFFSET_NOT_SET (-1)

cap_result CAPFile_Create(OUT cap_handle *phFile)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPFile *pstFile = NULL;

    IFVARERRASSIGNGOTO(phFile, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstFile = (SCAPFile *) malloc(sizeof(SCAPFile));
    ERRMEMGOTO(pstFile, result, _EXIT);

    pstFile->enId = HANDLEID_CAP_FILE;
    pstFile->lOffset = 0;
    pstFile->pFile = NULL;
    pstFile->strFilePath = NULL;

    *phFile = (cap_handle) pstFile;

    result = ERR_CAP_NOERROR;
_EXIT:
    if(result != ERR_CAP_NOERROR && pstFile != NULL)
    {
        CAPFile_Destroy((cap_handle) pstFile);
    }
    return result;
}

cap_result CAPFile_Destroy(IN OUT cap_handle *phFile)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPFile *pstFile = NULL;

    IFVARERRASSIGNGOTO(phFile, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if(IS_VALID_HANDLE(*phFile, HANDLEID_CAP_FILE) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstFile = (SCAPFile *) *phFile;

    if(pstFile->pFile != NULL)
    {
        fclose(pstFile->pFile);
        pstFile->pFile = NULL;
    }

    SAFE_CAPSTRING_DELETE(pstFile->strFilePath);

    SAFEMEMFREE(pstFile);

    *phFile = NULL;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

#define OPEN_MODE_BUF (3)

cap_result CAPFile_Open(cap_handle hFile, cap_string strPath, IN EFileMode enFileMode)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPFile *pstFile = NULL;
    char szOpenMode[OPEN_MODE_BUF];
    int nLen = 0;

    if(IS_VALID_HANDLE(hFile, HANDLEID_CAP_FILE) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(CAPString_Length(strPath), 0, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstFile = (SCAPFile *) hFile;

    if(pstFile->strFilePath == NULL)
    {
        pstFile->strFilePath = CAPString_New();
        ERRMEMGOTO(pstFile->strFilePath, result, _EXIT);
    }

    result = CAPString_Set(pstFile->strFilePath, strPath);
    ERRIFGOTO(result, _EXIT);

    switch(enFileMode)
    {
    case FILE_MODE_READ:
    case FILE_MODE_READ_PLUS:
        szOpenMode[nLen] = 'r';
        nLen++;
        break;
    case FILE_MODE_WRITE:
    case FILE_MODE_WRITE_PLUS:
        szOpenMode[nLen] = 'w';
        nLen++;
        break;
    case FILE_MODE_APPEND:
    case FILE_MODE_APPEND_PLUS:
        szOpenMode[nLen] = 'a';
        nLen++;
        break;
    default:
        ERRASSIGNGOTO(result, ERR_CAP_INTERNAL_FAIL, _EXIT);
        break;
    }

    if((enFileMode & CAP_PLUS_MODE) == CAP_PLUS_MODE)
    {
        szOpenMode[nLen] = '+';
        nLen++;
    }
    szOpenMode[nLen] = '\0';

    pstFile->pFile = fopen(CAPString_LowPtr(pstFile->strFilePath, NULL), szOpenMode);
    IFVARERRASSIGNGOTO(pstFile->pFile, NULL, result, ERR_CAP_OPEN_FAIL, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result CAPFile_Close(cap_handle hFile)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPFile *pstFile = NULL;

    if(IS_VALID_HANDLE(hFile, HANDLEID_CAP_FILE) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstFile = (SCAPFile *) hFile;

    if(pstFile->pFile != NULL)
    {
        fclose(pstFile->pFile);
        pstFile->pFile = NULL;
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result CAPFile_Read(cap_handle hFile, IN OUT char *pBuffer, IN int nDataToRead, OUT int *pnDataRead)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPFile *pstFile = NULL;
    size_t unDataRead = 0;

    if(IS_VALID_HANDLE(hFile, HANDLEID_CAP_FILE) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(pBuffer, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if(nDataToRead <= 0)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_PARAM, _EXIT);
    }

    pstFile = (SCAPFile *) hFile;

    IFVARERRASSIGNGOTO(pstFile->pFile, NULL, result, ERR_CAP_FILE_NOT_OPENED, _EXIT);

    unDataRead = fread(pBuffer, 1, nDataToRead, pstFile->pFile);

    if(pnDataRead != NULL)
    {
        *pnDataRead = unDataRead;
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result CAPFile_Write(cap_handle hFile, IN char *pData, IN int nDataLen, OUT int *pnDataWritten)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPFile *pstFile = NULL;
    size_t unDataWrite = 0;

    if(IS_VALID_HANDLE(hFile, HANDLEID_CAP_FILE) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE,_EXIT);
    }

    IFVARERRASSIGNGOTO(pData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if(nDataLen <= 0)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_PARAM, _EXIT);
    }

    pstFile = (SCAPFile *) hFile;

    IFVARERRASSIGNGOTO(pstFile->pFile, NULL, result, ERR_CAP_FILE_NOT_OPENED, _EXIT);

    unDataWrite = fwrite(pData, 1 , nDataLen, pstFile->pFile);

    if(pnDataWritten != NULL)
    {
        *pnDataWritten = unDataWrite;
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result CAPFile_Seek(cap_handle hFile, long lOffset, EFileSeek enSeek)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPFile *pstFile = NULL;
    int nSeek = SEEK_SET;
    int nRet = 0;

    if(IS_VALID_HANDLE(hFile, HANDLEID_CAP_FILE) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    if(lOffset < 0)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_PARAM, _EXIT);
    }

    pstFile = (SCAPFile *) hFile;

    IFVARERRASSIGNGOTO(pstFile->pFile, NULL, result, ERR_CAP_FILE_NOT_OPENED, _EXIT);

    switch(enSeek)
    {
    case FILE_SEEK_CUR:
        nSeek = SEEK_CUR;
        break;
    case FILE_SEEK_END:
        nSeek = SEEK_END;
        break;
    case FILE_SEEK_START:
        nSeek = SEEK_SET;
        break;
    default:
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_PARAM, _EXIT);
        break;
    }

    nRet = fseek(pstFile->pFile, lOffset, nSeek);
    if(nRet != 0)
    {
        ERRASSIGNGOTO(result, ERR_CAP_SEEK_FAIL, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result CAPFile_Flush(cap_handle hFile)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPFile *pstFile = NULL;
    int nRet = 0;

    if(IS_VALID_HANDLE(hFile, HANDLEID_CAP_FILE) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstFile = (SCAPFile *) hFile;

    IFVARERRASSIGNGOTO(pstFile->pFile, NULL, result, ERR_CAP_FILE_NOT_OPENED, _EXIT);

    nRet = fflush(pstFile->pFile);
    if(nRet != 0)
    {
        ERRASSIGNGOTO(result, ERR_CAP_SEEK_FAIL, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result CAPFile_GetFileSize(IN cap_handle hFile, OUT int *pnSize)
{
	cap_result result = ERR_CAP_UNKNOWN;
	SCAPFile *pstFile = NULL;
	int nFileSeekCurBuf;
	
	if(IS_VALID_HANDLE(hFile, HANDLEID_CAP_FILE) == FALSE)
	{
		ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
	}

	IFVARERRASSIGNGOTO(pnSize, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

	pstFile = (SCAPFile *) hFile;
    IFVARERRASSIGNGOTO(pstFile->pFile, NULL, result, ERR_CAP_FILE_NOT_OPENED, _EXIT);

    nFileSeekCurBuf = ftell(pstFile->pFile);
	
	result = CAPFile_Seek(pstFile, 0L, FILE_SEEK_END);
	ERRIFGOTO(result, _EXIT);
	
	*pnSize = ftell(pstFile->pFile);		

	result = CAPFile_Seek(pstFile, nFileSeekCurBuf, FILE_SEEK_START);
	ERRIFGOTO(result, _EXIT);

	result = ERR_CAP_NOERROR;
_EXIT:
	return result;
}


