/*
 * CAPSocket.c
 *
 *  Created on: 2015. 8. 19.
 *      Author: chjej202
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <CAPSocket.h>

typedef struct _SCAPSocket
{
    EHandleId enID;
    int nSocketfd;
    SSocketInfo stSocketInfo;
    cap_bool bIsServer;
} SCAPSocket;

#define SOCKET_FD_NOT_SET (-1)
#define MAX_SUN_PATH (107)

cap_result CAPSocket_Create(IN SSocketInfo *pstSocketInfo, IN cap_bool bIsServer, OUT cap_handle *phSocket)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPSocket *pstSocket = NULL;

    IFVARERRASSIGNGOTO(pstSocketInfo, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);
    IFVARERRASSIGNGOTO(phSocket, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    //IFVARERRASSIGNGOTO(CAPString_Length(pstSocketInfo->strSocketPath), 0, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if(bIsServer != TRUE && bIsServer != FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_PARAM, _EXIT);
    }

    if(pstSocketInfo->enSocketType != SOCKET_TYPE_UDS && pstSocketInfo->enSocketType != SOCKET_TYPE_TCP)
    {
        ERRASSIGNGOTO(result, ERR_CAP_NOT_SUPPORTED, _EXIT);
    }

    if(pstSocketInfo->enSocketType == SOCKET_TYPE_TCP && pstSocketInfo->nPort <= 0)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_PARAM, _EXIT);
    }

    pstSocket = (SCAPSocket *) malloc(sizeof(SCAPSocket));
    ERRMEMGOTO(pstSocket, result, _EXIT);

    pstSocket->enID = HANDLEID_CAP_SOCKET;
    pstSocket->bIsServer = bIsServer;
    pstSocket->nSocketfd = SOCKET_FD_NOT_SET;
    pstSocket->stSocketInfo.enSocketType = pstSocketInfo->enSocketType;
    pstSocket->stSocketInfo.nPort = pstSocketInfo->nPort;
    pstSocket->stSocketInfo.strSocketPath = NULL;

    pstSocket->stSocketInfo.strSocketPath = CAPString_New();
    ERRMEMGOTO(pstSocket->stSocketInfo.strSocketPath, result, _EXIT);
    
    if(CAPString_Length(pstSocketInfo->strSocketPath) > 0)
    {
        result = CAPString_Set(pstSocket->stSocketInfo.strSocketPath, pstSocketInfo->strSocketPath);
        ERRIFGOTO(result, _EXIT);
    }

    *phSocket = (cap_handle) pstSocket;

    result = ERR_CAP_NOERROR;
_EXIT:
    if(result != ERR_CAP_NOERROR && pstSocket != NULL)
    {
        CAPSocket_Destroy((cap_handle *)&pstSocket);
    }
    return result;
}

cap_result CAPSocket_Destroy(IN OUT cap_handle *phSocket)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPSocket *pstSocket = NULL;

    IFVARERRASSIGNGOTO(phSocket, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if(IS_VALID_HANDLE(*phSocket, HANDLEID_CAP_SOCKET) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstSocket = (SCAPSocket *) *phSocket;

    if(pstSocket->nSocketfd != SOCKET_FD_NOT_SET)
    {
        close(pstSocket->nSocketfd);
#ifndef __ANDROID__
		if(pstSocket->bIsServer == TRUE && CAPString_Length(pstSocket->stSocketInfo.strSocketPath) > 0)
		{
            unlink(CAPString_LowPtr(pstSocket->stSocketInfo.strSocketPath, NULL));
		}
#endif
    }

    SAFE_CAPSTRING_DELETE(pstSocket->stSocketInfo.strSocketPath);

    SAFEMEMFREE(pstSocket);

    *phSocket = NULL;

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


/*
static cap_result convertSocketErrorToCAPError(int nErrno)
{
    cap_result result = ERR_CAP_UNKNOWN;

    switch(nErrno)
    {
    case EADDRINUSE:
        result = ERR_CAP_BIND_ERROR;
        break;
    default:
        result = ERR_CAP_NET_ERROR;
        break;
    }

    return result;
}
*/


