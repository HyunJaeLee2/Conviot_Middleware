/*
 * CAPProcess.c
 *
 *  Created on: 2015. 8. 31.
 *      Author: chjej202
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#include <CAPString.h>
#include <CAPProcess.h>

typedef struct _SCAPProcess {
    EHandleId enId;
    pid_t nProcId;
    cap_string *pastrProcArgs;
    int nArgNum;
}SCAPProcess;

// includes first arguments
cap_result CAPProcess_Create(cap_string *pastrArgs, int nArgNum, OUT cap_handle *phProcess)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPProcess *pstProc = NULL;
    pid_t nProcId = 0;
    char **paszArgs = NULL;
    int nLoop = 0;

    IFVARERRASSIGNGOTO(phProcess, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(pastrArgs, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if(nArgNum <= 0)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_PARAM, _EXIT);
    }

    nProcId = fork();
    if(nProcId == 0) // child process
    {
        // Because the program will be changed by execv, we don't need to free this memory.
        paszArgs = malloc(sizeof(char *)*(nArgNum+1));
        if(paszArgs != NULL)
        {
            int nRet = 0;
            for(nLoop = 0 ; nLoop < nArgNum ; nLoop++)
            {
                paszArgs[nLoop] = CAPString_LowPtr(pastrArgs[nLoop], NULL);
            }
            paszArgs[nArgNum] = NULL;
            nRet = execv(paszArgs[0], paszArgs);
            printf("execv error!!!! (ret: %d, errno: %d, %s\n", nRet, errno, strerror(errno));
        }
        exit(-1);
    }
    else if(nProcId > 0) // parent process
    {
        pstProc = malloc(sizeof(SCAPProcess));
        ERRMEMGOTO(pstProc, result, _EXIT);
        pstProc->enId = HANDLEID_CAP_PROCESS;
        pstProc->nProcId = nProcId;
        pstProc->pastrProcArgs = NULL;
        pstProc->nArgNum = nArgNum;

        pstProc->pastrProcArgs = malloc(sizeof(cap_string)*nArgNum);
        ERRMEMGOTO(pstProc->pastrProcArgs, result, _EXIT);

        for(nLoop = 0 ; nLoop < pstProc->nArgNum; nLoop++)
        {
            pstProc->pastrProcArgs[nLoop] = CAPString_New();
            ERRMEMGOTO(pstProc->pastrProcArgs[nLoop], result, _EXIT);

            result = CAPString_Set(pstProc->pastrProcArgs[nLoop], pastrArgs[nLoop]);
            ERRIFGOTO(result, _EXIT);
        }
    }
    else
    {
        ERRASSIGNGOTO(result, ERR_CAP_INTERNAL_FAIL, _EXIT);
    }

    *phProcess = pstProc;

    result = ERR_CAP_NOERROR;
_EXIT:
    if(result != ERR_CAP_NOERROR && pstProc != NULL)
    {
        CAPProcess_Destroy((cap_handle *) &pstProc);
    }
    return result;
}


cap_result CAPProcess_Wait(cap_handle hProcess, IN cap_bool bBlocked, OUT int *pnExitCode)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPProcess *pstProc = NULL;
    int nStatus = 0;
    int nRet;

    if(IS_VALID_HANDLE(hProcess, HANDLEID_CAP_PROCESS) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstProc = (SCAPProcess *) hProcess;

    if(bBlocked == FALSE)
    {
        nRet = waitpid(pstProc->nProcId, &nStatus, WNOHANG);
        if(nRet < 0)
        {
            CAPASSIGNGOTO(result, ERR_CAP_INTERNAL_FAIL, _EXIT);
        }
        else if(nRet == 0)
        {
            CAPASSIGNGOTO(result, ERR_CAP_NO_CHANGE, _EXIT);
        } // else : nRet > 0
    }
    else // bBlocked == TRUE
    {
        nRet = waitpid(pstProc->nProcId, &nStatus, 0);
        if(nRet <= 0)
        {
            CAPASSIGNGOTO(result, ERR_CAP_INTERNAL_FAIL, _EXIT);
        } // else : nRet > 0
    }

    if(pnExitCode != NULL)
    {
        if(WIFEXITED(nStatus) == TRUE)
        {
            *pnExitCode = WEXITSTATUS(nStatus);
        }
        else if(WIFSIGNALED(nStatus) == TRUE)
        {
            *pnExitCode = WTERMSIG(nStatus);
        }
        else // unknown cases
        {
            ERRASSIGNGOTO(result, ERR_CAP_INTERNAL_FAIL, _EXIT);
        }
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result CAPProcess_Kill(cap_handle hProcess)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPProcess *pstProc = NULL;
    int nRet = 0;

    if(IS_VALID_HANDLE(hProcess, HANDLEID_CAP_PROCESS) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstProc = (SCAPProcess *) hProcess;

    nRet = kill(pstProc->nProcId, SIGKILL);
    if(nRet != 0)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INTERNAL_FAIL, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result CAPProcess_Destroy(cap_handle *phProcess)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPProcess *pstProc = NULL;
    int nLoop = 0;

    IFVARERRASSIGNGOTO(phProcess, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if(IS_VALID_HANDLE(*phProcess, HANDLEID_CAP_PROCESS) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstProc = (SCAPProcess *) *phProcess;

    for(nLoop = 0 ; nLoop < pstProc->nArgNum; nLoop++)
    {
        SAFE_CAPSTRING_DELETE(pstProc->pastrProcArgs[nLoop]);
    }

    SAFEMEMFREE(pstProc->pastrProcArgs);
    SAFEMEMFREE(pstProc);

    *phProcess = NULL;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result CAPProcess_GetProcessId(cap_handle hProcess, int *pnPid)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPProcess *pstProc = NULL;

    IFVARERRASSIGNGOTO(pnPid, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if(IS_VALID_HANDLE(hProcess, HANDLEID_CAP_PROCESS) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstProc = (SCAPProcess *) hProcess;

    *pnPid = pstProc->nProcId;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

cap_result CAPProcess_GetCurProcId(OUT int *pnPid)
{
    cap_result result = ERR_CAP_UNKNOWN;

    IFVARERRASSIGNGOTO(pnPid, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    *pnPid = getpid();

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}




