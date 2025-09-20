// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"

#include "mainwnd.h"
#include "tasklist.h"
#include "plugins.h"
extern "C"
{
#include "shexreg.h"
}
#include "salshlib.h"

#pragma warning(disable : 4074)
#pragma init_seg(compiler) // perform initialization as early as possible

#define NOHANDLES(function) function // prevents littering code with HANDLES macros via CheckHnd

CTaskList TaskList;

BOOL FirstInstance_3_or_later = FALSE;

// the process list is shared among all Salamanders in the local session
// from AS 3.0 the "Break" event triggers an exception in the target so we get a full bug report, but the target ends
// therefore the following constants were renamed from "AltapSalamander*" to
// "AltapSalamander3*" so we are separated from older versions

// NOTE: changing them requires updating salbreak.exe, let me know... Petr

const char* AS_PROCESSLIST_NAME = "AltapSalamander3bProcessList";                               // shared memory for CProcessList
const char* AS_PROCESSLIST_MUTEX_NAME = "AltapSalamander3bProcessListMutex";                    // synchronization for access to shared memory
const char* AS_PROCESSLIST_EVENT_NAME = "AltapSalamander3bProcessListEvent";                    // event trigger (what to do is stored in shared memory)
const char* AS_PROCESSLIST_EVENT_PROCESSED_NAME = "AltapSalamander3bProcessListEventProcessed"; // fired event was processed

const char* FIRST_SALAMANDER_MUTEX_NAME = "AltapSalamanderFirstInstance";     // introduced in AS 2.52 beta 1
const char* LOADSAVE_REGISTRY_MUTEX_NAME = "AltapSalamanderLoadSaveRegistry"; // introduced in AS 2.52 beta 1

// path where we store the bug report and minidump; later salamander packs them into a 7z and uploads them
char BugReportPath[MAX_PATH] = "";

CRITICAL_SECTION CommandLineParamsCS;
CCommandLineParams CommandLineParams;
HANDLE CommandLineParamsProcessed;

// handle of the main window (it's bad to access MainWindow from the control thread, it might become NULL)
HWND HSafeMainWindow = NULL;

void RaiseBreakException()
{
#ifndef CALLSTK_DISABLE
    CCallStack stack;
#endif                                                   // CALLSTK_DISABLE
    RaiseException(OPENSAL_EXCEPTION_BREAK, 0, 0, NULL); // our own "break" exception
                                                         // execution never returns here
}

//
// ****************************************************************************
// CTaskList
//

