// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"

//
// ****************************************************************************
// CControlConnectionSocket
//

BOOL CControlConnectionSocket::SetCurrentTransferMode(HWND parent, BOOL asciiMode, BOOL* success,
                                                      char* ftpReplyBuf, int ftpReplyBufSize,
                                                      BOOL forceRefresh, BOOL* canRetry,
                                                      char* retryMsg, int retryMsgBufSize)
{
    CALL_STACK_MESSAGE5("CControlConnectionSocket::SetCurrentTransferMode(, %d, , , %d, %d, , %d)",
                        asciiMode, ftpReplyBufSize, forceRefresh, retryMsgBufSize);

    if (success != NULL)
        *success = FALSE;
    if (ftpReplyBufSize > 0)
        ftpReplyBuf[0] = 0;
    if (canRetry != NULL)
        *canRetry = FALSE;
    if (retryMsgBufSize > 0)
        retryMsg[0] = 0;

    HANDLES(EnterCriticalSection(&SocketCritSect));
    BOOL leaveSect = TRUE;
    BOOL ret = TRUE;

    if (forceRefresh ||
        asciiMode && CurrentTransferMode != ctrmASCII ||
        !asciiMode && CurrentTransferMode != ctrmBinary)
    {
        char cmdBuf[50];
        char logBuf[50];

        HANDLES(LeaveCriticalSection(&SocketCritSect));
        leaveSect = FALSE;

        // change the transfer mode on the server
        PrepareFTPCommand(cmdBuf, 50, logBuf, 50, ftpcmdSetTransferMode, NULL, asciiMode); // cannot fail
        int ftpReplyCode;
        if (SendFTPCommand(parent, cmdBuf, logBuf, NULL, GetWaitTime(WAITWND_COMOPER), NULL,
                           &ftpReplyCode, ftpReplyBuf, ftpReplyBufSize, FALSE, FALSE, TRUE,
                           canRetry, retryMsg, retryMsgBufSize, NULL))
        {
            if (FTP_DIGIT_1(ftpReplyCode) == FTP_D1_SUCCESS) // success returned (should be 200)
            {
                HANDLES(EnterCriticalSection(&SocketCritSect));
                leaveSect = TRUE;
                CurrentTransferMode = (asciiMode ? ctrmASCII : ctrmBinary); // transfer mode changed
            }
            else
                CurrentTransferMode = ctrmUnknown; // unknown error; may not matter, but return failure so the caller can decide
        }
        else
            ret = FALSE; // error -> connection closed
    }

    if (leaveSect) // the requested transfer mode has already been set successfully
    {
        if (success != NULL)
            *success = TRUE;
        HANDLES(LeaveCriticalSection(&SocketCritSect));
    }
    return ret;
}

//
// **************************************************************************************
// CSendCmdUserIfaceForListAndDownload
//

BOOL CSendCmdUserIfaceForListAndDownload::HadError()
{
    DWORD netErr, tgtFileErr;
    BOOL lowMem;
    int sslErrorOccured;
    BOOL decomprErrorOccured;
    DataConnection->GetError(&netErr, &lowMem, &tgtFileErr, NULL, &sslErrorOccured, &decomprErrorOccured);
    return netErr != NO_ERROR || lowMem || tgtFileErr != NO_ERROR || sslErrorOccured != SSLCONERR_NOERROR ||
           decomprErrorOccured;
}

void CSendCmdUserIfaceForListAndDownload::GetError(DWORD* netErr, BOOL* lowMem, DWORD* tgtFileErr,
                                                   BOOL* noDataTrTimeout, int* sslErrorOccured,
                                                   BOOL* decomprErrorOccured)
{
    DataConnection->GetError(netErr, lowMem, tgtFileErr, noDataTrTimeout, sslErrorOccured,
                             decomprErrorOccured);
}

HANDLE
CSendCmdUserIfaceForListAndDownload::GetFinishedEvent()
{
    return DataConnection->GetTransferFinishedEvent();
}

void CSendCmdUserIfaceForListAndDownload::InitWnd(const char* fileName, const char* host,
                                                  const char* path, CFTPServerPathType pathType)
{
    char buf[500];
    if (!ForDownload)
        _snprintf_s(buf, _TRUNCATE, LoadStr(IDS_LISTWNDDOWNLOADING), host);
    else
        _snprintf_s(buf, _TRUNCATE, LoadStr(IDS_LISTWNDDOWNLOADINGFILE), fileName, host);
    WaitWnd.SetText(buf);
    WaitWnd.SetPath(path, pathType);
}

void CSendCmdUserIfaceForListAndDownload::AfterWrite(BOOL aborting, DWORD showTime)
{
    WaitWnd.Create(showTime);
    if (!aborting)
        DataConnection->ActivateConnection();
}

BOOL CSendCmdUserIfaceForListAndDownload::HandleESC(HWND parent, BOOL isSend, BOOL allowCmdAbort)
{
    BOOL offerAbort = isSend && allowCmdAbort ||           // abort is still available and we have not used it yet
                      DataConnection->IsTransfering(NULL); // without an active data transfer we cannot abort (only terminate the "control connection")

    WaitWnd.Show(FALSE);
    BOOL esc = SalamanderGeneral->SalMessageBox(parent,
                                                LoadStr(!AlreadyAborted ? (offerAbort ? (ForDownload ? IDS_LISTWNDDOWNLFILESENDCMDESC : IDS_LISTWNDSENDCOMMANDESC) : (ForDownload ? IDS_LISTWNDDOWNLFILESENDCMDESC2 : IDS_LISTWNDSENDCOMMANDESC2)) : IDS_LISTWNDABORTCOMMANDESC),
                                                LoadStr(IDS_FTPPLUGINTITLE),
                                                MB_YESNO | MSGBOXEX_ESCAPEENABLED | MB_ICONQUESTION) == IDYES;
    if (esc)
    {
        WaitWnd.SetText(LoadStr(ForDownload ? IDS_LISTWNDDOWNLFILEABORTING : IDS_LISTWNDABORTINGCOMMAND));
        if (!offerAbort)
            AlreadyAborted = TRUE;
        if (!AlreadyAborted)
        {
            if (DataConnection->IsTransfering(NULL) ||  // handle a connection that already closed while
                DataConnection->IsFlushingDataToDisk()) // nothing remains to flush; avoid spamming misleading log entries (still possible, that's OK)
            {
                DataConnection->CancelConnectionAndFlushing(); // close the "data connection"; the system attempts a graceful shutdown (outcome unknown)
                Logs.LogMessage(LogUID, LoadStr(IDS_LOGMSGDATACONTERMINATED), -1, TRUE);
            }
            AlreadyAborted = TRUE;
            if (!allowCmdAbort)
                esc = FALSE; // not yet ESC for listing
        }
    }
    else
        SalamanderGeneral->WaitForESCRelease(); // ensure the next action is not interrupted by the previous ESC press
    if (!esc)
        WaitWnd.Show(TRUE); // nothing happened; continue showing the wait window (it displays different text during abort)
    return esc;
}

void CSendCmdUserIfaceForListAndDownload::SendingFinished()
{
    WaitWnd.Destroy();
}

BOOL CSendCmdUserIfaceForListAndDownload::IsTimeout(DWORD* start, DWORD serverTimeout, int* errorTextID,
                                                    char* errBuf, int errBufSize)
{
    BOOL trFinished;
    BOOL ret = FALSE;
    if (DataConnection->IsTransfering(&trFinished))
        *start = GetTickCount(); // waiting for data, so this is not a timeout
    else
    {
        if (trFinished)
        {
            *start = DataConnection->GetSocketCloseTime();
            ret = (GetTickCount() - *start) >= serverTimeout; // measure timeout from connection close (moment when the server can react; it also learns about the close)
        }
        else
            ret = TRUE; // the connection has not opened yet -> treat as timeout
    }
    if (ret)
    {
        char errText[300];
        if (DataConnection->GetProxyTimeoutDescr(errText, 300))
        {
            if (errBufSize > 0)
                _snprintf_s(errBuf, errBufSize, _TRUNCATE, LoadStr(IDS_LOGMSGDATCONERROR), errText);
            *errorTextID = -1; // description stored directly in 'errBuf'
        }
        else
            *errorTextID = ForDownload ? IDS_LISTWNDDOWNLFILETIMEOUT : IDS_LISTCMDTIMEOUT;
    }
    return ret;
}

void CSendCmdUserIfaceForListAndDownload::CancelDataCon()
{
    DataConnection->CancelConnectionAndFlushing();
}

void CSendCmdUserIfaceForListAndDownload::MaybeSuccessReplyReceived(const char* reply, int replySize)
{
    DataConnection->EncryptPassiveDataCon();

    CQuadWord size;
    if (FTPGetDataSizeInfoFromSrvReply(size, reply, replySize))
    {
        // we have the total listing size - 'size'
        DataConnection->SetDataTotalSize(size);
    }
}