cap_result CAPSocket_Bind(cap_handle hServerSocket)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPSocket *pstSocket = NULL;
    struct sockaddr_un stServerAddr;
    struct sockaddr_in stTCPServerAddr;
    int nLen = 0;
    int nRet = 0;
    struct linger stLinger;

    if(IS_VALID_HANDLE(hServerSocket, HANDLEID_CAP_SOCKET) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstSocket = (SCAPSocket *) hServerSocket;

    if(pstSocket->bIsServer == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_SOCKET, _EXIT);
    }

    switch(pstSocket->stSocketInfo.enSocketType)
    {
    case SOCKET_TYPE_UDS:
        pstSocket->nSocketfd = socket(AF_UNIX, SOCK_STREAM, 0);
        if(pstSocket->nSocketfd < 0)
        {
            ERRASSIGNGOTO(result, ERR_CAP_SOCKET_ERROR, _EXIT);
        }

        stLinger.l_onoff = TRUE;
        stLinger.l_linger = 0;
        nRet = setsockopt(pstSocket->nSocketfd, SOL_SOCKET, SO_LINGER, &stLinger, sizeof(stLinger));
        if(nRet != 0)
        {
            ERRASSIGNGOTO(result, ERR_CAP_SOCKET_ERROR, _EXIT);
        }

        bzero(&stServerAddr, sizeof(stServerAddr));
        stServerAddr.sun_family = AF_UNIX;
        nLen = MIN(MAX_SUN_PATH-1, CAPString_Length(pstSocket->stSocketInfo.strSocketPath));
        // On Android platform first NULL character is needed to bind the socket
#ifdef __ANDROID__
        memcpy(stServerAddr.sun_path, "\0", 1);
        memcpy(stServerAddr.sun_path+1, CAPString_LowPtr(pstSocket->stSocketInfo.strSocketPath, NULL), nLen);
        stServerAddr.sun_path[nLen+1] = '\0';
#else
        memcpy(stServerAddr.sun_path, CAPString_LowPtr(pstSocket->stSocketInfo.strSocketPath, NULL), nLen);
        stServerAddr.sun_path[nLen] = '\0';
#endif
        nRet = bind(pstSocket->nSocketfd, (struct sockaddr *)&stServerAddr, sizeof(stServerAddr));
        if(nRet != 0)
        {
            printf("bind errno: %d, %s\n", errno, strerror(errno));
            ERRASSIGNGOTO(result, ERR_CAP_BIND_ERROR, _EXIT);
        }

       break;
   case SOCKET_TYPE_TCP:
        pstSocket->nSocketfd = socket(AF_INET, SOCK_STREAM, 0);
        if(pstSocket->nSocketfd < 0)
         {
            ERRASSIGNGOTO(result, ERR_CAP_SOCKET_ERROR, _EXIT);
         }

        stLinger.l_onoff = TRUE;
        stLinger.l_linger = 0;

        //For Test Purpose
        int optval = 1;
        nRet = setsockopt(pstSocket->nSocketfd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));

        //nRet = setsockopt(pstSocket->nSocketfd, SOL_SOCKET, TCP_NODELAY, &stLinger, sizeof(stLinger));
        //nRet = setsockopt(pstSocket->nSocketfd, SOL_SOCKET, SO_LINGER, &stLinger, sizeof(stLinger));
        if(nRet != 0)
        {
            ERRASSIGNGOTO(result, ERR_CAP_SOCKET_ERROR, _EXIT);
        }

        bzero(&stTCPServerAddr, sizeof(stTCPServerAddr));
        stTCPServerAddr.sin_family = AF_INET;
        stTCPServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        stTCPServerAddr.sin_port = htons(pstSocket->stSocketInfo.nPort);

        nRet = bind(pstSocket->nSocketfd, (struct sockaddr *)&stTCPServerAddr, sizeof(stTCPServerAddr));
        if(nRet != 0)
         {
            printf("bind errno: %d, %s\n", errno, strerror(errno));
            ERRASSIGNGOTO(result, ERR_CAP_CONNECT_ERROR, _EXIT);
         }
        break;
    default:
        ERRASSIGNGOTO(result, ERR_CAP_INTERNAL_FAIL, _EXIT);
        break;
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result CAPSocket_Listen(cap_handle hServerSocket)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPSocket *pstSocket = NULL;
    int nRet = 0;

    if(IS_VALID_HANDLE(hServerSocket, HANDLEID_CAP_SOCKET) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    pstSocket = (SCAPSocket *) hServerSocket;

    if(pstSocket->bIsServer == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_SOCKET, _EXIT);
    }

    nRet = listen(pstSocket->nSocketfd, SOMAXCONN);
    if(nRet != 0)
    {
        ERRASSIGNGOTO(result, ERR_CAP_LISTEN_ERROR, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

static cap_result selectTimeout(int nSocketfd, fd_set *pstReadSet, fd_set *pstWriteSet, fd_set *pstExceptSet, int nTimeout)
{
    cap_result result = ERR_CAP_UNKNOWN;
    struct timeval stTimeVal;
    int nRet = 0;

    if(pstReadSet != NULL)
    {
        FD_ZERO(pstReadSet);
        FD_SET(nSocketfd, pstReadSet);
    }

    if(pstWriteSet != NULL)
    {
        FD_ZERO(pstWriteSet);
        FD_SET(nSocketfd, pstWriteSet);
    }

    if(pstExceptSet != NULL)
   {
       FD_ZERO(pstExceptSet);
       FD_SET(nSocketfd, pstExceptSet);
   }

    stTimeVal.tv_sec = nTimeout;
    stTimeVal.tv_usec = 0;

    nRet = select(nSocketfd+1, pstReadSet, pstWriteSet, pstExceptSet, &stTimeVal);
    if(nRet < 0)
    {
        ERRASSIGNGOTO(result, ERR_CAP_SELECT_ERROR, _EXIT);
    }
    else if(nRet == 0)
    {
        ERRASSIGNGOTO(result, ERR_CAP_NET_TIMEOUT, _EXIT);
    }
    else
    {
        result = ERR_CAP_NOERROR;
    }
_EXIT:
    return result;
}

cap_result CAPSocket_Accept(cap_handle hServerSocket, IN int nTimeout, IN OUT cap_handle hSocket)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPSocket *pstSocket = NULL;
    fd_set stReadSet;
    SCAPSocket *pstCliSocket = NULL;
    struct sockaddr_un stClientAddr;
    socklen_t nLen = 0;

    if(IS_VALID_HANDLE(hServerSocket, HANDLEID_CAP_SOCKET) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    if(IS_VALID_HANDLE(hSocket, HANDLEID_CAP_SOCKET) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    if(nTimeout <= 0)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_PARAM, _EXIT);
    }

    pstSocket = (SCAPSocket *) hServerSocket;
    pstCliSocket = (SCAPSocket *) hSocket;

    if(pstSocket->bIsServer == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_SOCKET, _EXIT);
    }

    if(pstCliSocket->bIsServer == TRUE || pstCliSocket->nSocketfd > 0)
    {
        dlp("fd : %d\n", pstCliSocket->nSocketfd);
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_SOCKET, _EXIT);
    }

    result = selectTimeout(pstSocket->nSocketfd, &stReadSet, NULL, NULL, nTimeout);
    ERRIFGOTO(result, _EXIT);

    pstCliSocket->nSocketfd = accept(pstSocket->nSocketfd, (struct sockaddr *) &stClientAddr, &nLen);
    if(pstCliSocket->nSocketfd < 0)
    {
        ERRASSIGNGOTO(result, ERR_CAP_ACCEPT_ERROR, _EXIT);
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result CAPSocket_Connect(cap_handle hClientSocket, IN int nTimeout)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPSocket *pstSocket = NULL;
    struct sockaddr_un stClientAddr;
    struct sockaddr_in stTCPClientAddr;
    int nLen = 0;
    int nRet = 0;
    fd_set stReadSet;
    struct linger stLinger;

    if(IS_VALID_HANDLE(hClientSocket, HANDLEID_CAP_SOCKET) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    if(nTimeout <= 0)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_PARAM, _EXIT);
    }

    pstSocket = (SCAPSocket *) hClientSocket;

    if(pstSocket->bIsServer == TRUE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_SOCKET, _EXIT);
    }

    switch(pstSocket->stSocketInfo.enSocketType)
    {
    case SOCKET_TYPE_UDS:
        pstSocket->nSocketfd = socket(AF_UNIX, SOCK_STREAM, 0);
        if(pstSocket->nSocketfd < 0)
         {
            ERRASSIGNGOTO(result, ERR_CAP_SOCKET_ERROR, _EXIT);
         }

        stLinger.l_onoff = TRUE;
        stLinger.l_linger = 0;
        nRet = setsockopt(pstSocket->nSocketfd, SOL_SOCKET, SO_LINGER, &stLinger, sizeof(stLinger));
        if(nRet != 0)
        {
            ERRASSIGNGOTO(result, ERR_CAP_SOCKET_ERROR, _EXIT);
        }

        bzero(&stClientAddr, sizeof(stClientAddr));
        stClientAddr.sun_family = AF_UNIX;
        nLen = MIN(MAX_SUN_PATH-1, CAPString_Length(pstSocket->stSocketInfo.strSocketPath));
        // On Android platform first NULL character is needed to bind the socket

#ifdef __ANDROID__
        memcpy(stClientAddr.sun_path, "\0", 1);
        memcpy(stClientAddr.sun_path+1, CAPString_LowPtr(pstSocket->stSocketInfo.strSocketPath, NULL), nLen);
        stClientAddr.sun_path[nLen+1] = '\0';
#else
        memcpy(stClientAddr.sun_path, CAPString_LowPtr(pstSocket->stSocketInfo.strSocketPath, NULL), nLen);
        stClientAddr.sun_path[nLen] = '\0';
#endif
        result = selectTimeout(pstSocket->nSocketfd, &stReadSet, NULL, NULL, nTimeout);
        ERRIFGOTO(result, _EXIT);

        nRet = connect(pstSocket->nSocketfd, (struct sockaddr *)&stClientAddr, sizeof(stClientAddr));
        if(nRet != 0)
        {
            ERRASSIGNGOTO(result, ERR_CAP_CONNECT_ERROR, _EXIT);
        }

       break;
   case SOCKET_TYPE_TCP:
        pstSocket->nSocketfd = socket(AF_INET, SOCK_STREAM, 0);
        if(pstSocket->nSocketfd < 0)
        {
            ERRASSIGNGOTO(result, ERR_CAP_SOCKET_ERROR, _EXIT);
        }

        stLinger.l_onoff = TRUE;
        stLinger.l_linger = 0;
        nRet = setsockopt(pstSocket->nSocketfd, SOL_SOCKET, SO_LINGER, &stLinger, sizeof(stLinger));
        if(nRet != 0)
        {
            ERRASSIGNGOTO(result, ERR_CAP_SOCKET_ERROR, _EXIT);
        }

        bzero(&stTCPClientAddr, sizeof(stTCPClientAddr));
        stTCPClientAddr.sin_family = AF_INET;
        stTCPClientAddr.sin_addr.s_addr = inet_addr(CAPString_LowPtr(pstSocket->stSocketInfo.strSocketPath, NULL));
        stTCPClientAddr.sin_port = htons(pstSocket->stSocketInfo.nPort);

        result = selectTimeout(pstSocket->nSocketfd, &stReadSet, NULL, NULL, nTimeout);
        ERRIFGOTO(result, _EXIT);

        nRet = connect(pstSocket->nSocketfd, (struct sockaddr *)&stTCPClientAddr, sizeof(stTCPClientAddr));
        if(nRet != 0)
        {
            ERRASSIGNGOTO(result, ERR_CAP_CONNECT_ERROR, _EXIT);
        }

       break;

    default:
        ERRASSIGNGOTO(result, ERR_CAP_INTERNAL_FAIL, _EXIT);
        break;
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}



