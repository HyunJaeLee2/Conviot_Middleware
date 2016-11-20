#ifndef THINGHANDLER_H_
#define THINGHANDLER_H_

#include "capiot_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define THING_DB_ID_NOT_SET (-1)

typedef enum _EValueType {
    VALUE_TYPE_INT,
    VALUE_TYPE_DOUBLE,
    VALUE_TYPE_BOOL,
    VALUE_TYPE_STRING,
    VALUE_TYPE_BINARY,
} EValueType;

typedef struct _SValue {
    cap_string strValueName;
    EValueType enType;
    double dbMinValue;
    double dbMaxValue;
    char* pszFormat;
} SValue;

typedef struct _SFunction {
    cap_string strFunctionName;
    cap_bool bUseArgument;
    EValueType enArgumentType;
    double dbArgMinValue;
    double dbArgMaxValue;
} SFunction;

typedef struct _SThingInfo {
    cap_string strThingId;
    cap_string strThingClass;
    cap_string strDescription;
    int nIDInDB;
    int nAliveCycle;
    int nValueNum;
    SValue* pstValues;
    int nFunctionNum;
    SFunction* pstFunctions;
} SThingInfo;

cap_result ThingHandler_Create(IN const char* pszThingId, IN int nIdLength, OUT SThingInfo** ppstThing);
cap_result ThingHandler_Duplicate(IN SThingInfo* pstSrc, OUT SThingInfo** pstDst);
cap_result ThingHandler_Insert(IN SThingInfo* pstThing, IN char* pszJsonBuffer, IN int nJsonBufSize);
cap_result ThingHandler_Destroy(IN OUT SThingInfo** ppstThing);

#ifdef __cplusplus
}
#endif

#endif /* THINGHANDLER_H_ */
