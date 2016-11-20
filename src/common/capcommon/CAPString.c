/*
 * CAPString.c
 *
 *  Created on: 2015. 4. 2.
 *      Author: chjej202
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include <cap_common.h>

#include <CAPString.h>

#define CONST_STR_LEN(str)

//const struct _STCAPString strName_const = { "aaa", sizeof("aaa")-1, sizeof("aaa")-1 };
//const cap_string * strName = &strName_const;

#define CAPSTRING_CHARTYPE char
#define DEFAULT_APPEND_SIZE (64)

// create new string
cap_string CAPString_New()
{
    struct _SCAPString *pstStr = NULL;
    cap_result result = ERR_CAP_UNKNOWN;

    pstStr = (struct _SCAPString *) malloc(sizeof(struct _SCAPString));
    ERRMEMGOTO(pstStr, result, _EXIT);

    pstStr->nBufferLen = 0;
    pstStr->nStringLen = 0;
    pstStr->pszStr = NULL;

    result = ERR_CAP_NOERROR;
_EXIT:
    if(result != ERR_CAP_NOERROR)
    {
        SAFEMEMFREE(pstStr);
    }
    return pstStr;
}

cap_result CAPString_Delete(cap_string *pstrString)
{
    cap_result result = ERR_CAP_UNKNOWN;
    struct _SCAPString *pstStr = NULL;

    IFVARERRASSIGNGOTO(pstrString, NULL, result, ERR_CAP_INVALID_HANDLE, _EXIT);
    IFVARERRASSIGNGOTO(*pstrString, NULL, result, ERR_CAP_INVALID_HANDLE, _EXIT);

    pstStr = (struct _SCAPString *) *pstrString;

    SAFEMEMFREE(pstStr->pszStr);
    SAFEMEMFREE(pstStr);

    *pstrString = NULL;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result CAPString_Set(cap_string strDst, cap_string strSrc)
{
    cap_result result = ERR_CAP_UNKNOWN;
    struct _SCAPString *pstStrSrc = NULL;

    IFVARERRASSIGNGOTO(strSrc, NULL, result, ERR_CAP_INVALID_HANDLE, _EXIT);

    pstStrSrc = strSrc;

    result = CAPString_SetLow(strDst, pstStrSrc->pszStr, pstStrSrc->nBufferLen);

_EXIT:
    return result;
}

cap_result CAPString_SetSub(cap_string strDst, cap_string strSrc, int nSrcIndex, int nSrcLen)
{
    cap_result result = ERR_CAP_UNKNOWN;
    struct _SCAPString *pstStrSrc = NULL;

    IFVARERRASSIGNGOTO(strSrc, NULL, result, ERR_CAP_INVALID_HANDLE, _EXIT);

    pstStrSrc = strSrc;
    if(pstStrSrc->nStringLen - nSrcIndex < nSrcLen)
    {
        nSrcLen = pstStrSrc->nStringLen - nSrcIndex;
    }

    result = CAPString_SetLow(strDst, pstStrSrc->pszStr + nSrcIndex, nSrcLen);

_EXIT:
    return result;
}

cap_result CAPString_SetLow(cap_string strDst, const char *pszSrc, int nSrcBufLen)
{
    cap_result result = ERR_CAP_UNKNOWN;
    int nSrcRealLen = 0;
    int nLoop = 0;
    struct _SCAPString *pstStrDst = NULL;

    IFVARERRASSIGNGOTO(strDst, NULL, result, ERR_CAP_INVALID_HANDLE, _EXIT);

    pstStrDst = strDst;

    if(nSrcBufLen <= 0 || pszSrc == NULL)
    {
        // length is 0
        if(pstStrDst->pszStr != NULL)
        {
            pstStrDst->pszStr[0] = '\0';
            pstStrDst->nStringLen = 0;
        }
        else
        {
            pstStrDst->nStringLen = 0;
        }
    }
    else
    {
        for(nLoop = 0; nLoop < nSrcBufLen ; nLoop++)
        {
            if(pszSrc[nLoop] == '\0')
            {
                break;
            }
        }

        nSrcRealLen = nLoop;

        // memory allocation for new string
        if(nSrcRealLen >= pstStrDst->nBufferLen)
        {
            SAFEMEMFREE(pstStrDst->pszStr);
            pstStrDst->nBufferLen = 0;
            pstStrDst->nStringLen = 0;

            pstStrDst->pszStr = (char *) malloc(nSrcRealLen+1);
            ERRMEMGOTO(pstStrDst->pszStr, result, _EXIT);
            pstStrDst->nBufferLen = nSrcRealLen+1;
        }
        else // there is enough space to copy a source string
        {
            // do nothing
        }

        // copy string
        memcpy(pstStrDst->pszStr, pszSrc, nSrcRealLen*sizeof(CAPSTRING_CHARTYPE));
        pstStrDst->pszStr[nSrcRealLen] = '\0';
        pstStrDst->nStringLen = nSrcRealLen;
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

char *CAPString_LowPtr(cap_string strTarget, int *pnStringLen)
{
    char *pszString = NULL;
    struct _SCAPString *pstString = NULL;
    int nLen = 0;

    IFVARERRASSIGNGOTO(strTarget, NULL, pszString, NULL, _EXIT);

    pstString = strTarget;

    pszString = pstString->pszStr;
    nLen = pstString->nStringLen;

_EXIT:
    if(pnStringLen != NULL) {
        *pnStringLen = nLen;
    }
    return pszString;
}

cap_bool CAPString_StartsWith(cap_string strTarget, cap_string strPrefix)
{
    struct _SCAPString *pstTarget = NULL;
    struct _SCAPString *pstPrefix = NULL;
    cap_bool bRet = FALSE;

    IFVARERRASSIGNGOTO(strTarget, NULL, bRet, FALSE, _EXIT);
    IFVARERRASSIGNGOTO(strPrefix, NULL, bRet, FALSE, _EXIT);

    pstTarget = (struct _SCAPString *) strTarget;
    pstPrefix = (struct _SCAPString *) strPrefix;

    if(pstPrefix->nStringLen > pstTarget->nStringLen)
    {
        CAPASSIGNGOTO(bRet, FALSE, _EXIT);
    }

    if(memcmp(pstTarget->pszStr, pstPrefix->pszStr, pstPrefix->nStringLen*sizeof(CAPSTRING_CHARTYPE)) == 0)
    {
        bRet = TRUE;
    }
    else
    {
        bRet = FALSE;
    }

_EXIT:
    return bRet;
}

cap_bool CAPString_IsEqual(cap_string strCompare1, cap_string strCompare2)
{
    cap_bool bEqual = FALSE;
    struct _SCAPString *pstCompare1 = NULL;
    struct _SCAPString *pstCompare2 = NULL;

    IFVARERRASSIGNGOTO(strCompare1, NULL, bEqual, FALSE, _EXIT);
    IFVARERRASSIGNGOTO(strCompare2, NULL, bEqual, FALSE, _EXIT);

    pstCompare1 = (struct _SCAPString *) strCompare1;
    pstCompare2 = (struct _SCAPString *) strCompare2;

    if(pstCompare1->nStringLen != pstCompare2->nStringLen)
    {
        CAPASSIGNGOTO(bEqual, FALSE, _EXIT);
    }

    if(memcmp(pstCompare1->pszStr, pstCompare2->pszStr, pstCompare1->nStringLen*sizeof(CAPSTRING_CHARTYPE)) == 0)
    {
        bEqual = TRUE;
    }
    else
    {
        bEqual = FALSE;
    }
_EXIT:
    return bEqual;
}

int CAPString_FindChar(cap_string strTarget, IN int nIndex, IN char ch)
{
    int nLoop = 0;
    int nCharIndex = CAPSTR_INDEX_NOT_FOUND;
    struct _SCAPString *pstTarget = NULL;

    IFVARERRASSIGNGOTO(strTarget, NULL, nCharIndex, CAPSTR_INDEX_NOT_FOUND, _EXIT);

    pstTarget = strTarget;

    if(nIndex < 0 || nIndex >= pstTarget->nStringLen)
    {
        CAPASSIGNGOTO(nCharIndex, CAPSTR_INDEX_NOT_FOUND, _EXIT);
    }

    for(nLoop = nIndex ; nLoop < pstTarget->nStringLen; nLoop++)
    {
        if(pstTarget->pszStr[nLoop] == ch)
        {
            break;
        }
    }

    if(nLoop == pstTarget->nStringLen)
    {
        CAPASSIGNGOTO(nCharIndex, CAPSTR_INDEX_NOT_FOUND, _EXIT);
    }

    nCharIndex = nLoop;

_EXIT:
    return nCharIndex;
}

int CAPString_FindRChar(cap_string strTarget, IN int nIndex, IN char ch)
{
    int nLoop = 0;
    int nCharIndex = CAPSTR_INDEX_NOT_FOUND;
    struct _SCAPString *pstTarget = NULL;

    IFVARERRASSIGNGOTO(strTarget, NULL, nCharIndex, CAPSTR_INDEX_NOT_FOUND, _EXIT);

    pstTarget = strTarget;

    if(nIndex < 0 || nIndex >= pstTarget->nStringLen)
    {
        ERRASSIGNGOTO(nCharIndex, CAPSTR_INDEX_NOT_FOUND, _EXIT);
    }

    for(nLoop = pstTarget->nStringLen - 1 - nIndex ; nLoop >= 0; nLoop--)
    {
        if(pstTarget->pszStr[nLoop] == ch)
        {
            break;
        }
    }

    if(nLoop == pstTarget->nStringLen)
    {
        ERRASSIGNGOTO(nCharIndex, CAPSTR_INDEX_NOT_FOUND, _EXIT);
    }

    nCharIndex = nLoop;

_EXIT:
    return nCharIndex;
}

int CAPString_Length(cap_string strTarget)
{
    int nLen = 0;
    struct _SCAPString *pstTarget = NULL;

//    IFVARERRASSIGNGOTO(strTarget, NULL, nLen, 0, _EXIT);
    if(strTarget == NULL)
        return 0;

    pstTarget = (struct _SCAPString *) strTarget;

    nLen = pstTarget->nStringLen;
_EXIT:
    return nLen;
}

int CAPString_ToInteger(cap_string strTarget, int nIndex, OUT int *pnEndIndex)
{
    cap_result result = ERR_CAP_UNKNOWN;
    struct _SCAPString *pstStr = NULL;
    char *pTail = NULL;
    int nValue = 0;

    IFVARERRASSIGNGOTO(strTarget, NULL, result, ERR_CAP_INVALID_HANDLE, _EXIT);

    pstStr = (struct _SCAPString *) strTarget;

    nValue = (int) strtol(pstStr->pszStr + nIndex, &pTail, 10);
    if(nValue == 0 && pstStr->pszStr + nIndex == pTail)
    {
        // not converted
        ERRASSIGNGOTO(result, ERR_CAP_CONVERSION_ERROR, _EXIT);
    }

    if(pnEndIndex != NULL)
    {
        *pnEndIndex = pTail - (pstStr->pszStr + nIndex);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    if(result != ERR_CAP_NOERROR && pnEndIndex != NULL)
    {
        *pnEndIndex = 0;
        nValue = 0;
    }
    return nValue;
}

long long CAPString_ToLongLong(cap_string strTarget, int nIndex, OUT int *pnEndIndex)
{
    cap_result result = ERR_CAP_UNKNOWN;
    struct _SCAPString *pstStr = NULL;
    char *pTail = NULL;
    long long llValue = 0;

    IFVARERRASSIGNGOTO(strTarget, NULL, result, ERR_CAP_INVALID_HANDLE, _EXIT);

    pstStr = (struct _SCAPString *) strTarget;

    llValue = (long long) strtoll(pstStr->pszStr + nIndex, &pTail, 10);
    if(llValue == 0 && pstStr->pszStr + nIndex == pTail)
    {
        // not converted
        ERRASSIGNGOTO(result, ERR_CAP_CONVERSION_ERROR, _EXIT);
    }

    if(pnEndIndex != NULL)
    {
        *pnEndIndex = pTail - (pstStr->pszStr + nIndex);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    if(result != ERR_CAP_NOERROR && pnEndIndex != NULL)
    {
        *pnEndIndex = 0;
        llValue = 0;
    }
    return llValue;
}

double CAPString_ToDouble(cap_string strTarget, int nIndex, OUT int *pnEndIndex)
{
    cap_result result = ERR_CAP_UNKNOWN;
    struct _SCAPString *pstStr = NULL;
    char *pTail = NULL;
    double dbValue = 0;

    IFVARERRASSIGNGOTO(strTarget, NULL, result, ERR_CAP_INVALID_HANDLE, _EXIT);

    pstStr = (struct _SCAPString *) strTarget;

    dbValue = strtod(pstStr->pszStr + nIndex, &pTail);
    if(dbValue == 0 && pstStr->pszStr + nIndex == pTail)
    {
        // not converted
        ERRASSIGNGOTO(result, ERR_CAP_CONVERSION_ERROR, _EXIT);
    }

    if(pnEndIndex != NULL)
    {
        *pnEndIndex = pTail - (pstStr->pszStr + nIndex);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    if(result != ERR_CAP_NOERROR && pnEndIndex != NULL)
    {
        *pnEndIndex = 0;
        dbValue = 0;
    }
    return dbValue;
}

cap_result CAPString_Trim(cap_string strTarget)
{
    cap_result result = ERR_CAP_UNKNOWN;
    int nLoop = 0;
    struct _SCAPString *pstStr = NULL;
    int nSpaces = 0;

    IFVARERRASSIGNGOTO(strTarget, NULL, result, ERR_CAP_INVALID_HANDLE, _EXIT);

    pstStr = (struct _SCAPString *) strTarget;

    // remove spaces from head
    for(nLoop = 0 ; nLoop < pstStr->nStringLen; nLoop++)
    {
        if (isspace((int)pstStr->pszStr[nLoop]) == FALSE)
        {
            break;
        }
    }

    nSpaces = nLoop;

    if(nSpaces > 0)
    {
        memmove(pstStr->pszStr, pstStr->pszStr + nSpaces, pstStr->nStringLen - nSpaces);
        pstStr->nStringLen -= nSpaces;
    }

    nSpaces = 0;

    // remove spaces from tail
    for(nLoop = pstStr->nStringLen - 1 ; nLoop >= 0; nLoop--)
    {
        if (isspace((int)pstStr->pszStr[nLoop]) == FALSE)
        {
            break;
        }
        nSpaces++;
    }

    if(nSpaces > 0)
    {
        pstStr->nStringLen -= nSpaces;
    }

    pstStr->pszStr[pstStr->nStringLen] = '\0';

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result CAPString_SetLength(cap_string strTarget, int nLength)
{
    cap_result result = ERR_CAP_UNKNOWN;
    struct _SCAPString *pstStr = NULL;
    char *pszNewStr = NULL;

    IFVARERRASSIGNGOTO(strTarget, NULL, result, ERR_CAP_INVALID_HANDLE, _EXIT);

    pstStr = (struct _SCAPString *) strTarget;

    if(nLength <= pstStr->nStringLen)
    {
        pstStr->pszStr[nLength] = '\0';
        pstStr->nStringLen = nLength;
    }
    else // nLength > pstStr->nStringLen
    {
        // reallocation is needed
        if(nLength >= pstStr->nBufferLen)
        {
            pszNewStr = (char *)malloc(sizeof(CAPSTRING_CHARTYPE)*(nLength+1));
            ERRMEMGOTO(pszNewStr, result, _EXIT);

            memcpy(pszNewStr, pstStr->pszStr, sizeof(CAPSTRING_CHARTYPE)*nLength);
            pszNewStr[nLength] = '\0';

            SAFEMEMFREE(pstStr->pszStr);
            pstStr->pszStr = pszNewStr;
            pstStr->nBufferLen = nLength+1;
        }

        // set rest of string to white space
        memset(pstStr->pszStr + pstStr->nStringLen, 0x20, (nLength - pstStr->nStringLen)*sizeof(CAPSTRING_CHARTYPE));
        pstStr->nStringLen = nLength;
        pstStr->pszStr[pstStr->nStringLen] = '\0';
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result CAPString_AppendLow(cap_string strDst, char *pszSrc, int nSrcBufLen)
{
    cap_result result = ERR_CAP_UNKNOWN;
    struct _SCAPString *pstStrDst = NULL;
    int nSrcRealLen = 0;
    int nLoop = 0;
    int nNewBufLen = 0;
    int nNewStringLen = 0;
    char *pszNewStr = NULL;

    IFVARERRASSIGNGOTO(strDst, NULL, result, ERR_CAP_INVALID_HANDLE, _EXIT);

    pstStrDst = (struct _SCAPString *) strDst;

    if(nSrcBufLen <= 0 || pszSrc == NULL)
    {
        // do nothing
    }
    else
    {
        for(nLoop = 0; nLoop < nSrcBufLen ; nLoop++)
        {
            if(pszSrc[nLoop] == '\0')
            {
                break;
            }
        }

        nSrcRealLen = nLoop;
        nNewStringLen = pstStrDst->nStringLen + nSrcRealLen;

        // memory allocation for new string
        if(nNewStringLen >= pstStrDst->nBufferLen)
        {
            if(pstStrDst->nBufferLen > 0)
            {
                nNewBufLen = pstStrDst->nBufferLen * (nNewStringLen/(pstStrDst->nBufferLen) + 1) + 1;
            }
            else // pstStrDst->nBufferLen == 0
            {
                nNewBufLen = DEFAULT_APPEND_SIZE * (nNewStringLen/DEFAULT_APPEND_SIZE + 1) + 1;
            }

            pszNewStr = (char *)malloc(nNewBufLen);
            ERRMEMGOTO(pszNewStr, result, _EXIT);

            memcpy(pszNewStr, pstStrDst->pszStr, sizeof(CAPSTRING_CHARTYPE)*pstStrDst->nStringLen);
            pszNewStr[pstStrDst->nStringLen] = '\0';

            SAFEMEMFREE(pstStrDst->pszStr);
            pstStrDst->nBufferLen = 0;

            pstStrDst->pszStr = pszNewStr;
            pstStrDst->nBufferLen = nNewBufLen;
        }
        else // there is enough space to append source string
        {
            // do nothing
        }

        // copy string
        memcpy(pstStrDst->pszStr + pstStrDst->nStringLen, pszSrc, nSrcRealLen*sizeof(CAPSTRING_CHARTYPE));
        pstStrDst->pszStr[nNewStringLen] = '\0';
        pstStrDst->nStringLen = nNewStringLen;
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result CAPString_AppendString(cap_string strDst, cap_string strSrc)
{
    cap_result result = ERR_CAP_UNKNOWN;
    struct _SCAPString *pstStrSrc = NULL;
    char *pszSrc = NULL;
    int nSrcBufLen = 0;

    IFVARERRASSIGNGOTO(strDst, NULL, result, ERR_CAP_INVALID_HANDLE, _EXIT);
    IFVARERRASSIGNGOTO(strSrc, NULL, result, ERR_CAP_INVALID_HANDLE, _EXIT);

    pstStrSrc = (struct _SCAPString *) strSrc;

    nSrcBufLen = pstStrSrc->nStringLen;
    pszSrc = pstStrSrc->pszStr;

    result = CAPString_AppendLow(strDst, pszSrc, nSrcBufLen);
    ERRIFGOTO(result, _EXIT);

_EXIT:
    return result;
}

cap_result CAPString_VPrintFormat(cap_string strTarget, char *pszFormat, va_list argList)
{
    cap_result result = ERR_CAP_UNKNOWN;
    struct _SCAPString *pstStr = NULL;
    int nLength = 0;
    va_list b_list;
    char ch = '\0';

    IFVARERRASSIGNGOTO(strTarget, NULL, result, ERR_CAP_INVALID_HANDLE, _EXIT);
    IFVARERRASSIGNGOTO(pszFormat, NULL, result, ERR_CAP_INVALID_HANDLE, _EXIT);

    pstStr = (struct _SCAPString *) strTarget;

    va_copy(b_list, argList);

    // check the length first
    nLength = vsnprintf(&ch, 1, pszFormat, argList);

    if(nLength >= pstStr->nBufferLen) {
        SAFEMEMFREE(pstStr->pszStr);
        pstStr->nBufferLen = 0;
        pstStr->nStringLen = 0;

        pstStr->pszStr = (char *) malloc(nLength+1);
        ERRMEMGOTO(pstStr->pszStr, result, _EXIT);

        pstStr->nBufferLen = nLength+1;
    }

    nLength = vsnprintf(pstStr->pszStr, pstStr->nBufferLen, pszFormat, b_list);

    va_end(b_list);

    pstStr->nStringLen = nLength;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result CAPString_PrintFormat(cap_string strTarget, char *pszFormat, ...)
{
    cap_result result = ERR_CAP_UNKNOWN;
    va_list a_list;

    IFVARERRASSIGNGOTO(strTarget, NULL, result, ERR_CAP_INVALID_HANDLE, _EXIT);
    IFVARERRASSIGNGOTO(pszFormat, NULL, result, ERR_CAP_INVALID_HANDLE, _EXIT);

    va_start(a_list, pszFormat);
    result = CAPString_VPrintFormat(strTarget, pszFormat, a_list);
    va_end(a_list);

    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result CAPString_AppendFormat(cap_string strTarget, char *pszFormat, ...)
{
    cap_result result = ERR_CAP_UNKNOWN;
    struct _SCAPString *pstStr = NULL;
    int nLength = 0;
    va_list a_list;
    va_list b_list;
    char ch = '\0';
    int nNewBufferLen = 0;
    char *pszNewStr = NULL;

    IFVARERRASSIGNGOTO(strTarget, NULL, result, ERR_CAP_INVALID_HANDLE, _EXIT);
    IFVARERRASSIGNGOTO(pszFormat, NULL, result, ERR_CAP_INVALID_HANDLE, _EXIT);

    pstStr = (struct _SCAPString *) strTarget;

    va_start(a_list, pszFormat);

    va_copy(b_list, a_list);

    // check the length first
    nLength = vsnprintf(&ch, 1, pszFormat, a_list);

    if(pstStr->nStringLen + nLength >= pstStr->nBufferLen)
    {
        if(pstStr->nBufferLen > 0)
        {
            nNewBufferLen = pstStr->nBufferLen * ((pstStr->nStringLen + nLength)/pstStr->nBufferLen + 1) + 1;
        }
        else // pstStrDst->nBufferLen == 0
        {
            nNewBufferLen = DEFAULT_APPEND_SIZE * ((pstStr->nStringLen + nLength)/DEFAULT_APPEND_SIZE + 1) + 1;
        }

        pszNewStr = (char *) malloc(nNewBufferLen);
        ERRMEMGOTO(pszNewStr, result, _EXIT);

        memcpy(pszNewStr, pstStr->pszStr, sizeof(CAPSTRING_CHARTYPE)*pstStr->nStringLen);
        pszNewStr[pstStr->nStringLen] = '\0';

        SAFEMEMFREE(pstStr->pszStr);
        pstStr->nBufferLen = 0;

        pstStr->pszStr = pszNewStr;
        pstStr->nBufferLen = nNewBufferLen;
    }

    nLength = vsnprintf(pstStr->pszStr + pstStr->nStringLen, pstStr->nBufferLen - pstStr->nStringLen, pszFormat, b_list);

    va_end(b_list);
    va_end(a_list);

    pstStr->nStringLen = pstStr->nStringLen + nLength;
    pstStr->pszStr[pstStr->nStringLen] = '\0';

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result CAPString_ReplaceChar(cap_string strTarget, char chFrom, char chTo)
{
    cap_result result = ERR_CAP_UNKNOWN;
    int nIndex = 0;
    int nResultIndex = 0;
    int nStringLen = 0;
    struct _SCAPString *pstStr = NULL;

    IFVARERRASSIGNGOTO(strTarget, NULL, result, ERR_CAP_INVALID_HANDLE, _EXIT);
    IFVARERRASSIGNGOTO(chTo, '\0', result, ERR_CAP_INVALID_PARAM, _EXIT);

    nStringLen = CAPString_Length(strTarget);

    while(nIndex < nStringLen)
    {
        nResultIndex = CAPString_FindChar(strTarget, nIndex, chFrom);
        if(nResultIndex < 0)
        {
            break;
        }

        pstStr = strTarget;

        pstStr->pszStr[nResultIndex] = chTo;

        nIndex = nResultIndex + 1;
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result CAPString_ReplaceString(cap_string strTarget, cap_string strFrom, cap_string strTo)
{
    cap_result result = ERR_CAP_UNKNOWN;

    IFVARERRASSIGNGOTO(strTarget, NULL, result, ERR_CAP_INVALID_HANDLE, _EXIT);
    IFVARERRASSIGNGOTO(strFrom, NULL, result, ERR_CAP_INVALID_HANDLE, _EXIT);
    IFVARERRASSIGNGOTO(strTo, NULL, result, ERR_CAP_INVALID_HANDLE, _EXIT);


    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result CAPString_AppendChar(cap_string strDst, char ch)
{
    cap_result result = ERR_CAP_UNKNOWN;
    struct _SCAPString *pstStrDst = NULL;
    int nNewBufLen = 0;
    int nNewStringLen = 0;
    char *pszNewStr = NULL;

    IFVARERRASSIGNGOTO(strDst, NULL, result, ERR_CAP_INVALID_HANDLE, _EXIT);

    pstStrDst = (struct _SCAPString *) strDst;

    nNewStringLen = pstStrDst->nStringLen + 1;

    // memory allocation for new string
    if(nNewStringLen >= pstStrDst->nBufferLen)
    {
        if(pstStrDst->nBufferLen > 0)
        {
            nNewBufLen = pstStrDst->nBufferLen * (nNewStringLen/(pstStrDst->nBufferLen) + 1) + 1;
        }
        else // pstStrDst->nBufferLen == 0
        {
            nNewBufLen = DEFAULT_APPEND_SIZE * (nNewStringLen/DEFAULT_APPEND_SIZE + 1) + 1;
        }

        pszNewStr = (char *) malloc(nNewBufLen);
        ERRMEMGOTO(pszNewStr, result, _EXIT);

        memcpy(pszNewStr, pstStrDst->pszStr, sizeof(CAPSTRING_CHARTYPE)*pstStrDst->nStringLen);
        pszNewStr[pstStrDst->nStringLen] = '\0';

        SAFEMEMFREE(pstStrDst->pszStr);
        pstStrDst->nBufferLen = 0;

        pstStrDst->pszStr = pszNewStr;
        pstStrDst->nBufferLen = nNewBufLen;
    }
    else // there is enough space to append source string
    {
        // do nothing
    }

    // copy char
    pstStrDst->pszStr[pstStrDst->nStringLen] = ch;
    pstStrDst->pszStr[nNewStringLen] = '\0';
    pstStrDst->nStringLen = nNewStringLen;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

int CAPString_FindString(cap_string strTarget, IN int nIndex, IN cap_string strToFind)
{
    int nLoop = 0;
    int nCharIndex = CAPSTR_INDEX_NOT_FOUND;
    struct _SCAPString *pstTarget = NULL;
    struct _SCAPString *pstToFind = NULL;
    int nSrcIndex = 0;

    IFVARERRASSIGNGOTO(strTarget, NULL, nCharIndex, CAPSTR_INDEX_NOT_FOUND, _EXIT);
    IFVARERRASSIGNGOTO(strToFind, NULL, nCharIndex, CAPSTR_INDEX_NOT_FOUND, _EXIT);

    pstTarget = strTarget;
    pstToFind = strToFind;

    if(nIndex < 0 || nIndex >= pstTarget->nStringLen || pstToFind->nStringLen <= 0 ||
             pstToFind->nStringLen > pstTarget->nStringLen - nIndex)
    {
        CAPASSIGNGOTO(nCharIndex, CAPSTR_INDEX_NOT_FOUND, _EXIT);
    }

    for(nLoop = nIndex ; nLoop < pstTarget->nStringLen - pstToFind->nStringLen + nSrcIndex + 1; nLoop++)
    {
        if(pstTarget->pszStr[nLoop] != pstToFind->pszStr[nSrcIndex])
        {
            nSrcIndex = 0;
            continue;
        }
        else
        {
            nSrcIndex++;
            if(pstToFind->nStringLen == nSrcIndex)
            {
                break;
            }
        }
    }

    if(pstToFind->nStringLen != nSrcIndex)
    {
        CAPASSIGNGOTO(nCharIndex, CAPSTR_INDEX_NOT_FOUND, _EXIT);
    }

    nCharIndex = nLoop - pstToFind->nStringLen + 1;

_EXIT:
    return nCharIndex;
}