DWORD WINAPI FControlThread(void* param)
{
    // this thread isn't invoked with our CCallStack; when a handle leaked and I
    // tried to dump it during Salamander shutdown, Salam crashed

    CTaskList* tasklist = (CTaskList*)param;

    SetThreadNameInVC("ControlThread");

    HANDLE arr[3];
    arr[0] = tasklist->TerminateEvent;
    arr[1] = tasklist->Event;
    arr[2] = SalShExtDoPasteEvent;

    DWORD lastTodoUID = 0;

    DWORD ourPID = GetCurrentProcessId();

    BOOL loop = TRUE;
    while (loop)
    {
        DWORD waitRet = WaitForMultipleObjects(arr[2] == NULL ? 2 : 3, arr, FALSE, INFINITE);
        switch (waitRet)
        {
        case WAIT_OBJECT_0 + 0: // tasklist->TerminateEvent
        {
            loop = FALSE;
            break;
        }

        case WAIT_OBJECT_0 + 1: // tasklist->Event
        {
            // acquire the ProcessList
            waitRet = WaitForSingleObject(tasklist->FMOMutex, TASKLIST_TODO_TIMEOUT);
            if (waitRet == WAIT_FAILED)
                Sleep(50); // to avoid eating CPU
            if (waitRet == WAIT_FAILED || waitRet == WAIT_TIMEOUT)
                break;

            // protection against looping after executing a command
            if (tasklist->ProcessList->TodoUID <= lastTodoUID)
            {
                // release the ProcessList
                ReleaseMutex(tasklist->FMOMutex);
                Sleep(50); // give other processes a chance
                break;
            }
            else
                lastTodoUID = tasklist->ProcessList->TodoUID;

            // we have locked the ProcessList
            DWORD pid = tasklist->ProcessList->PID;
            if (pid != ourPID) // if the event is not for us
            {
                // release the ProcessList
                ReleaseMutex(tasklist->FMOMutex);
                Sleep(50); // give other processes a chance
                break;
            }

            // we are already running in the process that should receive the message;
            // because this is a side thread any communication with the main thread
            // requires additional synchronization

            // reset the Event because we know it belonged to us and there's no need
            // to keep other control threads running
            ResetEvent(tasklist->Event);

            // check the timestamp to see whether we already missed the window for processing the command
            DWORD tickCount = GetTickCount();
            if (tickCount - tasklist->ProcessList->TodoTimestamp >= TASKLIST_TODO_TIMEOUT)
            {
                // TIMEOUT
                // release the ProcessList
                ReleaseMutex(tasklist->FMOMutex);
                break;
            }

            // make a copy of the locked ProcessList
            CProcessList processList;
            memcpy(&processList, tasklist->ProcessList, sizeof(CProcessList));
            // and release the shared memory
            ReleaseMutex(tasklist->FMOMutex);

            switch (processList.Todo)
            {
            case TASKLIST_TODO_HIGHLIGHT:
            {
                SetEvent(tasklist->EventProcessed); // notification for the requesting process: done
                if (HSafeMainWindow != NULL)
                    PostMessage(HSafeMainWindow, WM_USER_FLASHWINDOW, 0, 0);
                break;
            }

            case TASKLIST_TODO_BREAK:
            {
                SetEvent(tasklist->EventProcessed); // notification for the requesting process: done

                RaiseBreakException();
                // execution never returns here

                break;
            }

            case TASKLIST_TODO_TERMINATE:
            {
                SetEvent(tasklist->EventProcessed); // notification for the requesting process: done

                HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
                if (h != NULL)
                {
                    TerminateProcess(h, 666);
                    CloseHandle(h);
                }
                break;
            }

            case TASKLIST_TODO_ACTIVATE:
            {
                // copy ProcessList to the global CommandLineParams variable,
                // which the main thread monitors on entering from idle
                NOHANDLES(EnterCriticalSection(&CommandLineParamsCS));
                memcpy(&CommandLineParams, &processList.CommandLineParams, sizeof(CCommandLineParams));
                ResetEvent(CommandLineParamsProcessed);
                NOHANDLES(LeaveCriticalSection(&CommandLineParamsCS));

                // if the main thread is idle, poke it and force it to check CommandLineParams::RequestUID
                // if it isn't idle, it will process the message once it enters IDLE (if we wait long enough)
                if (HSafeMainWindow != NULL)
                    PostMessage(HSafeMainWindow, WM_USER_WAKEUP_FROM_IDLE, 0, 0);

                // wait 5 seconds for the main thread to respond (do not enter the critical section yet so it can)
                WaitForSingleObject(CommandLineParamsProcessed, TASKLIST_TODO_TIMEOUT);

                // we can now enter the critical section
                NOHANDLES(EnterCriticalSection(&CommandLineParamsCS));
                CommandLineParams.RequestUID = 0;                             // prevent the main thread from taking further actions
                waitRet = WaitForSingleObject(CommandLineParamsProcessed, 0); // query the current state of the event
                if (waitRet == WAIT_OBJECT_0)
                    SetEvent(tasklist->EventProcessed); // notification for the requesting process: done
                NOHANDLES(LeaveCriticalSection(&CommandLineParamsCS));
                break;
            }

            default:
            {
                TRACE_E("FControlThread: unknown todo=" << processList.Todo);
                break;
            }
            }
            break;
        }

        case WAIT_OBJECT_0 + 2: // SalShExtDoPasteEvent
        {
            BOOL sleep = TRUE;
            if (SalShExtSharedMemMutex != NULL)
            {
                WaitForSingleObject(SalShExtSharedMemMutex, INFINITE);
                if (HSafeMainWindow != NULL && SalShExtSharedMemView != NULL &&
                    SalShExtSharedMemView->SalamanderMainWnd == (UINT64)(DWORD_PTR)HSafeMainWindow)
                {
                    ResetEvent(SalShExtDoPasteEvent); // the "source" Salamander was found; further searching is pointless
                    sleep = FALSE;
                    PostMessage(HSafeMainWindow, WM_USER_SALSHEXT_PASTE, SalShExtSharedMemView->PostMsgIndex, 0);
                }
                ReleaseMutex(SalShExtSharedMemMutex);
            }
            if (sleep)
                Sleep(50); // give other Salamander processes a chance
            break;
        }

        default: // this should not happen
        {
            Sleep(50); // to avoid eating CPU
            break;
        }
        }
    }

    return 0;
}

