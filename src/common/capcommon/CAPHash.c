/*
 * CAPHash.c
 *
 *  Created on: 2015. 8. 19.
 *      Author: chjej202
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <CAPLinkedList.h>
#include <CAPHash.h>

typedef struct _SHashNode
{
    cap_string strKey;
    void *pData;
} SHashNode;

typedef struct _SCAPHash
{
    EHandleId enId;
    int nBucketSize;
    cap_handle *paLinkedList;
} SCAPHash;

typedef struct _SHashUsrData
{
    CbFnCAPHash fnCallback;
    void *pUsrData;
} SHashUsrData;

typedef struct _SHashDupUsrData
{
    CbFnCAPHashDup fnCallback;
    void *pUsrData;
    cap_handle hDstHash;
} SHashDupUsrData;

typedef struct _SHashFindData
{
    cap_string strToCompare;
    int nOffsetToReturn;
} SHashFindData;


#define OFFSET_NOT_SET (-1)

// SAX stands for "Shift And Xor"
static cap_result CAPHash_SAXHash(cap_string strKey, unsigned int unBucketNum, OUT unsigned int *punBucketIndex)
{
    cap_result result = ERR_CAP_UNKNOWN;
    unsigned int unBucketIndex = 0;
    unsigned int unLoop = 0;
    unsigned int unHashVal = 0;
    int nStringSize = 0;
    const char *pszKey = NULL;

    IFVARERRASSIGNGOTO(punBucketIndex, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pszKey = CAPString_LowPtr(strKey, &nStringSize);
    ERRMEMGOTO(pszKey, result, _EXIT);

    nStringSize = nStringSize * sizeof(char);

    for(unLoop=0; unLoop < nStringSize; unLoop++)
    {
        unHashVal ^= (unHashVal << 5) + (unHashVal >> 2) + pszKey[unLoop];
    }
    unBucketIndex = unHashVal & (unBucketNum-1U);

    *punBucketIndex = unBucketIndex;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

#define HASH_FUNCTION CAPHash_SAXHash

cap_result CAPHash_Create(IN int nNumOfBuckets, OUT cap_handle *phHash)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPHash *pstHash = NULL;
    int nLoop = 0;

    IFVARERRASSIGNGOTO(phHash, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if(nNumOfBuckets <= 0)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_PARAM, _EXIT);
    }

    pstHash = malloc(sizeof(SCAPHash));
    ERRMEMGOTO(pstHash, result, _EXIT);

    pstHash->enId = HANDLEID_CAP_HASH;
    pstHash->nBucketSize = nNumOfBuckets;
    pstHash->paLinkedList = NULL;

    pstHash->paLinkedList = malloc(sizeof(cap_handle)*(pstHash->nBucketSize));
    ERRMEMGOTO(pstHash->paLinkedList, result, _EXIT);

    // initialize the pointer to be NULL
    for(nLoop = 0 ; nLoop < pstHash->nBucketSize ; nLoop++)
    {
        pstHash->paLinkedList[nLoop] = NULL;
    }

    for(nLoop = 0 ; nLoop < pstHash->nBucketSize ; nLoop++)
    {
         result = CAPLinkedList_Create(&(pstHash->paLinkedList[nLoop]));
         ERRIFGOTO(result, _EXIT);
    }

    *phHash = pstHash;

    result = ERR_CAP_NOERROR;
_EXIT:
    if(result != ERR_CAP_NOERROR)
    {
        CAPHash_Destroy((cap_handle *) &pstHash, NULL, NULL);
    }
    return result;
}

static cap_result hashDataDestroy(IN int nOffset, IN void *pData, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SHashNode *pstData = NULL;
    SHashUsrData *pstUserData = NULL;

    pstData = (SHashNode *) pData;

    if(pstData != NULL)
    {
        pstUserData = (SHashUsrData *) pUserData;
        if(pstUserData->fnCallback != NULL)
        {
           result = pstUserData->fnCallback(pstData->strKey, pstData->pData, pstUserData->pUsrData);
           ERRIFGOTO(result, _EXIT);
        }

        SAFE_CAPSTRING_DELETE(pstData->strKey);
        SAFEMEMFREE(pstData);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result CAPHash_Destroy(IN OUT cap_handle *phHash, IN CbFnCAPHash fnDestroyCallback, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPHash *pstHash = NULL;
    int nLoop = 0;
    SHashUsrData stUsrData;

    IFVARERRASSIGNGOTO(phHash, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if(IS_VALID_HANDLE(*phHash, HANDLEID_CAP_HASH) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstHash = (SCAPHash *) *phHash;
    stUsrData.fnCallback = fnDestroyCallback;
    stUsrData.pUsrData = pUserData;

    for(nLoop = 0 ; nLoop < pstHash->nBucketSize ; nLoop++)
    {
        // ignore results to free memory as much as possible
        result = CAPLinkedList_Traverse(pstHash->paLinkedList[nLoop], hashDataDestroy, &stUsrData);
        result = CAPLinkedList_Destroy(&(pstHash->paLinkedList[nLoop]));
    }
    SAFEMEMFREE(pstHash->paLinkedList);

    SAFEMEMFREE(pstHash);

    *phHash = NULL;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


static cap_result findKey(IN int nOffset, IN void *pData, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SHashNode *pstData = NULL;
    SHashFindData *pstUserData = NULL;

    IFVARERRASSIGNGOTO(pData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(pUserData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstData = (SHashNode *) pData;
    pstUserData = (SHashFindData *) pUserData;

    if(CAPString_IsEqual(pstData->strKey, pstUserData->strToCompare) == TRUE)
    {
        pstUserData->nOffsetToReturn = nOffset;
        CAPASSIGNGOTO(result, ERR_CAP_FOUND_DATA, _EXIT);
    }

    pstUserData->nOffsetToReturn = OFFSET_NOT_SET;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result CAPHash_AddKey(cap_handle hHash, IN cap_string strKey, IN void *pData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPHash *pstHash = NULL;
    unsigned int unBucketIndex = 0;
    SHashNode *pstHashNode = NULL;
    SHashFindData stUserData;

    if(IS_VALID_HANDLE(hHash, HANDLEID_CAP_HASH) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(CAPString_Length(strKey), 0, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstHash = (SCAPHash *) hHash;

    result = HASH_FUNCTION(strKey, pstHash->nBucketSize, &unBucketIndex);
    ERRIFGOTO(result, _EXIT);

    stUserData.strToCompare = strKey;
    stUserData.nOffsetToReturn = OFFSET_NOT_SET;

    result = CAPLinkedList_Traverse(pstHash->paLinkedList[unBucketIndex], findKey, (void *) &stUserData);
    if(result == ERR_CAP_FOUND_DATA)
    {
        result = ERR_CAP_DUPLICATED;
    }
    ERRIFGOTO(result, _EXIT);

    pstHashNode = (SHashNode *) malloc(sizeof(SHashNode));
    ERRMEMGOTO(pstHashNode, result, _EXIT);

    pstHashNode->strKey = NULL;
    pstHashNode->pData = NULL;

    pstHashNode->strKey = CAPString_New();
    ERRMEMGOTO(pstHashNode->strKey, result, _EXIT);

    result = CAPString_Set(pstHashNode->strKey, strKey);
    ERRIFGOTO(result, _EXIT);

    pstHashNode->pData = pData;

    result = CAPLinkedList_Add(pstHash->paLinkedList[unBucketIndex], LINKED_LIST_OFFSET_LAST, 0, pstHashNode);
    ERRIFGOTO(result, _EXIT);

    // Because the hash node is already inserted into the LinkedList, set pointer to NULL
    pstHashNode = NULL;

    result = ERR_CAP_NOERROR;
_EXIT:
    if(pstHashNode != NULL)
    {
        SAFE_CAPSTRING_DELETE(pstHashNode->strKey);
        SAFEMEMFREE(pstHashNode);
    }
    return result;
}


cap_result CAPHash_DeleteKey(cap_handle hHash, IN cap_string strKey, IN CbFnCAPHash fnDestroyCallback, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPHash *pstHash = NULL;
    unsigned int unBucketIndex = 0;
    SHashNode *pstHashNode = NULL;
    SHashFindData stUserData;

    if(IS_VALID_HANDLE(hHash, HANDLEID_CAP_HASH) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(CAPString_Length(strKey), 0, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstHash = (SCAPHash *) hHash;

    result = HASH_FUNCTION(strKey, pstHash->nBucketSize, &unBucketIndex);
    ERRIFGOTO(result, _EXIT);

    stUserData.nOffsetToReturn = OFFSET_NOT_SET;
    stUserData.strToCompare = strKey;

    result = CAPLinkedList_Traverse(pstHash->paLinkedList[unBucketIndex], findKey, (void *) &stUserData);
    if(result == ERR_CAP_FOUND_DATA)
    {
        result = CAPLinkedList_Get(pstHash->paLinkedList[unBucketIndex], LINKED_LIST_OFFSET_FIRST, stUserData.nOffsetToReturn, (void **) &pstHashNode);
        ERRIFGOTO(result, _EXIT);

        if(fnDestroyCallback != NULL)
        {
            result = fnDestroyCallback(pstHashNode->strKey, pstHashNode->pData, pUserData);
            ERRIFGOTO(result, _EXIT);
        }

        SAFE_CAPSTRING_DELETE(pstHashNode->strKey);
        SAFEMEMFREE(pstHashNode);

        result = CAPLinkedList_Remove(pstHash->paLinkedList[unBucketIndex], LINKED_LIST_OFFSET_FIRST, stUserData.nOffsetToReturn);
    }
    ERRIFGOTO(result, _EXIT);

    // no error is returned if the key is not found in hash

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

static cap_result hashLinkedListTraverse(IN int nOffset, IN void *pData, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SHashNode *pstData = NULL;
    SHashUsrData *pstUserData = NULL;

    pstData = (SHashNode *) pData;

    if(pstData != NULL)
    {
        pstUserData = (SHashUsrData *) pUserData;
        if(pstUserData->fnCallback != NULL)
        {
           result = pstUserData->fnCallback(pstData->strKey, pstData->pData, pstUserData->pUsrData);
           ERRIFGOTO(result, _EXIT);
        }
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


static int getItemNum(SCAPHash *pstHash)
{
    int nItemNum = 0;
    int nLoop = 0;
    int nLength = 0;
    cap_result result = ERR_CAP_UNKNOWN;

    for(nLoop = 0 ; nLoop < pstHash->nBucketSize ; nLoop++)
    {
        result = CAPLinkedList_GetLength(pstHash->paLinkedList[nLoop], &nLength);
        ERRIFGOTO(result, _EXIT);

        nItemNum += nLength;
    }
_EXIT:
    if(result != ERR_CAP_NOERROR)
    {
        nItemNum = -1;
    }
    return nItemNum;
}



cap_result CAPHash_GetNumberOfItems(cap_handle hHash, OUT int *pnItemNum)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPHash *pstHash = NULL;

    if(IS_VALID_HANDLE(hHash, HANDLEID_CAP_HASH) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(pnItemNum, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstHash = (SCAPHash *) hHash;

    *pnItemNum = getItemNum(pstHash);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result CAPHash_Traverse(cap_handle hHash, IN CbFnCAPHash fnCallback, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPHash *pstHash = NULL;
    int nBucketLoop = 0;
    SHashUsrData stUsrData;

    if(IS_VALID_HANDLE(hHash, HANDLEID_CAP_HASH) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(fnCallback, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstHash = (SCAPHash *) hHash;

    stUsrData.fnCallback = fnCallback;
    stUsrData.pUsrData = pUserData;

    for(nBucketLoop = 0 ; nBucketLoop < pstHash->nBucketSize ; nBucketLoop++)
    {
        result = CAPLinkedList_Traverse(pstHash->paLinkedList[nBucketLoop], hashLinkedListTraverse, &stUsrData);
        ERRIFGOTO(result, _EXIT);
    }


    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}



cap_result CAPHash_GetDataByKey(cap_handle hHash, IN cap_string strKey, OUT void **ppData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPHash *pstHash = NULL;
    unsigned int unBucketIndex = 0;
    SHashNode *pstHashNode = NULL;
    SHashFindData stUserData;

    if(IS_VALID_HANDLE(hHash, HANDLEID_CAP_HASH) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    IFVARERRASSIGNGOTO(CAPString_Length(strKey), 0, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(ppData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstHash = (SCAPHash *) hHash;

    result = HASH_FUNCTION(strKey, pstHash->nBucketSize, &unBucketIndex);
    ERRIFGOTO(result, _EXIT);

    stUserData.nOffsetToReturn = OFFSET_NOT_SET;
    stUserData.strToCompare = strKey;

    result = CAPLinkedList_Traverse(pstHash->paLinkedList[unBucketIndex], findKey, (void *) &stUserData);
    if(result == ERR_CAP_FOUND_DATA)
    {
        result = CAPLinkedList_Get(pstHash->paLinkedList[unBucketIndex], LINKED_LIST_OFFSET_FIRST, stUserData.nOffsetToReturn, (void **) &pstHashNode);
        ERRIFGOTO(result, _EXIT);

        *ppData = pstHashNode->pData;
    }
    else if(result == ERR_CAP_NOERROR)
    {
        CAPASSIGNGOTO(result, ERR_CAP_NOT_FOUND, _EXIT);
    }
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result CAPHash_RemoveAll(cap_handle hHash, IN CbFnCAPHash fnDestroyCallback, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPHash *pstHash = NULL;
    SHashUsrData stUsrData;
    int nLoop = 0;
    int nLinkedListSize = 0;
    int nInLoop = 0;

    if(IS_VALID_HANDLE(hHash, HANDLEID_CAP_HASH) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstHash = (SCAPHash *) hHash;

    stUsrData.fnCallback = fnDestroyCallback;
    stUsrData.pUsrData = pUserData;

    for(nLoop = 0 ; nLoop < pstHash->nBucketSize ; nLoop++)
    {
        result = CAPLinkedList_Traverse(pstHash->paLinkedList[nLoop], hashDataDestroy, &stUsrData);
        ERRIFGOTO(result, _EXIT);
        result = CAPLinkedList_GetLength(pstHash->paLinkedList[nLoop], &nLinkedListSize);
        for(nInLoop = 0; nInLoop < nLinkedListSize ; nInLoop++)
        {
            result = CAPLinkedList_Remove(pstHash->paLinkedList[nLoop], LINKED_LIST_OFFSET_FIRST, 0);
            ERRIFGOTO(result, _EXIT);
        }
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


static cap_result hashDataTraverse(IN cap_string strKey, IN void *pData, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SHashDupUsrData *pstUserData = NULL;
    void *pDstData = NULL;

    if(pUserData != NULL)
    {
        pstUserData = ( SHashDupUsrData *) pUserData;
        result = pstUserData->fnCallback(strKey, pData, pstUserData->pUsrData, &pDstData);
        ERRIFGOTO(result, _EXIT);

        result = CAPHash_AddKey(pstUserData->hDstHash, strKey, pDstData);
        ERRIFGOTO(result, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result CAPHash_Duplicate(IN OUT cap_handle hDstHash, IN cap_handle hSrcHash, IN CbFnCAPHashDup fnDataCopyCallback, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPHash *pstDstHash = NULL;
    SCAPHash *pstSrcHash = NULL;
    SHashDupUsrData stUsrData;

    if(IS_VALID_HANDLE(hDstHash, HANDLEID_CAP_HASH) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    if(IS_VALID_HANDLE(hSrcHash, HANDLEID_CAP_HASH) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstDstHash = (SCAPHash *) hDstHash;
    pstSrcHash = (SCAPHash *) hSrcHash;

    if(getItemNum(pstDstHash) != 0)
    {
        ERRASSIGNGOTO(result, ERR_CAP_NOT_EMTPY, _EXIT);
    }

    if(getItemNum(pstSrcHash) <= 0)
    {
        ERRASSIGNGOTO(result, ERR_CAP_NO_DATA, _EXIT);
    }

    stUsrData.fnCallback = fnDataCopyCallback;
    stUsrData.pUsrData = pUserData;
    stUsrData.hDstHash = (cap_handle) pstDstHash;

    result = CAPHash_Traverse((cap_handle) pstSrcHash, hashDataTraverse, &stUsrData);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}






