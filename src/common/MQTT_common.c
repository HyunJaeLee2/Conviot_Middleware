#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "MQTT_common.h"

#include "CAPThread.h"
#include "CAPLinkedList.h"

#define TOPIC_SEPATAOR '/'

CALLBACK cap_result MQTTData_Destroy(void* pData, void* pUsrData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SMQTTData* pstMQTTData = NULL;

    IFVARERRASSIGNGOTO(pData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
        
    pstMQTTData = (SMQTTData *)pData;

    SAFEMEMFREE(pstMQTTData->pszTopic);
    SAFEMEMFREE(pstMQTTData->pszPayload);
    SAFEMEMFREE(pstMQTTData);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

CALLBACK cap_result topicNameDataDestroy(int nOffset, void *pData, void *pUsrData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    cap_string strData = NULL;

    IFVARERRASSIGNGOTO(pData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    strData = (cap_string) pData;

    SAFE_CAPSTRING_DELETE(strData);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result getLastElementFromTopicList(cap_handle hTopicLinkedList, cap_string *pstrLastElement) {                   
    cap_result result = ERR_CAP_UNKNOWN;                                                                             
    cap_string strDeviceId = NULL;                                                                                   
    
    IFVARERRASSIGNGOTO(hTopicLinkedList, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);                                
    IFVARERRASSIGNGOTO(pstrLastElement, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);                                 
    
    result = CAPLinkedList_Get(hTopicLinkedList, LINKED_LIST_OFFSET_LAST, -1, (void **) &strDeviceId);               
    ERRIFGOTO(result, _EXIT);                                                                                        
    
    *pstrLastElement = strDeviceId;                                                                                  
    
    result = ERR_CAP_NOERROR;                                                                                        
_EXIT:
    return result;                                                                                                   
}     

cap_result assignLastElementFromTopicList(cap_handle hTopicLinkedList, OUT char **ppDeviceId, OUT int *pnDeviceIdBufSize) {
    cap_result result = ERR_CAP_UNKNOWN;
    cap_string strDeviceId = NULL;
    int nStringLen = 0;
    char *pszTemp = NULL;
    char *pszDeviceId = NULL;

    IFVARERRASSIGNGOTO(hTopicLinkedList, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(ppDeviceId, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(pnDeviceIdBufSize, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    result = CAPLinkedList_Get(hTopicLinkedList, LINKED_LIST_OFFSET_LAST, -1, (void **) &strDeviceId);
    ERRIFGOTO(result, _EXIT);

    nStringLen = CAPString_Length(strDeviceId);

    pszDeviceId = malloc((nStringLen+1)*sizeof(char));
    ERRMEMGOTO(pszDeviceId, result, _EXIT);

    pszTemp = CAPString_LowPtr(strDeviceId, NULL);
    ERRMEMGOTO(pszTemp, result, _EXIT);

    memcpy(pszDeviceId, pszTemp, sizeof(char)*nStringLen);
    pszDeviceId[nStringLen] = '\0';

    *ppDeviceId = pszDeviceId;
    *pnDeviceIdBufSize = nStringLen+1;

    result = ERR_CAP_NOERROR;
_EXIT:
    if (result != ERR_CAP_NOERROR) {
        SAFEMEMFREE(pszDeviceId);
    }
    return result;
}

cap_result assignIdentifierFromTopicList(cap_handle hTopicLinkedList, OUT char **ppIdentifier, OUT int *pnIdentifierBufSize) {
    cap_result result = ERR_CAP_UNKNOWN;
    cap_string strIdentifier = NULL;
    int nStringLen = 0;
    char *pszTemp = NULL;
    char *pszIdentifier = NULL;

    IFVARERRASSIGNGOTO(hTopicLinkedList, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(ppIdentifier, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(pnIdentifierBufSize, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    result = CAPLinkedList_Get(hTopicLinkedList, LINKED_LIST_OFFSET_LAST, -2, (void **) &strIdentifier);
    ERRIFGOTO(result, _EXIT);

    nStringLen = CAPString_Length(strIdentifier);

    pszIdentifier = malloc((nStringLen+1)*sizeof(char));
    ERRMEMGOTO(pszIdentifier, result, _EXIT);

    pszTemp = CAPString_LowPtr(strIdentifier, NULL);
    ERRMEMGOTO(pszTemp, result, _EXIT);

    memcpy(pszIdentifier, pszTemp, sizeof(char)*nStringLen);
    pszIdentifier[nStringLen] = '\0';

    *ppIdentifier = pszIdentifier;
    *pnIdentifierBufSize = nStringLen+1;

    result = ERR_CAP_NOERROR;
_EXIT:
    if (result != ERR_CAP_NOERROR) {
        SAFEMEMFREE(pszIdentifier);
    }
    return result;
}

cap_result divideTopicNameToList(IN cap_string strTopic, IN OUT cap_handle hLinkedList) {
    cap_result result = ERR_CAP_UNKNOWN;
    int nLinkedListSize = 0;
    int nLinkedListIndex = 0;
    cap_string strTemp = NULL;
    cap_string strToken = NULL;
    int nTopicStrIndex= 0;
    int nSeparatorIndex = 0;

    result = CAPLinkedList_GetLength(hLinkedList, &nLinkedListSize);
    ERRIFGOTO(result, _EXIT);

    while(nSeparatorIndex != CAPSTR_INDEX_NOT_FOUND) {
        strToken = CAPString_New();
        ERRMEMGOTO(strToken, result, _EXIT);

        nSeparatorIndex = CAPString_FindChar(strTopic, nTopicStrIndex, TOPIC_SEPATAOR);
        if (nSeparatorIndex == CAPSTR_INDEX_NOT_FOUND) {
            // TOPIC_SEPATAOR not found
            result = CAPString_SetSub(strToken, strTopic, nTopicStrIndex, CAPSTRING_MAX);
            ERRIFGOTO(result, _EXIT);
        } else {
            result = CAPString_SetSub(strToken, strTopic, nTopicStrIndex, nSeparatorIndex - nTopicStrIndex);
            ERRIFGOTO(result, _EXIT);
        }

        if (nLinkedListIndex < nLinkedListSize) {
            // delete an existing data to reuse the node
            result = CAPLinkedList_Get(hLinkedList, LINKED_LIST_OFFSET_FIRST, nLinkedListIndex, (void **) &strTemp);
            ERRIFGOTO(result, _EXIT);

            SAFE_CAPSTRING_DELETE(strTemp);

            result = CAPLinkedList_Set(hLinkedList, LINKED_LIST_OFFSET_FIRST, nLinkedListIndex, strToken);
            ERRIFGOTO(result, _EXIT);
        } else {
            result = CAPLinkedList_Add(hLinkedList, LINKED_LIST_OFFSET_LAST, 0, strToken);
            ERRIFGOTO(result, _EXIT);

            // update nLinkedListSize
            result = CAPLinkedList_GetLength(hLinkedList, &nLinkedListSize);
            ERRIFGOTO(result, _EXIT);
        }
        nLinkedListIndex++;

        // set NULL to not free inserted strToken
        strToken = NULL;

        // points the index next to TOPIC_SEPARATOR
        if (nSeparatorIndex != CAPSTR_INDEX_NOT_FOUND) {
            nTopicStrIndex = nSeparatorIndex + 1;
        }
    }

    // remove the remaining nodes
    if (nLinkedListIndex < nLinkedListSize) {
        // use the nLinkedListIndex as a loop variable
        for(; nLinkedListIndex < nLinkedListSize; nLinkedListIndex++) {
            result = CAPLinkedList_Get(hLinkedList, LINKED_LIST_OFFSET_LAST, -1, (void **) &strTemp);
            ERRIFGOTO(result, _EXIT);

            SAFE_CAPSTRING_DELETE(strTemp);

            result = CAPLinkedList_Remove(hLinkedList, LINKED_LIST_OFFSET_LAST, -1);
            ERRIFGOTO(result, _EXIT);
        }
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFE_CAPSTRING_DELETE(strToken);
    return result;
}