cap_result CAPSocket_Send(cap_handle hSocket, IN int nTimeout, IN char *pData, IN int nDataLen, OUT int *pnSentSize)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPSocket *pstSocket = NULL;
    fd_set stWriteSet;
    int nDataSent = 0;

    IFVARERRASSIGNGOTO(pData, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if(IS_VALID_HANDLE(hSocket, HANDLEID_CAP_SOCKET) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    if(nTimeout <= 0 || nDataLen <= 0)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_PARAM, _EXIT);
    }

    pstSocket = (SCAPSocket *) hSocket;

    if(pstSocket->bIsServer == TRUE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_SOCKET, _EXIT);
    }

    result = selectTimeout(pstSocket->nSocketfd, NULL, &stWriteSet, NULL, nTimeout);
    ERRIFGOTO(result, _EXIT);

    nDataSent = send(pstSocket->nSocketfd, pData, nDataLen, 0);
    if(nDataSent < 0)
    {
        ERRASSIGNGOTO(result, ERR_CAP_NET_SEND_ERROR, _EXIT);
    }

    if(pnSentSize != NULL)
    {
        *pnSentSize = nDataSent;
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}


cap_result CAPSocket_Receive(cap_handle hSocket, IN int nTimeout, IN OUT char *pBuffer, IN int nBufferLen, OUT int *pnReceivedSize)
{
    cap_result result = ERR_CAP_UNKNOWN;
    SCAPSocket *pstSocket = NULL;
    fd_set stReadSet;
    int nDataReceived = 0;

    IFVARERRASSIGNGOTO(pBuffer, NULL, result, ERR_CAP_INVALID_PARAM, _EXIT);

    if(IS_VALID_HANDLE(hSocket, HANDLEID_CAP_SOCKET) == FALSE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_HANDLE, _EXIT);
    }

    if(nTimeout <= 0 || nBufferLen <= 0)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_PARAM, _EXIT);
    }

    pstSocket = (SCAPSocket *) hSocket;

    if(pstSocket->bIsServer == TRUE)
    {
        ERRASSIGNGOTO(result, ERR_CAP_INVALID_SOCKET, _EXIT);
    }

    result = selectTimeout(pstSocket->nSocketfd, &stReadSet, NULL, NULL, nTimeout);
    ERRIFGOTO(result, _EXIT);

    // MSG_DONTWAIT
    nDataReceived = recv(pstSocket->nSocketfd, pBuffer, nBufferLen, 0);
    if(nDataReceived <= 0)
    {
        ERRASSIGNGOTO(result, ERR_CAP_NET_RECEIVE_ERROR, _EXIT);
    }

    if(pnReceivedSize != NULL)
    {
        *pnReceivedSize = nDataReceived;
    }

    result = ERR_CAP_NOERROR;
_EXIT:
    return result;
}