CTaskList::CTaskList()
{
    // runs in the 'compiler' group, i.e. before ms_init
    OK = FALSE;
    FMO = NULL;
    ProcessList = NULL;
    FMOMutex = NULL;
    Event = NULL;
    EventProcessed = NULL;
    TerminateEvent = NULL;
    ControlThread = NULL;
    // internal synchronization between ControlThread and the main thread
    NOHANDLES(InitializeCriticalSection(&CommandLineParamsCS));
    CommandLineParamsProcessed = NULL;
}

BOOL CTaskList::Init()
{
    OK = FALSE;

    PSID psidEveryone;
    PACL paclNewDacl;
    SECURITY_ATTRIBUTES sa;
    SECURITY_DESCRIPTOR sd;
    SECURITY_ATTRIBUTES* saPtr = CreateAccessableSecurityAttributes(&sa, &sd, GENERIC_ALL, &psidEveryone, &paclNewDacl);

    //---  a small side job: under Vista+ create an event for communication with the copy-hook (control-thread waits for it)
    if (WindowsVistaAndLater)
    {
        SalShExtDoPasteEvent = NOHANDLES(CreateEvent(saPtr, TRUE, FALSE, SALSHEXT_DOPASTEEVENTNAME));
        if (SalShExtDoPasteEvent == NULL)
            SalShExtDoPasteEvent = NOHANDLES(OpenEvent(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, SALSHEXT_DOPASTEEVENTNAME));
        if (SalShExtDoPasteEvent == NULL)
            TRACE_E("CTaskList::Init(): unable to create event object for communicating with copy-hook shell extension!");
    }

    //---  try to connect to the FMO mutex - also tests whether another Salamander is already running
    FMOMutex = NOHANDLES(OpenMutex(SYNCHRONIZE, FALSE, AS_PROCESSLIST_MUTEX_NAME));
    if (FMOMutex == NULL) // we are the first Salamander 3.0 or later in the local session
    {
        //---  create communication objects and acquire the FMO
        FMOMutex = NOHANDLES(CreateMutex(saPtr, TRUE, AS_PROCESSLIST_MUTEX_NAME)); // task list is valid only for this session; the mutex belongs to the local namespace
        if (FMOMutex == NULL)
            return FALSE; // fail
        FMO = NOHANDLES(CreateFileMapping(INVALID_HANDLE_VALUE, saPtr, PAGE_READWRITE | SEC_COMMIT,
                                          0, sizeof(CProcessList), AS_PROCESSLIST_NAME));
        if (FMO == NULL)
            return FALSE; // fail
        ProcessList = (CProcessList*)NOHANDLES(MapViewOfFile(FMO, FILE_MAP_WRITE, 0, 0, 0));
        if (ProcessList == NULL)
            return FALSE; // fail
        Event = NOHANDLES(CreateEvent(saPtr, TRUE, FALSE, AS_PROCESSLIST_EVENT_NAME));
        if (Event == NULL)
            return FALSE; // fail
        EventProcessed = NOHANDLES(CreateEvent(saPtr, TRUE, FALSE, AS_PROCESSLIST_EVENT_PROCESSED_NAME));
        if (EventProcessed == NULL)
            return FALSE; // fail

        //---  initialize shared memory
        ZeroMemory(ProcessList, sizeof(CProcessList));
        ProcessList->Version = 1; // 3.0 beta 4

        ProcessList->ItemsCount = 1;
        ProcessList->ItemsStateUID++;
        ProcessList->Items[0] = CProcessListItem();

        //---  release the FMO
        ReleaseMutex(FMOMutex);
    }
    else // another instance, just attach ...
    {
        //---  acquire the FMO
        DWORD waitRet = WaitForSingleObject(FMOMutex, TASKLIST_TODO_TIMEOUT);
        if (waitRet == WAIT_TIMEOUT)
            return FALSE; // fail

        //---  connect to the remaining system objects used for communication
        FMO = NOHANDLES(OpenFileMapping(FILE_MAP_WRITE, FALSE, AS_PROCESSLIST_NAME));
        if (FMO == NULL)
            return FALSE; // fail
        ProcessList = (CProcessList*)NOHANDLES(MapViewOfFile(FMO, FILE_MAP_WRITE, 0, 0, 0));
        if (ProcessList == NULL)
            return FALSE; // fail
        // SetEvent() requires EVENT_MODIFY_STATE and Wait* needs SYNCHRONIZE
        Event = NOHANDLES(OpenEvent(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, AS_PROCESSLIST_EVENT_NAME));
        if (Event == NULL)
            return FALSE; // fail
        EventProcessed = NOHANDLES(OpenEvent(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, AS_PROCESSLIST_EVENT_PROCESSED_NAME));
        if (EventProcessed == NULL)
            return FALSE; // fail

        //---  add a record to the shared memory
        BOOL attempt = 0;
    AGAIN:
        int c = ProcessList->ItemsCount;
        if (c < MAX_TL_ITEMS) // if there aren't too many, add this process
        {
            ProcessList->ItemsCount++;
            ProcessList->ItemsStateUID++;
            ProcessList->Items[c] = CProcessListItem();
        }
        else
        {
            if (attempt == 0)
            {
                // the array is full, try to shake it (some process might have died without notifying us)
                RemoveKilledItems(NULL);
                attempt++;
                goto AGAIN;
            }
        }

        //---  release the FMO
        ReleaseMutex(FMOMutex);
    }

    // detect other Salamander instances
    LPTSTR sid = NULL;
    if (!GetStringSid(&sid))
        sid = NULL;
    char mutexName[1000];
    if (sid == NULL)
    {
        // failed to obtain SID -- local namespace without the SID attached
        _snprintf_s(mutexName, _TRUNCATE, "%s", FIRST_SALAMANDER_MUTEX_NAME);
    }
    else
    {
        _snprintf_s(mutexName, _TRUNCATE, "Global\\%s_%s", FIRST_SALAMANDER_MUTEX_NAME, sid);
        LocalFree(sid);
    }
    HANDLE hMutex = NOHANDLES(CreateMutex(saPtr, FALSE, mutexName));
    DWORD lastError = GetLastError();
    if (hMutex != NULL)
    {
        FirstInstance_3_or_later = (lastError != ERROR_ALREADY_EXISTS);
    }
    else
    {
        hMutex = NOHANDLES(OpenMutex(SYNCHRONIZE, FALSE, mutexName));
        lastError = GetLastError();
        if (hMutex != NULL)
            FirstInstance_3_or_later = FALSE;
    }

    if (psidEveryone != NULL)
        FreeSid(psidEveryone);
    if (paclNewDacl != NULL)
        LocalFree(paclNewDacl);

    TerminateEvent = NOHANDLES(CreateEvent(NULL, TRUE, FALSE, NULL));
    if (TerminateEvent == NULL)
        return FALSE; // fail

    // internal synchronization between ControlThread and the main thread
    CommandLineParamsProcessed = CreateEvent(NULL, TRUE, FALSE, NULL); // manual, nonsignaled
    if (CommandLineParamsProcessed == NULL)
        return FALSE; // failed

    // we cannot use _beginthreadex because the library might not be initialized yet
    DWORD id;
    ControlThread = NOHANDLES(CreateThread(NULL, 0, FControlThread, this, 0, &id));
    if (ControlThread == NULL)
        return FALSE; // fail
    // this thread must get CPU time even if resources are scarce
    SetThreadPriority(ControlThread, THREAD_PRIORITY_TIME_CRITICAL);

    OK = TRUE;
    return TRUE;
}

