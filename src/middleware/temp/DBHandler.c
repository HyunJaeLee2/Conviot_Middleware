#include <config.h>
#ifdef HAVE_CONFIG_H
#endif
#include <sqlite3.h>

#include <CAPLinkedList.h>
#include <CAPString.h>
#include <CAPLogger.h>
#include <CAPThreadLock.h>
#include <CAPTime.h>

#include <Json_common.h>

#include "DBHandler.h"
#include "AppEngine.h"

#include <json-c/json_object.h>

#define VALUE_LOG_ATTACH_NAME "value_log_db"

static sqlite3 *g_pDBconn = NULL;
//static cap_handle g_hDBMutex = NULL;

static cap_result CallQuery(sqlite3* pDBconnParam, char* query)
{
    cap_result ret = ERR_CAP_UNKNOWN;
    char* errmsg;

    sqlite3_exec(pDBconnParam, query, 0, 0, &errmsg);
    if (NULL != errmsg) {
		CAPLogger_Write(g_hLogger, MSG_ERROR, "DBHandler: CallQuery Error : %s", errmsg);
        //fprintf(stderr, " DBHandler: CallQuery Error : %s\n", errmsg);	//ERROR
        ERRASSIGNGOTO(ret, ERR_CAP_DB_ERROR, _EXIT);
    }

    ret = ERR_CAP_NOERROR;
_EXIT:
    return ret;
}

//initialize latest_val, updated_on to null and set latest_time of thing
static cap_result initializeDatabase(){
    cap_result result = ERR_CAP_UNKNOWN;
    long long llCurrTime = 0;
    char query[1024 * 16];
    
    result = CAPTime_GetCurTimeInMilliSeconds(&llCurrTime);
    ERRIFGOTO(result, _EXIT);

    sprintf(query, "update %s.value_log set updated_on = NULL, latest_val = NULL;", VALUE_LOG_ATTACH_NAME);
    result = CallQuery(g_pDBconn, query);
    ERRIFGOTO(result, _EXIT);

    sprintf(query, "update things set latest_time = %lld;", llCurrTime);
    result = CallQuery(g_pDBconn, query);
    ERRIFGOTO(result, _EXIT);

_EXIT:
    return result;
}

