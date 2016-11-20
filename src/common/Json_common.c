#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "Json_common.h"

cap_result ParsingJson(OUT json_object **ppJsonObject, IN char *pszJsonBuffer, IN int nJsonBufSize)
{
    cap_result result = ERR_CAP_UNKNOWN;
    struct json_tokener* tok;
    struct json_object* obj = NULL;
    json_object *pJsonObject;

    tok = json_tokener_new();
    ERRMEMGOTO(tok, result, _EXIT);

    pJsonObject = json_tokener_parse_ex(tok, pszJsonBuffer, nJsonBufSize);

    if(tok->err != json_tokener_success) {
        if (obj != NULL)
            json_object_put(obj);
        obj = NULL;
    }   

    json_tokener_free(tok);

    *ppJsonObject = pJsonObject;

    result = ERR_CAP_NOERROR;

_EXIT:
    return result;
}

