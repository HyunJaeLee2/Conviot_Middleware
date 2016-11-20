#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <CAPString.h>

#include "ThingHandler.h"

#include <Json_common.h>
#include <json-c/json_object.h>
#include <json-c/json_tokener.h>

static cap_result insertOneValue(SValue* pstValue, json_object* pJsonValue)
{
    cap_result result = ERR_CAP_UNKNOWN;
    json_object* pJsonObject = NULL, * pJsonBound = NULL;
    const char* pszName = "name", * pszType = "type", * pszBound = "bound";
    const char* pszMin = "min_value", * pszMax = "max_value", * pszFormat = "format";

    IFVARERRASSIGNGOTO(pstValue, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(pJsonValue, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    // Insert name
    if (!json_object_object_get_ex(pJsonValue, pszName, &pJsonObject)) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }
    pstValue->strValueName = CAPString_New();
    ERRMEMGOTO(pstValue->strValueName, result, _EXIT);

    result = CAPString_SetLow(pstValue->strValueName,
            json_object_get_string(pJsonObject), json_object_get_string_len(pJsonObject));
    ERRIFGOTO(result, _EXIT);

    // Insert Type
    if (!json_object_object_get_ex(pJsonValue, pszType, &pJsonObject)) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }
    const char* pszTemp = json_object_get_string(pJsonObject);
    if (0 == strncmp("int", pszTemp, 3)) {
        pstValue->enType = VALUE_TYPE_INT;
    }
    else if (0 == strncmp("double", pszTemp, 6)) {
        pstValue->enType = VALUE_TYPE_DOUBLE;
    }
    else if (0 == strncmp("bool", pszTemp, 4)) {
        pstValue->enType = VALUE_TYPE_BOOL;
    }
    else if (0 == strncmp("binary", pszTemp, 6)) {
        pstValue->enType = VALUE_TYPE_BINARY;
    }
    else {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }

    // Insert Bound
    if (!json_object_object_get_ex(pJsonValue, pszBound, &pJsonObject)) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }
    if (!json_object_object_get_ex(pJsonObject, pszMin, &pJsonBound)) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }
    pstValue->dbMinValue = json_object_get_double(pJsonBound);
    if (!json_object_object_get_ex(pJsonObject, pszMax, &pJsonBound)) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }
    pstValue->dbMaxValue = json_object_get_double(pJsonBound);

    if(pstValue->enType == VALUE_TYPE_BINARY)
    {
        if (!json_object_object_get_ex(pJsonValue, pszFormat, &pJsonObject)) {
            ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
        }
        
        pstValue->pszFormat = strdup(json_object_get_string(pJsonObject));
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

static cap_result insertOneFunc(SFunction* pstFunc, json_object* pJsonFunc)
{
    cap_result result = ERR_CAP_UNKNOWN;
    json_object* pJsonObject = NULL, * pJsonBound = NULL;
    const char* pszName = "name", * pszUseArg = "useArg", * pszFuncArgType = "functionArgType", * pszFuncArgBound = "functionArgBound";
    const char* pszMin = "min_value", * pszMax = "max_value";

    IFVARERRASSIGNGOTO(pstFunc, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(pJsonFunc, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if (!json_object_object_get_ex(pJsonFunc, pszName, &pJsonObject)) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }

    pstFunc->strFunctionName = CAPString_New();
    ERRMEMGOTO(pstFunc->strFunctionName, result, _EXIT);

    result = CAPString_SetLow(pstFunc->strFunctionName,
            json_object_get_string(pJsonObject), json_object_get_string_len(pJsonObject));
    ERRIFGOTO(result, _EXIT);

    if (!json_object_object_get_ex(pJsonFunc, pszUseArg, &pJsonObject)) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
	}
	pstFunc->bUseArgument = json_object_get_int(pJsonObject);
	
	if(pstFunc->bUseArgument == 0){
		//do nothing
	} else if(pstFunc->bUseArgument == 1){
		//SAFEJSONFREE(pJsonObject);

		if (!json_object_object_get_ex(pJsonFunc, pszFuncArgType, &pJsonObject)) {
			ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
		}
		const char* pszTemp1 = json_object_get_string(pJsonObject);
		if (0 == strncmp("int", pszTemp1, 3)) {
			pstFunc->enArgumentType = VALUE_TYPE_INT;
		}
		else if (0 == strncmp("double", pszTemp1, 6)) {
			pstFunc->enArgumentType = VALUE_TYPE_DOUBLE;
		}
		else if (0 == strncmp("bool", pszTemp1, 4)) {
			pstFunc->enArgumentType = VALUE_TYPE_BOOL;
		}
        else if( 0 == strncmp("string", pszTemp1, 6)){
			pstFunc->enArgumentType = VALUE_TYPE_STRING;
        }
		else {
			ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
		}
		//SAFEJSONFREE(pJsonObject);
       
        //if type is string, function argument bound is optional
        if(0 != strncmp("string", pszTemp1, 6)){
            if (!json_object_object_get_ex(pJsonFunc, pszFuncArgBound, &pJsonObject)) {
                ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
            }

            if (!json_object_object_get_ex(pJsonObject, pszMin, &pJsonBound)) {
                ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
            }

            pstFunc->dbArgMinValue = json_object_get_double(pJsonBound);
            
            if (!json_object_object_get_ex(pJsonObject, pszMax, &pJsonBound)) {
                ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
            }

            pstFunc->dbArgMaxValue = json_object_get_double(pJsonBound);
        } else {
            if (json_object_object_get_ex(pJsonFunc, pszFuncArgBound, &pJsonObject)) {
                if (!json_object_object_get_ex(pJsonObject, pszMin, &pJsonBound)) {
                    ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
                }

                pstFunc->dbArgMinValue = json_object_get_double(pJsonBound);
                
                if (!json_object_object_get_ex(pJsonObject, pszMax, &pJsonBound)) {
                    ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
                }
                
                pstFunc->dbArgMaxValue = json_object_get_double(pJsonBound);
            }
        }
    } else {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}
cap_result ThingHandler_Create(IN const char* pszThingId, IN int nIdLength, OUT SThingInfo** ppstThing)
{

    cap_result result = ERR_CAP_UNKNOWN;
    SThingInfo* pstThing = NULL;

    IFVARERRASSIGNGOTO(ppstThing, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstThing = (SThingInfo*)malloc(sizeof(SThingInfo));
    ERRMEMGOTO(pstThing, result, _EXIT);

    pstThing->nFunctionNum = 0;
    pstThing->nIDInDB = 0;
    pstThing->strThingId = NULL;
    pstThing->strThingClass = NULL;
    pstThing->strDescription = NULL;
    pstThing->nAliveCycle = 0;
    pstThing->nValueNum = 0;
    pstThing->pstFunctions = NULL;
    pstThing->pstValues = NULL;

    if (pszThingId != NULL && nIdLength > 0) {
        pstThing->strThingId = CAPString_New();
        ERRMEMGOTO(pstThing->strThingId, result, _EXIT);
        result = CAPString_SetLow(pstThing->strThingId, pszThingId, nIdLength);
        ERRIFGOTO(result, _EXIT);
    }

    pstThing->strDescription = CAPString_New();
    ERRMEMGOTO(pstThing->strDescription, result, _EXIT);
    
    pstThing->strThingClass = CAPString_New();
    ERRMEMGOTO(pstThing->strThingClass, result, _EXIT);
    
    *ppstThing = pstThing;
    pstThing = NULL;

    result = ERR_CAP_NOERROR;

_EXIT:
    if (result != ERR_CAP_NOERROR && pstThing != NULL) {
        SAFE_CAPSTRING_DELETE(pstThing->strThingId);
        SAFE_CAPSTRING_DELETE(pstThing->strDescription);
        SAFE_CAPSTRING_DELETE(pstThing->strThingClass);
        SAFEMEMFREE(pstThing);
    }
    return result;
}

cap_result ThingHandler_Insert(IN SThingInfo* pstThing, IN char* pszJsonBuffer, IN int nJsonBufSize)
{
    cap_result result = ERR_CAP_UNKNOWN;
    json_object* pJsonObject, *pJsonValueArray, *pJsonFunctionArray, *pJsonValue, *pJsonFunction, *pJsonAliveCycle, *pJsonDescription;
    int nNumberOfObject = 0, nIter;
    const char* pszValue = "values", * pszFunction = "functions", *pszAliveCycle = "alive_cycle", *pszDescription = "description";
    int nIndex = 0;
    // struct json_tokener *pJsonTok = NULL;

    IFVARERRASSIGNGOTO(pstThing, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(pszJsonBuffer, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if (nJsonBufSize <= 0) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_PARAM, _EXIT);
    }

    result = ParsingJson(&pJsonObject, pszJsonBuffer, nJsonBufSize);
    ERRIFGOTO(result, _EXIT);
    
    if (!json_object_object_get_ex(pJsonObject, pszAliveCycle, &pJsonAliveCycle)) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }

    pstThing->nAliveCycle = json_object_get_int(pJsonAliveCycle);
   
    //description is not mandatory
    if (json_object_object_get_ex(pJsonObject, pszDescription, &pJsonDescription)) {
        result = CAPString_SetLow(pstThing->strDescription, json_object_get_string(pJsonDescription), CAPSTRING_MAX);
        ERRIFGOTO(result, _EXIT);

    }

    //Get class of thing from thingID
    nIndex = CAPString_FindChar(pstThing->strThingId, 0, '_');
    if(nIndex <= 0){
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    } 
    result = CAPString_SetSub(pstThing->strThingClass, pstThing->strThingId, 0, nIndex);
    ERRIFGOTO(result, _EXIT);
    
    if (!json_object_object_get_ex(pJsonObject, pszValue, &pJsonValueArray)) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }
    nNumberOfObject = json_object_array_length(pJsonValueArray);
    pstThing->nValueNum = nNumberOfObject;
    pstThing->pstValues = (SValue*)calloc(nNumberOfObject, sizeof(SValue));
    ERRMEMGOTO(pstThing->pstValues, result, _EXIT);

    for (nIter = 0; nIter < nNumberOfObject; nIter++) {
        pJsonValue = json_object_array_get_idx(pJsonValueArray, nIter);
        result = insertOneValue(&pstThing->pstValues[nIter], pJsonValue);
        ERRIFGOTO(result, _EXIT);
    }

    if (!json_object_object_get_ex(pJsonObject, pszFunction, &pJsonFunctionArray)) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }
    nNumberOfObject = json_object_array_length(pJsonFunctionArray);

    pstThing->nFunctionNum = nNumberOfObject;
    pstThing->pstFunctions = (SFunction*)calloc(nNumberOfObject, sizeof(SFunction));
    ERRMEMGOTO(pstThing->pstFunctions, result, _EXIT);

    for (nIter = 0; nIter < nNumberOfObject; nIter++) {
        pJsonFunction = json_object_array_get_idx(pJsonFunctionArray, nIter);
        result = insertOneFunc(&pstThing->pstFunctions[nIter], pJsonFunction);
        ERRIFGOTO(result, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEJSONFREE(pJsonObject);
    return result;
}

cap_result ThingHandler_Destroy(IN OUT SThingInfo** ppstThing)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SThingInfo* pstThingInfo = NULL;
    int nLoop = 0;

    IFVARERRASSIGNGOTO(ppstThing, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(*ppstThing, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstThingInfo = (SThingInfo*)*ppstThing;

    if (pstThingInfo->pstValues != NULL) {
        for (nLoop = 0; nLoop < pstThingInfo->nValueNum; nLoop++) {
            SAFE_CAPSTRING_DELETE(pstThingInfo->pstValues[nLoop].strValueName);
            if(pstThingInfo->pstValues[nLoop].enType == VALUE_TYPE_BINARY){
                SAFEMEMFREE(pstThingInfo->pstValues[nLoop].pszFormat);
            }
        }
        SAFEMEMFREE(pstThingInfo->pstValues);
    }

    if (pstThingInfo->pstFunctions != NULL) {
        for (nLoop = 0; nLoop < pstThingInfo->nFunctionNum; nLoop++) {
            SAFE_CAPSTRING_DELETE(pstThingInfo->pstFunctions[nLoop].strFunctionName);
        }
        SAFEMEMFREE(pstThingInfo->pstFunctions);
    }

    SAFE_CAPSTRING_DELETE(pstThingInfo->strThingId);
    SAFE_CAPSTRING_DELETE(pstThingInfo->strDescription);
    SAFE_CAPSTRING_DELETE(pstThingInfo->strThingClass);
    SAFEMEMFREE(pstThingInfo);

    *ppstThing = NULL;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}
