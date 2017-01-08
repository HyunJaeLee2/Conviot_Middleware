#ifndef MQTT_COMMON_H_
#define MQTT_COMMON_H_

#include "capiot_common.h"
#include "CAPString.h"

typedef enum _EConviotErrorCode {
	ERR_CONVIOT_NOERROR         = 0,
	ERR_CONVIOT_FAIL            = -1,
	ERR_CONVIOT_TIME_EXPIRED    = -2,
	ERR_CONVIOT_INVALID_PARAM   = -3,
	ERR_CONVIOT_DUPLICATED      = -4,
	ERR_CONVIOT_NOT_SUPPORTED   = -5,
	ERR_CONVIOT_INTERNAL_FAIL   = -6,
	ERR_CONVIOT_INVALID_REQUEST = -7,
} EConviotErrorCode;

typedef struct _SMQTTData{
    char* pszTopic;
    int nTopicLen;
    char* pszPayload;
    int nPayloadLen;
}SMQTTData;

cap_result assignLastElementFromTopicList(cap_handle hTopicLinkedList, OUT char **ppDeviceId, OUT int *pnDeviceIdBufSize);
cap_result assignIdentifierFromTopicList(cap_handle hTopicLinkedList, OUT char **ppIdentifier, OUT int *pnIdentifierBufSize);
cap_result getLastElementFromTopicList(cap_handle hTopicLinkedList, cap_string *pstrLastElement);
cap_result divideTopicNameToList(IN cap_string strTopic, IN OUT cap_handle hLinkedList);

#endif
