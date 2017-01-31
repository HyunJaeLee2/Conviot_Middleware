#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <json-c/json_object.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>

#include <CAPBase64.h>
#include <CAPString.h>
#include <CAPThread.h>
#include <CAPQueue.h>
#include <CAPLogger.h>
#include <CAPLinkedList.h>
#include <CAPThreadLock.h>
#include <CAPThreadEvent.h>
#include <CAPTime.h>

#include <MQTT_common.h>
#include <Json_common.h>

#include "ThingManager.h"
#include "MQTTMessageHandler.h"
#include "DBHandler.h"

#define SECOND 1000

static char* paszThingManagerSubcriptionList[] = {
    "TM/REGISTER/#",
    "TM/UNREGISTER/#",
    "TM/ALIVE/#",
    "TM/SEND_VARIABLE/#",
    "SM/SEND_VARIABLE/#",
};


#define MQTT_SUBSCRIPTION_NUM (sizeof(paszThingManagerSubcriptionList) / sizeof(char*))
#define MQTT_CLIENT_ID "Conviot_ThingManager"

#define TOPIC_SEPERATOR "/"

CAPSTRING_CONST(CAPSTR_MQTT_CLIENT_ID, MQTT_CLIENT_ID);
CAPSTRING_CONST(CAPSTR_CATEGORY_REGISTER, "REGISTER");
CAPSTRING_CONST(CAPSTR_CATEGORY_UNREGISTER, "UNREGISTER");
CAPSTRING_CONST(CAPSTR_CATEGORY_SEND_VARIABLE, "SEND_VARIABLE");
CAPSTRING_CONST(CAPSTR_CATEGORY_ALIVE, "ALIVE");
CAPSTRING_CONST(CAPSTR_TOPIC_SEPERATOR, TOPIC_SEPERATOR);
CAPSTRING_CONST(CAPSTR_SENDER_DEVICE, "TM");
CAPSTRING_CONST(CAPSTR_SENDER_SERVICE, "SM");

CAPSTRING_CONST(CAPSTR_REGISTER_RESULT, "MT/REGISTER_RESULT/");
CAPSTRING_CONST(CAPSTR_UNREGISTER_RESULT, "MT/UNREGISTER_RESULT/");

