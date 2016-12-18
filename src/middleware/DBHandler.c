#include <config.h>
#ifdef HAVE_CONFIG_H
#endif

#include <mysql.h>
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

static MYSQL *g_pDBconn = NULL;

static cap_result callQueryWithResult(MYSQL* pDBconnParam, char* query, MYSQL_RES **ppMysqlResult, int *pnRowCount)
{
    cap_result ret = ERR_CAP_UNKNOWN;
    int nQueryRet = 0;

    nQueryRet = mysql_query(pDBconnParam, query);
    if (nQueryRet != 0) {
		CAPLogger_Write(g_hLogger, MSG_ERROR, "DBHandler: callQueryWithResult Error : %d",nQueryRet );
        ERRASSIGNGOTO(ret, ERR_CAP_DB_ERROR, _EXIT);
    }

    /* Warning! 
     * MysqlResult has to be freed from where it is called
     */

    *ppMysqlResult = mysql_store_result(g_pDBconn);
    if(*ppMysqlResult == NULL) {
		CAPLogger_Write(g_hLogger, MSG_ERROR, "DBHandler: Result Store Error : %d", nQueryRet);
        ERRASSIGNGOTO(ret, ERR_CAP_DB_ERROR, _EXIT);
    }

    *pnRowCount = mysql_num_fields(*ppMysqlResult);

    ret = ERR_CAP_NOERROR;
_EXIT:
    return ret;
}

static cap_result callQuery(MYSQL* pDBconnParam, char* query)
{
    cap_result ret = ERR_CAP_UNKNOWN;
    int nQueryRet = 0;

    nQueryRet = mysql_query(pDBconnParam, query);
    if (nQueryRet != 0) {
		CAPLogger_Write(g_hLogger, MSG_ERROR, "DBHandler: callQuery Error : %d", nQueryRet);
        ERRASSIGNGOTO(ret, ERR_CAP_DB_ERROR, _EXIT);
    }

    ret = ERR_CAP_NOERROR;
_EXIT:
    return ret;
}

cap_result DBHandler_OpenDB(SDBInfo *pstDBInfo)
{
    cap_result result = ERR_CAP_UNKNOWN;

    g_pDBconn = mysql_init(NULL);
    if(g_pDBconn == NULL){
        ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
    }

    if(mysql_real_connect(g_pDBconn, pstDBInfo->pszDBHost, pstDBInfo->pszDBUser,\
                pstDBInfo->pszDBPassword, pstDBInfo->pszDBName, pstDBInfo->nDBPort, NULL, 0) == NULL) {
        ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
    }   
    
    result = ERR_CAP_NOERROR;

_EXIT:
    return result;
}