BOOL CSendCmdUserIfaceForListAndDownload::CanFinishSending(int replyCode, BOOL* useTimeout)
{
    BOOL trFinished;
    BOOL ret = DataConnection->IsTransfering(&trFinished);
    if (!ret && !trFinished && DataConnection->IsConnected()) // no connection established yet and the socket is open
    {
        if (FTP_DIGIT_1(replyCode) == FTP_D1_SUCCESS)
        {
            *useTimeout = TRUE; // keep the socket open; bypass WarFTPD bug returning "success" before accepting the data connection (before data transfer starts) - ignore timeout error in this scenario (WarFTPD never transfers data); list & view are less critical and this situation is unlikely
            ret = TRUE;         // simulate active data transfer
        }
        else
            DataConnection->CloseSocketEx(NULL); // close socket (waiting for connection only); server likely reported a command error (listing)
    }
    if (!ret && ForDownload)
    {
        SocketsThread->LockSocketsThread();
        ret = !DataConnection->AreAllDataFlushed(FALSE);
        SocketsThread->UnlockSocketsThread();
        if (!ret)
            DataConnection->CloseTgtFile(); // close file successfully after flushing data
    }
    return !ret; // either no connection happened or it is already closed
}

void CSendCmdUserIfaceForListAndDownload::BeforeWaitingForFinish(int replyCode, BOOL* useTimeout)
{
    if (FTP_DIGIT_1(replyCode) != FTP_D1_SUCCESS) // LIST did not return success - it might not close
    {                                             // the data connection at all (e.g. WarFTPD) - wait for remaining data, but use a timeout
        *useTimeout = TRUE;
        //    DataConnection->CloseSocketEx(NULL);   // to show something in the panel for Show Raw Listing, we must retrieve the remaining data
    }
}

void CSendCmdUserIfaceForListAndDownload::HandleDataConTimeout(DWORD* start)
{
    DWORD lastActTime = DataConnection->GetLastActivityTime();
    if (*start < lastActTime)
        *start = lastActTime;
    else
    {
        BOOL trFinished;
        if (!DataConnection->IsTransfering(&trFinished) && !trFinished &&
            DataConnection->IsConnected()) // no connection established yet and the socket is open
        {
            Logs.LogMessage(LogUID, LoadStr(IDS_LOGMSGDATACONNOTOPENED), -1, TRUE);
        }
        DatConCancelled = TRUE;
        DataConnection->CancelConnectionAndFlushing(); // stop waiting for data; timeout elapsed
    }
}

void CSendCmdUserIfaceForListAndDownload::HandleESCWhenWaitingForFinish(HWND parent)
{
    WaitWnd.Show(FALSE);
    BOOL esc = SalamanderGeneral->SalMessageBox(parent,
                                                LoadStr(ForDownload ? IDS_LISTWNDDOWNLFILESENDCMDESC : IDS_LISTWNDSENDCOMMANDESC),
                                                LoadStr(IDS_FTPPLUGINTITLE),
                                                MB_YESNO | MSGBOXEX_ESCAPEENABLED | MB_ICONQUESTION) == IDYES;
    if (esc)
    {
        // WaitWnd.SetText(LoadStr(IDS_LISTWNDABORTINGCOMMAND)); // unnecessary; the window will not be shown again
        // while the user decides how to respond to the abort prompt, the data connection may finish (hence
        // the "listing may be incomplete" warning); in that case ignoring the abort makes sense
        if (DataConnection->IsTransfering(NULL) || DataConnection->IsFlushingDataToDisk())
        {
            DataConnection->CancelConnectionAndFlushing(); // close the "data connection"; the system attempts a graceful shutdown (outcome unknown)
            Logs.LogMessage(LogUID, LoadStr(IDS_LOGMSGDATACONTERMINATED), -1, TRUE);
            AlreadyAborted = TRUE;
        }
    }
    else
        SalamanderGeneral->WaitForESCRelease(); // ensure the next action is not interrupted by the previous ESC press
    if (!esc)
        WaitWnd.Show(TRUE);
}

// ***********************************************************************************

BOOL CControlConnectionSocket::IsListCommandLIST_a()
{
    HANDLES(EnterCriticalSection(&SocketCritSect));
    BOOL ret = _stricmp(UseLIST_aCommand ? LIST_a_CMD_TEXT : (ListCommand != NULL && *ListCommand != 0 ? ListCommand : LIST_CMD_TEXT),
                        LIST_a_CMD_TEXT) == 0;
    HANDLES(LeaveCriticalSection(&SocketCritSect));
    return ret;
}

void CControlConnectionSocket::ToggleListCommandLIST_a()
{
    HANDLES(EnterCriticalSection(&SocketCritSect));
    if (IsListCommandLIST_a())
    {
        if (UseLIST_aCommand)
            UseLIST_aCommand = FALSE;
        else
        {
            if (ListCommand != NULL)
                free(ListCommand);
            ListCommand = NULL;
        }
    }
    else
        UseLIST_aCommand = TRUE;
    HANDLES(LeaveCriticalSection(&SocketCritSect));
}

