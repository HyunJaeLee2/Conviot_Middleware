/*
 * CAPKeyValue.c
 *
 *  Created on: 2015. 8. 28.
 *      Author: chjej202
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <CAPHash.h>
#include <CAPKeyValue.h>

typedef struct _SCAPKeyValue {
    EHandleId enId;
    cap_handle hHash;
} SCAPKeyValue;

typedef struct _STraverseUserData {
    cap_string strFullString;
} STraverseUserData;

#define SEPARATOR_LINE_FEED ('\n')
#define SEPARATOR_EQUAL ('=')

#define WRITE_STRING_FORMAT "%s=%s\n"

cap_result CAPKeyValue_Create(IN int nBucketNum, OUT cap_handle *phKeyValue)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPKeyValue *pstKeyValue = NULL;

    IFVARERRASSIGNGOTO(phKeyValue, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstKeyValue = (SCAPKeyValue *) malloc(sizeof(SCAPKeyValue));
    ERRMEMGOTO(pstKeyValue, result, _EXIT);

    pstKeyValue->enId = HANDLEID_CAP_KEYVALUE;
    pstKeyValue->hHash = NULL;

    result = CAPHash_Create(nBucketNum, &(pstKeyValue->hHash));
    ERRIFGOTO(result, _EXIT);

    *phKeyValue = pstKeyValue;

    result = ERR_CAP_NOERROR;
_EXIT:
    if(result != ERR_CAP_NOERROR && pstKeyValue != NULL)
    {
        CAPKeyValue_Destroy((cap_handle *) &pstKeyValue);
    }
    return result;
}


static cap_result keyValueDestroy(IN cap_string strKey, IN void *pData, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    cap_string strValue = NULL;

    if(pData != NULL)
    {
        strValue = (cap_string) pData;
        SAFE_CAPSTRING_DELETE(strValue);
    }

    result = ERR_CAP_NOERROR;

    return result;
}


cap_result CAPKeyValue_Destroy(IN OUT cap_handle *phKeyValue)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPKeyValue *pstKeyValue = NULL;

    IFVARERRASSIGNGOTO(phKeyValue, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if(IS_VALID_HANDLE(*phKeyValue, HANDLEID_CAP_KEYVALUE) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstKeyValue = (SCAPKeyValue *) *phKeyValue;

    // ignore error
    result = CAPHash_Destroy(&(pstKeyValue->hHash), keyValueDestroy, NULL);

    SAFEMEMFREE(pstKeyValue);

    *phKeyValue = NULL;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result CAPKeyValue_Add(cap_handle hKeyValue, IN cap_string strKey, IN cap_string strValue)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPKeyValue *pstKeyValue = NULL;
    cap_string strValueCopy = NULL;

    if(IS_VALID_HANDLE(hKeyValue, HANDLEID_CAP_KEYVALUE) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(strKey, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(strValue, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstKeyValue = (SCAPKeyValue *) hKeyValue;

    strValueCopy = CAPString_New();
    ERRMEMGOTO(strValueCopy, result, _EXIT);

    result = CAPString_Set(strValueCopy, strValue);
    ERRIFGOTO(result, _EXIT);

    result = CAPHash_AddKey(pstKeyValue->hHash, strKey, strValueCopy);
    ERRIFGOTO(result, _EXIT);

    // because the value is set into the hash, set strValueCopy to NULL
    strValueCopy = NULL;

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFE_CAPSTRING_DELETE(strValueCopy);
    return result;
}


cap_result CAPKeyValue_Remove(cap_handle hKeyValue, IN cap_string strKey)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPKeyValue *pstKeyValue = NULL;

    if(IS_VALID_HANDLE(hKeyValue, HANDLEID_CAP_KEYVALUE) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(strKey, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstKeyValue = (SCAPKeyValue *) hKeyValue;

    result = CAPHash_DeleteKey(pstKeyValue->hHash, strKey, keyValueDestroy, NULL);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result CAPKeyValue_SetValueByKey(cap_handle hKeyValue, IN cap_string strKey, IN cap_string strValue)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPKeyValue *pstKeyValue = NULL;

    if(IS_VALID_HANDLE(hKeyValue, HANDLEID_CAP_KEYVALUE) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(strKey, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(strValue, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstKeyValue = (SCAPKeyValue *) hKeyValue;

    result = CAPHash_DeleteKey(pstKeyValue->hHash, strKey, keyValueDestroy, NULL);
    ERRIFGOTO(result, _EXIT);

    result = CAPKeyValue_Add((cap_handle) pstKeyValue, strKey, strValue);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result CAPKeyValue_GetValueByKey(cap_handle hKeyValue, IN cap_string strKey,
        IN OUT cap_string strValue)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPKeyValue *pstKeyValue = NULL;
    cap_string strValueGet = NULL;

    if(IS_VALID_HANDLE(hKeyValue, HANDLEID_CAP_KEYVALUE) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(strKey, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(strValue, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstKeyValue = (SCAPKeyValue *) hKeyValue;

    result = CAPHash_GetDataByKey(pstKeyValue->hHash, strKey, (void **) &strValueGet);
    ERRIFGOTO(result, _EXIT);

    result = CAPString_Set(strValue, strValueGet);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result CAPKeyValue_RemoveAll(cap_handle hKeyValue)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPKeyValue *pstKeyValue = NULL;

    if(IS_VALID_HANDLE(hKeyValue, HANDLEID_CAP_KEYVALUE) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstKeyValue = (SCAPKeyValue *) hKeyValue;

    result = CAPHash_RemoveAll(pstKeyValue->hHash, keyValueDestroy, NULL);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result CAPKeyValue_ReadFromBuffer(cap_handle hKeyValue, IN char *pszBuffer, IN int nBufSize)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPKeyValue *pstKeyValue = NULL;
    cap_string strBuffer = NULL;
    cap_string strData = NULL;
    cap_string strKey = NULL;
    int nLineFeedIndex = 0;
    int nEqualIndex = 0;
    int nStartIndex = 0;

    if(IS_VALID_HANDLE(hKeyValue, HANDLEID_CAP_KEYVALUE) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(pszBuffer, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if(nBufSize <= 0)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_PARAM, _EXIT);
    }

    pstKeyValue = (SCAPKeyValue *) hKeyValue;

    strBuffer = CAPString_New();
    ERRMEMGOTO(strBuffer, result, _EXIT);

    strData = CAPString_New();
    ERRMEMGOTO(strData, result, _EXIT);

    strKey = CAPString_New();
    ERRMEMGOTO(strKey, result, _EXIT);

    result = CAPString_SetLow(strBuffer, pszBuffer, nBufSize);
    ERRIFGOTO(result, _EXIT);

    while(nLineFeedIndex >= 0)
    {
        nLineFeedIndex = CAPString_FindChar(strBuffer, nStartIndex, SEPARATOR_LINE_FEED);
        nEqualIndex = CAPString_FindChar(strBuffer, nStartIndex, SEPARATOR_EQUAL);
        if(nLineFeedIndex < 0)
        {
            if(nEqualIndex < 0)
            {
                break;
            }

            result = CAPString_SetSub(strData, strBuffer, nEqualIndex+1, CAPSTRING_MAX);
            ERRIFGOTO(result, _EXIT);
        }
        else
        {
            if(nEqualIndex >= nLineFeedIndex || nEqualIndex < 0)
            {
                ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
            }

            result = CAPString_SetSub(strData, strBuffer, nEqualIndex+1, nLineFeedIndex - (nEqualIndex+1));
            ERRIFGOTO(result, _EXIT);
        }

        result = CAPString_SetSub(strKey, strBuffer, nStartIndex, nEqualIndex - nStartIndex);
        ERRIFGOTO(result, _EXIT);

        result = CAPString_Trim(strKey);
        ERRIFGOTO(result, _EXIT);

        result = CAPString_Trim(strData);
        ERRIFGOTO(result, _EXIT);

        // insert key and data
        result = CAPKeyValue_Add((cap_handle) pstKeyValue, strKey, strData);
        ERRIFGOTO(result, _EXIT);

        if(nLineFeedIndex >= 0)
        {
            nStartIndex = nLineFeedIndex + 1;
        }
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFE_CAPSTRING_DELETE(strKey);
    SAFE_CAPSTRING_DELETE(strBuffer);
    SAFE_CAPSTRING_DELETE(strData);
    return result;
}

static cap_result keyValueTraverse(IN cap_string strKey, IN void *pData, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    cap_string strValue = NULL;
    STraverseUserData *pstUserData = NULL;

    pstUserData = pUserData;

    if(pData != NULL)
    {
        strValue = (cap_string) pData;

        result = CAPString_AppendFormat(pstUserData->strFullString, WRITE_STRING_FORMAT, CAPString_LowPtr(strKey, NULL), CAPString_LowPtr(strValue, NULL));
        ERRIFGOTO(result, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result CAPKeyValue_WriteToString(cap_handle hKeyValue, IN OUT cap_string strToString)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPKeyValue *pstKeyValue = NULL;
    cap_string strFullString = NULL;
    STraverseUserData stUserData;

    if(IS_VALID_HANDLE(hKeyValue, HANDLEID_CAP_KEYVALUE) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(strToString, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstKeyValue = (SCAPKeyValue *) hKeyValue;

    strFullString = CAPString_New();
    ERRMEMGOTO(strFullString, result, _EXIT);

    stUserData.strFullString = strFullString;

    result = CAPHash_Traverse(pstKeyValue->hHash, keyValueTraverse, &stUserData);
    ERRIFGOTO(result, _EXIT);

    result = CAPString_Set(strToString, strFullString);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFE_CAPSTRING_DELETE(strFullString);
    return result;
}

static cap_result duplicateHash(IN cap_string strKey, IN void *pDataSrc, IN void *pUserData, OUT void **ppDataDst)
{
    cap_result result = ERR_CAP_UNKNOWN;
    cap_string strSrcValue = NULL;
    cap_string strDstValue = NULL;

    IFVARERRASSIGNGOTO(ppDataDst, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    strSrcValue = pDataSrc;

    strDstValue = CAPString_New();
    ERRMEMGOTO(strDstValue, result, _EXIT);

    result = CAPString_Set(strDstValue, strSrcValue);
    ERRIFGOTO(result, _EXIT);

    //printf("test check: key: %s, src: %s, dst: %s\n", CAPString_LowPtr(strKey, NULL), CAPString_LowPtr(strDstValue, NULL), CAPString_LowPtr(strDstValue, NULL));

    *ppDataDst = strDstValue;
    strDstValue = NULL;

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFE_CAPSTRING_DELETE(strDstValue);
    return result;
}


cap_result CAPKeyValue_Duplicate(cap_handle hDstKeyValue, cap_handle hSrcKeyValue)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPKeyValue *pstDstKeyValue = NULL;
    SCAPKeyValue *pstSrcKeyValue = NULL;

    if(IS_VALID_HANDLE(hDstKeyValue, HANDLEID_CAP_KEYVALUE) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    if(IS_VALID_HANDLE(hSrcKeyValue, HANDLEID_CAP_KEYVALUE) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstDstKeyValue = (SCAPKeyValue *) hDstKeyValue;
    pstSrcKeyValue = (SCAPKeyValue *) hSrcKeyValue;

    result = CAPKeyValue_RemoveAll(hDstKeyValue);
    ERRIFGOTO(result, _EXIT);

    result = CAPHash_Duplicate(pstDstKeyValue->hHash, pstSrcKeyValue->hHash, duplicateHash, NULL);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}




