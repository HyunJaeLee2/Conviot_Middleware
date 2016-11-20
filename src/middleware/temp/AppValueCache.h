/*
 * AppValueCache.h
 *
 *  Created on: 2016. 8. 10.
 *      Author: chjej202
 */

#ifndef APPVALUECACHE_H_
#define APPVALUECACHE_H_

#include <CAPString.h>

#include <capiot_common.h>

#include "ThingHandler.h"

#ifdef __cplusplus
extern "C" {
#endif

cap_result AppValueCache_Create(int nMaxValueNum, OUT cap_handle *phCache);
cap_result AppValueCache_Add(cap_handle hCache, IN cap_string strKey, IN SValue *pstValue, IN double *pdbInitValue);
cap_result AppValueCache_GetBucketNumber(cap_handle hCache, OUT int *pnBucketNum);
cap_result AppValueCache_DuplicateIndexHash(cap_handle hCache, IN OUT cap_handle hIndexHash);
cap_result AppValueCache_UpdateByIndex(cap_handle hCache, IN int nIndex, IN double dbValue);
cap_result AppValueCache_GetByIndex(cap_handle hCache, IN int nIndex, OUT EValueType *penType, OUT double *pdbValue);
cap_result AppValueCache_Destroy(IN OUT cap_handle *phCache);

#ifdef __cplusplus
}
#endif

#endif /* APPVALUECACHE_H_ */
