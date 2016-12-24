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

#define QUERY_SIZE 1024*16
#define NULL_ERROR -1

//TODO
//Receive MYQL * as an argument, but use g_pDBconn internally.
//This is because it shows error when it uses pDBconnParam as an argument
//Check why this happens when it points to same address when checking with gdb 
static cap_result callQueryWithResult(MYSQL* pDBconn, char* query, MYSQL_RES **ppMysqlResult, int *pnRowCount)
{
    cap_result ret = ERR_CAP_UNKNOWN;
    int nQueryRet = 0;

    nQueryRet = mysql_query(pDBconn, query);

    //fprintf(stderr, "Mysql connection error : %s, mysql_errno : %d\n", mysql_error(&pDBconn), mysql_errno(&pDBconn));
    if (nQueryRet != 0) {
		CAPLogger_Write(g_hLogger, MSG_ERROR, "DBHandler: callQueryWithResult Error : %d",nQueryRet );
        ERRASSIGNGOTO(ret, ERR_CAP_DB_ERROR, _EXIT);
    }

    /* Warning! 
     * MysqlResult has to be freed from where it is called
     */

    *ppMysqlResult = mysql_store_result(pDBconn);
    if(*ppMysqlResult == NULL) {
		CAPLogger_Write(g_hLogger, MSG_ERROR, "DBHandler: Result Store Error : %d", nQueryRet);
        ERRASSIGNGOTO(ret, ERR_CAP_DB_ERROR, _EXIT);
    }

    *pnRowCount = mysql_num_fields(*ppMysqlResult);

    ret = ERR_CAP_NOERROR;
_EXIT:
    return ret;
}

static cap_result callQuery(MYSQL* pDBconn, char* query)
{
    cap_result ret = ERR_CAP_UNKNOWN;
    int nQueryRet = 0;
    
    nQueryRet = mysql_query(pDBconn, query);
    if (nQueryRet != 0) {
		CAPLogger_Write(g_hLogger, MSG_ERROR, "DBHandler: callQuery Error : %d", nQueryRet);
        ERRASSIGNGOTO(ret, ERR_CAP_DB_ERROR, _EXIT);
    }

    ret = ERR_CAP_NOERROR;
_EXIT:
    return ret;
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

cap_result DBHandler_VerifyApiKey(IN MYSQL *pDBconn, IN cap_string strDeviceId, IN char *pszApiKey)
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
                product.id = device.user_thing_id and\
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
    if(strncmp(pszApiKey, mysqlRow[0], strlen(pszApiKey)) != 0) {
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
                device.is_connected = 1\
            where\
                device.pin_code = '%s';", pszPinCode);

    result = callQuery(pDBconn, query);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEMYSQLFREE(pMysqlResult);
    return result;

}

cap_result DBHandler_UnregisterDevice(IN MYSQL *pDBconn,IN cap_string strDeviceId, IN char *pszPinCode)
{
    cap_result result = ERR_CAP_UNKNOWN;
    char query[QUERY_SIZE];
    
    result = checkDeviceWithId(pDBconn, strDeviceId);
    ERRIFGOTO(result, _EXIT);

    snprintf(query, QUERY_SIZE, "\
            UPDATE\
                things_device device\
            SET\
                device.is_connected = 0\
            where\
                device.pin_code = '%s';", pszPinCode);

    result = callQuery(pDBconn, query);
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

cap_result DBHandler_InsertVariableHistory(IN MYSQL *pDBconn,IN cap_string strDeviceId, IN cap_string strVariableName, IN char * pszVariable)
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
    
    snprintf(query, QUERY_SIZE, "\
            INSERT INTO\
                things_variablehistory(created_at, updated_at, customer_id, user_thing_id, variable_id, value)\
            VALUES(now(), now(), %d, %d, %d, %s);", nCustomerId, nUserThingId, nVariableId, pszVariable);

    result = callQuery(pDBconn, query);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEMYSQLFREE(pMysqlResult);
    return result;
}

cap_result DBHandler_InsertApplicationHistory(IN MYSQL *pDBconn,IN cap_string strDeviceId, IN cap_string strFunctionName, IN int nEcaId, IN int nErrorCode)
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

cap_result DBHandler_MakeConditionAndEcaList(IN MYSQL *pDBconn, IN cap_string strDeviceId,\
        IN cap_string strVariableName, IN OUT cap_handle hRelatedConditionList, IN OUT cap_handle hEcaList)
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
                cond.user_thing_id = device.user_thing_id and\
                cond.variable_id = variable.id and\
                cond.event_condition_action_id = eca.id;", CAPString_LowPtr(strDeviceId, NULL), CAPString_LowPtr(strVariableName, NULL));

    result = callQueryWithResult(pDBconn, query, &pMysqlResult, &nRowCount);
    ERRIFGOTO(result, _EXIT);

    while( (mysqlRow = mysql_fetch_row(pMysqlResult)) ) 
    {
        SConditionContext *pstConditionContext = (SConditionContext*)calloc(1, sizeof(SConditionContext));    
        SEcaContext *pstEcaContext = (SEcaContext*)calloc(1, sizeof(SEcaContext));    
    
        pstConditionContext->strExpression = CAPString_New();
        ERRMEMGOTO(pstConditionContext->strExpression, result, _EXIT);
        
        pstConditionContext->nConditionId = atoiIgnoreNull(mysqlRow[0]);

        result = CAPString_SetLow(pstConditionContext->strExpression, mysqlRow[1] , CAPSTRING_MAX);
        ERRIFGOTO(result, _EXIT);

        pstEcaContext->nEcaId = atoiIgnoreNull(mysqlRow[2]);

        //check operator
        if(strncmp(mysqlRow[3], "and", 3) == 0){
            pstEcaContext->enOp = OPERATOR_AND;
        }
        else if(strncmp(mysqlRow[3], "or", 2) == 0){
            pstEcaContext->enOp = OPERATOR_OR;
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
            pstConditionContext->bIsSingleCondition = TRUE;
        }
        else {
            pstConditionContext->bIsSingleCondition = FALSE;
        }
        
        result = CAPLinkedList_Add(hRelatedConditionList, LINKED_LIST_OFFSET_LAST, 0, pstConditionContext);
        ERRIFGOTO(result, _EXIT);

        result = CAPLinkedList_Add(hEcaList, LINKED_LIST_OFFSET_LAST, 0, pstEcaContext);
        ERRIFGOTO(result, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFEMYSQLFREE(pMysqlResult);
    return result;
}







