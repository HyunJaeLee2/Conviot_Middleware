/* 
* AppValueCache.c
 *
 *  Created on: 2016. 8. 11.
 *      Author: chjej202
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <CAPHash.h>

#include "AppValueCache.h"

typedef struct _SCachedValue {
    SValue *pstValueInfo;
    cap_bool bInit; // whether the value is set or not
    double dbValue;
} SCachedValue;

typedef struct _SValueCache {
    EIoTHandleId enID;
    cap_handle hIndexHash;
    SCachedValue **ppastCachedValue;
    int nArraySize;
    int nValueNum;
    int nBucketNum;
} SValueCache;


#define MAX_SEARCH_RANGE 100

int findPrime(int nStart, int nLength)
{
    int nMaxNum = 0;
    cap_bool bFlag = FALSE;
    int nLoop = 0;
    int nDividedNum = 0;
    int nTargetNum = 0;
    nMaxNum = nStart + nLength;
    nTargetNum = nStart;

    while(nTargetNum < nMaxNum)
    {
        bFlag = FALSE;
        nDividedNum = nTargetNum/2;
        for(nLoop = 2; nLoop <= nDividedNum ; ++nLoop)
        {
            if(nTargetNum % nLoop == 0)
            {
                bFlag = TRUE;
                break;
            }
        }

        if(bFlag == FALSE)
        {
            // found
            break;
        }
        ++nTargetNum;
    }

    return nTargetNum;
}

cap_result AppValueCache_Create(int nMaxValueNum, OUT cap_handle *phCache)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SValueCache *pstValueCache = NULL;
    int nLoop = 0;
    int nBucketNum = 0;

    IFVARERRASSIGNGOTO(phCache, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if(nMaxValueNum < 0) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_PARAM, _EXIT);
    }

    pstValueCache = malloc(sizeof(SValueCache));
    ERRMEMGOTO(pstValueCache, result, _EXIT);

    pstValueCache->enID = HANDLEID_APP_DATA_CACHE;
    pstValueCache->ppastCachedValue = NULL;
    pstValueCache->nArraySize = 0;
    pstValueCache->nValueNum = 0;
    pstValueCache->hIndexHash = NULL;
    pstValueCache->nBucketNum = 0;

    // for 0, just create empty AppValueCache
    if(nMaxValueNum > 0)
    {
        pstValueCache->ppastCachedValue = malloc(sizeof(SCachedValue *)*nMaxValueNum);
        ERRMEMGOTO(pstValueCache->ppastCachedValue, result, _EXIT);

        pstValueCache->nArraySize = nMaxValueNum;

        for(nLoop = 0 ; nLoop < pstValueCache->nArraySize ; nLoop++)
        {
            pstValueCache->ppastCachedValue[nLoop] = NULL;
        }

        nBucketNum = findPrime(nMaxValueNum, MAX_SEARCH_RANGE);

        pstValueCache->nBucketNum = nBucketNum;

        result = CAPHash_Create(nBucketNum, &(pstValueCache->hIndexHash));
        ERRIFGOTO(result, _EXIT);
    }

    *phCache = pstValueCache;

    result = ERR_CAP_NOERROR;
_EXIT:
    if(result != ERR_CAP_NOERROR && pstValueCache != NULL)
    {
        CAPHash_Destroy(&(pstValueCache->hIndexHash), NULL, NULL);
        SAFEMEMFREE(pstValueCache->ppastCachedValue);
        SAFEMEMFREE(pstValueCache);
    }
    return result;
}


cap_result AppValueCache_Add(cap_handle hCache, IN cap_string strKey, IN SValue *pstValue, IN double *pdbInitValue)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SValueCache *pstValueCache = NULL;
    int nIndex = 0;
    SCachedValue *pstCachedInfo = NULL;
    SValue *pstValueInfo = NULL;
    int *pnIndex = NULL;

    if (IS_VALID_HANDLE(hCache, HANDLEID_APP_DATA_CACHE) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(strKey, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(pstValue, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(pstValue->strValueName, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstValueCache = (SValueCache *) hCache;

    if(pstValueCache->nValueNum >= pstValueCache->nArraySize) {
        ERRASSIGNGOTO(result, ERR_CAP_REACH_TO_MAXIMUM, _EXIT);
    }

    nIndex = pstValueCache->nValueNum;

    pstCachedInfo = malloc(sizeof(SCachedValue));
    ERRMEMGOTO(pstCachedInfo, result, _EXIT);
    pstCachedInfo->bInit = FALSE;
    pstCachedInfo->dbValue = 0;
    pstCachedInfo->pstValueInfo = NULL;

    pstValueInfo = malloc(sizeof(SValue));
    ERRMEMGOTO(pstValueInfo, result, _EXIT);

    pstValueInfo->strValueName = NULL;
    // pszFormat is not stored because format information is not used in scenario.
    pstValueInfo->pszFormat = NULL;
    pstValueInfo->dbMaxValue = pstValue->dbMaxValue;
    pstValueInfo->dbMinValue = pstValue->dbMinValue;
    pstValueInfo->enType = pstValue->enType;

    pstValueInfo->strValueName = CAPString_New();
    ERRMEMGOTO(pstValueInfo->strValueName, result, _EXIT);

    result = CAPString_Set(pstValueInfo->strValueName, pstValue->strValueName);
    ERRIFGOTO(result, _EXIT);

    pstCachedInfo->pstValueInfo = pstValueInfo;

    if(pdbInitValue != NULL)
    {
        pstCachedInfo->dbValue = *pdbInitValue;
        pstCachedInfo->bInit = TRUE;
    }

    pnIndex = malloc(sizeof(int));
    ERRMEMGOTO(pnIndex, result, _EXIT);

    *pnIndex = nIndex;

    result = CAPHash_AddKey(pstValueCache->hIndexHash, strKey, pnIndex);
    ERRIFGOTO(result, _EXIT);

    pstValueCache->ppastCachedValue[nIndex] = pstCachedInfo;

    pstValueCache->nValueNum++;

    result = ERR_CAP_NOERROR;
_EXIT:
    if(result != ERR_CAP_NOERROR)
    {
        SAFEMEMFREE(pnIndex);
        if(pstValueInfo != NULL)
        {
            SAFE_CAPSTRING_DELETE(pstValueInfo->strValueName);
            SAFEMEMFREE(pstValueInfo);
        }
        SAFEMEMFREE(pstCachedInfo);

    }
    return result;
}

static CALLBACK cap_result destroyIndexValue(IN cap_string strKey, IN void *pData, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;

    SAFEMEMFREE(pData);

    result = ERR_CAP_NOERROR;

    return result;
}


static CALLBACK cap_result copyIndexValue(IN cap_string strKey, IN void *pDataSrc, IN void *pUserData, OUT void **ppDataDst)
{
    cap_result result = ERR_CAP_UNKNOWN;
    int *pnSrcIndex = NULL;
    int *pnDstIndex = NULL;

    IFVARERRASSIGNGOTO(strKey, NULL, result, ERR_CAP_INVALID_DATA, _EXIT);
    IFVARERRASSIGNGOTO(pDataSrc, NULL, result, ERR_CAP_INVALID_DATA, _EXIT);
    IFVARERRASSIGNGOTO(ppDataDst, NULL, result, ERR_CAP_INVALID_DATA, _EXIT);

    pnDstIndex = malloc(sizeof(int));
    ERRMEMGOTO(pnDstIndex, result, _EXIT);

    pnSrcIndex = pDataSrc;

    // copy data
    *pnDstIndex = *pnSrcIndex;

    // return pointer
    *ppDataDst = pnDstIndex;

    result = ERR_CAP_NOERROR;
_EXIT:
    if(result != ERR_CAP_NOERROR)
    {
        SAFEMEMFREE(pnDstIndex);
    }
    return result;
}

cap_result AppValueCache_DuplicateIndexHash(cap_handle hCache, IN OUT cap_handle hIndexHash)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SValueCache *pstValueCache = NULL;

    if (IS_VALID_HANDLE(hCache, HANDLEID_APP_DATA_CACHE) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(hIndexHash, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstValueCache = (SValueCache *) hCache;

    result = CAPHash_Duplicate(hIndexHash, pstValueCache->hIndexHash, copyIndexValue, NULL);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result AppValueCache_GetBucketNumber(cap_handle hCache, OUT int *pnBucketNum)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SValueCache *pstValueCache = NULL;

    if (IS_VALID_HANDLE(hCache, HANDLEID_APP_DATA_CACHE) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(pnBucketNum, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstValueCache = (SValueCache *) hCache;

    *pnBucketNum = pstValueCache->nBucketNum;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}



cap_result AppValueCache_UpdateByIndex(cap_handle hCache, IN int nIndex, IN double dbValue)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SValueCache *pstValueCache = NULL;
    int nValue = 0;
    SValue *pstValueInfo = NULL;

    if (IS_VALID_HANDLE(hCache, HANDLEID_APP_DATA_CACHE) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstValueCache = (SValueCache *) hCache;

    if(nIndex < 0 || nIndex >= pstValueCache->nValueNum) {
        ERRASSIGNGOTO(result, ERR_CAP_INDEX_OUT_OF_BOUND, _EXIT);
    }

    pstValueInfo = pstValueCache->ppastCachedValue[nIndex]->pstValueInfo;

    if(pstValueInfo->enType == VALUE_TYPE_BOOL &&
            !(DOUBLE_IS_APPROX_EQUAL(dbValue, TRUE) || DOUBLE_IS_APPROX_EQUAL(dbValue, FALSE)))
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }
    else if(pstValueInfo->enType == VALUE_TYPE_INT)
    {
        nValue = (int) dbValue;
        if(!DOUBLE_IS_APPROX_EQUAL(dbValue, nValue) ||
          DOUBLE_IS_GREATER(dbValue, pstValueInfo->dbMaxValue) ||
          DOUBLE_IS_LESS(dbValue, pstValueInfo->dbMinValue))
        {
            ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
        }
        pstValueCache->ppastCachedValue[nIndex]->dbValue = (int) dbValue;
    }
    else
    {
        if(DOUBLE_IS_GREATER(dbValue, pstValueInfo->dbMaxValue) || DOUBLE_IS_LESS(dbValue, pstValueInfo->dbMinValue))
        {
            ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
        }
        pstValueCache->ppastCachedValue[nIndex]->dbValue = dbValue;
    }
    pstValueCache->ppastCachedValue[nIndex]->bInit = TRUE;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result AppValueCache_GetByIndex(cap_handle hCache, IN int nIndex, OUT EValueType *penType, OUT double *pdbValue)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SValueCache *pstValueCache = NULL;

    if (IS_VALID_HANDLE(hCache, HANDLEID_APP_DATA_CACHE) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(penType, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(pdbValue, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstValueCache = (SValueCache *) hCache;

    if(nIndex < 0 || nIndex >= pstValueCache->nValueNum) {
        ERRASSIGNGOTO(result, ERR_CAP_INDEX_OUT_OF_BOUND, _EXIT);
    }

    if( pstValueCache->ppastCachedValue[nIndex]->bInit == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_NO_DATA, _EXIT);
    }
    else
    {
        *pdbValue = pstValueCache->ppastCachedValue[nIndex]->dbValue;
        *penType = pstValueCache->ppastCachedValue[nIndex]->pstValueInfo->enType;
    }


    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result AppValueCache_Destroy(IN OUT cap_handle *phCache)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SValueCache *pstValueCache = NULL;
    int nLoop = 0;

    IFVARERRASSIGNGOTO(phCache, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if (IS_VALID_HANDLE(*phCache, HANDLEID_APP_DATA_CACHE) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstValueCache = (SValueCache *) *phCache;

    CAPHash_Destroy(&(pstValueCache->hIndexHash), destroyIndexValue, NULL);
    
    for(nLoop = 0 ; nLoop < pstValueCache->nValueNum ; nLoop++)
    {
        SAFE_CAPSTRING_DELETE(pstValueCache->ppastCachedValue[nLoop]->pstValueInfo->strValueName);
        SAFEMEMFREE(pstValueCache->ppastCachedValue[nLoop]->pstValueInfo);
        SAFEMEMFREE(pstValueCache->ppastCachedValue[nLoop]);
    }
    

    SAFEMEMFREE(pstValueCache->ppastCachedValue);

    SAFEMEMFREE(pstValueCache);

    *phCache = NULL;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}



