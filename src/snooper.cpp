// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"

#include "plugins.h"
#include "fileswnd.h"
#include "mainwnd.h"
#include "snooper.h"

CWindowArray WindowArray(10, 5);
CObjectArray ObjectArray(10, 5);

HANDLE Thread = NULL;
HANDLE DataUsageMutex = NULL;       // because arrays with data are shared by the thread and the process
HANDLE RefreshFinishedEvent = NULL; // used for PostMessage; waits for completion
HANDLE WantDataEvent = NULL;        // the main thread wants to access shared data
HANDLE TerminateEvent = NULL;       // main thread wants to terminate the snooper thread
HANDLE ContinueEvent = NULL;        // helper event for synchronization
HANDLE BeginSuspendEvent = NULL;    // start of suspend mode
HANDLE EndSuspendEvent = NULL;      // end of suspend mode for the snooper
HANDLE SharesEvent = NULL;          // signaled when LanMan Shares change

int SnooperSuspended = 0;

CRITICAL_SECTION TimeCounterSection; // synchronizes access to MyTimeCounter
int MyTimeCounter = 0;               // current time

HANDLE SafeFindCloseThread = NULL;              // thread that closes handles safely
TDirectArray<HANDLE> SafeFindCloseCNArr(10, 5); // non-blocking closure of change-notify handles
CRITICAL_SECTION SafeFindCloseCS;               // critical section protecting the handle array
BOOL SafeFindCloseTerminate = FALSE;            // used to terminate the thread
HANDLE SafeFindCloseStart = NULL;               // thread starter; waits when not signaled
HANDLE SafeFindCloseFinished = NULL;            // signaled when all handles are closed

DWORD WINAPI ThreadFindCloseChangeNotification(void* param);

void DoWantDataEvent()
{
    ReleaseMutex(DataUsageMutex);                  // release data for the main thread
    WaitForSingleObject(WantDataEvent, INFINITE);  // wait until it grabs them
    WaitForSingleObject(DataUsageMutex, INFINITE); // once done the data are ours again
    SetEvent(ContinueEvent);                       // data are ours, let the main thread continue
}

