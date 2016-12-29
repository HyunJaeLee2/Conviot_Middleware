/*
 * CAPBase64.c
 *
 *  Created on: 2016. 4. 22.
 *      Author: kangdongh
 */

#include <stdio.h>

#include <CAPBase64.h>

static const unsigned char cap_decode_base64[256] =
{
    /* ASCII table */
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
    64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
    64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};

static const char cap_base64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

typedef union{
	struct{
#ifdef LITTLE_ENDIAN
		unsigned char c1, c2, c3;
#else
		unsigned char c3, c2, c1;
#endif
	};
	struct{
#ifdef LITTLE_ENDIAN
		unsigned int e1:6, e2:6, e3:6, e4:6;
#else
		unsigned int e4:6, e3:6, e2:6, e1:6;
#endif
	};
} BF;

cap_result CAPBase64_Encode_Len(IN int nLen, OUT int *pnEncodedLen)
{
	cap_result result = ERR_CAP_UNKNOWN;
	
	IFVARERRASSIGNGOTO(pnEncodedLen, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

	*pnEncodedLen = ((nLen + 2) / 3 * 4) + 1;

	result = ERR_CAP_NOERROR;
_EXIT:
	return result;
}

cap_result CAPBase64_Encode(IN char *pData, IN int nDataLen, OUT char** ppEncodedData, OUT int* pnEncodedLen)
{
	int ni = 0;
	int nj = 0;
	BF stemp;
	char *pBuffer = NULL;
	int nEncodeLen = 0;
	cap_result result = ERR_CAP_UNKNOWN;
	IFVARERRASSIGNGOTO(pData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
	
	if(nDataLen < 0)
	{
		ERRASSIGNGOTO(result, ERR_CAP_INVALID_PARAM, _EXIT);
	}

	IFVARERRASSIGNGOTO(ppEncodedData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
	
	IFVARERRASSIGNGOTO(pnEncodedLen, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

	result = CAPBase64_Encode_Len(nDataLen, &nEncodeLen);
	if(result != ERR_CAP_NOERROR)
	{
		goto _EXIT;
	}
	pBuffer = (char *)malloc(nEncodeLen * sizeof(char));
	ERRMEMGOTO(pBuffer, result, _EXIT);
	*ppEncodedData = pBuffer;

	for(ni = 0 ; ni < nDataLen ; ni = ni+3, nj = nj+4){
		stemp.c3 = pData[ni];
		if( ni > nDataLen - 1 ) stemp.c2 = 0;
		else stemp.c2 = pData[ni+1];
		if( ni > nDataLen - 2 ) stemp.c1 = 0;
		else stemp.c1 = pData[ni+2];

		pBuffer[nj] = cap_base64[stemp.e4];
		pBuffer[nj+1] = cap_base64[stemp.e3];
		pBuffer[nj+2] = cap_base64[stemp.e2];
		pBuffer[nj+3] = cap_base64[stemp.e1];

		if( ni > nDataLen -2) pBuffer[nj+2] = '=';
		if( ni > nDataLen -3) pBuffer[nj+3] = '=';
	}
	pBuffer[nj] = '\0';
	
	*pnEncodedLen = nEncodeLen;

	result = ERR_CAP_NOERROR;
_EXIT:
	return result;
}

cap_result CAPBase64_Decode_Len(IN char *pEncodedData, OUT int* pnDecodedLen)
{
	int nBytesDecoded = 0;
	int nPrbytes = 0;
	unsigned char *pBufferIn = NULL;

	cap_result result = ERR_CAP_UNKNOWN;
	IFVARERRASSIGNGOTO(pEncodedData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
	
	IFVARERRASSIGNGOTO(pnDecodedLen, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
	
	pBufferIn = (unsigned char*) pEncodedData;
	while(cap_decode_base64[*(pBufferIn++)] <= 63);
	nPrbytes = (pBufferIn - (unsigned char *)pEncodedData) - 1;
	nBytesDecoded = ((nPrbytes + 3) / 4) * 3;

	*pnDecodedLen = nBytesDecoded;
	result = ERR_CAP_NOERROR;
_EXIT:
	return result;
}

cap_result CAPBase64_Decode(IN char *pEncodedData, OUT char **ppDecodedData, OUT int *pnDecodedLen)
{
	cap_result result = ERR_CAP_UNKNOWN;
	char *pBufferOut = NULL;
    int nBytesEncoded = 0;
	int nBytesDecoded = 0;
	int ni = 0;
	int nj = 0;
	int nblank = 0;
	BF stemp;
	IFVARERRASSIGNGOTO(pEncodedData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
	
	IFVARERRASSIGNGOTO(ppDecodedData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
	
	IFVARERRASSIGNGOTO(pnDecodedLen, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

	result = CAPBase64_Decode_Len(pEncodedData, &nBytesDecoded);
	if( result != ERR_CAP_NOERROR ){
		goto _EXIT;
	}
	else result = ERR_CAP_UNKNOWN;

    result = CAPBase64_Encode_Len(nBytesDecoded, &nBytesEncoded);
	if( result != ERR_CAP_NOERROR ){
		goto _EXIT;
	}
	else result = ERR_CAP_UNKNOWN;

	pBufferOut = (char*)malloc((nBytesEncoded+1) * sizeof(char));
	ERRMEMGOTO(pBufferOut, result, _EXIT);
	
	for(ni = 0; ni < nBytesEncoded; ni = ni+4, nj = nj+3)
	{
		stemp.e4 = cap_decode_base64[(unsigned char)pEncodedData[ni]];
		stemp.e3 = cap_decode_base64[(unsigned char)pEncodedData[ni+1]];
		if(pEncodedData[ni+2] == '='){
			stemp.e2 = 0;
			nblank++;
		}
		else stemp.e2 = cap_decode_base64[(unsigned char)pEncodedData[ni+2]];
		if(pEncodedData[ni+3] == '='){
			stemp.e1 = 0;
			nblank++;
		}
		else stemp.e1 = cap_decode_base64[(unsigned char)pEncodedData[ni+3]];

		pBufferOut[nj] = stemp.c3;
		pBufferOut[nj+1] = stemp.c2;
		pBufferOut[nj+2] = stemp.c1;
	}
	*pnDecodedLen = nBytesDecoded - nblank;
	pBufferOut[nBytesDecoded - nblank] = '\0';
    *ppDecodedData = pBufferOut;

	result = ERR_CAP_NOERROR;
_EXIT:
	if(result != ERR_CAP_NOERROR)
	{
		if(pBufferOut != NULL)
		{
			free(pBufferOut);
		}
	}
	return result;
}

