// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

//
// ****************************************************************************

// TRUE = the first running instance of version 3.0 or newer
// determined via a mutex in the global namespace so it is shared
// across other sessions (remote desktop, fast user switching)
extern BOOL FirstInstance_3_or_later;

// shared memory holds:
//  DWORD                  - PID of the process to break into
//  DWORD                  - item count in the list
//  MAX_TL_ITEMS * CTLItem - list of items

#define MAX_TL_ITEMS 500 // maximum number of items in shared memory; cannot change

#define TASKLIST_TODO_HIGHLIGHT 1 // highlight the window of the process in 'PID'
#define TASKLIST_TODO_BREAK 2     // break the process in 'PID'
#define TASKLIST_TODO_TERMINATE 3 // terminate the process in 'PID'
#define TASKLIST_TODO_ACTIVATE 4  // activate the process in 'PID'

#define TASKLIST_TODO_TIMEOUT 5000 // 5 seconds for processes to handle the todo

#define PROCESS_STATE_STARTING 1 // our process is starting, the main window does not exist yet
#define PROCESS_STATE_RUNNING 2  // our process is running, the main window exists
#define PROCESS_STATE_ENDING 3   // our process is ending, the main window is already gone

#pragma pack(push, enter_include_tasklist) // keep structures independent of packing alignment
#pragma pack(4)

extern HANDLE HSalmonProcess;

// NOTE: x64 and x86 processes communicate through this structure, so watch for types (e.g. HANDLE) that have different widths
struct CProcessListItem
{
    DWORD PID;            // ProcessID; unique while the process runs and may be reused later
    SYSTEMTIME StartTime; // time when the process was started
    DWORD IntegrityLevel; // integrity level used to tell processes running under different privileges apart
    BYTE SID_MD5[16];     // MD5 computed from the process SID so we can identify processes of different users; SID length is unknown so only the hash is stored
    DWORD ProcessState;   // state of Salamander, see PROCESS_STATE_xxx
    UINT64 HMainWindow;   // (x64 friendly) handle of the main window if it already/still exists (set on create/destroy)
    DWORD SalmonPID;      // PID of Salmon so the breaking process can guarantee SetForegroundWindow rights

    CProcessListItem()
    {
        PID = GetCurrentProcessId();
        GetLocalTime(&StartTime);
        GetProcessIntegrityLevel(&IntegrityLevel);
        GetSidMD5(SID_MD5);
        ProcessState = PROCESS_STATE_STARTING;
        HMainWindow = NULL;
        SalmonPID = 0;
        if (HSalmonProcess != NULL)
            SalmonPID = SalGetProcessId(HSalmonProcess); // Salmon is already running at this time
    }
};

// NOTE: only add fields to this structure because older Salamander versions read it too
// NOTE: x64 and x86 processes communicate via this structure, so mind types (e.g. HANDLE) that differ in size
// NOTE: Increasing the version and enlarging the structure makes little sense, because
//       older Salamanders started first won't provide the new fields in shared memory.
//       The recommended approach is to change AS_PROCESSLIST_NAME and modify the data as needed
struct CCommandLineParams
{
    DWORD Version;               // newer Salamander versions may increase 'Version' and start using ReservedX
    DWORD RequestUID;            // unique increasing ID of the activation request
    DWORD RequestTimestamp;      // GetTickCount() value from the time the activation request was created
    char LeftPath[2 * MAX_PATH]; // panel paths (left, right or active); if empty, they should not be set
    char RightPath[2 * MAX_PATH];
    char ActivePath[2 * MAX_PATH];
    DWORD ActivatePanel;         // which panel to activate: 0-none, 1-left, 2-right
    BOOL SetTitlePrefix;         // if TRUE, set the title prefix according to TitlePrefix
    char TitlePrefix[MAX_PATH];  // title prefix; if empty, keep the existing one; MAX_PATH is used instead of TITLE_PREFIX_MAX which could change
    BOOL SetMainWindowIconIndex; // if TRUE, set the main window icon according to MainWindowIconIndex
    DWORD MainWindowIconIndex;   // 0: first icon, 1: second icon, ...
    // NOTE: the structure can be extended only while it remains the last member in CProcessList;
    // otherwise it is too late and must not be modified