unsigned ThreadSnooperBody(void* /*param*/) // do not call main-thread functions (not even TRACE) !!!
{
    CALL_STACK_MESSAGE1("ThreadSnooperBody()");
    SetThreadNameInVCAndTrace("Snooper");
    TRACE_I("Begin");

    DWORD res;
    HKEY sharesKey;
    res = HANDLES_Q(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                 "system\\currentcontrolset\\services\\lanmanserver\\shares",
                                 0, KEY_NOTIFY, &sharesKey));
    if (res != ERROR_SUCCESS)
    {
        sharesKey = NULL;
        TRACE_E("Unable to open key in registry (LanMan Shares). error: " << GetErrorText(res));
    }
    else // the key is fine, set up notifications (otherwise RegNotifyChangeKeyValue would never be called again)
    {
        if ((res = RegNotifyChangeKeyValue(sharesKey, TRUE, REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET, SharesEvent,
                                           TRUE)) != ERROR_SUCCESS)
        {
            TRACE_E("Unable to monitor registry (LanMan Shares). error: " << GetErrorText(res));
        }
    }

    if (WaitForSingleObject(DataUsageMutex, INFINITE) == WAIT_OBJECT_0)
    {
        SetEvent(ContinueEvent); // the snooper now owns the data; main thread can continue

        WindowArray.Add(NULL); // base objects, must be at the beginning!
        WindowArray.Add(NULL);
        WindowArray.Add(NULL);
        WindowArray.Add(NULL);
        ObjectArray.Add(WantDataEvent);
        ObjectArray.Add(TerminateEvent);
        ObjectArray.Add(BeginSuspendEvent);
        ObjectArray.Add(SharesEvent);

        BOOL ignoreRefreshes = FALSE;        // TRUE = ignore refreshes (directory changes); otherwise run normally
        DWORD ignoreRefreshesAbsTimeout = 0; // once (int)(GetTickCount() - ignoreRefreshesAbsTimeout) >= 0, we switch ignoreRefreshes to FALSE
        BOOL notEnd = TRUE;
        while (notEnd)
        {
            int timeout = ignoreRefreshes ? (int)(ignoreRefreshesAbsTimeout - GetTickCount()) : INFINITE;
            if (ignoreRefreshes && timeout <= 0)
            {
                ignoreRefreshes = FALSE;
                ignoreRefreshesAbsTimeout = 0;
                timeout = INFINITE;
            }
            //      TRACE_I("Snooper is waiting for: " << (ignoreRefreshes ? min(4, ObjectArray.Count) : ObjectArray.Count) << " events");
            res = WaitForMultipleObjects(ignoreRefreshes ? min(4, ObjectArray.Count) : ObjectArray.Count,
                                         (HANDLE*)ObjectArray.GetData(),
                                         FALSE, timeout);
            CALL_STACK_MESSAGE2("ThreadSnooperBody::wait_satisfied: 0x%X", res);
            switch (res)
            {
            case WAIT_OBJECT_0:
                DoWantDataEvent();
                break; // WantDataEvent
            case WAIT_OBJECT_0 + 1:
                notEnd = FALSE;
                break;              // TerminateEvent
            case WAIT_OBJECT_0 + 2: // BeginSuspendMode
            {
                TRACE_I("Start suspend mode");

                SetEvent(ContinueEvent); // now in suspend -> let the main thread continue

                TDirectArray<HWND> refreshPanels(10, 5); // in case the watched directory is deleted

                ObjectArray[2] = EndSuspendEvent; // instead of Begin we now wait for EndSuspendMode

                BOOL setSharesEvent = FALSE; // TRUE => re-enable registry monitoring
                BOOL suspendNotFinished = TRUE;
                while (suspendNotFinished) // wait until suspend mode ends
                {                          // handle everything except directory changes
                    timeout = ignoreRefreshes ? (int)(ignoreRefreshesAbsTimeout - GetTickCount()) : INFINITE;
                    if (ignoreRefreshes && timeout <= 0)
                    {
                        ignoreRefreshes = FALSE;
                        ignoreRefreshesAbsTimeout = 0;
                        timeout = INFINITE;
                    }
                    res = WaitForMultipleObjects(ignoreRefreshes ? min(4, ObjectArray.Count) : ObjectArray.Count,
                                                 (HANDLE*)ObjectArray.GetData(),
                                                 FALSE, timeout);

                    CALL_STACK_MESSAGE2("ThreadSnooperBody::suspend_wait_satisfied: 0x%X", res);
                    switch (res)
                    {
                    case WAIT_OBJECT_0:
                        DoWantDataEvent();
                        break; // WantDataEvent
                    case WAIT_OBJECT_0 + 1:
                        suspendNotFinished = notEnd = FALSE;
                        break; // TerminateEvent
                    case WAIT_OBJECT_0 + 2:
                        suspendNotFinished = FALSE;
                        break;              // EndSuspendEvent
                    case WAIT_OBJECT_0 + 3: // SharesEvent
                    {
                        // update the shares and refresh panels if needed (via WM_USER_REFRESH_SHARES)
                        setSharesEvent = TRUE;
                        break;
                    }

                    case WAIT_TIMEOUT:
                        break; // ignored (end of directory-change ignore mode)

                    default:
                    {
                        int index = res - WAIT_OBJECT_0;
                        if (index < 0 || index >= WindowArray.Count)
                        {
                            TRACE_E("Unexpected value returned from WaitForMultipleObjects(): " << res);
                            break; // in case of an unexpected value of 'res'
                        }

                        // calling FindCloseChangeNotification invalidates other handles for the same path
                        // (as it does for UNC paths), so we forcibly simulate the signaled state
                        HANDLE sameHandle = NULL; // != NULL -> handle for the same path
                        CFilesWindow* actWin = WindowArray[index];
                        int e;
                        for (e = 0; e < WindowArray.Count; e++)
                        {
                            CFilesWindow* w = WindowArray[e];
                            if (w != NULL && w != actWin && actWin->SamePath(w))
                            {
                                sameHandle = (HANDLE)ObjectArray[e];
                                break;
                            }
                        }

                        // a change already occurred; we ignore others, a refresh will follow after suspend
                        if (MainWindowCS.LockIfNotClosed())
                        {
                            //                  TRACE_I("Change notification in suspend mode: " << (MainWindow->LeftPanel == WindowArray[index] ? "left" : "right"));
                            MainWindowCS.Unlock();
                        }
                        HDEVNOTIFY panelDevNotification = WindowArray[index]->DeviceNotification;
                        if (panelDevNotification != NULL)
                        {
                            UnregisterDeviceNotification(panelDevNotification);
                            WindowArray[index]->DeviceNotification = NULL;
                        }
                        HANDLES(FindCloseChangeNotification((HANDLE)ObjectArray[index]));
                        refreshPanels.Add(WindowArray[index]->HWindow); // add to the list of panels to refresh
                        ObjectArray.Delete(index);                      // remove it from the list
                        WindowArray.Delete(index);

                        // if needed to work around a system bug, do it here
                        if (sameHandle != NULL)
                        {
                            for (index = 0; index < ObjectArray.Count; index++)
                            {
                                if ((HANDLE)ObjectArray[index] == sameHandle)
                                {
                                    HDEVNOTIFY panelDevNotification2 = WindowArray[index]->DeviceNotification;
                                    if (panelDevNotification2 != NULL)
                                    {
                                        UnregisterDeviceNotification(panelDevNotification2);
                                        WindowArray[index]->DeviceNotification = NULL;
                                    }
                                    HANDLES(FindCloseChangeNotification((HANDLE)ObjectArray[index]));
                                    refreshPanels.Add(WindowArray[index]->HWindow); // add to the list of panels to refresh
                                    ObjectArray.Delete(index);                      // remove it from the list
                                    WindowArray.Delete(index);
                                }
                            }
                        }
                        break;
                    }
                    }
                }
                SetEvent(ContinueEvent); // not in suspend anymore -> let the main thread continue

                if (setSharesEvent) // we will monitor further registry changes
                {
                    if (MainWindowCS.LockIfNotClosed())
                    {
                        if (MainWindow != NULL)
                            PostMessage(MainWindow->HWindow, WM_USER_REFRESH_SHARES, 0, 0);
                        MainWindowCS.Unlock();
                    }
                    if ((res = RegNotifyChangeKeyValue(sharesKey, TRUE, REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET, SharesEvent,
                                                       TRUE)) != ERROR_SUCCESS)
                    {
                        TRACE_E("Unable to monitor registry (LanMan Shares). error: " << GetErrorText(res));
                    }
                }

                ObjectArray[2] = BeginSuspendEvent;
                TRACE_I("End suspend mode");

                CALL_STACK_MESSAGE1("ThreadSnooperBody::post_refresh");

                HANDLES(EnterCriticalSection(&TimeCounterSection));
                // refresh the panels that changed
                int i;
                for (i = 0; i < refreshPanels.Count; i++)
                {
                    HWND wnd = refreshPanels[i];
                    if (IsWindow(wnd))
                    {
                        PostMessage(wnd, WM_USER_S_REFRESH_DIR, FALSE, MyTimeCounter++);
                    }
                }
                HANDLES(LeaveCriticalSection(&TimeCounterSection));
                // also send a notification that suspend mode ended
                if (MainWindowCS.LockIfNotClosed())
                {
                    if (MainWindow != NULL && MainWindow->LeftPanel != NULL && MainWindow->RightPanel != NULL)
                    {
                        PostMessage(MainWindow->LeftPanel->HWindow, WM_USER_SM_END_NOTIFY, 0, 0);
                        PostMessage(MainWindow->RightPanel->HWindow, WM_USER_SM_END_NOTIFY, 0, 0);
                    }
                    MainWindowCS.Unlock();
                }

                if (refreshPanels.Count > 0)
                {
                    // take a short break so the system is not overloaded
                    ignoreRefreshes = TRUE;
                    ignoreRefreshesAbsTimeout = GetTickCount() + REFRESH_PAUSE;
                }
                break;
            }

            case WAIT_OBJECT_0 + 3: // SharesEvent
            {                       // let the panels refresh
                if (MainWindowCS.LockIfNotClosed())
                {
                    if (MainWindow != NULL)
                        PostMessage(MainWindow->HWindow, WM_USER_REFRESH_SHARES, 0, 0);
                    MainWindowCS.Unlock();
                }
                // we will monitor further changes in the registry
                if ((res = RegNotifyChangeKeyValue(sharesKey, TRUE, REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET, SharesEvent,
                                                   TRUE)) != ERROR_SUCCESS)
                {
                    TRACE_E("Unable to monitor registry (LanMan Shares). error: " << GetErrorText(res));
                }
                break;
            }

            case WAIT_TIMEOUT:
                break; // ignored (end of directory-change ignore mode)

            default:
            {
                int index;
                index = res - WAIT_OBJECT_0;
                if (index < 0 || index >= WindowArray.Count)
                {
                    DWORD err = GetLastError();
                    TRACE_E("Unexpected value returned from WaitForMultipleObjects(): " << res);
                    break; // in case of an unexpected value of 'res'
                }

                // calling FindNextChangeNotification invalidates other handles for the same path
                // (as it does for UNC paths), therefore we forcibly simulate the signaled state
                HANDLE sameHandle = NULL; // != NULL -> handle for the same path
                CFilesWindow* actWin = WindowArray[index];
                int e;
                for (e = 0; e < WindowArray.Count; e++)
                {
                    CFilesWindow* w = WindowArray[e];
                    if (w != NULL && w != actWin && actWin->SamePath(w))
                    {
                        sameHandle = (HANDLE)ObjectArray[e];
                        break;
                    }
                }

                if (MainWindowCS.LockIfNotClosed())
                {
                    //            TRACE_I("Change notification: " << (MainWindow->LeftPanel == WindowArray[index] ? "left" : "right"));
                    MainWindowCS.Unlock();
                }
                HANDLES(EnterCriticalSection(&TimeCounterSection));
                PostMessage(WindowArray[index]->HWindow, WM_USER_REFRESH_DIR, TRUE, MyTimeCounter++);
                HANDLES(LeaveCriticalSection(&TimeCounterSection));
                FindNextChangeNotification((HANDLE)ObjectArray[index]); // cancel this change
                                                                        // indices may change...
            ERROR_BYPASS:

                HANDLE objects[4];
                objects[0] = WantDataEvent;        // in refresh data may change
                objects[1] = TerminateEvent;       // in case termination occurs before refresh finishes
                objects[2] = BeginSuspendEvent;    // for BeginSuspendMode calls during refresh
                objects[3] = RefreshFinishedEvent; // message from the main thread about completion

                BOOL refreshNotFinished = TRUE;
                while (refreshNotFinished) // wait until processing is done
                {                          // handle everything except directory changes
                    res = WaitForMultipleObjects(4, objects, FALSE, INFINITE);

                    switch (res)
                    {
                    case WAIT_OBJECT_0 + 0:
                        DoWantDataEvent();
                        break;              // WantDataEvent
                    case WAIT_OBJECT_0 + 1: // TerminateEvent
                        refreshNotFinished = notEnd = FALSE;
                        break;
                    case WAIT_OBJECT_0 + 2: // BeginSuspendEvent
                        refreshNotFinished = FALSE;
                        SetEvent(BeginSuspendEvent);
                        break;
                    default:
                        refreshNotFinished = FALSE;
                        break; // RefreshFinishedEvent
                    }
                }

                // work around a system bug if needed
                if (sameHandle != NULL)
                {
                    for (index = 0; index < ObjectArray.Count; index++)
                    {
                        if (sameHandle == (HANDLE)ObjectArray[index])
                        {
                            int r = WaitForSingleObject(sameHandle, 0); // simulate a wait call in case the error disappears
                            sameHandle = NULL;

                            HANDLES(EnterCriticalSection(&TimeCounterSection));
                            PostMessage(WindowArray[index]->HWindow, WM_USER_REFRESH_DIR, TRUE, MyTimeCounter++);
                            HANDLES(LeaveCriticalSection(&TimeCounterSection));

                            if (r != WAIT_TIMEOUT) // if there is no error look for the next change
                            {
                                FindNextChangeNotification((HANDLE)ObjectArray[index]); // cancel this change
                            }

                            goto ERROR_BYPASS;
                        }
                    }
                }

                // take a short break so the system is not overloaded
                ignoreRefreshes = TRUE;
                ignoreRefreshesAbsTimeout = GetTickCount() + REFRESH_PAUSE;

                break;
            }
            }
        }
        ReleaseMutex(DataUsageMutex);
    }
    if (sharesKey != NULL)
        HANDLES(RegCloseKey(sharesKey));
    TRACE_I("End");
    return 0;
}

