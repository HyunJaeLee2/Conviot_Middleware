/*
 * CAPTime.c
 *
 *  Created on: 2015. 8. 31.
 *      Author: chjej202
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <time.h>

#include <CAPTime.h>

cap_result CAPTime_GetCurTickInMilliSeconds(long long *pllTime)
{
    cap_result result = ERR_CAP_UNKNOWN;
    long long llTime = 0;
    struct timespec stTime;
    int nRet = 0;

    IFVARERRASSIGNGOTO(pllTime, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    nRet = clock_gettime(CLOCK_MONOTONIC, &stTime);
    if(nRet != 0)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INTERNAL_FAIL, _EXIT)
    }

    llTime = (((long long) stTime.tv_sec) * 1000) + (stTime.tv_nsec / 1000000);

    *pllTime = llTime;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

void CAPTime_Sleep(int nMillisec)
{
    struct timespec stTime;

    stTime.tv_sec = nMillisec/1000;
    stTime.tv_nsec = (nMillisec % 1000) * 1000000;
    nanosleep(&stTime, NULL);
}


cap_result CAPTime_GetCurDateAndTimeInString(cap_string strDateAndTime)
{
    cap_result result = ERR_CAP_UNKNOWN;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result CAPTime_GetCurDateInString(cap_string strDate)
{
    cap_result result = ERR_CAP_UNKNOWN;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}