    CCommandLineParams()
    {
        ZeroMemory(this, sizeof(CCommandLineParams));
    }
};

// Open Salamander Process List
// WARNING: only append new fields to this structure because older versions of Salamander expect the existing layout
struct CProcessList
{
    DWORD Version; // newer Salamander versions may increase 'Version' and start using the ReservedX variables

    DWORD ItemsCount;    // number of valid entries in the Items array
    DWORD ItemsStateUID; // "version" of the Items list; increases with each change and signals the Tasks dialog to refresh
    CProcessListItem Items[MAX_TL_ITEMS];

    DWORD Todo;                           // determines what to do after FireEvent; contains one of TASKLIST_TODO_*
    DWORD TodoUID;                        // sequence number of the request, increases for each new one
    DWORD TodoTimestamp;                  // GetTickCount() value when the Todo request was created
    DWORD PID;                            // PID for which the Todo action should be performed
    CCommandLineParams CommandLineParams; // panel paths and other activation parameters
                                          // NOTE: if this structure needs to grow, first extend CCommandLineParams
                                          // and reserve some MAX_PATH buffers and a few DWORDs for additional command line parameters
};

#pragma pack(pop, enter_include_tasklist)

class CTaskList
{
protected:
    HANDLE FMO;                // file-mapping object, shared memory
    CProcessList* ProcessList; // pointer to the shared memory
    HANDLE FMOMutex;           // mutex that guards access to the FMO
    HANDLE Event;              // event signaled so other processes check
                               // if they should perform the action in Todo
    HANDLE EventProcessed;     // set to signaled by a process after it performs the Todo
                               // to notify the controlling process that it finished
    HANDLE TerminateEvent;     // event used to terminate the break thread
    HANDLE ControlThread;      // control thread waiting for events and dispatching them
    BOOL OK;                   // did construction succeed?

public:
    CTaskList();
    ~CTaskList();

    BOOL Init();

    // fills the task-list items; 'items' must be an array of at least MAX_TL_ITEMS structures and
    // the function returns the number of items
    // 'items' may be NULL if only 'itemsStateUID' is needed
    // returns the "version" of the process list; it increases whenever the list changes
    // and is used by the dialog to know when to refresh; 'itemsStateUID' may be NULL
    // if 'timeouted' is not NULL, it reports whether waiting on shared memory timed out
    int GetItems(CProcessListItem* items, DWORD* itemsStateUID, BOOL* timeouted = NULL);

    // asks process 'pid' to perform the action according to 'todo' (except TASKLIST_TODO_ACTIVATE)
    // if 'timeouted' is not NULL, it reports whether waiting on shared memory timed out
    BOOL FireEvent(DWORD todo, DWORD pid, BOOL* timeouted = NULL);

    // if 'timeouted' is not NULL, it reports whether waiting on shared memory timed out
    BOOL ActivateRunningInstance(const CCommandLineParams* cmdLineParams, BOOL* timeouted = NULL);

    // searches the process list for this instance and sets 'ProcessState' and 'HMainWindow'; returns TRUE on success
    // if 'timeouted' is not NULL, it reports whether waiting on shared memory timed out
    BOOL SetProcessState(DWORD processState, HWND hMainWindow, BOOL* timeouted = NULL);

protected:
    // walks the process list and removes non-existing entries
    // must be called only after successfully entering the 'FMOMutex' critical section
    // sets 'changed' to TRUE if some item was dropped, otherwise FALSE
    BOOL RemoveKilledItems(BOOL* changed);

    friend DWORD WINAPI FControlThread(void* param);
};

extern CTaskList TaskList;

// protects access to CommandLineParams
extern CRITICAL_SECTION CommandLineParamsCS;
// used to pass activation parameters from the control thread to the main thread
extern CCommandLineParams CommandLineParams;
// event becomes signaled once the main thread consumes the parameters
extern HANDLE CommandLineParamsProcessed;