CTaskList::~CTaskList()
{
    if (ControlThread != NULL)
    {
        SetEvent(TerminateEvent);                     // terminate!
        WaitForSingleObject(ControlThread, INFINITE); // wait until the thread finishes
        NOHANDLES(CloseHandle(ControlThread));
    }
    if (TerminateEvent != NULL)
        NOHANDLES(CloseHandle(TerminateEvent));

    // remove ourselves from the list
    if (OK)
    {
        //---  acquire the FMO
        if (WaitForSingleObject(FMOMutex, TASKLIST_TODO_TIMEOUT) != WAIT_TIMEOUT)
        {
            CProcessListItem* ptr = ProcessList->Items;
            int c = ProcessList->ItemsCount;

            //---  remove the current process, it is terminating ...
            DWORD PID = GetCurrentProcessId();
            int i;
            for (i = 0; i < c; i++)
            {
                if (PID == ptr[i].PID)
                {
                    //---  kick the process from the list
                    memmove(ptr + i, ptr + i + 1, (c - i - 1) * sizeof(CProcessListItem));
                    c--;
                    i--;
                }
            }
            ProcessList->ItemsCount = c;
            ProcessList->ItemsStateUID++;

            //---  release the FMO
            ReleaseMutex(FMOMutex);
        }
    }

    if (ProcessList != NULL)
        NOHANDLES(UnmapViewOfFile(ProcessList));
    if (FMO != NULL)
        NOHANDLES(CloseHandle(FMO));
    if (FMOMutex != NULL)
        NOHANDLES(CloseHandle(FMOMutex));
    if (Event != NULL)
        NOHANDLES(CloseHandle(Event));
    if (EventProcessed != NULL)
        NOHANDLES(CloseHandle(EventProcessed));
    if (CommandLineParamsProcessed != NULL)
        NOHANDLES(CloseHandle(CommandLineParamsProcessed));
    NOHANDLES(DeleteCriticalSection(&CommandLineParamsCS));

    if (SalShExtDoPasteEvent != NULL)
        NOHANDLES(CloseHandle(SalShExtDoPasteEvent));
    SalShExtDoPasteEvent = NULL;
}