BOOL CControlConnectionSocket::ListWorkingPath(HWND parent, const char* path, char* userBuf,
                                               int userBufSize, char** allocatedListing,
                                               int* allocatedListingLen, CFTPDate* listingDate,
                                               BOOL* pathListingIsIncomplete, BOOL* pathListingIsBroken,
                                               BOOL* pathListingMayBeOutdated,
                                               DWORD* pathListingStartTime, BOOL forceRefresh,
                                               int* totalAttemptNum, BOOL* fatalError,
                                               BOOL dontClearCache)
{
    CALL_STACK_MESSAGE4("CControlConnectionSocket::ListWorkingPath(, %s, , %d, , , , , , , , %d, , ,)",
                        path, userBufSize, forceRefresh);

    *fatalError = FALSE;
    *pathListingIsBroken = FALSE;
    BOOL ok = TRUE;
    BOOL ret = TRUE;
    char cmdBuf[50 + FTP_MAX_PATH];
    char logBuf[50 + FTP_MAX_PATH];
    char replyBuf[700];
    char errBuf[900 + FTP_MAX_PATH];
    char listCmd[FTPCOMMAND_MAX_SIZE + 2];

    HANDLES(EnterCriticalSection(&SocketCritSect));
    lstrcpyn(listCmd, UseLIST_aCommand ? LIST_a_CMD_TEXT : (ListCommand != NULL && *ListCommand != 0 ? ListCommand : LIST_CMD_TEXT),
             FTPCOMMAND_MAX_SIZE);
    BOOL usePassiveModeAux = UsePassiveMode;
    int logUID = LogUID; // log UID of this connection
    int useListingsCacheAux = UseListingsCache;
    CFTPProxyForDataCon* dataConProxyServer = ProxyServer == NULL ? NULL : ProxyServer->AllocProxyForDataCon(ServerIP, Host, HostIP, Port);
    BOOL dataConProxyServerOK = ProxyServer == NULL || dataConProxyServer != NULL;
    HANDLES(LeaveCriticalSection(&SocketCritSect));
    strcat(listCmd, "\r\n");

    int attemptNum = 1;
    if (totalAttemptNum != NULL)
        attemptNum = *totalAttemptNum;
    const char* retryMsgAux = NULL;
    BOOL canRetry = FALSE;
    char retryMsgBuf[300];
    char hostTmp[HOST_MAX_SIZE];

    // obtain the date when the listing was created (assume the server generates it before sending)
    SYSTEMTIME st;
    GetLocalTime(&st);
    DWORD lstStTime = 0;

    // allocate an object for the "data connection"
    CDataConnectionSocket* dataConnection = dataConProxyServerOK ? new CDataConnectionSocket(FALSE, dataConProxyServer, EncryptDataConnection, pCertificate, CompressData, this) : NULL;
    if (dataConnection == NULL || !dataConnection->IsGood())
    {
        if (dataConnection != NULL)
            DeleteSocket(dataConnection); // only deallocate it
        else
        {
            if (dataConProxyServer != NULL)
                delete dataConProxyServer;
        }
        dataConnection = NULL;
        TRACE_E(LOW_MEMORY);
        *fatalError = TRUE; // fatal error
    }
    else
    {
        while (1)
        {
            ReuseSSLSessionFailed = FALSE;
            if (ok && usePassiveModeAux) // passive mode (PASV)
            {
                PrepareFTPCommand(cmdBuf, 50 + FTP_MAX_PATH, logBuf, 50 + FTP_MAX_PATH,
                                  ftpcmdPassive, NULL); // cannot fail
                int ftpReplyCode;
                if (SendFTPCommand(parent, cmdBuf, logBuf, NULL, GetWaitTime(WAITWND_COMOPER), NULL,
                                   &ftpReplyCode, replyBuf, 700, FALSE, FALSE, FALSE, &canRetry,
                                   retryMsgBuf, 300, NULL))
                {
                    DWORD ip;
                    unsigned short port;
                    if (FTP_DIGIT_1(ftpReplyCode) == FTP_D1_SUCCESS &&      // success (should be 227)
                        FTPGetIPAndPortFromReply(replyBuf, -1, &ip, &port)) // managed to obtain IP+port
                    {
                        dataConnection->SetPassive(ip, port, logUID);
                        dataConnection->PassiveConnect(NULL); // first attempt; result does not matter (checked later)
                    }
                    else // passive mode not supported
                    {
                        HANDLES(EnterCriticalSection(&SocketCritSect));
                        UsePassiveMode = usePassiveModeAux = FALSE; // fall back to active mode (PORT)
                        HANDLES(LeaveCriticalSection(&SocketCritSect));

                        Logs.LogMessage(logUID, LoadStr(IDS_LOGMSGPASVNOTSUPPORTED), -1);
                    }
                }
                else // error -> connection closed
                {
                    ok = FALSE;
                    if (canRetry)
                        retryMsgAux = retryMsgBuf; // "retry" allowed; go for another reconnect
                    else
                    {
                        *fatalError = TRUE; // fatal error
                        break;
                    }
                }
            }

            if (ok && !usePassiveModeAux) // active mode (PORT)
            {
                DWORD localIP;
                GetLocalIP(&localIP, NULL);   // should not fail
                unsigned short localPort = 0; // listen on any port
                dataConnection->SetActive(logUID);
                if (OpenForListeningAndWaitForRes(parent, dataConnection, &localIP, &localPort, &canRetry,
                                                  retryMsgBuf, 300, GetWaitTime(WAITWND_COMOPER),
                                                  errBuf, 900 + FTP_MAX_PATH))
                {
                    PrepareFTPCommand(cmdBuf, 50 + FTP_MAX_PATH, logBuf, 50 + FTP_MAX_PATH,
                                      ftpcmdSetPort, NULL, localIP, localPort); // cannot fail
                    int ftpReplyCode;
                    if (!SendFTPCommand(parent, cmdBuf, logBuf, NULL, GetWaitTime(WAITWND_COMOPER), NULL,
                                        &ftpReplyCode, replyBuf, 700, FALSE, FALSE, FALSE, &canRetry,
                                        retryMsgBuf, 300, NULL)) // ignore server reply; error appears later (timeout during listing)
                    {                                            // error -> connection closed
                        ok = FALSE;
                        if (canRetry)
                            retryMsgAux = retryMsgBuf; // "retry" allowed; go for another reconnect
                        else
                        {
                            *fatalError = TRUE; // fatal error
                            break;
                        }
                    }
                }
                else // failed to open the listening socket for the incoming data connection ->
                {    // close the control connection (to enable standard Retry)
                    ok = FALSE;
                    if (canRetry)
                        retryMsgAux = retryMsgBuf; // "retry" allowed; go for another reconnect
                    else
                    {
                        *fatalError = TRUE; // fatal error
                        break;
                    }
                }
            }

            if (ok) // still connected: switch transfer mode to ASCII (ignore success)
            {
                ok = SetCurrentTransferMode(parent, TRUE, NULL, NULL, 0, forceRefresh, &canRetry,
                                            retryMsgBuf, 300);
                if (!ok) // error -> connection closed
                {
                    if (canRetry)
                        retryMsgAux = retryMsgBuf; // "retry" allowed; go for another reconnect
                    else
                    {
                        *fatalError = TRUE; // fatal error
                        break;
                    }
                }
            }

            BOOL sslErrReconnect = FALSE;     // TRUE = reconnect due to SSL errors
            BOOL fastSSLErrReconnect = FALSE; // TRUE = server certificate changed; prefer immediate reconnect (no 20-second wait)
            if (ok)
            {
                int ftpReplyCode;
                CSendCmdUserIfaceForListAndDownload userIface(FALSE, parent, dataConnection, logUID);

                //        if (UsePassiveMode) {
                //          dataConnection->EncryptConnection();
                //        }
                HANDLES(EnterCriticalSection(&SocketCritSect));
                lstrcpyn(hostTmp, Host, HOST_MAX_SIZE);
                CFTPServerPathType pathType = ::GetFTPServerPathType(ServerFirstReply, ServerSystem, path);
                HANDLES(LeaveCriticalSection(&SocketCritSect));

                userIface.InitWnd(NULL, hostTmp, path, pathType);
                lstStTime = IncListingCounter();
                if (SendFTPCommand(parent, listCmd, listCmd, NULL, GetWaitTime(WAITWND_COMOPER), NULL,
                                   &ftpReplyCode, replyBuf, 700, FALSE, FALSE, FALSE, &canRetry,
                                   retryMsgBuf, 300, &userIface))
                {
                    if (!userIface.GetDatConCancelled() && !userIface.WasAborted() && !userIface.HadError() &&
                        FTP_DIGIT_1(ftpReplyCode) != FTP_D1_SUCCESS &&
                        FTP_DIGIT_2(ftpReplyCode) != FTP_D2_CONNECTION) // not just a connection (network) error
                    {                                                   // server refuses to list
                        BOOL skipMessage = FTPIsEmptyDirListErrReply(replyBuf);
                        if (!skipMessage)
                        {
                            _snprintf_s(errBuf, _TRUNCATE, LoadStr(IDS_LISTPATHERROR), path, replyBuf);
                            // if the user chooses IDNO we stop - the path cannot be listed -> requires path change
                            ret = SalamanderGeneral->SalMessageBox(parent, errBuf, LoadStr(IDS_FTPERRORTITLE),
                                                                   MB_YESNO | MSGBOXEX_ESCAPEENABLED |
                                                                       MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES;
                            if (!ret)
                                SalamanderGeneral->WaitForESCRelease(); // ensure the next action is not interrupted by the previous ESC press

                            *pathListingIsBroken = TRUE; // mark returned listing as invalid (VxWorks: during listing it can report "error reading entry: 16" plus "550 no files found or ...")
                        }
                        // VMS returns 550 for an empty directory: we cannot leave the path and can still
                        // treat the listing as OK (can be cached - no interruption and the server likely will not return a different listing)
                        // ret = FALSE;   // stop - the path cannot be listed -> requires path change

                        break; // report "successful listing" (allows working in an empty/non-listable directory)
                    }
                    else
                    {
                    if (userIface.WasAborted()) // user aborted the listing - finish with error (incomplete listing)
                        ok = FALSE;             // no need to show "list can be incomplete"; user was warned during abort
                    else
                    {
                        if (FTP_DIGIT_1(ftpReplyCode) != FTP_D1_SUCCESS &&
                                FTP_DIGIT_2(ftpReplyCode) == FTP_D2_CONNECTION || // network error only
                            userIface.HadError() ||                               // data connection reported a system error
                            userIface.GetDatConCancelled())                       // data connection cancelled (never opened or closed after server error in LIST reply)
                        {
                            ok = FALSE;

                                DWORD err;
                                BOOL lowMem, noDataTrTimeout;
                                int sslErrorOccured;
                                userIface.GetError(&err, &lowMem, NULL, &noDataTrTimeout, &sslErrorOccured, NULL);

                                BOOL sslReuseErr = ReuseSSLSessionFailed &&
                                                   (FTP_DIGIT_1(ftpReplyCode) == FTP_D1_TRANSIENTERROR ||
                                                    FTP_DIGIT_1(ftpReplyCode) == FTP_D1_ERROR);
                                if (sslErrorOccured == SSLCONERR_UNVERIFIEDCERT || sslErrorOccured == SSLCONERR_CANRETRY ||
                                    sslReuseErr)
                                {                                                                       // need to reconnect
                                    CloseControlConnection(parent);                                     // close current control connection
                                    lstrcpyn(retryMsgBuf, LoadStr(IDS_ERRDATACONSSLCONNECTERROR), 300); // set error text for reconnect wait window
                                    retryMsgAux = retryMsgBuf;
                                    sslErrReconnect = TRUE;
                                    fastSSLErrReconnect = sslErrorOccured == SSLCONERR_UNVERIFIEDCERT || sslReuseErr;
                                }
                                else
                                {
                                    // show "list can be incomplete" message; user has not been warned yet
                                    lstrcpyn(errBuf, LoadStr(IDS_UNABLETOREADLIST), 900 + FTP_MAX_PATH);
                                    int len = (int)strlen(errBuf);
                                    BOOL systErr = FALSE;
                                    BOOL trModeHint = FTP_DIGIT_1(ftpReplyCode) == FTP_D1_TRANSIENTERROR &&
                                                      FTP_DIGIT_2(ftpReplyCode) == FTP_D2_CONNECTION;

                                    if (FTP_DIGIT_1(ftpReplyCode) == FTP_D1_SUCCESS ||
                                        FTP_DIGIT_2(ftpReplyCode) != FTP_D2_CONNECTION ||
                                        noDataTrTimeout || sslErrorOccured != SSLCONERR_NOERROR)
                                    { // if the server lacks a network error description, rely on system description
                                        systErr = TRUE;
                                        if (!trModeHint)
                                            trModeHint = err == WSAETIMEDOUT || sslErrorOccured != SSLCONERR_NOERROR;
                                        if (sslErrorOccured != SSLCONERR_NOERROR)
                                        {
                                            lstrcpyn(replyBuf, LoadStr(IDS_ERRDATACONSSLCONNECTERROR), 700);
                                            strcat(replyBuf, "\r\n");
                                        }
                                        else
                                        {
                                            if (noDataTrTimeout)
                                                lstrcpyn(replyBuf, LoadStr(IDS_ERRDATACONNODATATRTIMEOUT), 700);
                                            else
                                            {
                                                if (err != NO_ERROR)
                                                {
                                                    if (!dataConnection->GetProxyError(replyBuf, 700, NULL, 0, TRUE))
                                                        FTPGetErrorText(err, replyBuf, 700);
                                                }
                                                else
                                                {
                                                    if (userIface.GetDatConCancelled())
                                                        lstrcpyn(replyBuf, LoadStr(IDS_ERRDATACONNOTOPENED), 700);
                                                    else
                                                        lstrcpyn(replyBuf, LoadStr(IDS_UNKNOWNERROR), 700);
                                                }
                                            }
                                        }
                                    }
                                    _snprintf_s(errBuf + len, 900 + FTP_MAX_PATH - len, _TRUNCATE,
                                                LoadStr(systErr ? (trModeHint ? IDS_UNABLETOREADLISTSUFFIX3 : IDS_UNABLETOREADLISTSUFFIX) : (trModeHint ? IDS_UNABLETOREADLISTSUFFIX4 : IDS_UNABLETOREADLISTSUFFIX2)),
                                                replyBuf);
                                    SalamanderGeneral->SalMessageBox(parent, errBuf,
                                                                     LoadStr(IDS_FTPERRORTITLE),
                                                                     MB_OK | MB_ICONEXCLAMATION);
                                }
                            }
                        }
                        if (!sslErrReconnect)
                            break; // aborted or total success (everything listed - 'ok'==TRUE)
                    }
                }
                else // connection closed
                {
                    if (userIface.WasAborted()) // user aborted listing, terminating the connection (e.g. sunsolve.sun.com or ftp.chg.ru) - finish with error (incomplete listing)
                    {
                        ok = FALSE;   // no need to show "list can be incomplete"; user was warned during abort
                        if (canRetry) // reuse the message in the dialog that announces the disconnected link
                        {
                            HANDLES(EnterCriticalSection(&SocketCritSect));
                            if (ConnectionLostMsg != NULL)
                                SalamanderGeneral->Free(ConnectionLostMsg);
                            ConnectionLostMsg = SalamanderGeneral->DupStr(retryMsgBuf);
                            HANDLES(LeaveCriticalSection(&SocketCritSect));
                        }
                        break; // aborted
                    }

                    // error -> 'ok' stays FALSE; attempt another reconnect
                    ok = FALSE;
                    if (canRetry)
                        retryMsgAux = retryMsgBuf; // "retry" allowed
                    else
                    {
                        *fatalError = TRUE; // fatal error
                        break;
                    }
                }
            }

            if (!ok) // connection interrupted; ask about reconnect
            {
                if (dataConnection->IsConnected())       // close old "data connection" (if FD_CONNECT never arrived)
                    dataConnection->CloseSocketEx(NULL); // shutdown (result unknown)

                SetStartTime();
                BOOL startRet = StartControlConnection(parent, userBuf, userBufSize, TRUE, NULL, 0,
                                                       &attemptNum, retryMsgAux, FALSE, sslErrReconnect ? IDS_LISTCOMMANDERROR : -1,
                                                       fastSSLErrReconnect);
                retryMsgAux = NULL;
                if (totalAttemptNum != NULL)
                    *totalAttemptNum = attemptNum;
                if (startRet) // connection restored
                {
                    if (pCertificate) // control-connection certificate might have changed; pass new one to data connection
                        dataConnection->SetCertificate(pCertificate);

                    // change path to 'path' (the path we are listing)
                    PrepareFTPCommand(cmdBuf, 50 + FTP_MAX_PATH, logBuf, 50 + FTP_MAX_PATH,
                                      ftpcmdChangeWorkingPath, NULL, path);
                    int ftpReplyCode;
                    if (SendFTPCommand(parent, cmdBuf, logBuf, NULL, GetWaitTime(WAITWND_COMOPER), NULL,
                                       &ftpReplyCode, replyBuf, 700, FALSE, TRUE, FALSE, &canRetry,
                                       retryMsgBuf, 300, NULL))
                    {
                        BOOL pathError = TRUE;
                        if (FTP_DIGIT_1(ftpReplyCode) == FTP_D1_SUCCESS) // success likely; double-check the path
                        {
                            if (GetCurrentWorkingPath(parent, cmdBuf, FTP_MAX_PATH, TRUE, &canRetry,
                                                      retryMsgBuf, 300))
                            {
                                if (strcmp(cmdBuf, path) == 0) // desired working directory on server confirmed
                                                               // (assumption: server always returns the same working path string)
                                {
                                    pathError = FALSE;
                                    ok = TRUE; // reconnect successful; list again
                                }
                            }
                            else
                            {
                                pathError = FALSE; // error -> connection closed; 'ok' stays FALSE, go for another reconnect
                                if (canRetry)
                                    retryMsgAux = retryMsgBuf; // "retry" allowed
                                else
                                {
                                    *fatalError = TRUE; // fatal error
                                    break;
                                }
                            }
                        }

                        if (pathError) // display path error and stop
                        {
                            _snprintf_s(errBuf, _TRUNCATE, LoadStr(IDS_CHANGEWORKPATHERROR), path, replyBuf);
                            SalamanderGeneral->SalMessageBox(parent, errBuf, LoadStr(IDS_FTPERRORTITLE),
                                                             MB_OK | MB_ICONEXCLAMATION);
                            ret = FALSE; // stop - path cannot be listed -> requires path change

                            // if no accessible path was found, disconnect here;
                            // the connect sequence handles the issue (see CControlConnectionSocket::ChangeWorkingPath());
                            // duplicating that logic here is unnecessary

                            break;
                        }
                    }
                    else // error -> connection closed; 'ok' stays FALSE, try another reconnect
                    {
                        if (canRetry)
                            retryMsgAux = retryMsgBuf; // "retry" allowed
                        else
                        {
                            *fatalError = TRUE; // fatal error
                            break;
                        }
                    }
                }
                else // reconnect failed - finish with error (incomplete listing)
                {
                    SalamanderGeneral->SalMessageBox(parent, LoadStr(IDS_UNABLETOREADLIST),
                                                     LoadStr(IDS_FTPERRORTITLE),
                                                     MB_OK | MB_ICONEXCLAMATION);
                    break;
                }
            }
        }
    }

    if (ret && !*fatalError) // no path error or fatal error
    {
        if (dataConnection->IsConnected()) // error: "data connection" should already be closed
        {
            TRACE_E("Unexpected situation in CControlConnectionSocket::ListWorkingPath(): data connection has left opened!");
            dataConnection->CloseSocketEx(NULL); // shutdown (result unknown)
        }

        // take data from the "data connection"
        BOOL decomprErr;
        *allocatedListing = dataConnection->GiveData(allocatedListingLen, &decomprErr);

        if (decomprErr && ok)
        {
            ok = FALSE;

            // show "list can be incomplete" message; user has not been warned yet
            lstrcpyn(errBuf, LoadStr(IDS_UNABLETOREADLIST), 900 + FTP_MAX_PATH);
            int len = (int)strlen(errBuf);
            _snprintf_s(errBuf + len, 900 + FTP_MAX_PATH - len, _TRUNCATE, LoadStr(IDS_UNABLETOREADLISTSUFFIX),
                        LoadStr(IDS_ERRDATACONDECOMPRERROR));
            SalamanderGeneral->SalMessageBox(parent, errBuf, LoadStr(IDS_FTPERRORTITLE),
                                             MB_OK | MB_ICONEXCLAMATION);
        }

        *pathListingIsIncomplete = !ok; // TRUE when the connection failed/interrupted/errored
        *pathListingMayBeOutdated = FALSE;

        // store the date the listing was created
        listingDate->Year = st.wYear;
        listingDate->Month = (BYTE)st.wMonth;
        listingDate->Day = (BYTE)st.wDay;
        *pathListingStartTime = lstStTime;

        char userTmp[USER_MAX_SIZE];
        if (forceRefresh &&  // hard refresh indicates distrust of the path; drop it from cache including
            !dontClearCache) // all subpaths (ignore useListingsCacheAux; it does not affect the distrust)
        {                    // called only when a replacement listing is available (until then the user prefers
                             // an outdated listing over none)
            HANDLES(EnterCriticalSection(&SocketCritSect));
            lstrcpyn(hostTmp, Host, HOST_MAX_SIZE);
            unsigned short portTmp = Port;
            lstrcpyn(userTmp, User, USER_MAX_SIZE);
            CFTPServerPathType pathType = ::GetFTPServerPathType(ServerFirstReply, ServerSystem, path);
            HANDLES(LeaveCriticalSection(&SocketCritSect));

            ListingCache.RefreshOnPath(hostTmp, portTmp, userTmp, pathType, path);
        }

        if (ok) // we have a complete listing
        {
            if (!*pathListingIsBroken && useListingsCacheAux && *allocatedListing != NULL)
            { // user wants to use cache -> add the newly loaded listing to cache
                HANDLES(EnterCriticalSection(&SocketCritSect));
                lstrcpyn(hostTmp, Host, HOST_MAX_SIZE);
                unsigned short portTmp = Port;
                lstrcpyn(userTmp, User, USER_MAX_SIZE);
                CFTPServerPathType pathType = ::GetFTPServerPathType(ServerFirstReply, ServerSystem, path);
                BOOL isFTPS = EncryptControlConnection == 1;
                HANDLES(LeaveCriticalSection(&SocketCritSect));

                ListingCache.AddOrUpdatePathListing(hostTmp, portTmp, userTmp, pathType, path,
                                                    listCmd, isFTPS, *allocatedListing,
                                                    *allocatedListingLen, listingDate,
                                                    *pathListingStartTime);
            }
        }
        else // failure/interruption/connection error = return whatever we have (user already knows "list can be incomplete")
        {
            if (*allocatedListing != NULL)
            { // buffer lacks a complete listing -> trim at last line ending (CRLF or LF) to make it easier to work with
                char* start = *allocatedListing;
                char* s = start + *allocatedListingLen;
                while (s > start && *(s - 1) != '\n')
                    s--;
                if (s < start + *allocatedListingLen) // there is room to write the terminating zero (for easier debugging)
                    *s = 0;                           // either at buffer start or right after the last LF
                *allocatedListingLen = (int)(s - start);
            }
        }
    }
    if (dataConnection != NULL) // release and possibly close the "data connection"
    {
        if (dataConnection->IsConnected())       // close the "data connection"; the system attempts a graceful shutdown
            dataConnection->CloseSocketEx(NULL); // (result unknown)
        DeleteSocket(dataConnection);
    }
    if (*fatalError)
        ret = FALSE; // never return success when a fatal error occurred
    return ret;
}

class CFinishingKeepAliveUserIface : public CSendCmdUserIfaceAbstract
{
protected:
    CWaitWindow* WaitWnd;
    HANDLE FinishedEvent;

public:
    CFinishingKeepAliveUserIface(CWaitWindow* waitWnd, HANDLE finishedEvent)
    {
        WaitWnd = waitWnd;
        FinishedEvent = finishedEvent;
    }

    virtual BOOL GetWindowClosePressed() { return WaitWnd->GetWindowClosePressed(); }
    virtual HANDLE GetFinishedEvent() { return FinishedEvent; }

    // other methods are not used
    virtual void Init(HWND parent, const char* logCmd, const char* waitWndText) {}
    virtual void BeforeAborting() {}
    virtual void AfterWrite(BOOL aborting, DWORD showTime) {}
    virtual BOOL HandleESC(HWND parent, BOOL isSend, BOOL allowCmdAbort) { return FALSE; }
    virtual void SendingFinished() {}
    virtual BOOL IsTimeout(DWORD* start, DWORD serverTimeout, int* errorTextID, char* errBuf, int errBufSize) { return FALSE; }
    virtual void MaybeSuccessReplyReceived(const char* reply, int replySize) {}
    virtual void CancelDataCon() {}
    virtual BOOL CanFinishSending(int replyCode, BOOL* useTimeout) { return FALSE; }
    virtual void BeforeWaitingForFinish(int replyCode, BOOL* useTimeout) {}
    virtual void HandleDataConTimeout(DWORD* start) {}
    virtual void HandleESCWhenWaitingForFinish(HWND parent) {}
};

void CControlConnectionSocket::WaitForEndOfKeepAlive(HWND parent, int waitWndTime)
{
    CALL_STACK_MESSAGE2("CControlConnectionSocket::WaitForEndOfKeepAlive(, %d)", waitWndTime);

    DWORD startTime = GetTickCount(); // operation start time

    HANDLES(EnterCriticalSection(&SocketCritSect));

#ifdef _DEBUG
    if (SocketCritSect.RecursionCount > 1)
        TRACE_E("Incorrect call to CControlConnectionSocket::WaitForEndOfKeepAlive(): from section SocketCritSect!");
#endif

    if (!KeepAliveEnabled && KeepAliveMode != kamNone)
        TRACE_E("CControlConnectionSocket::WaitForEndOfKeepAlive(): Keep-Alive is disabled, but Mode == " << (int)KeepAliveMode);

    if (KeepAliveEnabled &&
        (KeepAliveMode == kamProcessing ||               // keep-alive command in progress; must wait for completion
         KeepAliveMode == kamWaitingForEndOfProcessing)) // already waiting for completion (should not happen)
    {
        KeepAliveMode = kamWaitingForEndOfProcessing;
        HANDLE finishedEvent = KeepAliveFinishedEvent;
        int logUID = LogUID;
        HANDLES(LeaveCriticalSection(&SocketCritSect));

        // show wait window that we are waiting for keep-alive command completion
        CWaitWindow waitWnd(parent, TRUE);
        waitWnd.SetText(LoadStr(IDS_FINISHINGKEEPALIVECMD));
        DWORD start = GetTickCount();
        DWORD waitTime = start - startTime;
        waitWnd.Create(waitTime < (DWORD)waitWndTime ? waitWndTime - waitTime : 0);

        // wait for completion or interruption (ESC/timeout) of the keep-alive command
        int serverTimeout = Config.GetServerRepliesTimeout() * 1000;
        if (serverTimeout < 1000)
            serverTimeout = 1000; // at least one second
        CFinishingKeepAliveUserIface userIface(&waitWnd, finishedEvent);
        BOOL wait = TRUE;
        while (wait)
        {
            CControlConnectionSocketEvent event;
            DWORD data1, data2;
            DWORD now = GetTickCount();
            if (now - start > (DWORD)serverTimeout)
                now = start + (DWORD)serverTimeout;
            WaitForEventOrESC(parent, &event, &data1, &data2, serverTimeout - (now - start),
                              NULL, &userIface, TRUE);
            switch (event)
            {
            case ccsevESC:
            {
                waitWnd.Show(FALSE);
                if (SalamanderGeneral->SalMessageBox(parent, LoadStr(IDS_KEEPALIVECMDESC),
                                                     LoadStr(IDS_FTPPLUGINTITLE),
                                                     MB_YESNO | MSGBOXEX_ESCAPEENABLED |
                                                         MB_ICONQUESTION) == IDYES)
                { // cancel
                    Logs.LogMessage(logUID, LoadStr(IDS_LOGMSGACTIONCANCELED), -1, TRUE);
                    ReleaseKeepAlive(); // release keep-alive
                    CloseSocket(NULL);  // close connection
                    Logs.SetIsConnected(logUID, IsConnected());
                    Logs.RefreshListOfLogsInLogsDlg(); // report "connection inactive"
                    wait = FALSE;
                }
                else
                {
                    SalamanderGeneral->WaitForESCRelease(); // ensure the next action is not interrupted by the previous ESC press
                    waitWnd.Show(TRUE);
                }
                break;
            }

            case ccsevTimeout:
            {
                BOOL isTimeout = TRUE;

                HANDLES(EnterCriticalSection(&SocketCritSect));
                if (KeepAliveDataCon != NULL)
                {
                    BOOL trFinished;
                    if (KeepAliveDataCon->IsTransfering(&trFinished))
                    { // waiting for data, so this is not a timeout
                        start = GetTickCount();
                        isTimeout = FALSE;
                    }
                    else
                    {
                        if (trFinished)
                        {
                            start = KeepAliveDataCon->GetSocketCloseTime();
                            isTimeout = (GetTickCount() - start) >= (DWORD)serverTimeout; // measure timeout from connection close (moment when the server can react; it also learns about the close)
                        }
                        // else isTimeout = TRUE;  // connection has not opened yet -> treat as timeout
                    }
                }
                HANDLES(LeaveCriticalSection(&SocketCritSect));

                if (isTimeout)
                {
                    Logs.LogMessage(logUID, LoadStr(IDS_LOGMSGKEEPALIVECMDTIMEOUT), -1, TRUE);
                    ReleaseKeepAlive(); // release keep-alive
                    CloseSocket(NULL);  // close connection
                    Logs.SetIsConnected(logUID, IsConnected());
                    Logs.RefreshListOfLogsInLogsDlg(); // report "connection inactive"
                    wait = FALSE;
                }
                break;
            }

            case ccsevNewBytesRead:
                break; // ignore (at worst an old event; after this method, the command will be written and the server replies)

            case ccsevClosed: // connection closed; wrap up keep-alive and let someone else handle it
            {
                ReleaseKeepAlive();
                AddEvent(ccsevClosed, data1, data2);
                wait = FALSE;
                break;
            }

            case ccsevUserIfaceFinished:
                wait = FALSE;
                break; // keep-alive command finished

            default:
            {
                TRACE_E("CControlConnectionSocket::WaitForEndOfKeepAlive: Unexpected event (" << (int)event << ").");
                break;
            }
            }
        }
        waitWnd.Destroy();

        // keep-alive command already completed or was interrupted (ESC/timeout)
        HANDLES(EnterCriticalSection(&SocketCritSect));
        KeepAliveMode = kamForbidden;
        HANDLES(LeaveCriticalSection(&SocketCritSect));
    }
    else
    {
        if (KeepAliveEnabled)
        {
            BOOL deleteTimer = FALSE;
            int uid;
            if (KeepAliveMode == kamWaiting)
            {
                deleteTimer = TRUE;
                uid = UID;
            }
            KeepAliveMode = kamForbidden;
            HANDLES(LeaveCriticalSection(&SocketCritSect));

            if (deleteTimer)
            {
                // in 'kamForbidden' mode the keep-alive timer makes no sense (its timeout would be ignored), remove it
                SocketsThread->DeleteTimer(uid, CTRLCON_KEEPALIVE_TIMERID);
            }
        }
        else
            HANDLES(LeaveCriticalSection(&SocketCritSect));
    }
}

void CControlConnectionSocket::SetupKeepAliveTimer(BOOL immediate)
{
    CALL_STACK_MESSAGE2("CControlConnectionSocket::SetupKeepAliveTimer(%d)", immediate);

    HANDLES(EnterCriticalSection(&SocketCritSect));

#ifdef _DEBUG
    if (SocketCritSect.RecursionCount > 1)
        TRACE_E("Incorrect call to CControlConnectionSocket::SetupKeepAliveTimer(): from section SocketCritSect!");
#endif

    if (!KeepAliveEnabled && KeepAliveMode != kamNone)
        TRACE_E("CControlConnectionSocket::SetupKeepAliveTimer(): Keep-Alive is disabled, but Mode == " << (int)KeepAliveMode);
    BOOL timer = FALSE;
    int msg;
    int uid;
    DWORD ti;
    if (KeepAliveEnabled && KeepAliveMode == kamForbidden) // called after completing a regular command
    {
        KeepAliveMode = kamWaiting;
        timer = TRUE;
        msg = Msg;
        uid = UID;
        KeepAliveStart = GetTickCount();                                   // time of the last regular command in the control connection
        ti = KeepAliveStart + (immediate ? 0 : KeepAliveSendEvery * 1000); // when the first keep-alive command should be sent
    }
    else
    {
        if (KeepAliveMode != kamNone)
            TRACE_E("CControlConnectionSocket::SetupKeepAliveTimer(): unexpected Mode == " << (int)KeepAliveMode);
    }
    HANDLES(LeaveCriticalSection(&SocketCritSect));

    if (timer) // need to set up keep-alive timer
        SocketsThread->AddTimer(msg, uid, ti, CTRLCON_KEEPALIVE_TIMERID, NULL);
}

void CControlConnectionSocket::SetupNextKeepAliveTimer()
{
    CALL_STACK_MESSAGE1("CControlConnectionSocket::SetupNextKeepAliveTimer()");

    HANDLES(EnterCriticalSection(&SocketCritSect));

#ifdef _DEBUG
    if (SocketCritSect.RecursionCount > 1)
        TRACE_E("Incorrect call to CControlConnectionSocket::SetupNextKeepAliveTimer(): from section SocketCritSect!");
#endif

    if (!KeepAliveCmdAllBytesWritten)
    { // should never happen because the server replies only after the complete
        // command is written (and it is sent in one chunk; just a few bytes)
        TRACE_E("Unexpected situation in CControlConnectionSocket::SetupNextKeepAliveTimer(): KeepAliveCmdAllBytesWritten==FALSE!");
        KeepAliveCmdAllBytesWritten = TRUE;
    }

    if (KeepAliveDataCon != NULL || KeepAliveDataConState != kadcsNone)
        TRACE_E("Unexpected situation in CControlConnectionSocket::SetupNextKeepAliveTimer(): KeepAliveDataCon!=NULL or KeepAliveDataConState!=kadcsNone!");

    BOOL timer = FALSE;
    int msg;
    int uid;
    DWORD ti;
    if (KeepAliveMode == kamProcessing) // keep-alive finished normally; decide whether to schedule it again
    {
        ti = GetTickCount() + KeepAliveSendEvery * 1000; // when the next keep-alive command should be sent
        if ((int)((ti - KeepAliveStart) / 60000) < KeepAliveStopAfter)
        {
            KeepAliveMode = kamWaiting;
            timer = TRUE;
            msg = Msg;
            uid = UID;
        }
        else
        {
            KeepAliveMode = kamNone;                                         // no more keep-alive (connection not worth defending)
            Logs.LogMessage(LogUID, LoadStr(IDS_LOGMSGKASTOPPED), -1, TRUE); // inform user that keep-alive mode stopped
        }
    }
    else
    {
        if (KeepAliveMode == kamWaitingForEndOfProcessing) // main thread waits for keep-alive command to finish
        {
            SetEvent(KeepAliveFinishedEvent);
        }
        else
        {
            if (KeepAliveMode != kamNone) // kamNone = ReleaseKeepAlive() was called
                TRACE_E("CControlConnectionSocket::SetupNextKeepAliveTimer(): unexpected Mode == " << (int)KeepAliveMode);
        }
    }
    HANDLES(LeaveCriticalSection(&SocketCritSect));

    if (timer) // need to set up keep-alive timer
        SocketsThread->AddTimer(msg, uid, ti, CTRLCON_KEEPALIVE_TIMERID, NULL);
}

void CControlConnectionSocket::ReleaseKeepAlive()
{
    CALL_STACK_MESSAGE1("CControlConnectionSocket::ReleaseKeepAlive()");

    HANDLES(EnterCriticalSection(&SocketCritSect));

#ifdef _DEBUG
    if (SocketCritSect.RecursionCount > 1)
        TRACE_E("Incorrect call to CControlConnectionSocket::ReleaseKeepAlive(): from section SocketCritSect!");
#endif

    if (KeepAliveMode == kamProcessing || KeepAliveMode == kamWaitingForEndOfProcessing)
        SetEvent(KeepAliveFinishedEvent); // allow main thread to continue
    BOOL deleteTimer = FALSE;
    int uid;
    if (KeepAliveMode == kamWaiting)
    {
        deleteTimer = TRUE;
        uid = UID;
    }
    KeepAliveMode = kamNone; // reinitialize keep-alive
    KeepAliveCmdAllBytesWritten = TRUE;
    CKeepAliveDataConSocket* closeDataCon = KeepAliveDataCon;
    KeepAliveDataCon = NULL;
    KeepAliveDataConState = kadcsNone;
    HANDLES(LeaveCriticalSection(&SocketCritSect));

    // if a "data connection" is open, close it; it is no longer needed
    if (closeDataCon != NULL)
    {
        if (closeDataCon->IsConnected())       // close the "data connection"; the system attempts a graceful shutdown
            closeDataCon->CloseSocketEx(NULL); // (result unknown)
        DeleteSocket(closeDataCon);            // release the "data connection" via SocketsThread
    }

    if (deleteTimer)
    {
        // in 'kamNone' mode the keep-alive timer is meaningless (its timeout would be ignored), remove it
        SocketsThread->DeleteTimer(uid, CTRLCON_KEEPALIVE_TIMERID);
    }
}

void CControlConnectionSocket::PostMsgToCtrlCon(int msgID, void* msgParam)
{
    CALL_STACK_MESSAGE1("CControlConnectionSocket::PostMsgToCtrlCon()");

    HANDLES(EnterCriticalSection(&SocketCritSect));
    int msg = Msg;
    int uid = UID;
    HANDLES(LeaveCriticalSection(&SocketCritSect));

    SocketsThread->PostSocketMessage(msg, uid, msgID, msgParam);
}

void CControlConnectionSocket::ReceiveTimer(DWORD id, void* param)
{
    CALL_STACK_MESSAGE2("CControlConnectionSocket::ReceiveTimer(%u,)", id);
    if (id == CTRLCON_KEEPALIVE_TIMERID)
    {
        BOOL sendKACmd = FALSE;
        int cmd;
        HANDLES(EnterCriticalSection(&SocketCritSect));
        int logUID = LogUID;
        BOOL usePassiveModeAux;
        if (KeepAliveEnabled && KeepAliveMode == kamWaiting) // nothing prevents sending a keep-alive command
        {
            KeepAliveMode = kamProcessing;
            ResetEvent(KeepAliveFinishedEvent); // prepare the event to block the main thread until the keep-alive command finishes
            sendKACmd = TRUE;
            usePassiveModeAux = UsePassiveMode;

            if (KeepAliveCommand == 2 /* NLST */ || KeepAliveCommand == 3 /* LIST */)
            {
                // allocate an object for the "data connection"
                if (KeepAliveDataCon != NULL)
                    TRACE_E("Unexpected situation in CControlConnectionSocket::ReceiveTimer(): KeepAliveDataCon is not NULL!");
                CFTPProxyForDataCon* dataConProxyServer = ProxyServer == NULL ? NULL : ProxyServer->AllocProxyForDataCon(ServerIP, Host, HostIP, Port);
                BOOL dataConProxyServerOK = ProxyServer == NULL || dataConProxyServer != NULL;
                KeepAliveDataCon = dataConProxyServerOK ? new CKeepAliveDataConSocket(this, dataConProxyServer, EncryptDataConnection, pCertificate) : NULL;
                if (KeepAliveDataCon == NULL)
                {
                    if (dataConProxyServer != NULL)
                        delete dataConProxyServer;
                    if (dataConProxyServerOK)
                        TRACE_E(LOW_MEMORY);
                    KeepAliveCommand = 0; // send "NOOP" instead
                }
                else
                    KeepAliveDataConState = usePassiveModeAux ? kadcsWaitForPassiveReply : kadcsWaitForListen;
            }
            cmd = KeepAliveCommand;
        }
        CKeepAliveDataConSocket* keepAliveDataConAux = KeepAliveDataCon;
        HANDLES(LeaveCriticalSection(&SocketCritSect));

        if (sendKACmd)
        {
            // send the keep-alive command (no waiting, no delays allowed)
            char ftpCmd[200];
            ftpCmd[0] = 0;
            BOOL waitForListen = FALSE;
            switch (cmd)
            {
            case 0: // NOOP
            {
                PrepareFTPCommand(ftpCmd, 200, NULL, 0, ftpcmdNoOperation, NULL);
                break;
            }

            case 1: // PWD
            {
                PrepareFTPCommand(ftpCmd, 200, NULL, 0, ftpcmdPrintWorkingPath, NULL);
                break;
            }

            case 2: // NLST
            case 3: // LIST
            {
                if (usePassiveModeAux) // passive "data connection" mode
                {
                    PrepareFTPCommand(ftpCmd, 200, NULL, 0, ftpcmdPassive, NULL);
                }
                else // active "data connection" mode
                {
                    DWORD localIP;
                    GetLocalIP(&localIP, NULL);   // should not fail
                    unsigned short localPort = 0; // listen on any port
                    DWORD error;
                    keepAliveDataConAux->SetActive(logUID);
                    BOOL listenError;
                    if (!keepAliveDataConAux->OpenForListeningWithProxy(localIP, localPort, &listenError, &error))
                    { // failed to open listening socket for incoming data connection from the
                        // server (local operation; unlikely) and there may also be
                        // an error when connecting to the proxy server
                        Logs.LogMessage(logUID, LoadStr(listenError ? IDS_LOGMSGOPENACTDATACONERROR : IDS_LOGMSGOPENACTDATACONERROR2), -1, TRUE);
                    }
                    else
                        waitForListen = TRUE;
                }
                break;
            }

            default:
            {
                TRACE_E("CControlConnectionSocket::ReceiveTimer(): unknown keep-alive command!");
                ftpCmd[0] = 0;
                break;
            }
            }

            if (ftpCmd[0] != 0 || waitForListen)
                Logs.LogMessage(logUID, LoadStr(IDS_LOGMSGKEEPALIVE), -1, TRUE);
            if (ftpCmd[0] != 0)
                SendKeepAliveCmd(logUID, ftpCmd); // send keep-alive command
            else
            {
                if (!waitForListen)
                    ReleaseKeepAlive(); // nothing was sent (no reason to continue with keep-alive); cancel it
            }
        }
    }
}

void CControlConnectionSocket::ReceivePostMessage(DWORD id, void* param)
{
    CALL_STACK_MESSAGE2("CControlConnectionSocket::ReceivePostMessage(%u,)", id);
    switch (id)
    {
    case CTRLCON_KAPOSTSETUPNEXT: // keep-alive data connection just finished; listing completion reply already arrived -> call SetupNextKeepAliveTimer()
    {
        HANDLES(EnterCriticalSection(&SocketCritSect));
        BOOL call = (KeepAliveMode == kamProcessing || KeepAliveMode == kamWaitingForEndOfProcessing); // nothing unexpected happened?
        CKeepAliveDataConSocket* closeDataCon = KeepAliveDataCon;
        if (call)
        {
            KeepAliveDataCon = NULL;
            KeepAliveDataConState = kadcsNone;
        }
        HANDLES(LeaveCriticalSection(&SocketCritSect));

        if (call)
        {
            if (closeDataCon != NULL)
                DeleteSocket(closeDataCon);
            SetupNextKeepAliveTimer();
        }
        break;
    }

    case CTRLCON_LISTENFORCON: // message about opening a listening port (on proxy server)
    {
        AddEvent(ccsevListenForCon, (DWORD)(DWORD_PTR)param, 0);
        break;
    }

    case CTRLCON_KALISTENFORCON: // keep-alive: message about opening a listening port (on proxy server)
    {
        HANDLES(EnterCriticalSection(&SocketCritSect));
        if ((KeepAliveMode == kamProcessing || KeepAliveMode == kamWaitingForEndOfProcessing) &&
            KeepAliveDataConState == kadcsWaitForListen)
        {
            CKeepAliveDataConSocket* kaDataConnection = KeepAliveDataCon;
            int logUID = LogUID; // log UID of this connection
            HANDLES(LeaveCriticalSection(&SocketCritSect));

            if ((int)(INT_PTR)param == kaDataConnection->GetUID()) // process message only if it matches our data connection
            {
                DWORD listenOnIP;
                unsigned short listenOnPort;
                char buf[300];
                char errBuf[500];
                if (!kaDataConnection->GetListenIPAndPort(&listenOnIP, &listenOnPort)) // listen failure
                {
                    if (kaDataConnection->GetProxyError(buf, 300, NULL, 0, TRUE))
                    { // log the error
                        _snprintf_s(errBuf, _TRUNCATE, LoadStr(IDS_LOGMSGDATCONERROR), buf);
                        Logs.LogMessage(logUID, errBuf, -1, TRUE);
                    }
                    ReleaseKeepAlive(); // abort keep-alive
                }
                else // success; send "PORT" command
                {
                    HANDLES(EnterCriticalSection(&SocketCritSect));
                    KeepAliveDataConState = kadcsWaitForSetPortReply;
                    HANDLES(LeaveCriticalSection(&SocketCritSect));

                    PrepareFTPCommand(buf, 300, NULL, 0, ftpcmdSetPort, NULL, listenOnIP, listenOnPort);
                    SendKeepAliveCmd(logUID, buf);
                }
            }
        }
        else
            HANDLES(LeaveCriticalSection(&SocketCritSect));
        break;
    }
    }
}

BOOL CControlConnectionSocket::InitOperation(CFTPOperation* oper)
{
    CALL_STACK_MESSAGE1("CControlConnectionSocket::InitOperation()");
    HANDLES(EnterCriticalSection(&SocketCritSect));
    BOOL ret = oper->SetConnection(ProxyServer, Host, Port, User, Password, Account,
                                   InitFTPCommands, UsePassiveMode,
                                   UseLIST_aCommand ? LIST_a_CMD_TEXT : ListCommand,
                                   ServerIP, ServerSystem, ServerFirstReply,
                                   UseListingsCache, HostIP);
    HANDLES(LeaveCriticalSection(&SocketCritSect));
    return ret;
}

//
// ****************************************************************************
// CListingCacheItem
//

CListingCacheItem::CListingCacheItem(const char* host, unsigned short port, const char* user,
                                     const char* path, const char* listCmd, BOOL isFTPS,
                                     const char* cachedListing, int cachedListingLen,
                                     const CFTPDate& cachedListingDate,
                                     DWORD cachedListingStartTime, CFTPServerPathType pathType)
{
    // copy the data
    BOOL err = (host == NULL || path == NULL || listCmd == NULL);
    Host = SalamanderGeneral->DupStr(host);
    Port = port;
    if (user != NULL && strcmp(user, FTP_ANONYMOUS) == 0)
        user = NULL;
    User = SalamanderGeneral->DupStr(user); // remains NULL if input was NULL; 'err' stays unchanged
    Path = SalamanderGeneral->DupStr(path);
    ListCmd = SalamanderGeneral->DupStr(listCmd);
    IsFTPS = isFTPS;
    CachedListing = (char*)malloc(cachedListingLen + 1); // +1 to support zero-length listings
    if (CachedListing != NULL && cachedListing != NULL)
    {
        memcpy(CachedListing, cachedListing, cachedListingLen);
        CachedListing[cachedListingLen] = 0; // with memory allocated, null-terminate for debugging purposes
    }
    else
        err = TRUE;
    CachedListingLen = cachedListingLen;
    CachedListingDate = cachedListingDate;
    CachedListingStartTime = cachedListingStartTime;
    PathType = pathType;

    // on error, release and zero the data
    if (err)
    {
        if (User != NULL)
            SalamanderGeneral->Free(User);
        if (Host != NULL)
            SalamanderGeneral->Free(Host);
        if (Path != NULL)
            SalamanderGeneral->Free(Path);
        if (ListCmd != NULL)
            SalamanderGeneral->Free(ListCmd);
        if (CachedListing != NULL)
        {
            memset(CachedListing, 0, CachedListingLen); // might hold sensitive data; wipe it just in case
            free(CachedListing);
        }
        User = NULL;
        Host = NULL;
        Path = NULL;
        ListCmd = NULL;
        CachedListing = NULL;
    }
    UserLength = FTPGetUserLength(User);
}

CListingCacheItem::~CListingCacheItem()
{
    if (User != NULL)
        SalamanderGeneral->Free(User);
    if (Host != NULL)
        SalamanderGeneral->Free(Host);
    if (Path != NULL)
        SalamanderGeneral->Free(Path);
    if (ListCmd != NULL)
        SalamanderGeneral->Free(ListCmd);
    if (CachedListing != NULL)
    {
        memset(CachedListing, 0, CachedListingLen); // might hold sensitive data; wipe it just in case
        free(CachedListing);
    }
}

//
// ****************************************************************************
// CListingCache
//

CListingCache::CListingCache() : Cache(100, 50), TotalCacheSize(0, 0)
{
    HANDLES(InitializeCriticalSection(&CacheCritSect));
}

CListingCache::~CListingCache()
{
#ifdef _DEBUG
    int i;
    for (i = 0; i < Cache.Count; i++)
        TotalCacheSize -= CQuadWord(Cache[i]->CachedListingLen, 0);
    if (TotalCacheSize != CQuadWord(0, 0))
        TRACE_E("CListingCache::~CListingCache(): TotalCacheSize is not zero when cache is empty!");
#endif
    HANDLES(DeleteCriticalSection(&CacheCritSect));
}

BOOL CListingCache::Find(const char* host, unsigned short port, const char* user,
                         CFTPServerPathType pathType, const char* path, const char* listCmd,
                         BOOL isFTPS, int* index)
{
    if (user != NULL && strcmp(user, FTP_ANONYMOUS) == 0)
        user = NULL;
    int i;
    for (i = 0; i < Cache.Count; i++)
    {
        CListingCacheItem* item = Cache[i];
        if (SalamanderGeneral->StrICmp(host, item->Host) == 0 &&
            (user == NULL && item->User == NULL ||
             item->User != NULL && user != NULL && strcmp(user, item->User) == 0) &&
            port == item->Port &&
            FTPIsTheSameServerPath(pathType, path, item->Path) &&
            isFTPS == item->IsFTPS &&
            SalamanderGeneral->StrICmp(listCmd, item->ListCmd) == 0)
        {
            *index = i;
            return TRUE;
        }
    }
    return FALSE;
}

BOOL CListingCache::GetPathListing(const char* host, unsigned short port, const char* user,
                                   CFTPServerPathType pathType, char* path, int pathBufSize,
                                   const char* listCmd, BOOL isFTPS, char** cachedListing,
                                   int* cachedListingLen, CFTPDate* cachedListingDate,
                                   DWORD* cachedListingStartTime)
{
    HANDLES(EnterCriticalSection(&CacheCritSect));

    BOOL found = FALSE;
    int index;
    if (Find(host, port, user, pathType, path, listCmd, isFTPS, &index)) // update cache entry
    {
        found = TRUE;
        CListingCacheItem* item = Cache[index];
        *cachedListing = (char*)malloc(item->CachedListingLen + 1); // +1 to support zero-length listings
        if (*cachedListing != NULL)
        {
            memcpy(*cachedListing, item->CachedListing, item->CachedListingLen);
            (*cachedListing)[item->CachedListingLen] = 0; // with memory allocated, null-terminate for debugging purposes
            *cachedListingLen = item->CachedListingLen;
        }
        else
            TRACE_E(LOW_MEMORY); // *cachedListingLen stays 0; caller handles the memory error
        *cachedListingDate = item->CachedListingDate;
        *cachedListingStartTime = item->CachedListingStartTime;
        lstrcpyn(path, item->Path, pathBufSize);
    }

    HANDLES(LeaveCriticalSection(&CacheCritSect));
    return found;
}

void CListingCache::AddOrUpdatePathListing(const char* host, unsigned short port, const char* user,
                                           CFTPServerPathType pathType, const char* path,
                                           const char* listCmd, BOOL isFTPS,
                                           const char* cachedListing, int cachedListingLen,
                                           const CFTPDate* cachedListingDate,
                                           DWORD cachedListingStartTime)
{
    HANDLES(EnterCriticalSection(&CacheCritSect));

    // if the item already exists, delete it (updating in place is not worth the trouble)
    int index;
    if (Find(host, port, user, pathType, path, listCmd, isFTPS, &index))
    {
        TotalCacheSize -= CQuadWord(Cache[index]->CachedListingLen, 0);
        Cache.Delete(index);
        if (!Cache.IsGood())
            Cache.ResetState();
    }

    // insert a new item into the cache
    CListingCacheItem* item = new CListingCacheItem(host, port, user, path, listCmd, isFTPS,
                                                    cachedListing, cachedListingLen,
                                                    *cachedListingDate,
                                                    cachedListingStartTime, pathType);
    if (item != NULL && item->IsGood())
    {
        Cache.Add(item);
        if (Cache.IsGood())
        {
            TotalCacheSize += CQuadWord(item->CachedListingLen, 0);
            item = NULL; // successfully inserted; do not free it later in this method

            // if the cache holds too many items, remove the oldest ones; keep the most recently added
            int count = 0; // number of items to delete (remove them in bulk to avoid O(N^2) shifts)
            while (Cache.Count > count + 1 && TotalCacheSize > Config.CacheMaxSize)
                TotalCacheSize -= CQuadWord(Cache[count++]->CachedListingLen, 0);
            if (count > 0)
            {
                Cache.Delete(0, count);
                if (!Cache.IsGood())
                    Cache.ResetState();
            }
        }
        else
            Cache.ResetState();
    }
    if (item != NULL)
        delete item;

    HANDLES(LeaveCriticalSection(&CacheCritSect));
}

void CListingCache::RefreshOnPath(const char* host, unsigned short port, const char* user,
                                  CFTPServerPathType pathType, const char* path, BOOL ignorePath)
{
    HANDLES(EnterCriticalSection(&CacheCritSect));

    if (user != NULL && strcmp(user, FTP_ANONYMOUS) == 0)
        user = NULL;
    int delIndex = 0; // variables for block deletions (shifting array is O(N^2); optimize it)
    int delCount = 0;
    int i;
    for (i = 0; i < Cache.Count; i++)
    {
        CListingCacheItem* item = Cache[i];
        if (SalamanderGeneral->StrICmp(host, item->Host) == 0 &&
            (user == NULL && item->User == NULL ||
             item->User != NULL && user != NULL && strcmp(user, item->User) == 0) &&
            port == item->Port &&
            (ignorePath || FTPIsPrefixOfServerPath(pathType, path, item->Path, FALSE))) // include the path and its subpaths
        {
            // delete the item from cache
            TotalCacheSize -= CQuadWord(item->CachedListingLen, 0);
            if (delIndex + delCount == i)
                delCount++; // extend the current deletion block
            else            // need to create a new block and delete the previous one
            {
                if (delCount > 0)
                {
                    Cache.Delete(delIndex, delCount);
                    if (!Cache.IsGood())
                        Cache.ResetState();
                    i -= delCount; // adjust index after removing the previous block (must be entirely before 'i')
                }
                delIndex = i;
                delCount = 1;
            }
        }
    }
    if (delCount > 0)
    {
        Cache.Delete(delIndex, delCount);
        if (!Cache.IsGood())
            Cache.ResetState();
    }

    HANDLES(LeaveCriticalSection(&CacheCritSect));
}

void CListingCache::AcceptChangeOnPathNotification(const char* userPart, BOOL includingSubdirs)
{
    char buf[FTP_USERPART_SIZE];
    const char* pathPart = NULL;
    char *user, *host, *portStr, *pathStr;
    int port;
    int userLength = -1;

    HANDLES(EnterCriticalSection(&CacheCritSect));

    int delIndex = 0; // variables for block deletions (shifting array is O(N^2); optimize it)
    int delCount = 0;
    int i;
    for (i = 0; i < Cache.Count; i++)
    {
        CListingCacheItem* item = Cache[i];
        if (userLength == -1 || userLength != item->UserLength)
        {
            userLength = item->UserLength;
            lstrcpyn(buf, userPart, FTP_USERPART_SIZE);
            FTPSplitPath(buf, &user, NULL, &host, &portStr, &pathStr, NULL, userLength);
            if (pathStr != NULL && pathStr > buf)
                pathPart = userPart + (pathStr - buf) - 1;
            port = portStr != NULL ? atoi(portStr) : IPPORT_FTP;
            if (user != NULL && strcmp(user, FTP_ANONYMOUS) == 0)
                user = NULL;
            if (host == NULL || pathPart == NULL)
            { // may still be a coincidence; try again with unknown username length
                lstrcpyn(buf, userPart, FTP_USERPART_SIZE);
                FTPSplitPath(buf, &user, NULL, &host, &portStr, &pathStr, NULL, 0);
                if (pathStr != NULL && pathStr > buf)
                    pathPart = userPart + (pathStr - buf) - 1;
                port = portStr != NULL ? atoi(portStr) : IPPORT_FTP;
                if (user != NULL && strcmp(user, FTP_ANONYMOUS) == 0)
                    user = NULL;
                if (host == NULL || pathPart == NULL)
                {
                    TRACE_E("CListingCache::AcceptChangeOnPathNotification(): invalid (or relative) path received: " << userPart);
                    HANDLES(LeaveCriticalSection(&CacheCritSect));
                    return; // such items do not exist in the cache; nothing to do
                }
            }
        }
        if (SalamanderGeneral->StrICmp(host, item->Host) == 0 &&
            (user == NULL && item->User == NULL ||
             item->User != NULL && user != NULL && strcmp(user, item->User) == 0) &&
            port == item->Port &&
            FTPIsPrefixOfServerPath(item->PathType, FTPGetLocalPath(pathPart, item->PathType),
                                    item->Path, !includingSubdirs))
        { // item matches the changed path or its subdirectory; delete it from cache
            TotalCacheSize -= CQuadWord(item->CachedListingLen, 0);
            if (delIndex + delCount == i)
                delCount++; // extend the current deletion block
            else            // need to create a new block and delete the previous one
            {
                if (delCount > 0)
                {
                    Cache.Delete(delIndex, delCount);
                    if (!Cache.IsGood())
                        Cache.ResetState();
                    i -= delCount; // adjust index after removing the previous block (must be entirely before 'i')
                }
                delIndex = i;
                delCount = 1;
            }
        }
    }
    if (delCount > 0)
    {
        Cache.Delete(delIndex, delCount);
        if (!Cache.IsGood())
            Cache.ResetState();
    }

    HANDLES(LeaveCriticalSection(&CacheCritSect));
}
