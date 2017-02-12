#include <config.h>
#ifdef HAVE_CONFIG_H
#endif

#include <errno.h>

#include <CAPLinkedList.h>
#include <CAPString.h>
#include <CAPLogger.h>
#include <CAPThreadLock.h>
#include <CAPTime.h>

#include <Json_common.h>

#include "DBHandler.h"

#include <json-c/json_object.h>

#define MB 1024*1024
#define MAX_ARGUMENT_SIZE MB

#define QUERY_SIZE 1024*16
#define NULL_ERROR -1

static cap_result callQueryWithResult(MYSQL* pDBconn, char* query, MYSQL_RES **ppMysqlResult, int *pnRowCount)
{
    cap_result result = ERR_CAP_UNKNOWN;
    int nQueryRet = 0;

    nQueryRet = mysql_query(pDBconn, query);

    //fprintf(stderr, "Mysql connection error : %s, mysql_errno : %d\n", mysql_error(&pDBconn), mysql_errno(&pDBconn));
    if (nQueryRet != 0) {
		CAPLogger_Write(g_hLogger, MSG_ERROR, "DBHandler: callQueryWithResult Error : %d",nQueryRet );
        ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
    }

    /* Warning! 
     * MysqlResult has to be freed from where it is called
     */

    *ppMysqlResult = mysql_store_result(pDBconn);
    if(*ppMysqlResult == NULL) {
		CAPLogger_Write(g_hLogger, MSG_ERROR, "DBHandler: Result Store Error : %d", nQueryRet);
        ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
    }

    *pnRowCount = mysql_num_fields(*ppMysqlResult);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

static cap_result callQuery(MYSQL* pDBconn, char* query)
{
    cap_result result = ERR_CAP_UNKNOWN;
    int nQueryRet = 0;
    
    nQueryRet = mysql_query(pDBconn, query);
    if (nQueryRet != 0) {
		CAPLogger_Write(g_hLogger, MSG_ERROR, "DBHandler: callQuery Error : %d", nQueryRet);
        ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result DBHandler_OpenDB(IN SDBInfo *pstDBInfo, OUT MYSQL **ppDBconn)
{
    cap_result result = ERR_CAP_UNKNOWN;
    MYSQL *pDBconn = NULL;

    pDBconn = mysql_init(NULL);
    if(pDBconn == NULL){
        ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
    }

    if(mysql_real_connect(pDBconn, pstDBInfo->pszDBHost, pstDBInfo->pszDBUser,\
                pstDBInfo->pszDBPassword, pstDBInfo->pszDBName, pstDBInfo->nDBPort, NULL, 0) == NULL) {
        ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
    }   
   
    if(mysql_set_character_set(pDBconn, "utf8") != 0)
    {
        dlp("mysql charset error - current charset : %s\n", mysql_character_set_name(pDBconn));
        ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
    }
    
    *ppDBconn = pDBconn;
   
    result = ERR_CAP_NOERROR;

_EXIT:
    return result;
}

static int atoiIgnoreNull(const char* pszMysqlResult){
    if(pszMysqlResult == NULL){
        return NULL_ERROR;
    }
    else {
        return atoi(pszMysqlResult);
    }
}

//Replace variable in string with actual variable
//However, when variable does not exist, returns original string.
//This is for a case where an user actually uses predefined delimeter in string ( ex. check items {{water, cup, etc}} )
static cap_result replaceWithRealVariable(IN MYSQL *pDBconn, IN char *pszArgumentPayload, OUT char **ppszFinalArgumentPayload)
{
    cap_result result = ERR_CAP_UNKNOWN;
    char query[QUERY_SIZE];
    MYSQL_RES *pMysqlResult = NULL;
    MYSQL_ROW mysqlRow;
    int nRowCount = 0;
    char *pszVariableName = NULL;
    char pszFinalArgumentPayload[MAX_ARGUMENT_SIZE] = {0, }; //TODO Final Payload size is set 30MB for binary cases. However, it should be optimized with minimum size in the future
    int nArgLen = 0, nArgIndex = 0, nFinalArgIndex = 0;
    int nUserArgHead = 0, nUserArgTail = 0;
     
    nArgLen = strlen(pszArgumentPayload);

    //case : {{user_thing_id#variable_name}} 
    for(nArgIndex = 0; nArgIndex < nArgLen; nArgIndex++) {
        //If nArgIndex is pointing at the end of string, put it to final argument
        if(nArgIndex == nArgLen -1) {
            pszFinalArgumentPayload[nFinalArgIndex++] = pszArgumentPayload[nArgIndex];
        }
        else if(pszArgumentPayload[nArgIndex] == '{' && pszArgumentPayload[nArgIndex + 1] == '{') {
            int nLoop = 0;
            char *pszUserArgument = NULL, *pszUserThingId = NULL, *pszVariableName = NULL;
            int nUserThingId = 0, nLatestValueLen = 0, nUserArgLen = 0;

            //Each of nUserArgHead and nUserArgTail points to the start and the end of user arugument
            nUserArgHead = nArgIndex + 2;

            //Find index of "}}" 
            for(nLoop = nArgIndex + 2; nLoop < nArgLen - 1; nLoop++) {
                if(pszArgumentPayload[nLoop] == '}' && pszArgumentPayload[nLoop + 1] == '}') {
                    nUserArgTail = nLoop - 1;
                    break;
                } 
            }
            
            nUserArgLen = nUserArgTail - nUserArgHead + 1;
            //Allocate memory for user argument with size including null at the end
            pszUserArgument = (char *)malloc(sizeof(char) * (nUserArgLen + 1));

            //copy user argument from argument payload
            strncpy(pszUserArgument, pszArgumentPayload + nUserArgHead, nUserArgLen);
            pszUserArgument[nUserArgLen] = '\0';
           
            //tokenize string with delimeter
            pszUserThingId = strtok_r(pszUserArgument, "#", &pszVariableName); //TODO trim pszVariableName later
            nUserThingId = atoi(pszUserThingId);

            //Get latest value of request
            snprintf(query, QUERY_SIZE, "\
                    SELECT\
                        var_history.value\
                    FROM\
                        things_variablehistory var_history,\
                        things_variable var\
                    WHERE\
                        var.identifier = '%s' and\
                        var_history.user_thing_id = %d and\
                        var_history.variable_id = var.id\
                    ORDER BY\
                        var_history.updated_at desc limit 1;", pszVariableName, nUserThingId);

            result = callQueryWithResult(pDBconn, query, &pMysqlResult, &nRowCount);
            ERRIFGOTO(result, _EXIT);

            mysqlRow = mysql_fetch_row(pMysqlResult);

            //If there is no matching value, just insert original string
            if(mysqlRow == NULL){
                dlp("There is no matching value yet!\n");
                pszFinalArgumentPayload[nFinalArgIndex++] = pszArgumentPayload[nArgIndex];
                continue;
            }
            //If there exists latest value, replace with it.
            //nFinalArgIndex and nArgIndex is set according to the length of it.
            else {
                nLatestValueLen = strlen(mysqlRow[0]);
                strncpy(pszFinalArgumentPayload + nFinalArgIndex, mysqlRow[0], nLatestValueLen);
                nFinalArgIndex = nFinalArgIndex + nLatestValueLen;
                nArgIndex = nUserArgTail + 2;
                continue;
            }
        }
        else {
            pszFinalArgumentPayload[nFinalArgIndex++] = pszArgumentPayload[nArgIndex];
        }
    }

    pszFinalArgumentPayload[nFinalArgIndex] = '\0';

    *ppszFinalArgumentPayload = strdup(pszFinalArgumentPayload);

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEMEMFREE(pszVariableName);
    SAFEMYSQLFREE(pMysqlResult);
    return result;
}
static cap_result checkServiceWithName(IN MYSQL *pDBconn,IN cap_string strProductName) {
    cap_result result = ERR_CAP_UNKNOWN;
    char query[QUERY_SIZE];
    MYSQL_RES *pMysqlResult = NULL;
    MYSQL_ROW mysqlRow;
    int nRowCount = 0;
    //check if device is registered to system 
    snprintf(query, QUERY_SIZE, "\
            SELECT\
                product.id\
            FROM\
                things_product product\
            WHERE\
                product.identifier = '%s';", CAPString_LowPtr(strProductName, NULL));

    result = callQueryWithResult(pDBconn, query, &pMysqlResult, &nRowCount);
    ERRIFGOTO(result, _EXIT);

    mysqlRow = mysql_fetch_row(pMysqlResult);
    
    //if there is no matching device with device id 
    if(mysqlRow == NULL){
        dlp("There is no matching service!\n");
        ERRASSIGNGOTO(result, ERR_CAP_NO_DATA, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEMYSQLFREE(pMysqlResult);
    return result;
}

static cap_result checkDeviceWithId(IN MYSQL *pDBconn,IN cap_string strDeviceId) {
    cap_result result = ERR_CAP_UNKNOWN;
    char query[QUERY_SIZE];
    MYSQL_RES *pMysqlResult = NULL;
    MYSQL_ROW mysqlRow;
    int nRowCount = 0;
    //check if device is registered to system 
    snprintf(query, QUERY_SIZE, "\
            SELECT\
                device.is_connected\
            FROM\
                things_device device\
            WHERE\
                device.device_id = '%s';", CAPString_LowPtr(strDeviceId, NULL));

    result = callQueryWithResult(pDBconn, query, &pMysqlResult, &nRowCount);
    ERRIFGOTO(result, _EXIT);

    mysqlRow = mysql_fetch_row(pMysqlResult);
    
    //if there is no matching device with device id 
    if(mysqlRow == NULL){
        dlp("There is no matching device!\n");
        ERRASSIGNGOTO(result, ERR_CAP_NO_DATA, _EXIT);
    }

    //if thing is not registered 
    if(atoiIgnoreNull(mysqlRow[0]) == FALSE){
        dlp("Thing is not registered to system!\n");
        ERRASSIGNGOTO(result, ERR_CAP_DUPLICATED, _EXIT);

    }
    
    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEMYSQLFREE(pMysqlResult);
    return result;
}

cap_result DBHandler_CloseDB(MYSQL *pDBconn)
{
    cap_result result = ERR_CAP_UNKNOWN;

    mysql_close(pDBconn);

    result = ERR_CAP_NOERROR;
    return result;
}

cap_result DBHandler_RetrieveDeviceApiKey(IN MYSQL *pDBconn, IN cap_string strDeviceId, IN OUT char **ppszApiKey)
{
    cap_result result = ERR_CAP_UNKNOWN;
    char query[QUERY_SIZE];
    MYSQL_RES *pMysqlResult = NULL;
    MYSQL_ROW mysqlRow;

    int nRowCount = 0;

    snprintf(query, QUERY_SIZE, "\
            SELECT\
                vendor.api_key\
           FROM\
                things_device device,\
                things_userthing userthing,\
                things_product product,\
                things_vendor vendor\
            WHERE\
                device.device_id = '%s' and\
                userthing.id = device.user_thing_id and\
                product.id = userthing.product_id and\
                vendor.id = product.vendor_id;", CAPString_LowPtr(strDeviceId, NULL));

    result = callQueryWithResult(pDBconn, query, &pMysqlResult, &nRowCount);
    ERRIFGOTO(result, _EXIT);

    mysqlRow = mysql_fetch_row(pMysqlResult);

    //if there is no matching device
    if(mysqlRow == NULL){
        dlp("There is no matching device!\n");
    }
    else {
        *ppszApiKey = strdup(mysqlRow[0]);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEMYSQLFREE(pMysqlResult);
    return result;
}

cap_result DBHandler_RetrieveServiceApiKey(IN MYSQL *pDBconn, IN cap_string strProductName, IN OUT char **ppszApiKey)
{
    cap_result result = ERR_CAP_UNKNOWN;
    char query[QUERY_SIZE];
    MYSQL_RES *pMysqlResult = NULL;
    MYSQL_ROW mysqlRow;

    int nRowCount = 0;

    snprintf(query, QUERY_SIZE, "\
            SELECT\
                vendor.api_key\
            FROM\
                things_product product,\
                things_vendor vendor\
            WHERE\
                product.identifier = '%s' and\
                vendor.id = product.vendor_id;", CAPString_LowPtr(strProductName, NULL));

    result = callQueryWithResult(pDBconn, query, &pMysqlResult, &nRowCount);
    ERRIFGOTO(result, _EXIT);

    mysqlRow = mysql_fetch_row(pMysqlResult);

    //if there is no matching service
    if(mysqlRow == NULL){
        dlp("There is no matching service!\n");
    }
    else {
        *ppszApiKey = strdup(mysqlRow[0]);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEMYSQLFREE(pMysqlResult);
    return result;
}

cap_result DBHandler_VerifyServiceApiKey(IN MYSQL *pDBconn, IN cap_string strProductName, IN OUT char *pszApiKey)
{
    cap_result result = ERR_CAP_UNKNOWN;
    char query[QUERY_SIZE];
    MYSQL_RES *pMysqlResult = NULL;
    MYSQL_ROW mysqlRow;

    int nRowCount = 0;

    snprintf(query, QUERY_SIZE, "\
            SELECT\
                vendor.api_key\
           FROM\
                things_product product,\
                things_vendor vendor\
            WHERE\
                product.identifier = '%s' and\
                vendor.id = product.vendor_id;", CAPString_LowPtr(strProductName, NULL));

    result = callQueryWithResult(pDBconn, query, &pMysqlResult, &nRowCount);
    ERRIFGOTO(result, _EXIT);

    mysqlRow = mysql_fetch_row(pMysqlResult);

    //if there is no matching service
    if(mysqlRow == NULL){
        dlp("There is no matching service!\n");
        ERRASSIGNGOTO(result, ERR_CAP_NO_DATA, _EXIT);
    }

    //if api key doesn't match
    if(strcmp(pszApiKey, mysqlRow[0]) != 0) {
        dlp("Api Key doesn't Match!!\n");
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEMYSQLFREE(pMysqlResult);
    return result;

}

cap_result DBHandler_VerifyDeviceApiKey(IN MYSQL *pDBconn, IN cap_string strDeviceId, IN char *pszApiKey)
{
    cap_result result = ERR_CAP_UNKNOWN;
    char query[QUERY_SIZE];
    MYSQL_RES *pMysqlResult = NULL;
    MYSQL_ROW mysqlRow;

    int nRowCount = 0;

    snprintf(query, QUERY_SIZE, "\
            SELECT\
                vendor.api_key\
           FROM\
                things_device device,\
                things_userthing userthing,\
                things_product product,\
                things_vendor vendor\
            WHERE\
                device.device_id = '%s' and\
                userthing.id = device.user_thing_id and\
                product.id = userthing.product_id and\
                vendor.id = product.vendor_id;", CAPString_LowPtr(strDeviceId, NULL));

    result = callQueryWithResult(pDBconn, query, &pMysqlResult, &nRowCount);
    ERRIFGOTO(result, _EXIT);

    mysqlRow = mysql_fetch_row(pMysqlResult);

    //if there is no matching device
    if(mysqlRow == NULL){
        dlp("There is no matching device!\n");
        ERRASSIGNGOTO(result, ERR_CAP_NO_DATA, _EXIT);
    }

    //if api key doesn't match
    if(strcmp(pszApiKey, mysqlRow[0]) != 0) {
        dlp("Api Key doesn't Match!!\n");
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEMYSQLFREE(pMysqlResult);
    return result;

}

cap_result DBHandler_RegisterDevice(IN MYSQL *pDBconn,IN cap_string strDeviceId, IN char *pszPinCode)
{
    cap_result result = ERR_CAP_UNKNOWN;
    char query[QUERY_SIZE];
    MYSQL_RES *pMysqlResult = NULL;
    MYSQL_ROW mysqlRow;
    int nRowCount = 0;
    
    //check if thing already has been registered
    snprintf(query, QUERY_SIZE, "\
            SELECT\
                device.is_connected\
            FROM\
                things_device device\
            WHERE\
                device.pin_code = '%s';", pszPinCode);

    result = callQueryWithResult(pDBconn, query, &pMysqlResult, &nRowCount);
    ERRIFGOTO(result, _EXIT);

    mysqlRow = mysql_fetch_row(pMysqlResult);
    
    //if there is no matching device with pin code
    if(mysqlRow == NULL){
        dlp("There is no matching device!\n");
        ERRASSIGNGOTO(result, ERR_CAP_NO_DATA, _EXIT);
    }

    //if thing already has been registered 
    if(atoiIgnoreNull(mysqlRow[0]) == TRUE){
        dlp("Thing is already registered!\n");
        ERRASSIGNGOTO(result, ERR_CAP_DUPLICATED, _EXIT);
    }

    snprintf(query, QUERY_SIZE, "\
            UPDATE\
                things_device device\
            SET\
                device.is_connected = 1,\
                device.updated_at = now()\
            where\
                device.pin_code = '%s';", pszPinCode);

    result = callQuery(pDBconn, query);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEMYSQLFREE(pMysqlResult);
    return result;

}

cap_result disableDeviceAndEca(IN MYSQL *pDBconn,IN cap_string strDeviceId)
{
    cap_result result = ERR_CAP_UNKNOWN;
    char query[QUERY_SIZE];
    
    //disable device
    snprintf(query, QUERY_SIZE, "\
            UPDATE\
                things_device device\
            SET\
                device.is_connected = 0\
            where\
                device.device_id = '%s'", CAPString_LowPtr(strDeviceId, NULL));
   
    result = callQuery(pDBconn, query);
    ERRIFGOTO(result, _EXIT);
   
    //disable condition related eca
    snprintf(query, QUERY_SIZE, "\
            UPDATE\
                things_device device,\
                things_eventconditionaction eca,\
                things_userthing userthing,\
                things_condition cond\
            SET\
                eca.usable = 0\
            where\
                device.device_id = '%s' and\
                userthing.id = device.user_thing_id and\
                cond.user_thing_id = userthing.id and\
                eca.id = cond.event_condition_action_id;", CAPString_LowPtr(strDeviceId, NULL));
    
    result = callQuery(pDBconn, query);
    ERRIFGOTO(result, _EXIT);
    
    //disable action related eca
    snprintf(query, QUERY_SIZE, "\
            UPDATE\
                things_device device,\
                things_eventconditionaction eca,\
                things_userthing userthing,\
                things_action action_\
            SET\
                eca.usable = 0\
            where\
                device.device_id = '%s' and\
                userthing.id = device.user_thing_id and\
                action_.user_thing_id = userthing.id and\
                eca.id = action_.event_condition_action_id;", CAPString_LowPtr(strDeviceId, NULL));
    
    result = callQuery(pDBconn, query);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result DBHandler_DisableDeviceAndEca(IN MYSQL *pDBconn,IN cap_string strDeviceId)
{
    cap_result result = ERR_CAP_UNKNOWN;

    result = disableDeviceAndEca(pDBconn, strDeviceId);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;

}

cap_result DBHandler_UnregisterDeviceAndEca(IN MYSQL *pDBconn,IN cap_string strDeviceId, IN char *pszPinCode)
{
    cap_result result = ERR_CAP_UNKNOWN;
    
    result = checkDeviceWithId(pDBconn, strDeviceId);
    ERRIFGOTO(result, _EXIT);

    result = disableDeviceAndEca(pDBconn, strDeviceId);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;

}

cap_result DBHandler_UpdateLatestTime(IN MYSQL *pDBconn,IN cap_string strDeviceId)
{
    cap_result result = ERR_CAP_UNKNOWN;
    char query[QUERY_SIZE];
    MYSQL_RES *pMysqlResult = NULL;
    
    result = checkDeviceWithId(pDBconn, strDeviceId);
    ERRIFGOTO(result, _EXIT);
    
    snprintf(query, QUERY_SIZE, "\
            UPDATE\
                things_device device\
            SET\
                device.updated_at = now()\
            where\
                device.device_id = '%s';", CAPString_LowPtr(strDeviceId, NULL));

    result = callQuery(pDBconn, query);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEMYSQLFREE(pMysqlResult);
    return result;
}

//TODO
//Current logic has too much overhead that it inserts general service variable to each service.
//However, since all variables are the same, it could be handled with one service.
static cap_result insertGeneralServiceVariableHistory(IN MYSQL *pDBconn, IN cap_string strProductName,\
        IN int nUserId, IN cap_string strVariableName, IN char * pszVariable)
{
    cap_result result = ERR_CAP_UNKNOWN;
    char query[QUERY_SIZE];
    MYSQL_RES *pMysqlResult = NULL;
    MYSQL_ROW mysqlRow;
    int nRowCount = 0;
    int nUserThingId = 0, nCustomerId = 0, nVariableId = 0;
    
    snprintf(query, QUERY_SIZE, "\
            SELECT\
                userthing.id,\
                userthing.customer_id,\
                variable.id\
            FROM\
                things_product product,\
                things_userthing userthing,\
                things_variable variable\
            WHERE\
                product.identifier = '%s' and\
                userthing.product_id = product.id and\
                variable.identifier = '%s' and\
                variable.product_id = product.id;", CAPString_LowPtr(strProductName, NULL), CAPString_LowPtr(strVariableName, NULL));
    
    result = callQueryWithResult(pDBconn, query, &pMysqlResult, &nRowCount);
    ERRIFGOTO(result, _EXIT);

    while( (mysqlRow = mysql_fetch_row(pMysqlResult)) ) 
    {
        //if there is no matching service
        if(mysqlRow == NULL){
            dlp("There is no matching service!\n");
            ERRASSIGNGOTO(result, ERR_CAP_NO_DATA, _EXIT);
        }
        else {
            nUserThingId = atoiIgnoreNull(mysqlRow[0]);
            nCustomerId = atoiIgnoreNull(mysqlRow[1]);
            nVariableId = atoiIgnoreNull(mysqlRow[2]);
        }
   
        //If custumor hasn't connected their service, service's customer id is set as null
        if(nCustomerId == NULL_ERROR) {
            snprintf(query, QUERY_SIZE, "\
                    INSERT INTO\
                    things_variablehistory(created_at, updated_at, user_thing_id, variable_id, value)\
                    VALUES(now(), now(), %d, %d, '%s');", nUserThingId, nVariableId, pszVariable);
            
        }
        else {
            snprintf(query, QUERY_SIZE, "\
                    INSERT INTO\
                    things_variablehistory(created_at, updated_at, customer_id, user_thing_id, variable_id, value)\
                    VALUES(now(), now(), %d, %d, %d, '%s');", nCustomerId, nUserThingId, nVariableId, pszVariable);
        }

        result = callQuery(pDBconn, query);
        ERRIFGOTO(result, _EXIT);
    
    }
    
    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEMYSQLFREE(pMysqlResult);
    return result;
}

static cap_result insertSpecificServiceVariableHistory(IN MYSQL *pDBconn, IN cap_string strProductName,\
        IN int nUserId, IN cap_string strVariableName, IN char * pszVariable)
{
    cap_result result = ERR_CAP_UNKNOWN;
    char query[QUERY_SIZE];
    MYSQL_RES *pMysqlResult = NULL;
    MYSQL_ROW mysqlRow;
    int nRowCount = 0;
    int nUserThingId = 0, nCustomerId = 0, nVariableId = 0;
    
    snprintf(query, QUERY_SIZE, "\
            SELECT\
                userthing.id,\
                userthing.customer_id,\
                variable.id\
            FROM\
                things_product product,\
                things_userthing userthing,\
                things_variable variable,\
                things_customer customer\
            WHERE\
                product.identifier = '%s' and\
                customer.user_id = %d and\
                userthing.product_id = product.id and\
                userthing.customer_id = customer.id and\
                variable.identifier = '%s' and\
                variable.product_id = product.id;", CAPString_LowPtr(strProductName, NULL), nUserId, CAPString_LowPtr(strVariableName, NULL));
    
   
    result = callQueryWithResult(pDBconn, query, &pMysqlResult, &nRowCount);
    ERRIFGOTO(result, _EXIT);

    mysqlRow = mysql_fetch_row(pMysqlResult);
    
    //if there is no matching service
    if(mysqlRow == NULL){
        dlp("There is no matching service!\n");
        ERRASSIGNGOTO(result, ERR_CAP_NO_DATA, _EXIT);
    }
    else {
        nUserThingId = atoiIgnoreNull(mysqlRow[0]);
        nCustomerId = atoiIgnoreNull(mysqlRow[1]);
        nVariableId = atoiIgnoreNull(mysqlRow[2]);
    }
   
    //If custumor hasn't connected their service, service's customer id is set as null
    if(nCustomerId == NULL_ERROR) {
        snprintf(query, QUERY_SIZE, "\
                INSERT INTO\
                things_variablehistory(created_at, updated_at, user_thing_id, variable_id, value)\
                VALUES(now(), now(), %d, %d, '%s');", nUserThingId, nVariableId, pszVariable);
        
    }
    else {
        snprintf(query, QUERY_SIZE, "\
                INSERT INTO\
                things_variablehistory(created_at, updated_at, customer_id, user_thing_id, variable_id, value)\
                VALUES(now(), now(), %d, %d, %d, '%s');", nCustomerId, nUserThingId, nVariableId, pszVariable);
    }

    result = callQuery(pDBconn, query);
    ERRIFGOTO(result, _EXIT);
    
    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEMYSQLFREE(pMysqlResult);
    return result;
}

cap_result DBHandler_InsertServiceVariableHistory(IN MYSQL *pDBconn, IN cap_string strProductName,\
        IN int nUserId, IN cap_string strVariableName, IN char * pszVariable)
{
    cap_result result = ERR_CAP_UNKNOWN;
    
    result = checkServiceWithName(pDBconn, strProductName);
    ERRIFGOTO(result, _EXIT);
    
    //If nUserId == -1, it means a service is a general service such as date and time
    if(nUserId == -1)
    {
        result = insertGeneralServiceVariableHistory(pDBconn, strProductName, nUserId, strVariableName, pszVariable);
        ERRIFGOTO(result, _EXIT);
    }
    else {
        result = insertSpecificServiceVariableHistory(pDBconn, strProductName, nUserId, strVariableName, pszVariable);
        ERRIFGOTO(result, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result DBHandler_InsertDeviceVariableHistory(IN MYSQL *pDBconn,IN cap_string strDeviceId, IN cap_string strVariableName, IN char * pszVariable)
{
    cap_result result = ERR_CAP_UNKNOWN;
    char query[QUERY_SIZE];
    MYSQL_RES *pMysqlResult = NULL;
    MYSQL_ROW mysqlRow;
    int nRowCount = 0;
    int nUserThingId = 0, nCustomerId = 0, nVariableId = 0;
    
    result = checkDeviceWithId(pDBconn, strDeviceId);
    ERRIFGOTO(result, _EXIT);

    snprintf(query, QUERY_SIZE, "\
            SELECT\
                device.user_thing_id,\
                userthing.customer_id,\
                variable.id\
            FROM\
                things_device device,\
                things_userthing userthing,\
                things_variable variable\
            WHERE\
                device.device_id = '%s' and\
                userthing.id = device.user_thing_id and\
                variable.identifier = '%s';", CAPString_LowPtr(strDeviceId, NULL), CAPString_LowPtr(strVariableName, NULL));
    
    result = callQueryWithResult(pDBconn, query, &pMysqlResult, &nRowCount);
    ERRIFGOTO(result, _EXIT);

    mysqlRow = mysql_fetch_row(pMysqlResult);
    
    //if there is no matching device with pin code
    if(mysqlRow == NULL){
        dlp("There is no matching device!\n");
        ERRASSIGNGOTO(result, ERR_CAP_NO_DATA, _EXIT);
    }
    else {
        nUserThingId = atoiIgnoreNull(mysqlRow[0]);
        nCustomerId = atoiIgnoreNull(mysqlRow[1]);
        nVariableId = atoiIgnoreNull(mysqlRow[2]);
    }
   
    //If custumor hasn't connected their device with pin code, device's customer id is set as null
    if(nCustomerId == NULL_ERROR) {
        snprintf(query, QUERY_SIZE, "\
                INSERT INTO\
                things_variablehistory(created_at, updated_at, user_thing_id, variable_id, value)\
                VALUES(now(), now(), %d, %d, '%s');", nUserThingId, nVariableId, pszVariable);
        
    }
    else {
        snprintf(query, QUERY_SIZE, "\
                INSERT INTO\
                things_variablehistory(created_at, updated_at, customer_id, user_thing_id, variable_id, value)\
                VALUES(now(), now(), %d, %d, %d, '%s');", nCustomerId, nUserThingId, nVariableId, pszVariable);
    }

    result = callQuery(pDBconn, query);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEMYSQLFREE(pMysqlResult);
    return result;
}

cap_result DBHandler_InsertServiceApplicationHistory(IN MYSQL *pDBconn, IN cap_string strProductName, IN cap_string strFunctionName, IN int nEcaId, IN int nUserId, IN int nErrorCode)
{
    cap_result result = ERR_CAP_UNKNOWN;
    char query[QUERY_SIZE];
    MYSQL_RES *pMysqlResult = NULL;
    MYSQL_ROW mysqlRow;
    int nRowCount = 0;
    int nCustomerId = 0, nFunctionId = 0;
    
    snprintf(query, QUERY_SIZE, "\
            SELECT\
                function.id\
            FROM\
                things_function function\
            WHERE\
                function.identifier = '%s';", CAPString_LowPtr(strFunctionName, NULL));
   
    result = callQueryWithResult(pDBconn, query, &pMysqlResult, &nRowCount);
    ERRIFGOTO(result, _EXIT);

    mysqlRow = mysql_fetch_row(pMysqlResult);
    
    //if there is no matching service
    if(mysqlRow == NULL){
        dlp("There is no matching service!\n");
        ERRASSIGNGOTO(result, ERR_CAP_NO_DATA, _EXIT);
    }
    else {
        nFunctionId = atoiIgnoreNull(mysqlRow[0]);
    }
 
    memset(query, 0, QUERY_SIZE);
    snprintf(query, QUERY_SIZE, "\
            SELECT\
                customer.id\
            FROM\
                things_customer customer\
            WHERE\
                customer.user_id = %d;", nUserId);
   
    result = callQueryWithResult(pDBconn, query, &pMysqlResult, &nRowCount);
    ERRIFGOTO(result, _EXIT);

    mysqlRow = mysql_fetch_row(pMysqlResult);
    
    //if there is no matching service
    if(mysqlRow == NULL){
        dlp("There is no matching service!\n");
        ERRASSIGNGOTO(result, ERR_CAP_NO_DATA, _EXIT);
    }
    else {
        nCustomerId = atoiIgnoreNull(mysqlRow[0]);
    } 
    //TODO
    //Add message to database with error code
    if(nErrorCode == 0) {
        snprintf(query, QUERY_SIZE, "\
                INSERT INTO\
                things_applicationhistory(created_at, updated_at, customer_id, event_condition_action_id, function_id, status)\
                VALUES(now(), now(), %d, %d, %d, '%s');", nCustomerId, nEcaId, nFunctionId, "success");
    }
    else {
        snprintf(query, QUERY_SIZE, "\
                INSERT INTO\
                things_applicationhistory(created_at, updated_at, customer_id, event_condition_action_id, function_id, status, message)\
                VALUES(now(), now(), %d, %d, %d, '%s', '%d');", nCustomerId, nEcaId, nFunctionId, "failed", nErrorCode);
    }

    result = callQuery(pDBconn, query);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEMYSQLFREE(pMysqlResult);
    return result;
}

cap_result DBHandler_InsertDeviceApplicationHistory(IN MYSQL *pDBconn,IN cap_string strDeviceId, IN cap_string strFunctionName, IN int nEcaId, IN int nErrorCode)
{
    cap_result result = ERR_CAP_UNKNOWN;
    char query[QUERY_SIZE];
    MYSQL_RES *pMysqlResult = NULL;
    MYSQL_ROW mysqlRow;
    int nRowCount = 0;
    int nCustomerId = 0, nFunctionId = 0;
    
    result = checkDeviceWithId(pDBconn, strDeviceId);
    ERRIFGOTO(result, _EXIT);

    snprintf(query, QUERY_SIZE, "\
            SELECT\
                userthing.customer_id,\
                function.id\
            FROM\
                things_device device,\
                things_userthing userthing,\
                things_function function\
            WHERE\
                device.device_id = '%s' and\
                userthing.id = device.user_thing_id and\
                function.identifier = '%s';", CAPString_LowPtr(strDeviceId, NULL), CAPString_LowPtr(strFunctionName, NULL));
   
    result = callQueryWithResult(pDBconn, query, &pMysqlResult, &nRowCount);
    ERRIFGOTO(result, _EXIT);

    mysqlRow = mysql_fetch_row(pMysqlResult);
    
    //if there is no matching device with pin code
    if(mysqlRow == NULL){
        dlp("There is no matching device!\n");
        ERRASSIGNGOTO(result, ERR_CAP_NO_DATA, _EXIT);
    }
    else {
        nCustomerId = atoiIgnoreNull(mysqlRow[0]);
        nFunctionId = atoiIgnoreNull(mysqlRow[1]);
    }
  
    //TODO
    //Added message to database with error code
    if(nErrorCode == 0) {
        snprintf(query, QUERY_SIZE, "\
                INSERT INTO\
                things_applicationhistory(created_at, updated_at, customer_id, event_condition_action_id, function_id, status)\
                VALUES(now(), now(), %d, %d, %d, '%s');", nCustomerId, nEcaId, nFunctionId, "success");
    }
    else {
        snprintf(query, QUERY_SIZE, "\
                INSERT INTO\
                things_applicationhistory(created_at, updated_at, customer_id, event_condition_action_id, function_id, status)\
                VALUES(now(), now(), %d, %d, %d, %s);", nCustomerId, nEcaId, nFunctionId, "failed");
    }
    result = callQuery(pDBconn, query);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEMYSQLFREE(pMysqlResult);
    return result;
}

cap_result DBHandler_MakeConditionListWithService(IN MYSQL *pDBconn, IN cap_string strProductName,\
        IN cap_string strVariableName, IN int nUserId, IN OUT cap_handle hRelatedConditionList)
{
    cap_result result = ERR_CAP_UNKNOWN;
    char query[QUERY_SIZE];
    MYSQL_RES *pMysqlResult = NULL;
    MYSQL_ROW mysqlRow;
    int nRowCount = 0;

    result = checkServiceWithName(pDBconn, strProductName);
    ERRIFGOTO(result, _EXIT);

    //If nUserId == -1, it means a service is a general service such as date and time
    if(nUserId == -1) {
        snprintf(query, QUERY_SIZE, "\
                SELECT\
                    cond.id,\
                    cond.expression,\
                    eca.id,\
                    eca.operator,\
                    variable.type,\
        			(SELECT count(*) FROM things_condition cond WHERE cond.event_condition_action_id = eca.id) as cnt\
                FROM\
                    things_product product,\
                    things_userthing userthing,\
                    things_variable variable,\
                    things_condition cond,\
                    things_eventconditionaction eca\
                WHERE\
                    product.identifier = '%s' and\
                    userthing.product_id = product.id and\
                    variable.identifier = '%s' and\
                    variable.product_id = product.id and\
                    eca.customer_id = userthing.customer_id  and\
                    eca.usable = 1 and\
                    cond.user_thing_id = userthing.id and\
                    cond.variable_id = variable.id and\
                    cond.event_condition_action_id = eca.id;", CAPString_LowPtr(strProductName, NULL), CAPString_LowPtr(strVariableName, NULL));
    }
    else {
        snprintf(query, QUERY_SIZE, "\
                SELECT\
                    cond.id,\
                    cond.expression,\
                    eca.id,\
                    eca.operator,\
                    variable.type,\
        			(SELECT count(*) FROM things_condition cond WHERE cond.event_condition_action_id = eca.id) as cnt\
                FROM\
                    things_product product,\
                    things_userthing userthing,\
                    things_variable variable,\
                    things_customer customer,\
                    things_condition cond,\
                    things_eventconditionaction eca\
                WHERE\
                    product.identifier = '%s' and\
                    customer.user_id = %d and\
                    userthing.product_id = product.id and\
                    userthing.customer_id = customer.id and\
                    variable.identifier = '%s' and\
                    variable.product_id = product.id and\
                    eca.customer_id = userthing.customer_id  and\
                    eca.usable = 1 and\
                    cond.user_thing_id = userthing.id and\
                    cond.variable_id = variable.id and\
                    cond.event_condition_action_id = eca.id;", CAPString_LowPtr(strProductName, NULL), nUserId, CAPString_LowPtr(strVariableName, NULL));
    }

    result = callQueryWithResult(pDBconn, query, &pMysqlResult, &nRowCount);
    ERRIFGOTO(result, _EXIT);

    while( (mysqlRow = mysql_fetch_row(pMysqlResult)) ) 
    {
        SConditionContext *pstConditionContext = (SConditionContext*)calloc(1, sizeof(SConditionContext));    
    
        pstConditionContext->strExpression = CAPString_New();
        ERRMEMGOTO(pstConditionContext->strExpression, result, _EXIT);
        
        pstConditionContext->nConditionId = atoiIgnoreNull(mysqlRow[0]);

        result = CAPString_SetLow(pstConditionContext->strExpression, mysqlRow[1] , CAPSTRING_MAX);
        ERRIFGOTO(result, _EXIT);

        pstConditionContext->nEcaId = atoiIgnoreNull(mysqlRow[2]);

        //check operator
        if(strncmp(mysqlRow[3], "all", 3) == 0){
            pstConditionContext->enEcaOp = OPERATOR_AND;
        }
        else if(strncmp(mysqlRow[3], "any", 3) == 0){
            pstConditionContext->enEcaOp = OPERATOR_OR;
        }
        else {
            //Not supported
            dlp("not supported operator\n");
        }
       
        //check type 
        if(strncmp(mysqlRow[4], "integer", 7) == 0){
            pstConditionContext->enType = TYPE_INTEGER;
        }
        else if(strncmp(mysqlRow[4], "double", 6) == 0){
            pstConditionContext->enType = TYPE_DOUBLE;
        }
        else if(strncmp(mysqlRow[4], "binary", 6) == 0){
            pstConditionContext->enType = TYPE_BINARY;
        }
        else if(strncmp(mysqlRow[4], "string", 6) == 0){
            pstConditionContext->enType = TYPE_STRING;
        }
        else if(strncmp(mysqlRow[4], "select", 6) == 0){
            pstConditionContext->enType = TYPE_SELECT;
        }
        else {
            //Not supported
            dlp("not supported type\n");
        }

        //check if there is only one condition in eca
        if(strncmp(mysqlRow[5], "1", 1) == 0){
            dlp("is a single condition\n");
            pstConditionContext->bIsSingleCondition = TRUE;
        }
        else {
            dlp("not a single condition\n");
            pstConditionContext->bIsSingleCondition = FALSE;
        }
        
        result = CAPLinkedList_Add(hRelatedConditionList, LINKED_LIST_OFFSET_LAST, 0, pstConditionContext);
        ERRIFGOTO(result, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEMYSQLFREE(pMysqlResult);
    return result;
}
cap_result DBHandler_MakeConditionListWithDevice(IN MYSQL *pDBconn, IN cap_string strDeviceId,\
        IN cap_string strVariableName, IN OUT cap_handle hRelatedConditionList)
{
    cap_result result = ERR_CAP_UNKNOWN;
    char query[QUERY_SIZE];
    MYSQL_RES *pMysqlResult = NULL;
    MYSQL_ROW mysqlRow;
    int nRowCount = 0;

    result = checkDeviceWithId(pDBconn, strDeviceId);
    ERRIFGOTO(result, _EXIT);
   
    snprintf(query, QUERY_SIZE, "\
            SELECT\
                cond.id,\
                cond.expression,\
                eca.id,\
                eca.operator,\
                variable.type,\
    			(SELECT count(*) FROM things_condition cond WHERE cond.event_condition_action_id = eca.id) as cnt\
            FROM\
                things_device device,\
                things_userthing userthing,\
                things_variable variable,\
                things_condition cond,\
                things_eventconditionaction eca\
            WHERE\
                device.device_id = '%s' and\
                variable.identifier = '%s' and \
                userthing.id = device.user_thing_id and\
                eca.customer_id = userthing.customer_id  and\
                eca.usable = 1 and\
                cond.user_thing_id = device.user_thing_id and\
                cond.variable_id = variable.id and\
                cond.event_condition_action_id = eca.id;", CAPString_LowPtr(strDeviceId, NULL), CAPString_LowPtr(strVariableName, NULL));

    result = callQueryWithResult(pDBconn, query, &pMysqlResult, &nRowCount);
    ERRIFGOTO(result, _EXIT);

    while( (mysqlRow = mysql_fetch_row(pMysqlResult)) ) 
    {
        SConditionContext *pstConditionContext = (SConditionContext*)calloc(1, sizeof(SConditionContext));    
    
        pstConditionContext->strExpression = CAPString_New();
        ERRMEMGOTO(pstConditionContext->strExpression, result, _EXIT);
        
        pstConditionContext->nConditionId = atoiIgnoreNull(mysqlRow[0]);

        result = CAPString_SetLow(pstConditionContext->strExpression, mysqlRow[1] , CAPSTRING_MAX);
        ERRIFGOTO(result, _EXIT);

        pstConditionContext->nEcaId = atoiIgnoreNull(mysqlRow[2]);

        //check operator
        if(strncmp(mysqlRow[3], "all", 3) == 0){
            pstConditionContext->enEcaOp = OPERATOR_AND;
        }
        else if(strncmp(mysqlRow[3], "any", 3) == 0){
            pstConditionContext->enEcaOp = OPERATOR_OR;
        }
        else {
            //Not supported
            dlp("not supported operator\n");
        }
       
        //check type 
        if(strncmp(mysqlRow[4], "integer", 7) == 0){
            pstConditionContext->enType = TYPE_INTEGER;
        }
        else if(strncmp(mysqlRow[4], "double", 6) == 0){
            pstConditionContext->enType = TYPE_DOUBLE;
        }
        else if(strncmp(mysqlRow[4], "binary", 6) == 0){
            pstConditionContext->enType = TYPE_BINARY;
        }
        else if(strncmp(mysqlRow[4], "string", 6) == 0){
            pstConditionContext->enType = TYPE_STRING;
        }
        else if(strncmp(mysqlRow[4], "select", 6) == 0){
            pstConditionContext->enType = TYPE_SELECT;
        }
        else {
            //Not supported
            dlp("not supported type\n");
        }

        //check if there is only one condition in eca
        if(strncmp(mysqlRow[5], "1", 1) == 0){
            dlp("is a single condition\n");
            pstConditionContext->bIsSingleCondition = TRUE;
        }
        else {
            dlp("not a single condition\n");
            pstConditionContext->bIsSingleCondition = FALSE;
        }
        
        result = CAPLinkedList_Add(hRelatedConditionList, LINKED_LIST_OFFSET_LAST, 0, pstConditionContext);
        ERRIFGOTO(result, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEMYSQLFREE(pMysqlResult);
    return result;
}

static cap_result retrieveDeviceActionList(IN MYSQL *pDBconn, IN int nEcaId, IN OUT cap_handle hActionList)
{
    cap_result result = ERR_CAP_UNKNOWN;
    char query[QUERY_SIZE];
    MYSQL_RES *pMysqlResult = NULL;
    MYSQL_ROW mysqlRow;
    int nRowCount = 0;
    char *pszArgumentPayload = NULL;
    
    snprintf(query, QUERY_SIZE, "\
            SELECT\
                function.identifier,\
                action.arguments,\
                device.device_id\
            FROM\
                things_function function,\
                things_action action,\
                things_product product,\
                things_userthing userthing,\
                things_device device\
            WHERE\
                action.event_condition_action_id = %d and\
                function.id = action.function_id and \
                function.product_id = product.id and\
                action.user_thing_id = userthing.id and\
                device.user_thing_id = userthing.id;", nEcaId);

    result = callQueryWithResult(pDBconn, query, &pMysqlResult, &nRowCount);
    ERRIFGOTO(result, _EXIT);

    while( (mysqlRow = mysql_fetch_row(pMysqlResult)) ) 
    {
        SActionContext *pstActionContext = (SActionContext*)calloc(1, sizeof(SActionContext));   
        
        pstActionContext->strFunctionName = CAPString_New();
        ERRMEMGOTO(pstActionContext->strFunctionName, result, _EXIT);
        
        result = CAPString_SetLow(pstActionContext->strFunctionName, mysqlRow[0] , CAPSTRING_MAX);
        ERRIFGOTO(result, _EXIT);
        
        //Handle if argument is null
        if(mysqlRow[1] != NULL && ( strcmp("null", mysqlRow[1]) != 0 ) ){
            pstActionContext->strArgumentPayload = CAPString_New();
            ERRMEMGOTO(pstActionContext->strArgumentPayload, result, _EXIT);
        
            //replaceWithRealVariable then add it to structure 
            result = replaceWithRealVariable(pDBconn, mysqlRow[1], &pszArgumentPayload);
            ERRIFGOTO(result, _EXIT);

            result = CAPString_SetLow(pstActionContext->strArgumentPayload, pszArgumentPayload , CAPSTRING_MAX);
            ERRIFGOTO(result, _EXIT);

            SAFEMEMFREE(pszArgumentPayload);
        }
        else {
            //if there is no argument, set payload as null
            pstActionContext->strArgumentPayload = NULL;
        }

        pstActionContext->strReceiverId = CAPString_New();
        ERRMEMGOTO(pstActionContext->strReceiverId, result, _EXIT);
        
        result = CAPString_SetLow(pstActionContext->strReceiverId, mysqlRow[2] , CAPSTRING_MAX);
        ERRIFGOTO(result, _EXIT);

        result = CAPLinkedList_Add(hActionList, LINKED_LIST_OFFSET_LAST, 0, pstActionContext);
        ERRIFGOTO(result, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEMYSQLFREE(pMysqlResult);
    return result;
}


static cap_result retrieveServiceActionList(IN MYSQL *pDBconn, IN int nEcaId, IN OUT cap_handle hActionList)
{
    cap_result result = ERR_CAP_UNKNOWN;
    char query[QUERY_SIZE];
    MYSQL_RES *pMysqlResult = NULL;
    MYSQL_ROW mysqlRow;
    int nRowCount = 0;
    char *pszArgumentPayload = NULL;

    snprintf(query, QUERY_SIZE, "\
            SELECT\
                function.identifier,\
                action.arguments,\
                customer.user_id,\
                product.identifier\
            FROM\
                things_function function,\
                things_action action,\
                things_product product,\
                things_service service,\
                things_userthing userthing,\
                things_customer customer\
            WHERE\
                action.event_condition_action_id = %d and\
                function.id = action.function_id and\
                function.product_id = product.id and\
                action.user_thing_id = userthing.id and\
                service.user_thing_id = userthing.id and\
                userthing.customer_id = customer.id;", nEcaId);
    result = callQueryWithResult(pDBconn, query, &pMysqlResult, &nRowCount);
    ERRIFGOTO(result, _EXIT);

    while( (mysqlRow = mysql_fetch_row(pMysqlResult)) ) 
    {
        SActionContext *pstActionContext = (SActionContext*)calloc(1, sizeof(SActionContext));   
        
        pstActionContext->strFunctionName = CAPString_New();
        ERRMEMGOTO(pstActionContext->strFunctionName, result, _EXIT);
        
        result = CAPString_SetLow(pstActionContext->strFunctionName, mysqlRow[0] , CAPSTRING_MAX);
        ERRIFGOTO(result, _EXIT);

        //Handle if argument is null
        if(mysqlRow[1] != NULL && ( strcmp("null", mysqlRow[1]) != 0 ) ){
            pstActionContext->strArgumentPayload = CAPString_New();
            ERRMEMGOTO(pstActionContext->strArgumentPayload, result, _EXIT);
        
            //replaceWithRealVariable then add it to structure 
            result = replaceWithRealVariable(pDBconn, mysqlRow[1], &pszArgumentPayload);
            ERRIFGOTO(result, _EXIT);

            result = CAPString_SetLow(pstActionContext->strArgumentPayload, pszArgumentPayload , CAPSTRING_MAX);
            ERRIFGOTO(result, _EXIT);

            SAFEMEMFREE(pszArgumentPayload);
        }
        else {
            //if there is no argument, set payload as null
            pstActionContext->strArgumentPayload = NULL;
        }

        pstActionContext->bIsServiceType = 1;

        pstActionContext->nUserId = atoiIgnoreNull(mysqlRow[2]); 
        
        pstActionContext->strReceiverId = CAPString_New();
        ERRMEMGOTO(pstActionContext->strReceiverId, result, _EXIT);
        
        result = CAPString_SetLow(pstActionContext->strReceiverId, mysqlRow[3] , CAPSTRING_MAX);
        ERRIFGOTO(result, _EXIT);

        result = CAPLinkedList_Add(hActionList, LINKED_LIST_OFFSET_LAST, 0, pstActionContext);
        ERRIFGOTO(result, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEMYSQLFREE(pMysqlResult);
    return result;
}

cap_result DBHandler_RetrieveActionList(IN MYSQL *pDBconn, IN int nEcaId, IN OUT cap_handle hActionList)
{
    cap_result result = ERR_CAP_UNKNOWN;

    result = retrieveDeviceActionList(pDBconn, nEcaId, hActionList);
    ERRIFGOTO(result, _EXIT);
    
    result = retrieveServiceActionList(pDBconn, nEcaId, hActionList);
    ERRIFGOTO(result, _EXIT);
    
    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result DBHandler_RetrieveSatisfiedEcaList(IN MYSQL *pDBconn, IN OUT cap_handle hSatisfiedEcaList)
{
    cap_result result = ERR_CAP_UNKNOWN;
    char query[QUERY_SIZE];
    MYSQL_RES *pMysqlResult = NULL;
    MYSQL_ROW mysqlRow;
    int nRowCount = 0;

    //Retrieve all eca where all conditions are satisfied 
    snprintf(query, QUERY_SIZE, "\
            SELECT\
                eca.id\
            FROM\
                things_eventconditionaction eca\
            WHERE\
                NOT EXISTS(\
                    SELECT\
                        *\
                    FROM\
                        things_condition cond\
                    WHERE\
                        cond.event_condition_action_id = eca.id and\
                         cond.is_satisfied = 0\
                    );");

    result = callQueryWithResult(pDBconn, query, &pMysqlResult, &nRowCount);
    ERRIFGOTO(result, _EXIT);

    while( (mysqlRow = mysql_fetch_row(pMysqlResult)) ) 
    {
        int *pnSatisfiedEca = (int *)malloc(sizeof(int));
        *pnSatisfiedEca = atoiIgnoreNull(mysqlRow[0]);

        result = CAPLinkedList_Add(hSatisfiedEcaList, LINKED_LIST_OFFSET_LAST, 0, (void *)pnSatisfiedEca);
        ERRIFGOTO(result, _EXIT);

        //make is_satisfied to false of related satisfied condition, once added to satisfied eca list
        snprintf(query, QUERY_SIZE, "\
                UPDATE\
                    things_condition cond\
                SET\
                    cond.is_satisfied = 0\
                WHERE\
                   cond.event_condition_action_id = %d;", *pnSatisfiedEca);
        result = callQuery(pDBconn, query);
        ERRIFGOTO(result, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEMYSQLFREE(pMysqlResult);
    return result;
}

cap_result DBHandler_InsertSatisfiedCondition(IN MYSQL *pDBconn, IN cap_handle hRelatedConditionList)
{
    cap_result result = ERR_CAP_UNKNOWN;
    char query[QUERY_SIZE];
    MYSQL_RES *pMysqlResult = NULL;
    int nLength = 0;
    int nLoop = 0;
    SConditionContext* pstConditionContext = NULL;

    result = CAPLinkedList_GetLength(hRelatedConditionList, &nLength);
    ERRIFGOTO(result, _EXIT);

    for(nLoop = 0; nLoop < nLength; nLoop++){
        result = CAPLinkedList_Get(hRelatedConditionList, LINKED_LIST_OFFSET_FIRST, nLoop, (void**)&pstConditionContext);
        ERRIFGOTO(result, _EXIT);

        //if condition is satisfied and there is only one condition or operator 'any' condition -> do nothing 
        //else -> set is_satisfied to TRUE
        if(pstConditionContext->bIsSatisfied) {
            if(pstConditionContext->bIsSingleCondition || pstConditionContext->enEcaOp == OPERATOR_OR) {
                //do nothing
            }
            else {
                snprintf(query, QUERY_SIZE, "\
                        UPDATE\
                            things_condition cond\
                        SET\
                            cond.is_satisfied = 1\
                        where\
                            cond.id = %d;", pstConditionContext->nConditionId);
                result = callQuery(pDBconn, query);
                ERRIFGOTO(result, _EXIT);
            }
        }
    }
    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEMYSQLFREE(pMysqlResult);
    return result;
}


cap_result DBHandler_MakeThingAliveInfoArray(IN MYSQL *pDBconn, IN OUT SThingAliveInfo **ppstThingAliveInfoArray, IN int *pnArrayLength)
{
    cap_result result = ERR_CAP_UNKNOWN;
    char query[QUERY_SIZE];
    MYSQL_RES *pMysqlResult = NULL;
    MYSQL_ROW mysqlRow;
    int nRowCount = 0;
    int nLoop = 0;
    int nArrayNum = 0;
    SThingAliveInfo *pstThingAliveInfoArray = NULL;

    snprintf(query, QUERY_SIZE, "\
            SELECT\
                COUNT(*)\
            FROM\
                things_device device\
            WHERE\
                device.is_connected = 1;");

    result = callQueryWithResult(pDBconn, query, &pMysqlResult, &nRowCount);
    ERRIFGOTO(result, _EXIT);

    mysqlRow = mysql_fetch_row(pMysqlResult);
    
    //if there is no device 
    if(mysqlRow == NULL){
        dlp("There is no registered device at the time\n");
        ERRASSIGNGOTO(result, ERR_CAP_NO_DATA, _EXIT);
    }
    else {
        nArrayNum = atoiIgnoreNull(mysqlRow[0]);
    } 
    SAFEMYSQLFREE(pMysqlResult);

    //Allocate Memory to Array
    pstThingAliveInfoArray = (SThingAliveInfo*)calloc(nArrayNum, sizeof(SThingAliveInfo));
    ERRMEMGOTO(pstThingAliveInfoArray, result, _EXIT);
    
    for (nLoop = 0; nLoop < nArrayNum; nLoop++)
    {
        pstThingAliveInfoArray[nLoop].strDeviceId = NULL;
    }

    snprintf(query, QUERY_SIZE, "\
            SELECT\
                device.device_id,\
                product.alive_cycle,\
                UNIX_TIMESTAMP(device.updated_at)\
            FROM\
                things_device device,\
                things_product product,\
                things_userthing userthing\
            WHERE\
                device.is_connected = 1 and\
                device.user_thing_id = userthing.id and\
                userthing.product_id = product.id;");
    
    result = callQueryWithResult(pDBconn, query, &pMysqlResult, &nRowCount);
    ERRIFGOTO(result, _EXIT);

    //Make Thing Alive Array
    for (nLoop = 0; (mysqlRow = mysql_fetch_row(pMysqlResult)) ; nLoop++) {
        if(nLoop >= nArrayNum){
            ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
        }
        
        pstThingAliveInfoArray[nLoop].strDeviceId = CAPString_New();
        ERRMEMGOTO(pstThingAliveInfoArray[nLoop].strDeviceId, result, _EXIT);

        result = CAPString_SetLow(pstThingAliveInfoArray[nLoop].strDeviceId, (const char *)mysqlRow[0], CAPSTRING_MAX);
        ERRIFGOTO(result, _EXIT);

        pstThingAliveInfoArray[nLoop].llAliveCycle = (long long)atoiIgnoreNull(mysqlRow[1]);

        if(mysqlRow[2] != NULL){
            pstThingAliveInfoArray[nLoop].llLatestTime =(long long) (atof(mysqlRow[2]) * 1000); //convert second to millisecond
        }
    }

    *ppstThingAliveInfoArray = pstThingAliveInfoArray;
    *pnArrayLength = nArrayNum;

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEMYSQLFREE(pMysqlResult);
    return result;
}