BOOL CTaskList::SetProcessState(DWORD processState, HWND hMainWindow, BOOL* timeouted)
{
    if (timeouted != NULL)
        *timeouted = FALSE;

    HSafeMainWindow = hMainWindow;

    if (OK)
    {
        DWORD ret = WaitForSingleObject(FMOMutex, TASKLIST_TODO_TIMEOUT);
        if (ret != WAIT_FAILED && ret != WAIT_TIMEOUT)
        {
            // locate ourselves in the process list and set processState and hMainWindow
            CProcessListItem* ptr = ProcessList->Items;
            int c = ProcessList->ItemsCount;
            DWORD PID = GetCurrentProcessId();
            int i;
            for (i = 0; i < c; i++)
            {
                if (PID == ptr[i].PID)
                {
                    ptr[i].ProcessState = processState;
                    ptr[i].HMainWindow = (UINT64)(DWORD_PTR)hMainWindow; // 64b for x64/x86 compatibility
                    break;
                }
            }
            ReleaseMutex(FMOMutex);
            return TRUE;
        }
        else
        {
            if (timeouted != NULL)
                *timeouted = TRUE;
            TRACE_E("SetProcessState(): WaitForSingleObject failed!");
        }
    }
    return FALSE;
}

int CTaskList::GetItems(CProcessListItem* items, DWORD* itemsStateUID, BOOL* timeouted)
{
    if (timeouted != NULL)
        *timeouted = FALSE;
    if (OK)
    {
        BOOL changed = FALSE;
        //---  acquire the FMO
        if (WaitForSingleObject(FMOMutex, TASKLIST_TODO_TIMEOUT) == WAIT_TIMEOUT)
        {
            if (timeouted != NULL)
                *timeouted = TRUE;
            return 0; // fail
        }

        CProcessListItem* ptr = ProcessList->Items;

        //---  remove killed processes
        RemoveKilledItems(&changed);

        //---  return values
        if (items != NULL)
            memcpy(items, ptr, ProcessList->ItemsCount * sizeof(CProcessListItem));
        if (changed)
            ProcessList->ItemsStateUID++;
        if (itemsStateUID != NULL)
            *itemsStateUID = ProcessList->ItemsStateUID;

        int count = ProcessList->ItemsCount;
        //---  release the FMO
        ReleaseMutex(FMOMutex);
        return count;
    }
    else
        return 0;
}