unsigned ThreadSnooperEH(void* param)
{
#ifndef CALLSTK_DISABLE
    __try
    {
#endif // CALLSTK_DISABLE
        return ThreadSnooperBody(param);
#ifndef CALLSTK_DISABLE
    }
    __except (CCallStack::HandleException(GetExceptionInformation()))
    {
        TRACE_I("Thread Snooper: calling ExitProcess(1).");
        //    ExitProcess(1);
        TerminateProcess(GetCurrentProcess(), 1); // a harder exit (this still performs additional calls)
        return 1;
    }
#endif // CALLSTK_DISABLE
}

DWORD WINAPI ThreadSnooper(void* param)
{
#ifndef CALLSTK_DISABLE
    CCallStack stack;
#endif // CALLSTK_DISABLE
    return ThreadSnooperEH(param);
}

BOOL InitializeThread()
{
    //---  create events and a mutex for synchronization
    DataUsageMutex = HANDLES(CreateMutex(NULL, FALSE, NULL));
    if (DataUsageMutex == NULL)
    {
        TRACE_E("Unable to create DataUsageMutex mutex.");
        return FALSE;
    }
    WantDataEvent = HANDLES(CreateEvent(NULL, FALSE, FALSE, NULL));
    if (WantDataEvent == NULL)
    {
        TRACE_E("Unable to create WantDataEvent event.");
        return FALSE;
    }
    ContinueEvent = HANDLES(CreateEvent(NULL, FALSE, FALSE, NULL));
    if (ContinueEvent == NULL)
    {
        TRACE_E("Unable to create ContinueEvent event.");
        return FALSE;
    }
    RefreshFinishedEvent = HANDLES(CreateEvent(NULL, FALSE, FALSE, NULL));
    if (RefreshFinishedEvent == NULL)
    {
        TRACE_E("Unable to create RefreshFinishedEvent event.");
        return FALSE;
    }
    TerminateEvent = HANDLES(CreateEvent(NULL, FALSE, FALSE, NULL));
    if (TerminateEvent == NULL)
    {
        TRACE_E("Unable to create TerminateEvent event.");
        return FALSE;
    }
    BeginSuspendEvent = HANDLES(CreateEvent(NULL, FALSE, FALSE, NULL));
    if (BeginSuspendEvent == NULL)
    {
        TRACE_E("Unable to create BeginSuspendEvent event.");
        return FALSE;
    }
    EndSuspendEvent = HANDLES(CreateEvent(NULL, FALSE, FALSE, NULL));
    if (EndSuspendEvent == NULL)
    {
        TRACE_E("Unable to create EndSuspendEvent event.");
        return FALSE;
    }
    SharesEvent = HANDLES(CreateEvent(NULL, FALSE, FALSE, NULL));
    if (SharesEvent == NULL)
    {
        TRACE_E("Unable to create SharesEvent event.");
        return FALSE;
    }

    // event that starts the "safe handle killer" thread
    SafeFindCloseStart = HANDLES(CreateEvent(NULL, FALSE, FALSE, NULL));
    if (SafeFindCloseStart == NULL)
    {
        TRACE_E("Unable to create SafeFindCloseStart event.");
        return FALSE;
    }
    SafeFindCloseFinished = HANDLES(CreateEvent(NULL, FALSE, FALSE, NULL));
    if (SafeFindCloseFinished == NULL)
    {
        TRACE_E("Unable to create SafeFindCloseFinished event.");
        return FALSE;
    }

    HANDLES(InitializeCriticalSection(&TimeCounterSection));
    //---  start the snooper thread
    DWORD ThreadID;
    Thread = HANDLES(CreateThread(NULL, 0, ThreadSnooper, NULL, 0, &ThreadID));
    if (Thread == NULL)
    {
        TRACE_E("Unable to start Snooper thread.");
        return FALSE;
    }
    //  SetThreadPriority(Thread, THREAD_PRIORITY_LOWEST);
    WaitForSingleObject(ContinueEvent, INFINITE); // wait until the snooper takes the data

    HANDLES(InitializeCriticalSection(&SafeFindCloseCS));
    //---  start the "safe handle killer" thread
    SafeFindCloseThread = HANDLES(CreateThread(NULL, 0, ThreadFindCloseChangeNotification, NULL, 0, &ThreadID));
    if (SafeFindCloseThread == NULL)
    {
        TRACE_E("Unable to start safe-handle-killer thread.");
        return FALSE;
    }
    // it is necessary to raise the priority so it runs before the main thread
    // the main thread needs the handles closed immediately; on error there is no busy wait, nice
    SetThreadPriority(SafeFindCloseThread, THREAD_PRIORITY_HIGHEST);

    return TRUE;
}