static cap_result checkDeviceWithId(IN cap_string strDeviceId) {
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

    result = callQueryWithResult(g_pDBconn, query, &pMysqlResult, &nRowCount);
    ERRIFGOTO(result, _EXIT);

    mysqlRow = mysql_fetch_row(pMysqlResult);
    
    //if there is no matching device with device id 
    if(mysqlRow == NULL){
        dlp("There is no matching device!\n");
        ERRASSIGNGOTO(result, ERR_CAP_NO_DATA, _EXIT);
    }

    //if thing is not registered 
    if(atoi(mysqlRow[0]) == FALSE){
        dlp("Thing is not registered to system!\n");
        ERRASSIGNGOTO(result, ERR_CAP_DUPLICATED, _EXIT);
    }
    
    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result DBHandler_CloseDB()
{
    cap_result result = ERR_CAP_UNKNOWN;

    mysql_close(g_pDBconn);

    result = ERR_CAP_NOERROR;
    return result;
}

cap_result DBHandler_VerifyApiKey(IN cap_string strDeviceId, IN char *pszApiKey){
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
                things_thing thing,\
                things_vendor vendor\
            WHERE\
                device.device_id = '%s' and\
                device.user_thing_id = userthing.id and\
                userthing.thing_id = thing.id and\
                thing.vendor_id = vendor.id;", CAPString_LowPtr(strDeviceId, NULL));

    result = callQueryWithResult(g_pDBconn, query, &pMysqlResult, &nRowCount);
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
    return result;

}

cap_result DBHandler_RegisterDevice(IN cap_string strDeviceId, IN char *pszPinCode){
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

    result = callQueryWithResult(g_pDBconn, query, &pMysqlResult, &nRowCount);
    ERRIFGOTO(result, _EXIT);

    mysqlRow = mysql_fetch_row(pMysqlResult);
    
    //if there is no matching device with pin code
    if(mysqlRow == NULL){
        dlp("There is no matching device!\n");
        ERRASSIGNGOTO(result, ERR_CAP_NO_DATA, _EXIT);
    }

    //if thing already has been registered 
    if(atoi(mysqlRow[0]) == TRUE){
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

    result = callQuery(g_pDBconn, query);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;

}

cap_result DBHandler_UnregisterDevice(IN cap_string strDeviceId, IN char *pszPinCode){
    cap_result result = ERR_CAP_UNKNOWN;
    char query[QUERY_SIZE];
    MYSQL_RES *pMysqlResult = NULL;
    MYSQL_ROW mysqlRow;
    int nRowCount = 0;
    
    result = checkDeviceWithId(strDeviceId);
    ERRIFGOTO(result, _EXIT);

    snprintf(query, QUERY_SIZE, "\
            UPDATE\
                things_device device\
            SET\
                device.is_connected = 0\
            where\
                device.pin_code = '%s';", pszPinCode);

    result = callQuery(g_pDBconn, query);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;

}

cap_result DBHandler_UpdateLatestTime(IN cap_string strDeviceId) {
    cap_result result = ERR_CAP_UNKNOWN;
    char query[QUERY_SIZE];
    MYSQL_RES *pMysqlResult = NULL;
    MYSQL_ROW mysqlRow;
    int nRowCount = 0;
    
    result = checkDeviceWithId(strDeviceId);
    ERRIFGOTO(result, _EXIT);
    //TODO
    //update latest time of device
    /*
    snprintf(query, QUERY_SIZE, "\
            UPDATE\
                things_device device\
            SET\
                device.is_connected = 0\
            where\
                device.pin_code = '%s';", pszPinCode);
    */

    result = callQuery(g_pDBconn, query);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result DBHandler_InsertVariableHistory(IN cap_string strDeviceId, IN cap_string strVariableName, IN char * pszVariable) {
    cap_result result = ERR_CAP_UNKNOWN;
    char query[QUERY_SIZE];
    MYSQL_RES *pMysqlResult = NULL;
    MYSQL_ROW mysqlRow;
    int nRowCount = 0;
    int nUserThingId = 0, nCostomerId = 0, nVariableId = 0;
    
    result = checkDeviceWithId(strDeviceId);
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
                device.user_thing_id = userthing.id and\
                variable.name = '%s';", CAPString_LowPtr(strDeviceId, NULL), CAPString_LowPtr(strVariableName, NULL));

    result = callQueryWithResult(g_pDBconn, query, &pMysqlResult, &nRowCount);
    ERRIFGOTO(result, _EXIT);

    mysqlRow = mysql_fetch_row(pMysqlResult);
    
    //if there is no matching device with pin code
    if(mysqlRow == NULL){
        dlp("There is no matching device!\n");
        ERRASSIGNGOTO(result, ERR_CAP_NO_DATA, _EXIT);
    }
    else {
        nUserThingId = atoi(mysqlRow[0]);
        nCostomerId = atoi(mysqlRow[1]);
        nVariableId = atoi(mysqlRow[2]);
    }
    
    snprintf(query, QUERY_SIZE, "\
            INSERT INTO\
                things_variablehistory(customer_id, user_thing_id, variable_id, value)\
            VALUES(%d, %d, %d, %s);", nUserThingId, nCostomerId, nVariableId, pszVariable);

    result = callQuery(g_pDBconn, query);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
    //TODO
}

cap_result DBHandler_InsertApplicationHistory(IN cap_string strDeviceId, IN cap_string strVariableName, IN char * pszVariable) {

}