BOOL CTaskList::FireEvent(DWORD todo, DWORD pid, BOOL* timeouted)
{
    if (timeouted != NULL)
        *timeouted = FALSE;
    if (OK)
    {
        // lock the ProcessList
        DWORD waitRet = WaitForSingleObject(FMOMutex, 2000);
        if (waitRet == WAIT_FAILED)
            return FALSE;
        if (waitRet == WAIT_TIMEOUT)
        {
            if (timeouted != NULL)
                *timeouted = TRUE;
            return FALSE; // fail
        }

        // set the passed parameters
        ProcessList->Todo = todo;
        ProcessList->TodoUID++;
        ProcessList->TodoTimestamp = GetTickCount();
        ProcessList->PID = pid;

        // when breaking another Salamander instance allow its Salmon to take focus over us
        if (todo == TASKLIST_TODO_BREAK)
        {
            for (DWORD i = 0; i < ProcessList->ItemsCount; i++)
            {
                if (ProcessList->Items[i].PID == pid)
                {
                    AllowSetForegroundWindow(ProcessList->Items[i].PID);       // better allow our own Salamander too, although it's probably unnecessary...
                    AllowSetForegroundWindow(ProcessList->Items[i].SalmonPID); // definitely let its Salmon rise above us
                    break;
                }
            }
        }

        // release ProcessList
        ReleaseMutex(FMOMutex);

        // start the check in all Salamanders
        ResetEvent(EventProcessed);
        SetEvent(Event);

        //---  give them a moment to react (during this period some process should "catch" it and complete the task)
        BOOL ret = (WaitForSingleObject(EventProcessed, 1000) == WAIT_OBJECT_0);

        //---  tell all Salamanders to get ready for the next command
        ResetEvent(Event);

        //---  restore break-PID
        //    ProcessList->Todo = 0;
        //    ProcessList->PID = 0;

        //---  release the FMO

        return ret;
    }
    return FALSE;
}

BOOL CTaskList::ActivateRunningInstance(const CCommandLineParams* cmdLineParams, BOOL* timeouted)
{
    if (timeouted != NULL)
        *timeouted = FALSE;

    if (!OK)
        return FALSE;

    CProcessListItem ourProcessInfo;

    // locate a running process in our group, or a starting one (wait a bit to see if it initializes)
    int firstStarting = -1; // index of a process from our group (same Integrity Level and SID) that doesn't have its main window yet
    int firstRunnig = -1;   // index of a process from our group (same Integrity Level and SID) already running with its main window
    DWORD timeStamp = GetTickCount();
    do
    {
        firstStarting = -1;
        firstRunnig = -1;
        DWORD ret = WaitForSingleObject(FMOMutex, 200);
        if (ret == WAIT_FAILED)
            return FALSE;
        if (ret != WAIT_TIMEOUT) // we obtained the mutex
        {
            int i;
            for (i = 0; i < (int)ProcessList->ItemsCount; i++)
            {
                CProcessListItem* item = &ProcessList->Items[i];
                // only consider processes from our group (same Integrity Level and SID)
                if (item->PID != ourProcessInfo.PID &&
                    item->IntegrityLevel == ourProcessInfo.IntegrityLevel &&
                    memcmp(item->SID_MD5, ourProcessInfo.SID_MD5, 16) == 0)
                {
                    if (item->ProcessState == PROCESS_STATE_RUNNING)
                    {
                        firstRunnig = i;
                        break; // once we find a running instance no need to search starting ones
                    }
                    if (item->ProcessState == PROCESS_STATE_STARTING && firstStarting == -1)
                        firstStarting = i;
                }
            }

            if (firstRunnig == -1) // no process from our group has a main window yet
            {
                ReleaseMutex(FMOMutex); // release memory for others
                if (firstStarting == -1)
                    return FALSE; // no starting candidate found, exit
                else
                    Sleep(200); // starting candidate found, pause 200ms so it can call SetProcessState()
            }
        }
    } while (firstRunnig == -1 && (GetTickCount() - timeStamp < TASKLIST_TODO_TIMEOUT)); // wait at most 5s for a running instance

    // if we didn't find a running instance from our group or waiting took 5s, give up
    if (firstRunnig == -1)
        return FALSE;

    CProcessListItem* item = &ProcessList->Items[firstRunnig];

    // set Todo, PID and parameters
    ProcessList->Todo = TASKLIST_TODO_ACTIVATE;
    ProcessList->TodoUID++; // tell processes that a new command will be processed
    ProcessList->TodoTimestamp = GetTickCount();
    ProcessList->PID = item->PID;

    // copy parameters from the command line
    memcpy(&ProcessList->CommandLineParams, cmdLineParams, sizeof(CCommandLineParams));
    // and set our internal variables
    ProcessList->CommandLineParams.Version = 1;
    ProcessList->CommandLineParams.RequestUID = ProcessList->TodoUID;
    ProcessList->CommandLineParams.RequestTimestamp = ProcessList->TodoTimestamp;

    // allow the activated process to call SetForegroundWindow so it can come to the front
    AllowSetForegroundWindow(item->PID);

    // start checking in all Salamanders
    // release the shared memory
    ReleaseMutex(FMOMutex);

    ResetEvent(EventProcessed);
    SetEvent(Event);

    // give some time to react (during this period someone should catch the task)
    // 500ms is our reserve to safely cover child threads
    BOOL ret = (WaitForSingleObject(EventProcessed, TASKLIST_TODO_TIMEOUT + 500) == WAIT_OBJECT_0);

    // tell all Salamanders to prepare for another command (also resets in the control thread if a process is executing a todo)
    ResetEvent(Event);

    // clear the todo
    // ProcessList->Todo = 0; // we should lock the FMOMutex first, but in this case nothing can go wrong and we can zero the values
    // ProcessList->PID = 0;

    return ret;
}