void TerminateThread()
{
    if (Thread != NULL) // shutting down the snooper thread
    {
        SetEvent(TerminateEvent);              // request the snooper to finish
        WaitForSingleObject(Thread, INFINITE); // wait for it to die
        HANDLES(CloseHandle(Thread));          // close the thread handle
    }
    if (DataUsageMutex != NULL)
        HANDLES(CloseHandle(DataUsageMutex));
    if (RefreshFinishedEvent != NULL)
        HANDLES(CloseHandle(RefreshFinishedEvent));
    if (WantDataEvent != NULL)
        HANDLES(CloseHandle(WantDataEvent));
    if (ContinueEvent != NULL)
        HANDLES(CloseHandle(ContinueEvent));
    if (TerminateEvent != NULL)
        HANDLES(CloseHandle(TerminateEvent));
    if (BeginSuspendEvent != NULL)
        HANDLES(CloseHandle(BeginSuspendEvent));
    if (EndSuspendEvent != NULL)
        HANDLES(CloseHandle(EndSuspendEvent));
    if (SharesEvent != NULL)
        HANDLES(CloseHandle(SharesEvent));
    HANDLES(DeleteCriticalSection(&TimeCounterSection));

    if (SafeFindCloseThread != NULL)
    {
        SafeFindCloseTerminate = TRUE; // request the thread to exit
        SetEvent(SafeFindCloseStart);
        if (WaitForSingleObject(SafeFindCloseThread, 1000) == WAIT_TIMEOUT) // wait for it to end
        {
            TerminateThread(SafeFindCloseThread, 666);          // failed to exit, kill it
            WaitForSingleObject(SafeFindCloseThread, INFINITE); // wait until the thread really ends, sometimes it takes quite a while
        }
        HANDLES(CloseHandle(SafeFindCloseThread));
    }
    if (SafeFindCloseStart != NULL)
        HANDLES(CloseHandle(SafeFindCloseStart));
    if (SafeFindCloseFinished != NULL)
        HANDLES(CloseHandle(SafeFindCloseFinished));
    HANDLES(DeleteCriticalSection(&SafeFindCloseCS));
}

