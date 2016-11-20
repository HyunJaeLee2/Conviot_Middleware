/*
 * MQTTMessageHandler.h
 *
 *  Created on: 2016. 8. 8.
 *      Author: chjej202
 */

#ifndef _MQTTMESSAGEHANDLER_H_
#define _MQTTMESSAGEHANDLER_H_

#include <capiot_common.h>
#include <MQTTClient.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef cap_result (*CbFnMQTTHandler)(cap_string strTopic, cap_handle hTopicItemList, char *pszPayload, int nPayloadLen, IN void *pUserData);

typedef struct _SMQTTMessageHandler {
    EIoTHandleId enID;
    MQTTClient hClient;
    cap_handle hMQTTMessageQueue;
    cap_handle hMQTTMessageHandlingThread;
    volatile MQTTClient_deliveryToken nDeliveredToken;
    CbFnMQTTHandler fnCallback;
    void *pUserData;
    cap_handle hSubscriptionList;
    cap_bool bConnected;
    cap_handle hLock;
    int nPublishWaitTime;
} SMQTTMessageHandler;

cap_result MQTTMessageHandler_Create(cap_string strBrokerURI, cap_string strClientID, int nPublishWaitTime, OUT cap_handle *phHandler);
cap_result MQTTMessageHandler_SetReceiveCallback(cap_handle hHandler, CbFnMQTTHandler fnCbMessageArrive, void *pUserData);
cap_result MQTTMessageHandler_Connect(cap_handle hHandler);
cap_result MQTTMessageHandler_Disconnect(cap_handle hHandler);
cap_result MQTTMessageHandler_Publish(cap_handle hHandler, cap_string strTopic, char *pszPayload, int nPayloadLen);
cap_result MQTTMessageHandler_Subscribe(cap_handle hHandler, cap_string strTopic);
cap_result MQTTMessageHandler_SubscribeMany(cap_handle hHandler, char * const *paszTopicArray, int nTopicNum);
cap_result MQTTMessageHandler_Destroy(IN OUT cap_handle *phHandler);

#ifdef __cplusplus
}
#endif

#endif /* _MQTTMESSAGEHANDLER_H_ */