BOOL CTaskList::RemoveKilledItems(BOOL* changed)
{
    if (!OK)
        return FALSE;

    if (changed != NULL)
        *changed = FALSE;
    CProcessListItem* ptr = ProcessList->Items;
    int c = ProcessList->ItemsCount;

    int i;
    for (i = 0; i < c; i++)
    {
        HANDLE h = NOHANDLES(OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, ptr[i].PID));
        if (h != NULL)
        {
            // on older Windows we obtain a handle even for a terminated process
            // therefore we also query the exit code; probably unnecessary on W2K and later
            BOOL cont = FALSE;
            DWORD exitcode;
            if (!GetExitCodeProcess(h, &exitcode) || exitcode == STILL_ACTIVE)
                cont = TRUE;
            NOHANDLES(CloseHandle(h));
            if (cont)
                continue; // keep the process in the list
        }
        else
        {
            DWORD lastError = GetLastError();
            if (lastError == ERROR_ACCESS_DENIED)
            {
                continue; // keep the process in the list
            }
        }
        memmove(ptr + i, ptr + i + 1, (c - i - 1) * sizeof(CProcessListItem));
        c--;
        i--;
        if (changed != NULL)
            *changed = TRUE;
    }
    ProcessList->ItemsCount = c;

    /*
// does not work under XP when processes in one session run under different users
// we have no right to open a handle to another process
//---  remove killed processes
int i;
    for (i = 0; i < c; i++)
    {
      HANDLE h = NOHANDLES(OpenProcess(PROCESS_TERMINATE, FALSE, ptr[i].PID));
      if (h != NULL)
      {
        BOOL cont = FALSE;
        DWORD exitcode;
        if (!GetExitCodeProcess(h, &exitcode) || exitcode == STILL_ACTIVE) cont = TRUE;
        NOHANDLES(CloseHandle(h));
        if (cont) continue;  // leave the process in the list
      }
//---  kick the process from the list
      memmove(ptr + i, ptr + i + 1, (c - i - 1) * sizeof(CTLItem));
      c--;
      i--;
    }
    ((DWORD *)SharedMem)[0] = c;   // items-count
    memcpy(items, ptr, c * sizeof(CTLItem));
*/

    return TRUE;
}