void AddDirectory(CFilesWindow* win, const char* path, BOOL registerDevNotification)
{
    CALL_STACK_MESSAGE3("AddDirectory(, %s, %d)", path, registerDevNotification);
    SetEvent(WantDataEvent);                       // request the snooper to release DataUsageMutex
    WaitForSingleObject(DataUsageMutex, INFINITE); // wait for it
    SetEvent(WantDataEvent);                       // snooper can again wait for DataUsageMutex
                                                   //---  now the main thread has the data, snooper waits
    // If the path ends with a space or a dot we must append '\\', otherwise
    // FindFirstChangeNotification trims the space or dot and works with a different path
    char pathCopy[3 * MAX_PATH];
    MakeCopyWithBackslashIfNeeded(path, pathCopy);
    HANDLE h = HANDLES_Q(FindFirstChangeNotification(path, FALSE,
                                                     FILE_NOTIFY_CHANGE_FILE_NAME |
                                                         FILE_NOTIFY_CHANGE_DIR_NAME |
                                                         FILE_NOTIFY_CHANGE_ATTRIBUTES |
                                                         FILE_NOTIFY_CHANGE_SIZE |
                                                         FILE_NOTIFY_CHANGE_LAST_WRITE));
    if (h != INVALID_HANDLE_VALUE)
    {
        win->SetAutomaticRefresh(TRUE);
        WindowArray.Add(win);
        ObjectArray.Add(h);

        if (registerDevNotification)
        {
            // Register the panel window to receive notifications about media changes (removal, etc.)
            DEV_BROADCAST_HANDLE dbh;
            memset(&dbh, 0, sizeof(dbh));
            dbh.dbch_size = sizeof(dbh);
            dbh.dbch_devicetype = DBT_DEVTYP_HANDLE;
            dbh.dbch_handle = h;
            if (win->DeviceNotification != NULL)
            {
                TRACE_E("AddDirectory(): unexpected situation: win->DeviceNotification != NULL");
                UnregisterDeviceNotification(win->DeviceNotification);
            }
            win->DeviceNotification = RegisterDeviceNotificationA(win->HWindow, &dbh, DEVICE_NOTIFY_WINDOW_HANDLE);
        }
    }
    else
    {
        win->SetAutomaticRefresh(FALSE);
        TRACE_W("Unable to receive change notifications for directory '" << path << "' (auto-refresh will not work).");
    }
    //---
    ReleaseMutex(DataUsageMutex);                 // release DataUsageMutex for the snooper
    WaitForSingleObject(ContinueEvent, INFINITE); // wait until the snooper grabs it again
}