static cap_result getScenarioIdByName(cap_string strScenarioName, cap_string strTemp, int *pnScenarioId)
{
    cap_result result = ERR_CAP_UNKNOWN;
    sqlite3_stmt* pDBres = NULL;
    int err = 0;
    int nScenarioId = 0;

    IFVARERRASSIGNGOTO(pnScenarioId, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    result = CAPString_PrintFormat(strTemp, "select id from scenarios where name is \"%s\";",
            CAPString_LowPtr(strScenarioName, NULL));
    ERRIFGOTO(result, _EXIT);

    err = sqlite3_prepare_v2(g_pDBconn, CAPString_LowPtr(strTemp, NULL), -1, &pDBres, 0);
    if (err != SQLITE_OK) {
        ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
    }

    if (sqlite3_step(pDBres) == SQLITE_ROW)
    {
        nScenarioId = sqlite3_column_int(pDBres, 0);
    }
    else
    {
        ERRASSIGNGOTO(result, ERR_CAP_NOT_FOUND, _EXIT);
    }

    *pnScenarioId = nScenarioId;

    result = ERR_CAP_NOERROR;
_EXIT:
    if(pDBres != NULL)
    {
        sqlite3_finalize(pDBres);
    }
    return result;
}

static cap_result getScenarioNameById(cap_string strScenarioName, cap_string strTemp, int nScenarioId)
{
    cap_result result = ERR_CAP_UNKNOWN;
    sqlite3_stmt* pDBres = NULL;
    int err = 0;

    IFVARERRASSIGNGOTO(strScenarioName, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    result = CAPString_PrintFormat(strTemp, "select name from scenarios where id is %d;", nScenarioId);
    ERRIFGOTO(result, _EXIT);

    err = sqlite3_prepare_v2(g_pDBconn, CAPString_LowPtr(strTemp, NULL), -1, &pDBres, 0);
    if (err != SQLITE_OK) {
        ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
    }

    if (sqlite3_step(pDBres) == SQLITE_ROW)
    {
        result = CAPString_SetLow(strScenarioName, (const char *) sqlite3_column_text(pDBres, 0), CAPSTRING_MAX);
        ERRIFGOTO(result, _EXIT);
    }
    else
    {
        ERRASSIGNGOTO(result, ERR_CAP_NOT_FOUND, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    if(pDBres != NULL)
    {
        sqlite3_finalize(pDBres);
    }
    return result;
}


static cap_result getThingId(cap_string strThingName, cap_string strTemp, int *pnThingId)
{
    cap_result result = ERR_CAP_UNKNOWN;
    sqlite3_stmt* pDBres = NULL;
    int err = 0;
    int nThingId = 0;

    IFVARERRASSIGNGOTO(pnThingId, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    // Check if thing id is found in DB
    result = CAPString_PrintFormat(strTemp, "select id from things where name is \"%s\";",
            CAPString_LowPtr(strThingName, NULL));
    ERRIFGOTO(result, _EXIT);

    err = sqlite3_prepare_v2(g_pDBconn, CAPString_LowPtr(strTemp, NULL), -1, &pDBres, 0);
    if (err != SQLITE_OK) {
        ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
    }

    //if thing already exists
    if (sqlite3_step(pDBres) == SQLITE_ROW)
    {
        nThingId = sqlite3_column_int(pDBres, 0);
    }
    else
    {
        ERRASSIGNGOTO(result, ERR_CAP_NOT_FOUND, _EXIT);
    }

    *pnThingId = nThingId;

    result = ERR_CAP_NOERROR;
_EXIT:
    if(pDBres != NULL)
    {
        sqlite3_finalize(pDBres);
    }
    return result;
}


cap_result DBHandler_OpenDB(IN char *pszMainDBFileName, IN char *pszValueLogDBFileName)
{
    cap_result result = ERR_CAP_UNKNOWN;
    int err = 0;
    char query[1024 * 16];

    err = sqlite3_open(pszMainDBFileName, &g_pDBconn);
    if (err)
        ERRASSIGNGOTO(result, ERR_CAP_FILE_IO_ERROR, _EXIT);

    snprintf(query, 16 * 1024, "ATTACH DATABASE '%s' as '%s';", pszValueLogDBFileName, VALUE_LOG_ATTACH_NAME);

    result = CallQuery(g_pDBconn, query);
    if (result != ERR_CAP_NOERROR) {
        goto _EXIT;
    }

    result = initializeDatabase();
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;

_EXIT:
    return result;
}


cap_result DBHandler_SetSensorValuePayload(IN char *pszThingId, IN char *pszValueName, IN char *pszPayload, OUT char **ppszMessage)
{
    cap_result result = ERR_CAP_UNKNOWN;
    double dbValue = 0;
    char *pszValueType = NULL;
    char *pszTimeStr = NULL;
    char *pszValue = NULL;
    json_object* pJsonObject, *pJsonType, *pJsonValue;
    const char* constType = "type", * constValue = "value";
    sqlite3_stmt* pDBres = NULL;

    //parse payload
    result = ParsingJson(&pJsonObject, pszPayload, (int)strlen(pszPayload));
    ERRIFGOTO(result, _EXIT);

    if (!json_object_object_get_ex(pJsonObject, constType, &pJsonType)) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }

    pszValueType = strdup(json_object_get_string(pJsonType));

    if(strncmp("binary", pszValueType, 6) == 0){
        if (!json_object_object_get_ex(pJsonObject, constValue, &pJsonValue)) {
            ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
        }
        pszValue = strdup(json_object_get_string(pJsonValue));
    }else{
        if (!json_object_object_get_ex(pJsonObject, constValue, &pJsonValue)) {
            ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
        }
        dbValue = json_object_get_double(pJsonValue);

    }
    SAFEJSONFREE(pJsonObject);

    result = DBHandler_GetDBTime(&pszTimeStr);
    ERRIFGOTO(result, _EXIT);

    pJsonObject = json_object_new_object();

    json_object_object_add(pJsonObject, "msg_type", json_object_new_string((const char*)"sensor_value"));
    json_object_object_add(pJsonObject, "time", json_object_new_string((const char*)pszTimeStr));
    json_object_object_add(pJsonObject, "thing_name", json_object_new_string((const char*)pszThingId));
    json_object_object_add(pJsonObject, "value_name", json_object_new_string((const char*)pszValueName));
    
    if(strncmp("binary", pszValueType, 6) == 0){
        json_object_object_add(pJsonObject, "value", json_object_new_string(pszValue));
    }else{
        json_object_object_add(pJsonObject, "value", json_object_new_double(dbValue));
    }

    *ppszMessage = strdup(json_object_to_json_string(pJsonObject));
_EXIT:
    SAFEMEMFREE(pszValueType);
    SAFEMEMFREE(pszValue);
    SAFEMEMFREE(pszTimeStr);
    if (pDBres != NULL)
        sqlite3_finalize(pDBres);
    SAFEJSONFREE(pJsonObject);
    return result;
}

cap_result DBHandler_CheckIsReadyScenario(IN cap_string strThingId, IN OUT cap_handle hScenarioList){
    cap_result result = ERR_CAP_UNKNOWN;
    cap_string strQuery = NULL;
    int err = 0;
    int nArrayNum = 0;
    int nLoop = 0;
    int *pnScenarioIdArray = NULL;
    sqlite3_stmt* pDBres = NULL;
    
    strQuery = CAPString_New();
    ERRMEMGOTO(strQuery, result, _EXIT);

//    result = getThingId(strThingId, strQuery, &nThingId);
//    ERRIFGOTO(result, _EXIT);
    
    //Get number of rows whose id is dependent on retrieved thing id
    result = CAPString_PrintFormat(strQuery, "select count(*) from thing_left_to_enable where thing_name is \"%s\";",\
            CAPString_LowPtr(strThingId, NULL));
    ERRIFGOTO(result, _EXIT);

    err = sqlite3_prepare_v2(g_pDBconn, CAPString_LowPtr(strQuery, NULL), -1, &pDBres, 0);

    if (err != SQLITE_OK) {
        ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
    }
    
    if (sqlite3_step(pDBres) == SQLITE_ROW) {
        nArrayNum = sqlite3_column_int(pDBres, 0);
    }
    
    sqlite3_finalize(pDBres);
    pDBres = NULL;
 
    //make scenario array based on number of row
    pnScenarioIdArray = (int*)calloc(nArrayNum, sizeof(int));
    ERRMEMGOTO(pnScenarioIdArray, result, _EXIT);
    
    //Retrieve list of scenarios with retrived thing id
    result = CAPString_PrintFormat(strQuery, "select scenario_id from thing_left_to_enable where thing_name is \"%s\";",\
            CAPString_LowPtr(strThingId, NULL));
    ERRIFGOTO(result, _EXIT);
    
    
    err = sqlite3_prepare_v2(g_pDBconn, CAPString_LowPtr(strQuery, NULL), -1, &pDBres, 0);
    if (err != SQLITE_OK) {
        ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
    }

    //put result in scenario array
    for (nLoop = 0; sqlite3_step(pDBres) == SQLITE_ROW; nLoop++) {
        if(nLoop >= nArrayNum){
            ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
        }
        pnScenarioIdArray[nLoop] = sqlite3_column_int(pDBres, 0);
    }

    sqlite3_finalize(pDBres);
    pDBres = NULL;
  
    //Delete row from thing_left_to_enable with retrieved id
    result = CAPString_PrintFormat(strQuery, "delete from thing_left_to_enable where thing_name is \"%s\";",\
            CAPString_LowPtr(strThingId, NULL));
    ERRIFGOTO(result, _EXIT);
    
    
    result = CallQuery(g_pDBconn, CAPString_LowPtr(strQuery, NULL));
    ERRIFGOTO(result, _EXIT);
    
    //Check if each scenario in list still exists in thing_left_to_enable database.
    for(nLoop = 0; nLoop < nArrayNum; nLoop++){
        result = CAPString_PrintFormat(strQuery, "select * from thing_left_to_enable where scenario_id is %d;", pnScenarioIdArray[nLoop]);
        ERRIFGOTO(result, _EXIT);

        
        err = sqlite3_prepare_v2(g_pDBconn, CAPString_LowPtr(strQuery, NULL), -1, &pDBres, 0);
        if (err != SQLITE_OK) {
            ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
        }

        //if row doesn't exist(ready to enable), put it in linkedlist then return
        if(sqlite3_step(pDBres) == SQLITE_DONE){
            cap_string strScenarioName = NULL;

            strScenarioName = CAPString_New();
            ERRMEMGOTO(strScenarioName, result, _EXIT);

            result = getScenarioNameById(strScenarioName, strQuery, pnScenarioIdArray[nLoop]);
            ERRIFGOTO(result, _EXIT);
            
            result = CAPLinkedList_Add(hScenarioList, LINKED_LIST_OFFSET_LAST, 0, strScenarioName);
            ERRIFGOTO(result, _EXIT);
        }
    
        sqlite3_finalize(pDBres);
        pDBres = NULL;
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    if(pDBres != NULL)
    {
        sqlite3_finalize(pDBres);
    }
    SAFEMEMFREE(pnScenarioIdArray);
    SAFE_CAPSTRING_DELETE(strQuery);
    return result;
}

cap_result DBHandler_CheckScenarioToDisable(IN cap_string strThingId, IN OUT cap_handle hScenarioList){
    cap_result result = ERR_CAP_UNKNOWN;
    cap_string strQuery = NULL;
    int err = 0;
    int nThingId = 0;
    int nArrayNum = 0;
    int nLoop = 0;
    int *pnScenarioIdArray = NULL;
    sqlite3_stmt* pDBres = NULL;
    
    strQuery = CAPString_New();
    ERRMEMGOTO(strQuery, result, _EXIT);
    
    result = getThingId(strThingId, strQuery, &nThingId);
    ERRIFGOTO(result, _EXIT);
    
    //Get number of rows whose id is dependent on retrieved thing id
    result = CAPString_PrintFormat(strQuery, "select count(*) from scenario_thing_dependency where thing_name is \"%s\";",\
            CAPString_LowPtr(strThingId, NULL));
    ERRIFGOTO(result, _EXIT);

    err = sqlite3_prepare_v2(g_pDBconn, CAPString_LowPtr(strQuery, NULL), -1, &pDBres, 0);

    if (err != SQLITE_OK) {
        ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
    }
    
    if (sqlite3_step(pDBres) == SQLITE_ROW) {
        nArrayNum = sqlite3_column_int(pDBres, 0);
    }
    
    sqlite3_finalize(pDBres);
    pDBres = NULL;

    //make scenario array based on number of row
    pnScenarioIdArray = (int*)calloc(nArrayNum, sizeof(int));
    ERRMEMGOTO(pnScenarioIdArray, result, _EXIT);
    
    //Retrieve list of scenarios with retrived thing id
    result = CAPString_PrintFormat(strQuery, "select scenario_id from scenario_thing_dependency where thing_name is \"%s\";",\
            CAPString_LowPtr(strThingId, NULL));
    ERRIFGOTO(result, _EXIT);
        
    
    err = sqlite3_prepare_v2(g_pDBconn, CAPString_LowPtr(strQuery, NULL), -1, &pDBres, 0);
    if (err != SQLITE_OK) {
        ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
    }

    //put result in scenario array
    for (nLoop = 0; sqlite3_step(pDBres) == SQLITE_ROW; nLoop++) {
        if(nLoop >= nArrayNum){
            ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
        }
        pnScenarioIdArray[nLoop] = sqlite3_column_int(pDBres, 0);
    }

    sqlite3_finalize(pDBres);
    pDBres = NULL;
    
    //add rows to thing_left_to_enable
    //In addition, put it in linkedlist then return
    for(nLoop = 0; nLoop < nArrayNum; nLoop++){
        cap_string strScenarioName = NULL;
        //Query then retrieve scenario string
        strScenarioName = CAPString_New();
        ERRMEMGOTO(strScenarioName, result, _EXIT);

        result = getScenarioNameById(strScenarioName, strQuery, pnScenarioIdArray[nLoop]);
        ERRIFGOTO(result, _EXIT);

        result = CAPLinkedList_Add(hScenarioList, LINKED_LIST_OFFSET_LAST, 0, strScenarioName);
        ERRIFGOTO(result, _EXIT);

        result = CAPString_PrintFormat(strQuery, "insert into thing_left_to_enable(thing_name,scenario_id) values('%s',%d);",\
                CAPString_LowPtr(strThingId, NULL), pnScenarioIdArray[nLoop]);
        ERRIFGOTO(result, _EXIT);

        result = CallQuery(g_pDBconn, CAPString_LowPtr(strQuery, NULL));
        ERRIFGOTO(result, _EXIT);
        
    }
    
    result = ERR_CAP_NOERROR;
_EXIT:
    if(pDBres != NULL)
    {
        sqlite3_finalize(pDBres);
    }
    SAFEMEMFREE(pnScenarioIdArray);
    SAFE_CAPSTRING_DELETE(strQuery);
    return result;
}

cap_result DBHandler_UpdateLatestTime(IN char *pszThingId){
    cap_result result = ERR_CAP_UNKNOWN;
    long long llCurrTime = 0;
    char query[1024 * 16];
    
    result = CAPTime_GetCurTimeInMilliSeconds(&llCurrTime);
    ERRIFGOTO(result, _EXIT);

    sprintf(query, "update things set latest_time = %lld where name = \"%s\";", llCurrTime, pszThingId);
    result = CallQuery(g_pDBconn, query);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;

}


cap_result DBHandler_SetVirtualThingId(IN char *pszPayload, IN int nPayloadLen){
    cap_result result = ERR_CAP_UNKNOWN;
    int err = 0;
    char *pszOldId = NULL, *pszNewId = NULL;
    json_object* pJsonObject, *pJsonOldId, *pJsonNewId;
    const char* pszConstOldId = "old_id", * pszConstNewId = "new_id";
    sqlite3_stmt* pDBres = NULL;
    char query[1024];

    IFVARERRASSIGNGOTO(pszPayload, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    
    //parse payload
    result = ParsingJson(&pJsonObject, pszPayload, nPayloadLen);
    ERRIFGOTO(result, _EXIT);
   
    if (!json_object_object_get_ex(pJsonObject, pszConstOldId, &pJsonOldId)) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }

    if (!json_object_object_get_ex(pJsonObject, pszConstNewId, &pJsonNewId)) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }

    pszOldId = strdup(json_object_get_string(pJsonOldId));
    pszNewId = strdup(json_object_get_string(pJsonNewId));

    // Check if virtaul_name that user wants exists in database
    sprintf(query, "select id from things where virtual_name is \"%s\";", pszNewId);
    err = sqlite3_prepare_v2(g_pDBconn, query, -1, &pDBres, 0);
    if (err != SQLITE_OK) {
        ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
    }

    //if thing already exists
    if (sqlite3_step(pDBres) == SQLITE_ROW) {
        result = ERR_CAP_DUPLICATED;
        goto _EXIT;
    }
    
    sqlite3_finalize(pDBres);
    pDBres = NULL;

    // Check if thing_id that user wants to change exists in database
    // This is not required for functional reason but to check if user request is valid.
    // If old_name does not exist, it should return error
    sprintf(query, "select id from things where virtual_name is \"%s\";", pszOldId);
    err = sqlite3_prepare_v2(g_pDBconn, query, -1, &pDBres, 0);
    if (err != SQLITE_OK) {
        ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
    }

    //if old thing does not exists
    if(sqlite3_step(pDBres) == SQLITE_DONE){
        result = ERR_CAP_INVALID_DATA;
        goto _EXIT;
    }
    
    sqlite3_finalize(pDBres);
    pDBres = NULL;
    
    sprintf(query, "update things set virtual_name = \"%s\" where name = \"%s\"", \
            pszNewId, pszOldId);
    result = CallQuery(g_pDBconn, query);
    ERRIFGOTO(result, _EXIT);

_EXIT:
    SAFEMEMFREE(pszOldId);
    SAFEMEMFREE(pszNewId)
    SAFEJSONFREE(pJsonObject);
    if (pDBres != NULL)
        sqlite3_finalize(pDBres);
    return result;
}

cap_result DBHandler_UpdateValue(IN char *pszThingId, IN char *pszValueName, IN char *pszPayload){
    cap_result result = ERR_CAP_UNKNOWN;
    double dbValue = 0;
    json_object* pJsonObject, *pJsonType, *pJsonValue;
    char *pszTimeStr = NULL;
    char *pszValueType = NULL;
    const char* pszType = "type", * pszValue = "value";
    sqlite3_stmt* pDBres = NULL;
    char query[1024 * 16];
    int err = 0;

    //parse payload
    result = ParsingJson(&pJsonObject, pszPayload, (int)strlen(pszPayload));
    ERRIFGOTO(result, _EXIT);

    if (!json_object_object_get_ex(pJsonObject, pszType, &pJsonType)) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }

    if (!json_object_object_get_ex(pJsonObject, pszValue, &pJsonValue)) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }

    pszValueType = strdup(json_object_get_string(pJsonType));

    sprintf(query, "select id from %s.value_log where thing_name is \"%s\" and value_name = \"%s\";", \
            VALUE_LOG_ATTACH_NAME, pszThingId, pszValueName);
    err = sqlite3_prepare_v2(g_pDBconn, query, -1, &pDBres, 0);
    if (err != SQLITE_OK) {
        ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
    }

    int nThingId = 0;
    if (sqlite3_step(pDBres) == SQLITE_ROW) {
        nThingId = sqlite3_column_int(pDBres, 0);
    }
    
    sqlite3_finalize(pDBres);
    pDBres = NULL;

    result = DBHandler_GetDBTime(&pszTimeStr);
    ERRIFGOTO(result, _EXIT);

    //If a thing doesn't exist in value_log_db, insert values. If it does, update existing value.
    if(nThingId == 0){
        if(strncmp(pszValueType, "binary", 6) == 0){
            // insert scenarios_name into scenarios_table
            sprintf(query, "insert into %s.value_log(thing_name, value_name, updated_on) values('%s','%s','%s');", VALUE_LOG_ATTACH_NAME, pszThingId, pszValueName, pszTimeStr);
            result = CallQuery(g_pDBconn, query);
            ERRIFGOTO(result, _EXIT);
        } else {
            dbValue = json_object_get_double(pJsonValue);

            sprintf(query, "insert into %s.value_log(thing_name, value_name, latest_val, updated_on) values('%s','%s',%lf,'%s');", VALUE_LOG_ATTACH_NAME, pszThingId, pszValueName, dbValue, pszTimeStr);
            result = CallQuery(g_pDBconn, query);
            ERRIFGOTO(result, _EXIT);
        }
    } else {
        if(strncmp(pszValueType, "binary", 6) == 0){
            sprintf(query, "update %s.value_log set updated_on = \"%s\" where thing_name = \"%s\" and value_name = \"%s\";", \
                    VALUE_LOG_ATTACH_NAME, pszTimeStr, pszThingId, pszValueName);
            result = CallQuery(g_pDBconn, query);
            ERRIFGOTO(result, _EXIT);
        } else {
            dbValue = json_object_get_double(pJsonValue);

            sprintf(query, "update %s.value_log set latest_val = %lf, updated_on = \"%s\" where thing_name = \"%s\" and value_name = \"%s\";",\
                    VALUE_LOG_ATTACH_NAME, dbValue, pszTimeStr, pszThingId, pszValueName);
            result = CallQuery(g_pDBconn, query);
            ERRIFGOTO(result, _EXIT);
        }
    }
    /////////////

_EXIT:
    SAFEMEMFREE(pszTimeStr);
    SAFEMEMFREE(pszValueType);
    if (pDBres != NULL)
        sqlite3_finalize(pDBres);
    SAFEJSONFREE(pJsonObject);
    return result;
}
cap_result DBHandler_GetDBTime(OUT char **ppszTimeStr){
    cap_result result = ERR_CAP_UNKNOWN;
    sqlite3_stmt* pDBres = NULL;
    char query[1024 * 16];
    int err = 0;

    //+9 hours because default time is set as a international standard time.
    sprintf(query, "select strftime(\"%%Y-%%m-%%d %%H:%%M:%%f\", 'now', '+9 hours');");

    err = sqlite3_prepare_v2(g_pDBconn, query, -1, &pDBres, 0);
    if (err != SQLITE_OK) {
        ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
    }

    if (sqlite3_step(pDBres) == SQLITE_ROW) {
        const unsigned char* pszTimeStr;
        pszTimeStr = sqlite3_column_text(pDBres, 0);
        *ppszTimeStr = strdup((const char*)pszTimeStr);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    if (pDBres != NULL)
        sqlite3_finalize(pDBres);
    return result;
}


cap_result DBHandler_MakeThingListPayload(OUT char** ppszPayload, OUT int* nPayloadLen)
{
    cap_result result = ERR_CAP_UNKNOWN;
    int err = 0;
    sqlite3_stmt* pDBres1 = NULL, * pDBres2 = NULL, *pDBres3 = NULL;
    char query[1024 * 16];
    json_object* pJsonObject, *pThingIndivisualObject, *pThingArrayObject, *pValueArrayObject, *pFunctionArrayObject;

    pJsonObject = json_object_new_object();
    pThingArrayObject = json_object_new_array();

    sprintf(query, "select virtual_name, id, class, description from things;");
    err = sqlite3_prepare_v2(g_pDBconn, query, -1, &pDBres1, 0);
    if (err != SQLITE_OK) {
        ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
    }

    while (sqlite3_step(pDBres1) == SQLITE_ROW) {
        pThingIndivisualObject = json_object_new_object();

        const unsigned char* pszThingName;
        const unsigned char* pszClass;
        const unsigned char* pszDescription;
        int nThingId;

        pszThingName = sqlite3_column_text(pDBres1, 0);
        nThingId = sqlite3_column_int(pDBres1, 1);
        pszClass = sqlite3_column_text(pDBres1, 2);
        pszDescription = sqlite3_column_text(pDBres1, 3);
        

        json_object_object_add(pThingIndivisualObject, "id", json_object_new_string((const char*)pszThingName));
        json_object_object_add(pThingIndivisualObject, "class", json_object_new_string((const char*)pszClass));
        if(pszDescription != NULL){
            json_object_object_add(pThingIndivisualObject, "description", json_object_new_string((const char*)pszDescription));
        }

        //make values json
        sprintf(query, "select name, min, max, type, format from thing_values where thing_id = %d;", nThingId);
        err = sqlite3_prepare_v2(g_pDBconn, query, -1, &pDBres2, 0);
        if (err != SQLITE_OK) {
            ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
        }

        pValueArrayObject = json_object_new_array();

        while (sqlite3_step(pDBres2) == SQLITE_ROW) {
            const unsigned char* pszValueName;
            const unsigned char* pszFormat = NULL;
            const unsigned char* pszUpdatedOn = NULL;
            double dValueMin = 0, dValueMax = 0, dLatestVal = 0;
            int nValueType;
            char pszValueMinTemp[100] = {0}, pszValueMaxTemp[100] = {0}, pszLatestValTemp[100] = {0};
      

            pszValueName = sqlite3_column_text(pDBres2, 0);
            dValueMin = sqlite3_column_double(pDBres2, 1);
            dValueMax = sqlite3_column_double(pDBres2, 2);
            nValueType = sqlite3_column_int(pDBres2, 3);
            pszFormat = sqlite3_column_text(pDBres2, 4);
            
            //Retrieve from value_log database
            sprintf(query, "select latest_val, updated_on from %s.value_log where thing_name is \"%s\" and value_name is \"%s\";",\
                    VALUE_LOG_ATTACH_NAME, pszThingName, pszValueName);
            

            err = sqlite3_prepare_v2(g_pDBconn, query, -1, &pDBres3, 0);
            if (err != SQLITE_OK) {
                ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
            }

            if (sqlite3_step(pDBres3) == SQLITE_ROW) {
                dLatestVal = sqlite3_column_double(pDBres3, 0);
                pszUpdatedOn = sqlite3_column_text(pDBres3, 1);
            }

            //Write double to buffer. It is implemented this way due to incorrectness of json library when writing double. (ex. 1.0000 -> 0.99999)
            if(nValueType == VALUE_TYPE_DOUBLE){
                sprintf(pszValueMinTemp, "%lf", dValueMin);
                sprintf(pszValueMaxTemp, "%lf", dValueMax);
                sprintf(pszLatestValTemp, "%lf", dLatestVal);
            } 
            else {
                sprintf(pszValueMinTemp, "%d", (int)dValueMin);
                sprintf(pszValueMaxTemp, "%d", (int)dValueMax);
                sprintf(pszLatestValTemp, "%d", (int)dLatestVal);
            }
           
            
            json_object* pValueObject;
            pValueObject = json_object_new_object();

            json_object_object_add(pValueObject, "value", json_object_new_string((const char*)pszValueName));
            json_object_object_add(pValueObject, "min", json_object_new_string((const char *)pszValueMinTemp));
            json_object_object_add(pValueObject, "max", json_object_new_string((const char *)pszValueMaxTemp));
            json_object_object_add(pValueObject, "type", json_object_new_int(nValueType));
           
            //If a value of sensor is never received, ignore it
            if(pszUpdatedOn != NULL){
                json_object_object_add(pValueObject, "latest_val", json_object_new_string((const char *)pszLatestValTemp));
                json_object_object_add(pValueObject, "updated_on", json_object_new_string((const char *)pszUpdatedOn));
            }
            
            if(pszFormat != NULL){
                 json_object_object_add(pValueObject, "format", json_object_new_string((const char*)pszFormat));
            }
            json_object_array_add(pValueArrayObject, pValueObject);
            
            sqlite3_finalize(pDBres3);
            pDBres3 = NULL;
           
        }
        json_object_object_add(pThingIndivisualObject, "values", pValueArrayObject);

        sqlite3_finalize(pDBres2);
        pDBres2 = NULL;

        //make functions json
        sprintf(query, "select name, bUseArg, ArgType, ArgMin, ArgMax from thing_functions where thing_id = %d;", nThingId);
        err = sqlite3_prepare_v2(g_pDBconn, query, -1, &pDBres2, 0);
        if (err != SQLITE_OK) {
            ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
        }

        pFunctionArrayObject = json_object_new_array();

        while (sqlite3_step(pDBres2) == SQLITE_ROW) {
            const unsigned char* pszFunctionName;
            int bFunctionUseArg, nFunctionArgType;
            double dFunctionArgMin, dFunctionArgMax;
            char pszFunctionArgMinTemp[100] = {0}, pszFunctionArgMaxTemp[100] = {0};
            
            pszFunctionName = sqlite3_column_text(pDBres2, 0);
            bFunctionUseArg = sqlite3_column_int(pDBres2, 1);
            nFunctionArgType = sqlite3_column_int(pDBres2, 2);
            dFunctionArgMin = sqlite3_column_double(pDBres2, 3);
            dFunctionArgMax = sqlite3_column_double(pDBres2, 4);

            if(nFunctionArgType== VALUE_TYPE_DOUBLE){
                sprintf(pszFunctionArgMinTemp, "%lf", dFunctionArgMin);
                sprintf(pszFunctionArgMaxTemp, "%lf", dFunctionArgMax);
            } 
            else {
                sprintf(pszFunctionArgMinTemp, "%d", (int)dFunctionArgMin);
                sprintf(pszFunctionArgMaxTemp, "%d", (int)dFunctionArgMax);
            }
            
            json_object* pFunctionObject;
            pFunctionObject = json_object_new_object();

            json_object_object_add(pFunctionObject, "function", json_object_new_string((const char*)pszFunctionName));
            json_object_object_add(pFunctionObject, "bUseArg", json_object_new_int(bFunctionUseArg));
            json_object_object_add(pFunctionObject, "ArgType", json_object_new_int(nFunctionArgType));
            json_object_object_add(pFunctionObject, "ArgMin", json_object_new_string((const char *)pszFunctionArgMinTemp));
            json_object_object_add(pFunctionObject, "ArgMax", json_object_new_string((const char *)pszFunctionArgMaxTemp));

            json_object_array_add(pFunctionArrayObject, pFunctionObject);
        }
        json_object_object_add(pThingIndivisualObject, "functions", pFunctionArrayObject);
        json_object_array_add(pThingArrayObject, pThingIndivisualObject);

        sqlite3_finalize(pDBres2);
        pDBres2 = NULL;
    }
    json_object_object_add(pJsonObject, "things", pThingArrayObject);

    *ppszPayload = strdup(json_object_to_json_string(pJsonObject));
    *nPayloadLen = strlen(*ppszPayload);

    result = ERR_CAP_NOERROR;

_EXIT:
    if (pDBres1 != NULL) {
        sqlite3_finalize(pDBres1);
    }
    if (pDBres2 != NULL)
        sqlite3_finalize(pDBres2);
    SAFEJSONFREE(pJsonObject);
    return result;
}


cap_result DBHandler_TraverseAllScenarios(CbFnTraverseScenario fnScenarioTraverse, void *pUserData)
{
    cap_result result = ERR_CAP_UNKNOWN;
    cap_string strQuery = NULL;
    cap_string strScenarioName = NULL;
    cap_string strScenarioText = NULL;
    int err = 0;
    int nScenarioId = 0;
    sqlite3_stmt* pDBres = NULL;

    IFVARERRASSIGNGOTO(fnScenarioTraverse, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    strQuery = CAPString_New();
    ERRMEMGOTO(strQuery, result, _EXIT);

    strScenarioName = CAPString_New();
    ERRMEMGOTO(strScenarioName, result, _EXIT);

    strScenarioText = CAPString_New();
    ERRMEMGOTO(strScenarioText, result, _EXIT);

    result = CAPString_SetLow(strQuery, "select id, name, oriStr from scenarios;", CAPSTRING_MAX);
    ERRIFGOTO(result, _EXIT);

    err = sqlite3_prepare_v2(g_pDBconn, CAPString_LowPtr(strQuery, NULL), -1, &pDBres, 0);
    if (err != SQLITE_OK) {
        ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
    }

    while (sqlite3_step(pDBres) == SQLITE_ROW) {

        nScenarioId = sqlite3_column_int(pDBres, 0);
        result = CAPString_SetLow(strScenarioName, (const char *) sqlite3_column_text(pDBres, 1), CAPSTRING_MAX);
        ERRIFGOTO(result, _EXIT);

        result = CAPString_SetLow(strScenarioText, (const char *) sqlite3_column_text(pDBres, 2), CAPSTRING_MAX);
        ERRIFGOTO(result, _EXIT);

        result = fnScenarioTraverse(nScenarioId, strScenarioName, strScenarioText, pUserData);
        ERRIFGOTO(result, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    if (pDBres != NULL)
        sqlite3_finalize(pDBres);
    SAFE_CAPSTRING_DELETE(strQuery);
    SAFE_CAPSTRING_DELETE(strScenarioText);
    SAFE_CAPSTRING_DELETE(strScenarioName);
    return result;
}

static cap_result getScenarioState(IN cap_handle hAppEngine, IN char* pszScenarioName, json_object *pScenarioIndivisualObject)
{
    cap_result result = ERR_CAP_UNKNOWN;
    
    cap_string strScenarioName = NULL;
    ERunnerState enState;

    strScenarioName = CAPString_New();
    ERRMEMGOTO(strScenarioName, result, _EXIT);

    result = CAPString_SetLow(strScenarioName, (const char *) pszScenarioName, CAPSTRING_MAX);
    ERRIFGOTO(result, _EXIT);

    result = AppEngine_GetScenarioState(hAppEngine, strScenarioName, &enState);
    ERRIFGOTO(result, _EXIT);

    switch(enState){
        case RUNNER_STATE_INIT:
            json_object_object_add(pScenarioIndivisualObject, "state", json_object_new_string("initialized"));
            break;
        case RUNNER_STATE_RUNNING:
            json_object_object_add(pScenarioIndivisualObject, "state", json_object_new_string("running"));
            break;
        case RUNNER_STATE_PAUSE:
            json_object_object_add(pScenarioIndivisualObject, "state", json_object_new_string("paused"));
            break;
        case RUNNER_STATE_STOP:
            json_object_object_add(pScenarioIndivisualObject, "state", json_object_new_string("stopped"));
            break;
        case RUNNER_STATE_COMPLETED:
            json_object_object_add(pScenarioIndivisualObject, "state", json_object_new_string("completed"));
            break;
        default:
            ERRASSIGNGOTO(result, ERR_CAP_NOT_SUPPORTED, _EXIT);
    }

_EXIT:
    SAFE_CAPSTRING_DELETE(strScenarioName);
    return result;
}


cap_result DBHandler_MakeScenarioListPayload(IN cap_handle hAppEngine, OUT char** ppszPayload, OUT int* nPayloadLen)
{
    cap_result result = ERR_CAP_UNKNOWN;
    int err = 0;
    sqlite3_stmt* pDBres = NULL;
    char query[1024 * 16];
    json_object* pJsonObject, *pScenarioIndivisualObject, *pScenarioArrayObject;

    pJsonObject = json_object_new_object();
    pScenarioArrayObject = json_object_new_array();

    sprintf(query, "select name, id, oriStr, is_disabled from scenarios;");
    err = sqlite3_prepare_v2(g_pDBconn, query, -1, &pDBres, 0);
    if (err != SQLITE_OK) {
        ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
    }

    while (sqlite3_step(pDBres) == SQLITE_ROW) {
        pScenarioIndivisualObject = json_object_new_object();

        const unsigned char* pszScenarioName;
        const unsigned char* pszScenarioContent;
        int nScenarioId;
        int bDisabled;

        pszScenarioName = sqlite3_column_text(pDBres, 0);
        nScenarioId = sqlite3_column_int(pDBres, 1);
        pszScenarioContent = sqlite3_column_text(pDBres, 2);
        bDisabled = sqlite3_column_int(pDBres, 3);

        json_object_object_add(pScenarioIndivisualObject, "id", json_object_new_int(nScenarioId));
        json_object_object_add(pScenarioIndivisualObject, "name", json_object_new_string((const char*)pszScenarioName));
        json_object_object_add(pScenarioIndivisualObject, "contents", json_object_new_string((const char*)pszScenarioContent));

        if(bDisabled == TRUE){
            json_object_object_add(pScenarioIndivisualObject, "state", json_object_new_string("disabled"));
        }
        else{
            getScenarioState(hAppEngine, (char *)pszScenarioName, pScenarioIndivisualObject);
        }
        json_object_array_add(pScenarioArrayObject, pScenarioIndivisualObject);

    }
    json_object_object_add(pJsonObject, "scenarios", pScenarioArrayObject);


    *ppszPayload = strdup(json_object_to_json_string(pJsonObject));
    *nPayloadLen = strlen(*ppszPayload);

    result = ERR_CAP_NOERROR;
_EXIT:
    if (pDBres != NULL)
        sqlite3_finalize(pDBres);
    SAFEJSONFREE(pJsonObject);
    return result;
}

cap_result DBHandler_MakeThingAliveInfoArray(IN OUT SThingAliveInfo **ppstThingAliveInfoArray, IN int *pnArrayLength){
    cap_result result = ERR_CAP_UNKNOWN;
    int err = 0;
    sqlite3_stmt* pDBres = NULL;
    char query[1024 * 16];
    int nLoop = 0;
    int nArrayNum = 0;
    SThingAliveInfo *pstThingAliveInfoArray = NULL;

    //Get number of rows of things
    sprintf(query, "select count(*) from things;");
    err = sqlite3_prepare_v2(g_pDBconn, query, -1, &pDBres, 0);

    if (err != SQLITE_OK) {
        ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
    }
    
    if (sqlite3_step(pDBres) == SQLITE_ROW) {
        nArrayNum = sqlite3_column_int(pDBres, 0);
    }
    
    sqlite3_finalize(pDBres);
    pDBres = NULL;
    
    pstThingAliveInfoArray = (SThingAliveInfo*)calloc(nArrayNum, sizeof(SThingAliveInfo));
    ERRMEMGOTO(pstThingAliveInfoArray, result, _EXIT);
    
    for (nLoop = 0; nLoop < nArrayNum; nLoop++)
    {
        pstThingAliveInfoArray[nLoop].strThingId = NULL;
    }

    sprintf(query, "select name, alive_cycle, latest_time from things;");
    err = sqlite3_prepare_v2(g_pDBconn, query, -1, &pDBres, 0);

    if (err != SQLITE_OK) {
        ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
    }

    //Make Thing Alive Array
    for (nLoop = 0; sqlite3_step(pDBres) == SQLITE_ROW; nLoop++) {
        if(nLoop >= nArrayNum){
            ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
        }
        const unsigned char* pszThingId = sqlite3_column_text(pDBres, 0);
        int nAliveCycle = sqlite3_column_int(pDBres, 1);
        long long llLatestTime = (long long)sqlite3_column_int64(pDBres, 2);

        pstThingAliveInfoArray[nLoop].strThingId = CAPString_New();
        ERRMEMGOTO(pstThingAliveInfoArray[nLoop].strThingId, result, _EXIT);

        result = CAPString_SetLow(pstThingAliveInfoArray[nLoop].strThingId, (const char *)pszThingId, CAPSTRING_MAX);
        ERRIFGOTO(result, _EXIT);

        pstThingAliveInfoArray[nLoop].nAliveCycle = nAliveCycle;
        pstThingAliveInfoArray[nLoop].llLatestTime = llLatestTime;
    }

    *ppstThingAliveInfoArray = pstThingAliveInfoArray;
    *pnArrayLength = nArrayNum;

    result = ERR_CAP_NOERROR;
_EXIT:
    if(result != ERR_CAP_NOERROR && pstThingAliveInfoArray != NULL)
    {
        for (nLoop = 0; nLoop < nArrayNum; nLoop++)
        {
            SAFE_CAPSTRING_DELETE(pstThingAliveInfoArray[nLoop].strThingId);
        }
        SAFEMEMFREE(pstThingAliveInfoArray);
    }
    if (pDBres != NULL)
        sqlite3_finalize(pDBres);
    return result;
}
/*
cap_result DBHandler_FillThingList(IN cap_handle hThingManager)
{
    cap_result ret = ERR_CAP_UNKNOWN;
    SThingManager* pstThingManager = NULL;
    int err = 0;
    sqlite3_stmt* pDBres = NULL;
    char query[1024 * 16];
    cap_string strThingId = NULL;

    IFVARERRASSIGNGOTO(hThingManager, NULL, ret, ERR_CAP_INVALID_PARAM, _EXIT);

    pstThingManager = (SThingManager*)hThingManager;

    sprintf(query, "select name from things;");
    err = sqlite3_prepare_v2(g_pDBconn, query, -1, &pDBres, 0);

    if (err != SQLITE_OK) {
        ERRASSIGNGOTO(ret, ERR_CAP_DB_ERROR, _EXIT);
    }

    while (sqlite3_step(pDBres) == SQLITE_ROW) {
        const unsigned char* pszThingId = sqlite3_column_text(pDBres, 0);

        strThingId = CAPString_New();
        ERRMEMGOTO(strThingId, ret, _EXIT);

        ret =CAPString_SetLow(strThingId, (char*)pszThingId, CAPSTRING_MAX);
        ERRIFGOTO(ret, _EXIT);

        ret = CAPLinkedList_Add(pstThingManager->hThingList, LINKED_LIST_OFFSET_LAST, 0, strThingId);
        ERRIFGOTO(ret, _EXIT);
    }

    ret = ERR_CAP_NOERROR;
_EXIT:
    if (pDBres != NULL)
        sqlite3_finalize(pDBres);
    return ret;
}
*/


static cap_result addScenarioThingDependency(cap_handle hDependentThingList, cap_string strTemp, int nScenarioId){
    cap_result result = ERR_CAP_UNKNOWN;
    int nThingListLength = 0;
    int i = 0;

    result = CAPLinkedList_GetLength(hDependentThingList, &nThingListLength);
    ERRIFGOTO(result, _EXIT);

    for (i = 0; i < nThingListLength; i++) {
        cap_string strRealThingName = NULL;

        result = CAPLinkedList_Get(hDependentThingList, LINKED_LIST_OFFSET_FIRST, i, (void**)&strRealThingName);
        ERRIFGOTO(result, _EXIT);

        //add rows to scenario_thing_dependency 
        result = CAPString_PrintFormat(strTemp, "insert into scenario_thing_dependency(thing_name,scenario_id) values('%s',%d);",\
                CAPString_LowPtr(strRealThingName, NULL), nScenarioId);
        ERRIFGOTO(result, _EXIT);

        result = CallQuery(g_pDBconn, CAPString_LowPtr(strTemp, NULL));
        ERRIFGOTO(result, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}
cap_result DBHandler_AddScenario(cap_string strScenarioName, cap_string strScenarioText, cap_handle hDependentThingList)
{
    cap_result result = ERR_CAP_UNKNOWN;
    cap_string strQuery = NULL;
    int nScenarioId = 0;

    IFVARERRASSIGNGOTO(strScenarioName, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(strScenarioText, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if(IS_VALID_HANDLE(hDependentThingList, HANDLEID_CAP_LINKED_LIST) == FALSE) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    strQuery = CAPString_New();
    ERRMEMGOTO(strQuery, result, _EXIT);

    // insert scenarios_name into scenarios_table
    CAPString_PrintFormat(strQuery, "insert into scenarios(name, oriStr, is_disabled) values('%s','%s',%d);",
            CAPString_LowPtr(strScenarioName, NULL), CAPString_LowPtr(strScenarioText, NULL), FALSE);

    result = CallQuery(g_pDBconn, CAPString_LowPtr(strQuery, NULL));
    ERRIFGOTO(result, _EXIT);

    result = getScenarioIdByName(strScenarioName, strQuery, &nScenarioId);
    ERRIFGOTO(result, _EXIT);

    // Make Thing list based on hDependentThingList
    result = addScenarioThingDependency(hDependentThingList, strQuery, nScenarioId);
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFE_CAPSTRING_DELETE(strQuery);
    return result;
}

cap_result DBHandler_IsScenarioDisabled(IN cap_string strScenarioName, cap_bool *pbDisabled)
{
    cap_result result = ERR_CAP_UNKNOWN;
    cap_string strQuery = NULL;
    int nScenarioId = 0;
    int err = 0;
    sqlite3_stmt* pDBres = NULL;
    cap_bool bDisabled = FALSE;

    IFVARERRASSIGNGOTO(strScenarioName, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    strQuery = CAPString_New();
    ERRMEMGOTO(strQuery, result, _EXIT);

    result = getScenarioIdByName(strScenarioName, strQuery, &nScenarioId);
    ERRIFGOTO(result, _EXIT);
    
    result = CAPString_PrintFormat(strQuery, "select is_disabled from scenarios where id is %d;", nScenarioId);
    ERRIFGOTO(result, _EXIT);
    
    err = sqlite3_prepare_v2(g_pDBconn, CAPString_LowPtr(strQuery, NULL), -1, &pDBres, 0);
    if (err != SQLITE_OK) {
        ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
    }

    if (sqlite3_step(pDBres) == SQLITE_ROW) {
        bDisabled = sqlite3_column_int(pDBres, 0);
    }
    else {
        ERRASSIGNGOTO(result, ERR_CAP_NOT_FOUND, _EXIT);
    }

    *pbDisabled = bDisabled;

    result = ERR_CAP_NOERROR;
_EXIT:
    if (pDBres != NULL)
        sqlite3_finalize(pDBres);
    SAFE_CAPSTRING_DELETE(strQuery);
    return result;
}

cap_result DBHandler_ChangeScenarioState(IN cap_string strScenarioName, cap_bool bDisabled)
{
    cap_result result = ERR_CAP_UNKNOWN;
    cap_string strQuery = NULL;
    int nScenarioId = 0;

    IFVARERRASSIGNGOTO(strScenarioName, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    strQuery = CAPString_New();
    ERRMEMGOTO(strQuery, result, _EXIT);

    result = getScenarioIdByName(strScenarioName, strQuery, &nScenarioId);
    ERRIFGOTO(result, _EXIT);
    
    result = CAPString_PrintFormat(strQuery, "update scenarios set is_disabled = %d where id is %d;",
            bDisabled, nScenarioId);
    ERRIFGOTO(result, _EXIT);

    result = CallQuery(g_pDBconn, CAPString_LowPtr(strQuery, NULL));
    ERRIFGOTO(result, _EXIT);

    result = ERR_CAP_NOERROR;
_EXIT:
    SAFE_CAPSTRING_DELETE(strQuery);
    return result;
}


cap_result DBHandler_AddThing(IN SThingInfo* pstThing)
{
    cap_result ret = ERR_CAP_UNKNOWN;
    int i;
    int err = 0;
    sqlite3_stmt* pDBres = NULL;
    char query[1024];

    IFVARERRASSIGNGOTO(pstThing, NULL, ret, ERR_CAP_INVALID_PARAM, _EXIT);

    // Check if thing Already has been inserted in DB
    sprintf(query, "select id from things where name is \"%s\";", CAPString_LowPtr(pstThing->strThingId, NULL));
    err = sqlite3_prepare_v2(g_pDBconn, query, -1, &pDBres, 0);
    if (err != SQLITE_OK) {
        ERRASSIGNGOTO(ret, ERR_CAP_DB_ERROR, _EXIT);
    }

    //if thing already exists
    if (sqlite3_step(pDBres) == SQLITE_ROW) {
        ret = ERR_CAP_DUPLICATED;
        goto _EXIT;
    }

    sqlite3_finalize(pDBres);
    pDBres = NULL;

    // insert things into things_table
    // if description is NULL, just leave it that way
    if(CAPString_LowPtr(pstThing->strDescription, NULL) == NULL){
        sprintf(query, "insert into things(name, class, virtual_name, alive_cycle) values('%s','%s', '%s', %d);",\
                CAPString_LowPtr(pstThing->strThingId, NULL), CAPString_LowPtr(pstThing->strThingClass, NULL), \
                CAPString_LowPtr(pstThing->strThingId, NULL), pstThing->nAliveCycle);
    }
    else{
        sprintf(query, "insert into things(name, class, virtual_name, description, alive_cycle) values('%s','%s', '%s', '%s', %d);",\
                CAPString_LowPtr(pstThing->strThingId, NULL), CAPString_LowPtr(pstThing->strThingClass, NULL), \
                CAPString_LowPtr(pstThing->strThingId, NULL), CAPString_LowPtr(pstThing->strDescription, NULL), pstThing->nAliveCycle);
    }
    ret = CallQuery(g_pDBconn, query);
    ERRIFGOTO(ret, _EXIT);

    sprintf(query, "select id from things where name is \"%s\";", CAPString_LowPtr(pstThing->strThingId, NULL));
    err = sqlite3_prepare_v2(g_pDBconn, query, -1, &pDBres, 0);
    if (err != SQLITE_OK) {
        ERRASSIGNGOTO(ret, ERR_CAP_DB_ERROR, _EXIT);
    }

    int things_id;
    if (sqlite3_step(pDBres) == SQLITE_ROW) {
        things_id = sqlite3_column_int(pDBres, 0);
    }

    sqlite3_finalize(pDBres);
    pDBres = NULL;

    // insert things values into values_table
    for (i = 0; i < pstThing->nValueNum; i++) {
        SValue* pstValues = &(pstThing->pstValues[i]);
        if (sqlite3_prepare_v2(g_pDBconn, "insert into thing_values(name, min, max, type, thing_id, format) values(?1,?2,?3,?4,?5,?6);", -1, &pDBres, 0) == SQLITE_OK) {
            sqlite3_bind_text(pDBres, 1, CAPString_LowPtr(pstValues->strValueName, NULL), -1, SQLITE_STATIC);
            sqlite3_bind_double(pDBres, 2, pstValues->dbMinValue);
            sqlite3_bind_double(pDBres, 3, pstValues->dbMaxValue);
            sqlite3_bind_int(pDBres, 4, (int)pstValues->enType);
            sqlite3_bind_int(pDBres, 5, things_id);
            sqlite3_bind_text(pDBres, 6, pstValues->pszFormat, -1, SQLITE_STATIC);

            if (sqlite3_step(pDBres) != SQLITE_DONE) {
                ERRASSIGNGOTO(ret, ERR_CAP_DB_ERROR, _EXIT);
            }
        }
        else {
            ERRASSIGNGOTO(ret, ERR_CAP_DB_ERROR, _EXIT);
        }
        sqlite3_finalize(pDBres);
        pDBres = NULL;
    }

    // insert things function into function_table
    for (i = 0; i < pstThing->nFunctionNum; i++) {
        SFunction* pstFunctions = &(pstThing->pstFunctions[i]);
        if (sqlite3_prepare_v2(g_pDBconn, "insert into thing_functions(name, bUseArg, ArgType, ArgMin, ArgMax,\
             thing_id) values(?1,?2,?3,?4,?5,?6);",
                -1, &pDBres, 0) == SQLITE_OK) {
            sqlite3_bind_text(pDBres, 1, CAPString_LowPtr(pstFunctions->strFunctionName, NULL), -1, SQLITE_STATIC);
            sqlite3_bind_int(pDBres, 2, (int)pstFunctions->bUseArgument);
            sqlite3_bind_int(pDBres, 3, (int)pstFunctions->enArgumentType);
            sqlite3_bind_double(pDBres, 4, (double)pstFunctions->dbArgMinValue);
            sqlite3_bind_double(pDBres, 5, (double)pstFunctions->dbArgMaxValue);
            sqlite3_bind_int(pDBres, 6, things_id);

            if (sqlite3_step(pDBres) != SQLITE_DONE) {
                ERRASSIGNGOTO(ret, ERR_CAP_DB_ERROR, _EXIT);
            }
        }
        else {
            ERRASSIGNGOTO(ret, ERR_CAP_DB_ERROR, _EXIT);
        }
        sqlite3_finalize(pDBres);
        pDBres = NULL;
    }

_EXIT:
    if (pDBres != NULL)
        sqlite3_finalize(pDBres);
    return ret;
}

//TODO
//author : hyunjae
//when deleting scenario or thing, should handle with disabling apprunner as well

cap_result DBHandler_DeleteScenario(IN int nScenarioId)
{
    cap_result ret = ERR_CAP_UNKNOWN;
    sqlite3_stmt* pDBres = NULL;
    char query[1024];
    char* errmsg;
    int err = 0;

    IFVARERRASSIGNGOTO(nScenarioId, 0, ret, ERR_CAP_INVALID_PARAM, _EXIT);

    // Check if scenarioId exist in DB
    sprintf(query, "select id from scenarios where id is %d;", nScenarioId);
    err = sqlite3_prepare_v2(g_pDBconn, query, -1, &pDBres, 0);
    if (err != SQLITE_OK) {
        ERRASSIGNGOTO(ret, ERR_CAP_DB_ERROR, _EXIT);
    }

    //if thing does not exists
    if (sqlite3_step(pDBres) != SQLITE_ROW) {
        ret = ERR_CAP_INVALID_DATA;
        goto _EXIT;
    }

    sqlite3_finalize(pDBres);
    pDBres = NULL;

    // delete scenario into scenario_table
    sprintf(query, "delete from scenarios where id is %d;", nScenarioId);
    sqlite3_exec(g_pDBconn, query, NULL, NULL, &errmsg);

    ret = ERR_CAP_NOERROR;

_EXIT:
    if (pDBres != NULL)
        sqlite3_finalize(pDBres);
    return ret;
}

cap_result DBHandler_DeleteThing(cap_string strDeviceName)
{
    cap_result ret = ERR_CAP_UNKNOWN;
    int err = 0, things_id;
    sqlite3_stmt* pDBres = NULL;
    char query[1024];
    char* errmsg;

    IFVARERRASSIGNGOTO(strDeviceName, NULL, ret, ERR_CAP_INVALID_PARAM, _EXIT);

    // Check if thing has been inserted in DB
    sprintf(query, "select id from things where name is \"%s\";", CAPString_LowPtr(strDeviceName, NULL));
    err = sqlite3_prepare_v2(g_pDBconn, query, -1, &pDBres, 0);
    if (err != SQLITE_OK) {
        ERRASSIGNGOTO(ret, ERR_CAP_DB_ERROR, _EXIT);
    }

    if (sqlite3_step(pDBres) != SQLITE_ROW) {
        //if thing does not exists
        ret = ERR_CAP_INVALID_DATA;
        goto _EXIT;
    } else {
        //delete it if it exists
        things_id = sqlite3_column_int(pDBres, 0);
    }

    sqlite3_finalize(pDBres);
    pDBres = NULL;

    sprintf(query, "delete from things where id is %d;", things_id);
    sqlite3_exec(g_pDBconn, query, NULL, NULL, &errmsg);

    ret = ERR_CAP_NOERROR;

_EXIT:
    if (pDBres != NULL)
        sqlite3_finalize(pDBres);
    return ret;
}

cap_result DBHandler_CloseDB()
{
    cap_result result = ERR_CAP_UNKNOWN;
    int err = 0;

    err = sqlite3_close(g_pDBconn);
    if (SQLITE_OK != err) {
		CAPLogger_Write(g_hLogger, MSG_ERROR, "DBHandler : DB close err : %d", err);
        //fprintf(stderr, "DBHandler : DB close err : %d\n", err);	//ERROR
        ERRASSIGNGOTO(result, ERR_CAP_FILE_IO_ERROR, _EXIT);
    }

    result = ERR_CAP_NOERROR;

_EXIT:
    return result;
}


cap_result DBHandler_GetScenarioNamebyId(IN int nScenarioId, IN OUT cap_string strScenarioName)
{
    cap_result result = ERR_CAP_UNKNOWN;
    sqlite3_stmt* pDBres = NULL;
    cap_string strQuery = NULL;
    int err = 0;

    IFVARERRASSIGNGOTO(strScenarioName, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    strQuery = CAPString_New();
    ERRMEMGOTO(strQuery, result, _EXIT);

    // Get scenarios from DB
    result = CAPString_PrintFormat(strQuery, "select name from scenarios where id = %d;", nScenarioId);
    ERRIFGOTO(result, _EXIT);
    err = sqlite3_prepare_v2(g_pDBconn, CAPString_LowPtr(strQuery, NULL), -1, &pDBres, 0);
    if (err != SQLITE_OK) {
        ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
    }

    if (sqlite3_step(pDBres) == SQLITE_ROW) {
        result = CAPString_SetLow(strScenarioName, (const char *) sqlite3_column_text(pDBres, 0), CAPSTRING_MAX);
        ERRIFGOTO(result, _EXIT);
    }
    else {
        ERRASSIGNGOTO(result, ERR_CAP_NOT_FOUND, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    if (pDBres != NULL)
    {
        sqlite3_finalize(pDBres);
    }
    SAFE_CAPSTRING_DELETE(strQuery);
    return result;
}


cap_result DBHandler_GetThingValue(IN cap_string strThingId, IN cap_string strValueName,
                                    OUT SValue **ppstValue, OUT double *pdblatestVal)
{
    cap_result result = ERR_CAP_UNKNOWN;
    sqlite3_stmt* pDBres = NULL;
    int err = 0;
    cap_string strQuery = NULL;
    int nThingId = 0;
    double dbLastVal = 0;
    SValue *pstValueInfo = NULL;

    IFVARERRASSIGNGOTO(strThingId, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(strValueName, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(ppstValue, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    strQuery = CAPString_New();
    ERRMEMGOTO(strQuery, result, _EXIT);

    pstValueInfo = malloc(sizeof(SValue));
    ERRMEMGOTO( pstValueInfo, result, _EXIT);
    pstValueInfo->dbMaxValue = 0;
    pstValueInfo->dbMinValue = 0;
    // format is not used.
    pstValueInfo->pszFormat = NULL;
    pstValueInfo->enType = VALUE_TYPE_DOUBLE;
    pstValueInfo->strValueName = NULL;

    pstValueInfo->strValueName = CAPString_New();
    ERRMEMGOTO(pstValueInfo->strValueName, result, _EXIT);

    result = CAPString_Set(pstValueInfo->strValueName, strValueName);
    ERRIFGOTO(result, _EXIT);

    result = getThingId(strThingId, strQuery, &nThingId);
    ERRIFGOTO(result, _EXIT);

    result = CAPString_PrintFormat(strQuery, "select min, max ,type from thing_values where name is \"%s\" and thing_id is %d;",
            CAPString_LowPtr(strValueName, NULL), nThingId);
    ERRIFGOTO(result, _EXIT);
    err = sqlite3_prepare_v2(g_pDBconn, CAPString_LowPtr(strQuery, NULL), -1, &pDBres, 0);
    if (err != SQLITE_OK) {
        ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
    }

    if (sqlite3_step(pDBres) == SQLITE_ROW)
    {
        pstValueInfo->dbMinValue = sqlite3_column_double(pDBres, 0);
        pstValueInfo->dbMaxValue = sqlite3_column_double(pDBres, 1);
        pstValueInfo->enType = (EValueType) sqlite3_column_int(pDBres, 2);
    }
    else
    {
        ERRASSIGNGOTO(result, ERR_CAP_NOT_FOUND, _EXIT);
    }

    sqlite3_finalize(pDBres);
    pDBres = NULL;

    // set here first
    *ppstValue = pstValueInfo;

    if(pdblatestVal != NULL)
    {
        result = CAPString_PrintFormat(strQuery, "select latest_val from %s.value_log where thing_name is \"%s\" and value_name is \"%s\";",
                VALUE_LOG_ATTACH_NAME, CAPString_LowPtr(strThingId, NULL), CAPString_LowPtr(strValueName, NULL));
        ERRIFGOTO(result, _EXIT);
        err = sqlite3_prepare_v2(g_pDBconn, CAPString_LowPtr(strQuery, NULL), -1, &pDBres, 0);
        // At least ppstValue is returned even though latest_val is not set yet.
        if (err != SQLITE_OK)
        {
            ERRASSIGNGOTO(result, ERR_CAP_NO_DATA, _EXIT);
        }

        if(sqlite3_step(pDBres) == SQLITE_ROW)
        {
            dbLastVal = sqlite3_column_double(pDBres, 0);
        }
        else
        {
            ERRASSIGNGOTO(result, ERR_CAP_NO_DATA, _EXIT);
        }

        sqlite3_finalize(pDBres);
        pDBres = NULL;

        *pdblatestVal = dbLastVal;
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    if(result != ERR_CAP_NOERROR)
    {
        if(result != ERR_CAP_NO_DATA && pstValueInfo != NULL)
        {
            SAFE_CAPSTRING_DELETE(pstValueInfo->strValueName);
            SAFEMEMFREE(pstValueInfo);
        }
        if(pDBres != NULL)
        {
            sqlite3_finalize(pDBres);
        }

    }
    SAFE_CAPSTRING_DELETE(strQuery);
    return result;
}


cap_result DBHandler_GetThingFunction(IN cap_string strThingId, IN cap_string strFunctionName,
                                    OUT SFunction **ppstFunction)
{
    cap_result result = ERR_CAP_UNKNOWN;
    sqlite3_stmt* pDBres = NULL;
    int err = 0;
    cap_string strQuery = NULL;
    int nThingId = 0;
    SFunction *pstFuncInfo = NULL;

    IFVARERRASSIGNGOTO(strThingId, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(strFunctionName, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(ppstFunction, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    strQuery = CAPString_New();
    ERRMEMGOTO(strQuery, result, _EXIT);

    pstFuncInfo = malloc(sizeof(SValue));
    ERRMEMGOTO( pstFuncInfo, result, _EXIT);
    pstFuncInfo->bUseArgument = FALSE;
    pstFuncInfo->dbArgMaxValue = 0;
    pstFuncInfo->dbArgMinValue = 0;
    pstFuncInfo->enArgumentType = VALUE_TYPE_INT;
    pstFuncInfo->strFunctionName = NULL;

    pstFuncInfo->strFunctionName = CAPString_New();
    ERRMEMGOTO(pstFuncInfo->strFunctionName, result, _EXIT);

    result = CAPString_Set(pstFuncInfo->strFunctionName, strFunctionName);
    ERRIFGOTO(result, _EXIT);

    result = getThingId(strThingId, strQuery, &nThingId);
    ERRIFGOTO(result, _EXIT);

    result = CAPString_PrintFormat(strQuery, "select bUseArg, ArgType, ArgMin, ArgMax from thing_functions where name is \"%s\" and thing_id is %d;",
            CAPString_LowPtr(strFunctionName, NULL), nThingId);
    ERRIFGOTO(result, _EXIT);
    err = sqlite3_prepare_v2(g_pDBconn, CAPString_LowPtr(strQuery, NULL), -1, &pDBres, 0);
    if (err != SQLITE_OK) {
        ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
    }

    if (sqlite3_step(pDBres) == SQLITE_ROW)
    {
        pstFuncInfo->bUseArgument = sqlite3_column_int(pDBres, 0);
        pstFuncInfo->enArgumentType = sqlite3_column_int(pDBres, 1);
        pstFuncInfo->dbArgMinValue = (EValueType) sqlite3_column_int(pDBres, 2);
        pstFuncInfo->dbArgMaxValue = (EValueType) sqlite3_column_int(pDBres, 3);
    }
    else
    {
        ERRASSIGNGOTO(result, ERR_CAP_NOT_FOUND, _EXIT);
    }

    sqlite3_finalize(pDBres);
    pDBres = NULL;

    *ppstFunction = pstFuncInfo;

    result = ERR_CAP_NOERROR;
_EXIT:
    if(result != ERR_CAP_NOERROR)
    {
        if(pstFuncInfo != NULL)
        {
            SAFE_CAPSTRING_DELETE(pstFuncInfo->strFunctionName);
            SAFEMEMFREE(pstFuncInfo);
        }
        if(pDBres != NULL)
        {
            sqlite3_finalize(pDBres);
        }

    }
    SAFE_CAPSTRING_DELETE(strQuery);
    return result;
}


cap_result DBHandler_CheckScenarioName(cap_string strScenarioName)
{
    cap_result result = ERR_CAP_UNKNOWN;
    int err = 0;
    sqlite3_stmt* pDBres = NULL;
    cap_string strQuery = NULL;

    if(CAPString_Length(strScenarioName) <= 0) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_PARAM, _EXIT);
    }

    strQuery = CAPString_New();
    ERRMEMGOTO(strQuery, result, _EXIT);

    result = CAPString_PrintFormat(strQuery, "select name, oriStr from scenarios where name = \"%s\";",
            CAPString_LowPtr(strScenarioName, NULL));
    ERRIFGOTO(result, _EXIT);

    // Get scenarios from DB
    err = sqlite3_prepare_v2(g_pDBconn, CAPString_LowPtr(strQuery, NULL), -1, &pDBres, 0);
    if (err != SQLITE_OK) {
        // if same scenario name is not found, code will pass this point
        CAPASSIGNGOTO(result, ERR_CAP_NOERROR, _EXIT);
    }

    if (sqlite3_step(pDBres) == SQLITE_ROW) {
        ERRASSIGNGOTO(result, ERR_CAP_DUPLICATED, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    if (pDBres != NULL)
        sqlite3_finalize(pDBres);
    SAFE_CAPSTRING_DELETE(strQuery);
    return result;
}


cap_result DBHandler_GetRealThingName(cap_string strVirtualThingName, IN OUT cap_string strRealThingName)
{
    cap_result result = ERR_CAP_UNKNOWN;
    int err = 0;
    sqlite3_stmt* pDBres = NULL;
    cap_string strQuery = NULL;

    if(CAPString_Length(strVirtualThingName) <= 0) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_PARAM, _EXIT);
    }

    IFVARERRASSIGNGOTO(strRealThingName, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    strQuery = CAPString_New();
    ERRMEMGOTO(strQuery, result, _EXIT);

    // Check if virtaul_name that user wants exists in database
    result = CAPString_PrintFormat(strQuery, "select name from things where virtual_name is \"%s\";",\
            CAPString_LowPtr(strVirtualThingName, NULL));
    ERRIFGOTO(result, _EXIT);

    // Get scenarios from DB
    err = sqlite3_prepare_v2(g_pDBconn, CAPString_LowPtr(strQuery, NULL), -1, &pDBres, 0);
    if (err != SQLITE_OK) {
        ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
    }

    if (sqlite3_step(pDBres) == SQLITE_ROW) {
        const unsigned char* pszRealThingName;
        pszRealThingName = sqlite3_column_text(pDBres, 0);

        
        result = CAPString_SetLow(strRealThingName, (const char*)pszRealThingName, CAPSTRING_MAX);
        ERRIFGOTO(result, _EXIT);
    }
    else{
        CAPLogger_Write(g_hLogger, MSG_ERROR, "DBHandler : Error : Virtual Name %s does not exist!!",\
                CAPString_LowPtr(strVirtualThingName, NULL));
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_DATA, _EXIT);
    }
    
    result = ERR_CAP_NOERROR;
_EXIT:
    if (pDBres != NULL)
        sqlite3_finalize(pDBres);
    SAFE_CAPSTRING_DELETE(strQuery);
    return result;
}



cap_result DBHandler_CheckFunctionName(cap_string strThingName, cap_string strFunctionName)
{
    cap_result result = ERR_CAP_UNKNOWN;
    int err = 0;
    sqlite3_stmt* pDBres = NULL;
    cap_string strQuery = NULL;
    int nThingId = 0;

    if(CAPString_Length(strThingName) <= 0 || CAPString_Length(strFunctionName) <= 0) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_PARAM, _EXIT);
    }

    strQuery = CAPString_New();
    ERRMEMGOTO(strQuery, result, _EXIT);

    result = getThingId(strThingName, strQuery, &nThingId);
    ERRIFGOTO(result, _EXIT);

    // Check if value name is found in DB
    result = CAPString_PrintFormat(strQuery, "select name from thing_functions where thing_id is %d and name is \"%s\";",
            nThingId, CAPString_LowPtr(strFunctionName, NULL));
    ERRIFGOTO(result, _EXIT);

    err = sqlite3_prepare_v2(g_pDBconn, CAPString_LowPtr(strQuery, NULL), -1, &pDBres, 0);
    if (err != SQLITE_OK) {
        ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
    }

    if (sqlite3_step(pDBres) == SQLITE_ROW) {
        // do nothing
    }
    else
    {
        ERRASSIGNGOTO(result, ERR_CAP_NOT_FOUND, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result DBHandler_CheckValueName(cap_string strThingName, cap_string strValueName)
{
    cap_result result = ERR_CAP_UNKNOWN;
    int err = 0;
    sqlite3_stmt* pDBres = NULL;
    cap_string strQuery = NULL;
    int nThingId = 0;

    if(CAPString_Length(strThingName) <= 0 || CAPString_Length(strValueName) <= 0) {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_PARAM, _EXIT);
    }

    strQuery = CAPString_New();
    ERRMEMGOTO(strQuery, result, _EXIT);

    result = getThingId(strThingName, strQuery, &nThingId);
    ERRIFGOTO(result, _EXIT);

    // Check if value name is found in DB
    result = CAPString_PrintFormat(strQuery, "select name from thing_values where thing_id is %d and name is \"%s\";",
            nThingId, CAPString_LowPtr(strValueName, NULL));
    ERRIFGOTO(result, _EXIT);

    err = sqlite3_prepare_v2(g_pDBconn, CAPString_LowPtr(strQuery, NULL), -1, &pDBres, 0);
    if (err != SQLITE_OK) {
        ERRASSIGNGOTO(result, ERR_CAP_DB_ERROR, _EXIT);
    }

    if (sqlite3_step(pDBres) == SQLITE_ROW) {
        // do nothing
    }
    else
    {
        ERRASSIGNGOTO(result, ERR_CAP_NOT_FOUND, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    if(pDBres != NULL) {
        sqlite3_finalize(pDBres);
    }
    SAFE_CAPSTRING_DELETE(strQuery);
    return result;
}



