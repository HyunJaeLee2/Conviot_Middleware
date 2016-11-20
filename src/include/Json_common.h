#ifndef JSON_COMMON_H_
#define JSON_COMMON_H_

#include "capiot_common.h"
#include <json-c/json_object.h> 
#include <json-c/json_tokener.h> 

cap_result ParsingJson(OUT json_object **ppJsonObject, IN char *pszJsonBuffer, IN int nJsonBufSize);

#endif