// thread that closes handles on a disconnected network device (may wait a long time)
unsigned ThreadFindCloseChangeNotificationBody(void* param)
{
    CALL_STACK_MESSAGE1("ThreadFindCloseChangeNotificationBody()");
    SetThreadNameInVCAndTrace("SafeHandleKiller");
    //  TRACE_I("Begin");

    while (!SafeFindCloseTerminate)
    {
        WaitForSingleObject(SafeFindCloseStart, INFINITE); // wait for start or termination

        while (1)
        {
            // retrieve a handle
            HANDLES(EnterCriticalSection(&SafeFindCloseCS));
            HANDLE h;
            BOOL br = FALSE;

            if (SafeFindCloseCNArr.IsGood() && SafeFindCloseCNArr.Count > 0)
            {
                h = SafeFindCloseCNArr[SafeFindCloseCNArr.Count - 1];
                SafeFindCloseCNArr.Delete(SafeFindCloseCNArr.Count - 1);
                if (!SafeFindCloseCNArr.IsGood())
                    SafeFindCloseCNArr.ResetState(); // cannot fail; only reports insufficient memory when shrinking the array
            }
            else
                br = TRUE;
            HANDLES(LeaveCriticalSection(&SafeFindCloseCS));

            if (br)
                break; // nothing left to close; wait for the next start

            // close the handle
            //      TRACE_I("Killing ... " << h);
            HANDLES(FindCloseChangeNotification(h));
        }

        SetEvent(SafeFindCloseFinished); // let the main thread continue...
    }
    //  TRACE_I("End");
    return 0;
}

unsigned ThreadFindCloseChangeNotificationEH(void* param)
{
#ifndef CALLSTK_DISABLE
    __try
    {
#endif // CALLSTK_DISABLE
        return ThreadFindCloseChangeNotificationBody(param);
#ifndef CALLSTK_DISABLE
    }
    __except (CCallStack::HandleException(GetExceptionInformation()))
    {
        TRACE_I("Safe Handle Killer: calling ExitProcess(1).");
        //    ExitProcess(1);
        TerminateProcess(GetCurrentProcess(), 1); // a harder exit (this still performs additional calls)
        return 1;
    }
#endif // CALLSTK_DISABLE
}

DWORD WINAPI ThreadFindCloseChangeNotification(void* param)
{
#ifndef CALLSTK_DISABLE
    CCallStack stack;
#endif // CALLSTK_DISABLE
    return ThreadFindCloseChangeNotificationEH(param);
}