CAP_THREAD_HEAD aliveHandlingThread(IN void* pUserData)
{
	cap_result result = ERR_CAP_UNKNOWN;
	SThingManager* pstThingManager = NULL;
    int nAliveCheckingPeriod = 0;
	int nArrayLength = 0;
    int nLoop = 0;
    long long llCurrTime = 0;
    long long llLatestTime = 0;
    long long llAliveCycle = 0;
    
	IFVARERRASSIGNGOTO(pUserData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

	pstThingManager = (SThingManager *)pUserData;

	nAliveCheckingPeriod = pstThingManager->nAliveCheckingPeriod;

	while (g_bExit == FALSE) {
		//timeout doesn't count as an error
		result = CAPThreadEvent_WaitTimeEvent(pstThingManager->hEvent, (long long) nAliveCheckingPeriod*SECOND);
		if(result == ERR_CAP_TIME_EXPIRED)
		{
		    result = ERR_CAP_NOERROR;
		}
		ERRIFGOTO(result, _EXIT);

        result = DBHandler_MakeThingAliveInfoArray(pstThingManager->pDBconn, &pstThingManager->pstThingAliveInfoArray, &nArrayLength);
        ERRIFGOTO(result, _EXIT);

        //If there is no thing in database, continue
        if(pstThingManager->pstThingAliveInfoArray == NULL)
            continue;
        
        result = CAPTime_GetCurTimeInMilliSeconds(&llCurrTime);
        ERRIFGOTO(result, _EXIT);
           
        for(nLoop = 0; nLoop < nArrayLength; nLoop++){
            llLatestTime = pstThingManager->pstThingAliveInfoArray[nLoop].llLatestTime;
            //Minor Adjustment to alive cycle considering network overhead
            llAliveCycle = pstThingManager->pstThingAliveInfoArray[nLoop].llAliveCycle + 1 * SECOND;
               
           //dlp("curr : %lld, latest : %lld, alive : %lld, sub : %lld\n", llCurrTime, llLatestTime, llAliveCycle, llCurrTime- llLatestTime);
            if(llCurrTime - llLatestTime > llAliveCycle){
                result = DBHandler_DisableDeviceAndEca(pstThingManager->pDBconn, pstThingManager->pstThingAliveInfoArray[nLoop].strDeviceId);  
                ERRIFGOTO(result, _EXIT);

                CAPLogger_Write(g_hLogger, MSG_INFO, "ThingManager has disabled %s and removed related ECAs for not receiving alive message.",\
                        CAPString_LowPtr(pstThingManager->pstThingAliveInfoArray[nLoop].strDeviceId, NULL));
            }
        }
        //Free all the memory of ThingAliveInfoArray 
        for (nLoop = 0; nLoop < nArrayLength; nLoop++) {
            SAFE_CAPSTRING_DELETE(pstThingManager->pstThingAliveInfoArray[nLoop].strDeviceId);
        }
        SAFEMEMFREE(pstThingManager->pstThingAliveInfoArray);
    }

    result = ERR_CAP_NOERROR;

_EXIT:
    if(result != ERR_CAP_NOERROR && pstThingManager != NULL && pstThingManager->pstThingAliveInfoArray != NULL){
        //Free all the memory of ThingAliveInfoArray 
        for (nLoop = 0; nLoop < nArrayLength; nLoop++) {
            SAFE_CAPSTRING_DELETE(pstThingManager->pstThingAliveInfoArray[nLoop].strDeviceId);
        }
        SAFEMEMFREE(pstThingManager->pstThingAliveInfoArray);
    }

    CAP_THREAD_END;
}

/*
 -Get file path from config file
 -write binary then save it into db
*/
static cap_result saveBinaryFileThenGetPath(IN cap_string strDeviceId, IN cap_string strVariableName, IN char * pszVariable, OUT cap_string strBinaryDBPath, IN char * pszFormat)
{
    cap_result result = ERR_CAP_UNKNOWN;
    FILE* pFile = NULL;
    //TODO : set path prefix in config file
    const char *pszConstFilePathPrefix = "/var/www/Conviot/static/repository";
    const char *pszConstDBPathPrefix = "http://www.conviot.com/static/repository";
    cap_string strFilePath = NULL;
    char *pszDecodedData = NULL;
    int nDecodedLen = 0;

    strFilePath = CAPString_New();
    ERRMEMGOTO(strFilePath, result, _EXIT);
    
    result = CAPString_PrintFormat(strFilePath, "%s/%s_%s_%lu.%s", pszConstFilePathPrefix, CAPString_LowPtr(strDeviceId, NULL), \
            CAPString_LowPtr(strVariableName, NULL), (unsigned long)time(NULL), pszFormat);
    ERRIFGOTO(result, _EXIT);
    
    result = CAPString_PrintFormat(strBinaryDBPath, "%s/%s_%s_%lu.%s", pszConstDBPathPrefix,  CAPString_LowPtr(strDeviceId, NULL), \
            CAPString_LowPtr(strVariableName, NULL), (unsigned long)time(NULL), pszFormat);
    ERRIFGOTO(result, _EXIT);

    result = CAPBase64_Decode(pszVariable, &pszDecodedData, &nDecodedLen);
    ERRIFGOTO(result, _EXIT);
   
    dlp("path1 : %s, path2 : %s\n", CAPString_LowPtr(strFilePath, NULL), CAPString_LowPtr(strBinaryDBPath, NULL));
    pFile = fopen(CAPString_LowPtr(strFilePath, NULL),"wb");
    if(pFile == NULL) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }      

    fwrite(pszDecodedData, nDecodedLen, 1, pFile);

    fclose(pFile);

    SAFE_CAPSTRING_DELETE(strFilePath);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

static cap_result ThingManager_PublishErrorCode(IN int errorCode, cap_handle hThingManager, cap_string strMessageReceiverId, cap_string strTopicCategory)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SThingManager* pstThingManager = NULL;
    cap_string strTopic = NULL;
    char* pszPayload;
    char *pszApiKey = NULL;
    int nPayloadLen = 0;
    EConviotErrorCode enError;
    json_object* pJsonObject;

    IFVARERRASSIGNGOTO(hThingManager, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(strMessageReceiverId, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstThingManager = (SThingManager*)hThingManager;

    //set topic
    strTopic = CAPString_New();
    ERRMEMGOTO(strTopic, result, _EXIT);

    result = CAPString_Set(strTopic, strTopicCategory);
    ERRIFGOTO(result, _EXIT);

    result = CAPString_AppendString(strTopic, strMessageReceiverId);
    ERRIFGOTO(result, _EXIT);

    //make error code depend on errorcode value
    if (errorCode == ERR_CAP_NOERROR) {
        enError = ERR_CONVIOT_NOERROR;
    }
    else if (errorCode == ERR_CAP_DUPLICATED) {
        enError = ERR_CONVIOT_DUPLICATED;
    }
    else if (errorCode == ERR_CAP_INVALID_DATA) {
        enError = ERR_CONVIOT_INVALID_REQUEST;
    }
    else {
        enError = ERR_CONVIOT_FAIL;
    }
    
    //set payload
    pJsonObject = json_object_new_object();
    json_object_object_add(pJsonObject, "error", json_object_new_int(enError));

    result = DBHandler_RetrieveApiKey(pstThingManager->pDBconn, strMessageReceiverId, &pszApiKey);
    ERRIFGOTO(result, _EXIT);

    if(pszApiKey == NULL) {
        //do nothing
    } 
    else {
        json_object_object_add(pJsonObject, "apikey", json_object_new_string(pszApiKey));
    }

    pszPayload = strdup(json_object_to_json_string(pJsonObject));
    nPayloadLen = strlen(pszPayload);

    result = MQTTMessageHandler_Publish(pstThingManager->hMQTTHandler, strTopic, pszPayload, nPayloadLen);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEJSONFREE(pJsonObject);
    SAFEMEMFREE(pszPayload);
    SAFEMEMFREE(pszApiKey);
    SAFE_CAPSTRING_DELETE(strTopic);

    return result;
}

static cap_result handleDeviceMessage(cap_string strTopic, cap_handle hTopicItemList,
        char *pszPayload, int nPayloadLen, IN SThingManager *pstThingManager)
{
    cap_result result = ERR_CAP_UNKNOWN;
    cap_result result_save = ERR_CAP_UNKNOWN;
    json_object* pJsonObject, *pJsonApiKey;
    const char* pszConstApiKey = "apikey";
    cap_string strCategory = NULL;
    cap_string strDeviceId = NULL;
    cap_string strBinaryDBPath = NULL;

    IFVARERRASSIGNGOTO(pstThingManager, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    //Get Category
    result = CAPLinkedList_Get(hTopicItemList, LINKED_LIST_OFFSET_FIRST, TOPIC_LEVEL_SECOND, (void**)&strCategory);
    ERRIFGOTO(result, _EXIT);
   
    //Get Thing ID
    result = CAPLinkedList_Get(hTopicItemList, LINKED_LIST_OFFSET_FIRST, TOPIC_LEVEL_THIRD, (void**)&strDeviceId);
    ERRIFGOTO(result, _EXIT);
   
   
    //Parse Payload to check its api key
    result = ParsingJson(&pJsonObject, pszPayload, nPayloadLen);
    ERRIFGOTO(result, _EXIT);
   
    if (!json_object_object_get_ex(pJsonObject, pszConstApiKey, &pJsonApiKey)) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }

    //TODO
    //Add Error code to each specific situation

    //save result to report error later
    //ERR_CAP_NO_DATA : no matching device
    //ERR_CAP_INVALID_DATA : api key doesn't match
    result_save = DBHandler_VerifyApiKey(pstThingManager->pDBconn, strDeviceId, (char *)json_object_get_string(pJsonApiKey));

    if (CAPString_IsEqual(strCategory, CAPSTR_CATEGORY_REGISTER) == TRUE) {
        json_object* pJsonPinCode;
        const char* pszConstPinCode = "pincode";

        //If api key error has occured, publish error code then return 
        if(result_save != ERR_CAP_NOERROR){
            result = ThingManager_PublishErrorCode(result_save, pstThingManager, strDeviceId, CAPSTR_REGISTER_RESULT);
            ERRIFGOTO(result, _EXIT);

            goto _EXIT;
        }
        
        if (!json_object_object_get_ex(pJsonObject, pszConstPinCode, &pJsonPinCode)) {
            ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
        }

        //publish error code of function result
        result_save = DBHandler_RegisterDevice(pstThingManager->pDBconn, strDeviceId, (char *)json_object_get_string(pJsonPinCode));  
        
        result = ThingManager_PublishErrorCode(result_save, pstThingManager, strDeviceId, CAPSTR_REGISTER_RESULT);
        ERRIFGOTO(result, _EXIT);

    }
    else if (CAPString_IsEqual(strCategory, CAPSTR_CATEGORY_UNREGISTER) == TRUE) {
        json_object* pJsonPinCode;
        const char* pszConstPinCode = "pincode";
        
        //If api key error has occured, publish error code then return 
        if(result_save != ERR_CAP_NOERROR){
            result = ThingManager_PublishErrorCode(result_save, pstThingManager, strDeviceId, CAPSTR_UNREGISTER_RESULT);
            ERRIFGOTO(result, _EXIT);

            goto _EXIT;
        }
        
        if (!json_object_object_get_ex(pJsonObject, pszConstPinCode, &pJsonPinCode)) {
            ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
        }

        //publish error code of function result
        result_save = DBHandler_UnregisterDeviceAndEca(pstThingManager->pDBconn, strDeviceId, (char *)json_object_get_string(pJsonPinCode));  
        
        result = ThingManager_PublishErrorCode(result_save, pstThingManager, strDeviceId, CAPSTR_UNREGISTER_RESULT);
        ERRIFGOTO(result, _EXIT);

    }
    else if (CAPString_IsEqual(strCategory, CAPSTR_CATEGORY_ALIVE) == TRUE) {
        //If api key error has occured, goto exit 
        if(result_save != ERR_CAP_NOERROR){
            goto _EXIT;
        }

        //ignore error because alive message does not have a return type
        result = DBHandler_UpdateLatestTime(pstThingManager->pDBconn, strDeviceId);
    }
    else if (CAPString_IsEqual(strCategory, CAPSTR_CATEGORY_SEND_VARIABLE) == TRUE) {
        cap_string strVariableName = NULL;
        json_object *pJsonVariable = NULL, *pJsonFormat = NULL;
        const char* pszConstVariable = "variable", *pszConstFormat = "format";
        
        //If api key error has occured, goto exit 
        if(result_save != ERR_CAP_NOERROR){
            goto _EXIT;
        }
        
        //Get variable name 
        result = CAPLinkedList_Get(hTopicItemList, LINKED_LIST_OFFSET_FIRST, TOPIC_LEVEL_FOURTH, (void**)&strVariableName);
        ERRIFGOTO(result, _EXIT);
        
        
        if (!json_object_object_get_ex(pJsonObject, pszConstVariable, &pJsonVariable)) {
            ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
        }
        
        //Handle if type is binary
        if (json_object_object_get_ex(pJsonObject, pszConstFormat, &pJsonFormat)) {
            strBinaryDBPath = CAPString_New();
            ERRMEMGOTO(strBinaryDBPath, result, _EXIT);

            result = saveBinaryFileThenGetPath(strDeviceId, strVariableName, (char *)json_object_get_string(pJsonVariable), strBinaryDBPath,\
                    (char *)json_object_get_string(pJsonFormat));
            ERRIFGOTO(result, _EXIT);
            
            //ignore error because send variable does not have a return type
            result = DBHandler_InsertVariableHistory(pstThingManager->pDBconn, strDeviceId, strVariableName, CAPString_LowPtr(strBinaryDBPath, NULL));
        }
        else {
            //ignore error because send variable does not have a return type
            result = DBHandler_InsertVariableHistory(pstThingManager->pDBconn, strDeviceId, strVariableName, (char *)json_object_get_string(pJsonVariable));
        }
    }
    else {
        ERRASSIGNGOTO(result, ERR_CAP_NOT_SUPPORTED, _EXIT);
    }
    result = ERR_CAP_NOERROR;

_EXIT:
    SAFE_CAPSTRING_DELETE(strBinaryDBPath);
    if(result != ERR_CAP_NOERROR){
        //Added if clause for a case where a thread is terminated without accepting any data at all
    }

    return result;
}

static cap_result handleServiceMessage(cap_string strTopic, cap_handle hTopicItemList,
        char *pszPayload, int nPayloadLen, IN SThingManager *pstThingManager)
{
    cap_result result = ERR_CAP_UNKNOWN;
    json_object* pJsonObject, *pJsonApiKey, *pJsonUserId;
    int nUserId = 0;
    const char* pszConstApiKey = "apikey", *pszConstUserId = "user_id";
    cap_string strCategory = NULL;
    cap_string strProductName = NULL;
    cap_string strBinaryDBPath = NULL;

    IFVARERRASSIGNGOTO(pstThingManager, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    //Get Category
    result = CAPLinkedList_Get(hTopicItemList, LINKED_LIST_OFFSET_FIRST, TOPIC_LEVEL_SECOND, (void**)&strCategory);
    ERRIFGOTO(result, _EXIT);
   
    //Get Thing ID
    result = CAPLinkedList_Get(hTopicItemList, LINKED_LIST_OFFSET_FIRST, TOPIC_LEVEL_THIRD, (void**)&strProductName);
    ERRIFGOTO(result, _EXIT);
   
    //Parse Payload to check its api key
    result = ParsingJson(&pJsonObject, pszPayload, nPayloadLen);
    ERRIFGOTO(result, _EXIT);
   
    if (!json_object_object_get_ex(pJsonObject, pszConstApiKey, &pJsonApiKey)) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }
    
    if (!json_object_object_get_ex(pJsonObject, pszConstUserId, &pJsonUserId)) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }

    nUserId = json_object_get_int(pJsonUserId);
    //save result to report error later
    //ERR_CAP_NO_DATA : no matching device
    //ERR_CAP_INVALID_DATA : api key doesn't match
    result = DBHandler_VerifyServiceApiKey(pstThingManager->pDBconn, strProductName, (char *)json_object_get_string(pJsonApiKey));
    ERRIFGOTO(result, _EXIT);

    if (CAPString_IsEqual(strCategory, CAPSTR_CATEGORY_SEND_VARIABLE) == TRUE) {
        cap_string strVariableName = NULL;
        json_object *pJsonVariable = NULL, *pJsonFormat = NULL;
        const char* pszConstVariable = "variable", *pszConstFormat = "format";
        
        //Get variable name 
        result = CAPLinkedList_Get(hTopicItemList, LINKED_LIST_OFFSET_FIRST, TOPIC_LEVEL_FOURTH, (void**)&strVariableName);
        ERRIFGOTO(result, _EXIT);
        
        
        if (!json_object_object_get_ex(pJsonObject, pszConstVariable, &pJsonVariable)) {
            ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
        }
        
        //Handle if type is binary
        if (json_object_object_get_ex(pJsonObject, pszConstFormat, &pJsonFormat)) {
            strBinaryDBPath = CAPString_New();
            ERRMEMGOTO(strBinaryDBPath, result, _EXIT);
            
            //TODO
            //add user thing to binary file name when it is a servicee
            result = saveBinaryFileThenGetPath(strProductName, strVariableName, (char *)json_object_get_string(pJsonVariable), strBinaryDBPath,\
                    (char *)json_object_get_string(pJsonFormat));
            ERRIFGOTO(result, _EXIT);
            
            //ignore error because send variable does not have a return type
            result = DBHandler_InsertServiceVariableHistory(pstThingManager->pDBconn, strProductName, nUserId, strVariableName, CAPString_LowPtr(strBinaryDBPath, NULL));
        }
        else {
            //ignore error because send variable does not have a return type
            result = DBHandler_InsertServiceVariableHistory(pstThingManager->pDBconn, strProductName, nUserId, strVariableName, (char *)json_object_get_string(pJsonVariable));
        }
    }
    else {
        ERRASSIGNGOTO(result, ERR_CAP_NOT_SUPPORTED, _EXIT);
    }
    result = ERR_CAP_NOERROR;

_EXIT:
    SAFE_CAPSTRING_DELETE(strBinaryDBPath);
    if(result != ERR_CAP_NOERROR){
        //Added if clause for a case where a thread is terminated without accepting any data at all
    }

    return result;
}
static CALLBACK cap_result mqttMessageHandlingCallback(cap_string strTopic, cap_handle hTopicItemList,
        char *pszPayload, int nPayloadLen, IN void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    cap_result result_save = ERR_CAP_UNKNOWN;
    SThingManager* pstThingManager = NULL;
    cap_string strSender = NULL;

    IFVARERRASSIGNGOTO(pUserData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstThingManager = (SThingManager*)pUserData;

    /*Topics are set as follow
     *[TM]|[SM]/[TOPIC CATEGORY]/[THING ID] and functio name of value name could be set at last topic level
     */
    dlp("ThingManager received message! topic : %s, payload : %s\n", CAPString_LowPtr(strTopic, NULL),pszPayload);

    //Get Sender (device or service)
    result = CAPLinkedList_Get(hTopicItemList, LINKED_LIST_OFFSET_FIRST, TOPIC_LEVEL_SECOND, (void**)&strSender);
    ERRIFGOTO(result, _EXIT);
    
    if (CAPString_IsEqual(strSender, CAPSTR_SENDER_DEVICE) == TRUE) {
        result = handleDeviceMessage(strTopic, hTopicItemList, pszPayload, nPayloadLen, pstThingManager);
        ERRIFGOTO(result, _EXIT);
    }
    else if (CAPString_IsEqual(strSender, CAPSTR_SENDER_SERVICE) == TRUE) {
        result = handleServiceMessage(strTopic, hTopicItemList, pszPayload, nPayloadLen, pstThingManager);
        ERRIFGOTO(result, _EXIT);
    }
    else {
        ERRASSIGNGOTO(result, ERR_CAP_NOT_SUPPORTED, _EXIT);
    }
    
    result = ERR_CAP_NOERROR;
_EXIT:
    if(result != ERR_CAP_NOERROR){
        //Added if clause for a case where a thread is terminated without accepting any data at all
    }

    return result;
}


cap_result ThingManager_Create(OUT cap_handle* phThingManager, IN cap_string strBrokerURI, IN SDBInfo *pstDBInfo)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SThingManager* pstThingManager = NULL;
    IFVARERRASSIGNGOTO(phThingManager, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    pstThingManager = (SThingManager*)malloc(sizeof(SThingManager));
    ERRMEMGOTO(pstThingManager, result, _EXIT);
    
    pstThingManager->enID = HANDLEID_THING_MANAGER;
    pstThingManager->bCreated = FALSE;
    pstThingManager->hAliveHandlingThread = NULL;
    pstThingManager->pstThingAliveInfoArray = NULL;
    pstThingManager->hEvent = NULL;
    pstThingManager->hMQTTHandler = NULL;
    pstThingManager->nAliveCheckingPeriod = 0;
    
    result = DBHandler_OpenDB(pstDBInfo, &pstThingManager->pDBconn);
    ERRIFGOTO(result, _EXIT);

    //create event for timedwait
    result = CAPThreadEvent_Create(&pstThingManager->hEvent);
    ERRIFGOTO(result, _EXIT);

    //Create MQTT Client
    result = MQTTMessageHandler_Create(strBrokerURI, CAPSTR_MQTT_CLIENT_ID, 0, &(pstThingManager->hMQTTHandler));
    ERRIFGOTO(result, _EXIT);

    *phThingManager = pstThingManager;

    result = ERR_CAP_NOERROR;
_EXIT:
    if (result != ERR_CAP_NOERROR && pstThingManager != NULL) {
        CAPThreadEvent_Destroy(&(pstThingManager->hEvent));
        SAFEMEMFREE(pstThingManager);
    }
    return result;
}

cap_result ThingManager_Run(IN cap_handle hThingManager, IN int nAliveCheckingPeriod)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SThingManager* pstThingManager = NULL;

    if (IS_VALID_HANDLE(hThingManager, HANDLEID_THING_MANAGER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }
    pstThingManager = (SThingManager*)hThingManager;

    // Already created
    if (pstThingManager->bCreated == TRUE) {
        CAPASSIGNGOTO(result, ERR_CAP_NOERROR, _EXIT);
    }

    result = MQTTMessageHandler_SetReceiveCallback(pstThingManager->hMQTTHandler,
            mqttMessageHandlingCallback, pstThingManager);
    ERRIFGOTO(result, _EXIT);

    result = MQTTMessageHandler_Connect(pstThingManager->hMQTTHandler);
    ERRIFGOTO(result, _EXIT);

    result = MQTTMessageHandler_SubscribeMany(pstThingManager->hMQTTHandler, paszThingManagerSubcriptionList, MQTT_SUBSCRIPTION_NUM);
    ERRIFGOTO(result, _EXIT);

    pstThingManager->nAliveCheckingPeriod = nAliveCheckingPeriod;

    //pstAliveHandlingThreadData will be freed in a thread
    result = CAPThread_Create(aliveHandlingThread, pstThingManager, &(pstThingManager->hAliveHandlingThread));
    ERRIFGOTO(result, _EXIT);

    pstThingManager->bCreated = TRUE;

    result = ERR_CAP_NOERROR;
_EXIT:
    if(result != ERR_CAP_NOERROR){
        //deallocate threadData if it fails to create a thread
    }
    return result;
}

cap_result ThingManager_Join(IN cap_handle hThingManager)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SThingManager* pstThingManager = NULL;

    if (IS_VALID_HANDLE(hThingManager, HANDLEID_THING_MANAGER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstThingManager = (SThingManager*)hThingManager;

    if (pstThingManager->bCreated == TRUE) {
        result = CAPThreadEvent_SetEvent(pstThingManager->hEvent);
        ERRIFGOTO(result, _EXIT);
        
        result = CAPThread_Destroy(&(pstThingManager->hAliveHandlingThread));
        ERRIFGOTO(result, _EXIT);

        result = MQTTMessageHandler_Disconnect(pstThingManager->hMQTTHandler);
        ERRIFGOTO(result, _EXIT);

        result = DBHandler_CloseDB(pstThingManager->pDBconn);
        ERRIFGOTO(result, _EXIT);
        
        pstThingManager->bCreated = FALSE;
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result ThingManager_Destroy(IN OUT cap_handle* phThingManager)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SThingManager* pstThingManager = NULL;

    IFVARERRASSIGNGOTO(phThingManager, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if (IS_VALID_HANDLE(*phThingManager, HANDLEID_THING_MANAGER) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstThingManager = (SThingManager*)*phThingManager;

    MQTTMessageHandler_Destroy(&(pstThingManager->hMQTTHandler));

    CAPThreadEvent_Destroy(&(pstThingManager->hEvent));

    SAFEMEMFREE(pstThingManager);

    *phThingManager = NULL;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}