void ChangeDirectory(CFilesWindow* win, const char* newPath, BOOL registerDevNotification)
{
    CALL_STACK_MESSAGE3("ChangeDirectory(, %s, %d)", newPath, registerDevNotification);
    SetEvent(WantDataEvent);                       // ask the snooper to release DataUsageMutex
    WaitForSingleObject(DataUsageMutex, INFINITE); // wait for it
    SetEvent(WantDataEvent);                       // snooper may start waiting for the mutex again
    BOOL registerDevNot = FALSE;
    HANDLE registerDevNotHandle = NULL;
    //---  now the main thread owns the data, the snooper waits
    if (win->DeviceNotification != NULL)
    {
        UnregisterDeviceNotification(win->DeviceNotification);
        win->DeviceNotification = NULL;
    }

    int i;
    for (i = 0; i < WindowArray.Count; i++)
        if (win == WindowArray[i])
        {
            // if the change notification is on a disconnected network drive
            // we can't wait here... let another thread close it
            HANDLES(EnterCriticalSection(&SafeFindCloseCS));
            SafeFindCloseCNArr.Add(ObjectArray[i]);
            if (!SafeFindCloseCNArr.IsGood())
                SafeFindCloseCNArr.ResetState(); // ignore errors when shrinking the array
            HANDLES(LeaveCriticalSection(&SafeFindCloseCS));
            ResetEvent(SafeFindCloseFinished);               // we will wait for the cleanup thread to start
            SetEvent(SafeFindCloseStart);                    // start the cleanup
            WaitForSingleObject(SafeFindCloseFinished, 200); // 200 ms timeout for closing the handle

            // If the path ends with a space or a dot we must append '\\', otherwise
            // FindFirstChangeNotification trims the space or dot and uses a different path
            char newPathCopy[3 * MAX_PATH];
            MakeCopyWithBackslashIfNeeded(newPath, newPathCopy);
            ObjectArray[i] = HANDLES_Q(FindFirstChangeNotification(newPath, FALSE,
                                                                   FILE_NOTIFY_CHANGE_FILE_NAME |
                                                                       FILE_NOTIFY_CHANGE_DIR_NAME |
                                                                       FILE_NOTIFY_CHANGE_ATTRIBUTES |
                                                                       FILE_NOTIFY_CHANGE_SIZE |
                                                                       FILE_NOTIFY_CHANGE_LAST_WRITE));
            if ((HANDLE)ObjectArray[i] == INVALID_HANDLE_VALUE)
            {
                win->SetAutomaticRefresh(FALSE);
                ObjectArray.Delete(i); // remove it from the list
                WindowArray.Delete(i);
                TRACE_W("Unable to receive change notifications for directory '" << newPath << "' (auto-refresh will not work).");
            }
            else
            {
                if (registerDevNotification)
                {
                    registerDevNot = TRUE;
                    registerDevNotHandle = (HANDLE)ObjectArray[i];
                }
            }
            break;
        }
    //--- not found -> add it
    if (i == WindowArray.Count)
    {
        // If the path ends with a space or a dot we must append '\\', otherwise
        // FindFirstChangeNotification trims the space or dot and uses a different path
        char newPathCopy[3 * MAX_PATH];
        MakeCopyWithBackslashIfNeeded(newPath, newPathCopy);
        HANDLE h = HANDLES_Q(FindFirstChangeNotification(newPath, FALSE,
                                                         FILE_NOTIFY_CHANGE_FILE_NAME |
                                                             FILE_NOTIFY_CHANGE_DIR_NAME |
                                                             FILE_NOTIFY_CHANGE_ATTRIBUTES |
                                                             FILE_NOTIFY_CHANGE_SIZE |
                                                             FILE_NOTIFY_CHANGE_LAST_WRITE));
        if (h != INVALID_HANDLE_VALUE)
        {
            win->SetAutomaticRefresh(TRUE);
            WindowArray.Add(win);
            ObjectArray.Add(h);
            if (registerDevNotification)
            {
                registerDevNot = TRUE;
                registerDevNotHandle = h;
            }
        }
        else
        {
            win->SetAutomaticRefresh(FALSE);
            TRACE_W("Unable to receive change notifications for directory '" << newPath << "' (auto-refresh will not work).");
        }
    }
    if (registerDevNot)
    {
        // register the panel window to receive media change notifications (removal, etc.)
        DEV_BROADCAST_HANDLE dbh;
        memset(&dbh, 0, sizeof(dbh));
        dbh.dbch_size = sizeof(dbh);
        dbh.dbch_devicetype = DBT_DEVTYP_HANDLE;
        dbh.dbch_handle = registerDevNotHandle;
        win->DeviceNotification = RegisterDeviceNotificationA(win->HWindow, &dbh, DEVICE_NOTIFY_WINDOW_HANDLE);
    }
    //---
    ReleaseMutex(DataUsageMutex);                 // release DataUsageMutex for the snooper
    WaitForSingleObject(ContinueEvent, INFINITE); // wait until the snooper grabs it again
}

void DetachDirectory(CFilesWindow* win, BOOL waitForHandleClosure, BOOL closeDevNotifification)
{
    CALL_STACK_MESSAGE3("DetachDirectory(, %d, %d)", waitForHandleClosure, closeDevNotifification);
    SetEvent(WantDataEvent);                       // request the snooper to release DataUsageMutex
    WaitForSingleObject(DataUsageMutex, INFINITE); // wait for it
    SetEvent(WantDataEvent);                       // snooper may start waiting for the mutex again
                                                   //---  now the main thread owns the data; the snooper waits
    if (closeDevNotifification && win->DeviceNotification != NULL)
    {
        UnregisterDeviceNotification(win->DeviceNotification);
        win->DeviceNotification = NULL;
    }

    int i;
    for (i = 0; i < WindowArray.Count; i++)
        if (win == WindowArray[i])
        {
            // if the change notification is on a disconnected network drive
            // we can't afford to wait... let another thread close it
            HANDLES(EnterCriticalSection(&SafeFindCloseCS));
            SafeFindCloseCNArr.Add(ObjectArray[i]);
            if (!SafeFindCloseCNArr.IsGood())
                SafeFindCloseCNArr.ResetState(); // ignore errors when shrinking the array
            HANDLES(LeaveCriticalSection(&SafeFindCloseCS));
            ResetEvent(SafeFindCloseFinished);                                             // we'll wait for the cleanup to start...
            SetEvent(SafeFindCloseStart);                                                  // start the cleanup
            WaitForSingleObject(SafeFindCloseFinished, waitForHandleClosure ? 5000 : 200); // 200 ms timeout for closing the handle

            ObjectArray.Delete(i); // remove it from the list
            WindowArray.Delete(i);
            win->SetAutomaticRefresh(FALSE);
        }
    //---
    ReleaseMutex(DataUsageMutex);                 // release DataUsageMutex for the snooper
    WaitForSingleObject(ContinueEvent, INFINITE); // wait until the snooper grabs it again
}

/*
#define SUSPMODESTACKSIZE 50

class CSuspModeStack
{
  protected:
    DWORD CallerCalledFromArr[SUSPMODESTACKSIZE];  // array of return addresses of functions where BeginSuspendMode() was called
    DWORD CalledFromArr[SUSPMODESTACKSIZE];        // array of addresses from which BeginSuspendMode() was invoked
    int Count;                                     // number of elements in the two previous arrays
    int Ignored;                                   // count of BeginSuspendMode() calls we had to ignore (SUSPMODESTACKSIZE too small -> increase if needed)

  public:
    CSuspModeStack() {Count = 0; Ignored = 0;}
    ~CSuspModeStack() {CheckIfEmpty(1);}  // one BeginSuspendMode() is fine: called when the main Salamander window is deactivated (before closing)

    void Push(DWORD caller_called_from, DWORD called_from);
    void Pop(DWORD caller_called_from, DWORD called_from);
    void CheckIfEmpty(int checkLevel);
};

void
CSuspModeStack::Push(DWORD caller_called_from, DWORD called_from)
{
  if (Count < SUSPMODESTACKSIZE)
  {
    CallerCalledFromArr[Count] = caller_called_from;
    CalledFromArr[Count] = called_from;
    Count++;
  }
  else
  {
    Ignored++;
    TRACE_E("CSuspModeStack::Push(): you should increase SUSPMODESTACKSIZE! ignored=" << Ignored);
  }
}

void
CSuspModeStack::Pop(DWORD caller_called_from, DWORD called_from)
{
  if (Ignored == 0)
  {
    if (Count > 0)
    {
      Count--;
      if (CallerCalledFromArr[Count] != caller_called_from)
      {
        TRACE_E("CSuspModeStack::Pop(): strange situation: BeginCallerCalledFrom!=StopCallerCalledFrom - BeginCalledFrom,StopCalledFrom");
        TRACE_E("CSuspModeStack::Pop(): strange situation: 0x" << std::hex <<
                CallerCalledFromArr[Count] << "!=0x" << caller_called_from << " - 0x" <<
                CalledFromArr[Count] << ",0x" << called_from << std::dec);
      }
    }
    else TRACE_E("CSuspModeStack::Pop(): unexpected call!");
  }
  else Ignored--;
}

void
CSuspModeStack::CheckIfEmpty(int checkLevel)
{
  if (Count > checkLevel)
  {
    TRACE_E("CSuspModeStack::CheckIfEmpty(" << checkLevel << "): listing remaining BeginSuspendMode calls: CallerCalledFrom,CalledFrom");
    int i;
    for (i = 0; i < Count; i++)
    {
      TRACE_E("CSuspModeStack::CheckIfEmpty():: 0x" << std::hex <<
              CallerCalledFromArr[i] << ",0x" << CalledFromArr[i] << std::dec);
    }
  }
}

CSuspModeStack SuspModeStack;
*/

void BeginSuspendMode(BOOL debugDoNotTestCaller)
{
    /*
#ifdef _DEBUG     // check whether BeginSuspendMode() and EndSuspendMode() are called from the same function
                   // (based on the caller's return address, so it does not detect a "bug" when different functions are both called from the same place)
  DWORD *register_ebp;
  __asm mov register_ebp, ebp
  DWORD called_from, caller_called_from;
  __try
  {
    called_from = *(DWORD*)((char*)register_ebp + 4);

// if this code ever needs to be revived, note it can be replaced (x86 and x64):
    called_from = *(DWORD_PTR *)_AddressOfReturnAddress();

    caller_called_from = *(DWORD*)((char*)(*register_ebp) + 4);
  }
  __except (EXCEPTION_EXECUTE_HANDLER)
  {
    called_from = -1;
    caller_called_from = -1;
  }
  SuspModeStack.Push(debugDoNotTestCaller ? 0 : caller_called_from, called_from);
#endif // _DEBUG
*/

    if (SnooperSuspended == 0)
    {
        SetEvent(BeginSuspendEvent);
        WaitForSingleObject(ContinueEvent, INFINITE);
    }
    SnooperSuspended++;
}

//#ifdef _DEBUG
//void EndSuspendModeBody()
//#else // _DEBUG
void EndSuspendMode(BOOL debugDoNotTestCaller)
//#endif // _DEBUG
{
    CALL_STACK_MESSAGE1("EndSuspendMode()");

    if (SnooperSuspended < 1)
    {
        TRACE_E("Incorrect call to EndSuspendMode()");
        SnooperSuspended = 0; // is someone misusing CM_LEFTREFRESH, CM_RIGHTREFRESH, or CM_ACTIVEREFRESH again?
    }
    else
    {
        if (SnooperSuspended == 1)
        {
            SetEvent(EndSuspendEvent);
            WaitForSingleObject(ContinueEvent, INFINITE);
        }
        SnooperSuspended--;
    }
}

/*
#ifdef _DEBUG     // check whether BeginSuspendMode() and EndSuspendMode() are called from the same function
                   // (based on the caller's return address, so it does not detect a "bug" when different functions are both called from the same place)
void EndSuspendMode(BOOL debugDoNotTestCaller)
{
  DWORD *register_ebp;
  __asm mov register_ebp, ebp
  DWORD called_from, caller_called_from;
  __try
  {
    called_from = *(DWORD*)((char*)register_ebp + 4);

// if this code ever needs to be revived, note it can be replaced (x86 and x64):
    called_from = *(DWORD_PTR *)_AddressOfReturnAddress();

    caller_called_from = *(DWORD*)((char*)(*register_ebp) + 4);
  }
  __except (EXCEPTION_EXECUTE_HANDLER)
  {
    called_from = -1;
    caller_called_from = -1;
  }
  SuspModeStack.Pop(debugDoNotTestCaller ? 0 : caller_called_from, called_from);

  EndSuspendModeBody();
}
#endif // _DEBUG
*/
