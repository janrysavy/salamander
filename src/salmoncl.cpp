// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"

#include "salmoncl.h"

CSalmonSharedMemory* SalmonSharedMemory = NULL;
HANDLE SalmonFileMapping = NULL;
HANDLE HSalmonProcess = NULL;

//****************************************************************************

// WARNING: this runs from the entry point before initializing the RTL and
// global objects, so do not call TRACE, HANDLES, RTL, ...

HANDLE GetBugReporterRegistryMutex()
{
    // access rights completely open for all processes
    SECURITY_ATTRIBUTES secAttr;
    char secDesc[SECURITY_DESCRIPTOR_MIN_LENGTH];
    secAttr.nLength = sizeof(secAttr);
    secAttr.bInheritHandle = FALSE;
    secAttr.lpSecurityDescriptor = &secDesc;
    InitializeSecurityDescriptor(secAttr.lpSecurityDescriptor, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(secAttr.lpSecurityDescriptor, TRUE, 0, FALSE);
    // It would be handy to add the SID to the mutex name because processes with
    // different SIDs run with a different HKCU tree, but for simplicity we skip
    // that and the mutex will really be global
    const char* MUTEX_NAME = "Global\\AltapSalamanderBugReporterRegistryMutex";
    HANDLE hMutex = NOHANDLES(CreateMutex(&secAttr, FALSE, MUTEX_NAME));
    if (hMutex == NULL) // Create can open an existing mutex, but it may fail, so we try Open afterwards
        hMutex = NOHANDLES(OpenMutex(SYNCHRONIZE, FALSE, MUTEX_NAME));
    return hMutex;
}

BOOL SalmonGetBugReportUID(DWORD64* uid)
{
    const char* BUG_REPORTER_KEY = "Software\\Open Salamander\\Bug Reporter";
    const char* BUG_REPORTER_UID = "ID";

    // this section is executed at Salamander startup and concurrent registry
    // reading/writing could theoretically occur, therefore we guard the access
    // by a global mutex
    HANDLE hMutex = GetBugReporterRegistryMutex();
    if (hMutex != NULL)
        WaitForSingleObject(hMutex, INFINITE);
    *uid = 0;
    HKEY hKey;
    LONG res = NOHANDLES(RegOpenKeyEx(HKEY_CURRENT_USER, BUG_REPORTER_KEY, 0, KEY_READ, &hKey));
    if (res == ERROR_SUCCESS)
    {
        // try to load the old value if it exists
        DWORD gettedType;
        DWORD bufferSize = sizeof(*uid);
        res = RegQueryValueEx(hKey, BUG_REPORTER_UID, 0, &gettedType, (BYTE*)uid, &bufferSize);
        if (res != ERROR_SUCCESS || gettedType != REG_QWORD)
            *uid = 0;
        NOHANDLES(RegCloseKey(hKey));
    }
    // if the UID does not exist yet, create it and store it
    if (*uid == 0)
    {
        GUID guid;
        if (CoCreateGuid(&guid) == S_OK)
        {
            // we won't store and send the whole GUID, half of it XORed with the other half is enough
            DWORD64* dw64 = (DWORD64*)&guid;
            *uid = dw64[0] ^ dw64[1];

            // try to store it
            DWORD createType;
            LONG res2 = NOHANDLES(RegCreateKeyEx(HKEY_CURRENT_USER, BUG_REPORTER_KEY, 0, NULL, REG_OPTION_NON_VOLATILE,
                                                 KEY_READ | KEY_WRITE, NULL, &hKey, &createType));
            if (res2 == ERROR_SUCCESS)
            {
                res2 = RegSetValueEx(hKey, BUG_REPORTER_UID, 0, REG_QWORD, (BYTE*)uid, sizeof(*uid));
                if (res2 != ERROR_SUCCESS)
                    *uid = 0; // use zero if storing fails
                NOHANDLES(RegCloseKey(hKey));
            }
        }
    }
    if (hMutex != NULL)
    {
        ReleaseMutex(hMutex);
        NOHANDLES(CloseHandle(hMutex));
    }

    return TRUE;
}

BOOL SalmonSharedMemInit(CSalmonSharedMemory* mem)
{
    SECURITY_ATTRIBUTES sa; // allow handle inheritance to child processes (they can work directly with our event)
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    RtlFillMemory(mem, sizeof(CSalmonSharedMemory), 0);

    mem->Version = SALMON_SHARED_MEMORY_VERSION;
    // salmon will run as a child process with bInheritHandles==TRUE, so it can access these handles directly
    mem->ProcessId = GetCurrentProcessId();
    mem->Process = NOHANDLES(OpenProcess(SYNCHRONIZE | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, TRUE, mem->ProcessId));
    mem->Fire = NOHANDLES(CreateEvent(&sa, TRUE, FALSE, NULL));      // "nonsignaled" state, manual
    mem->Done = NOHANDLES(CreateEvent(&sa, TRUE, FALSE, NULL));      // "nonsignaled" state, manual
    mem->SetSLG = NOHANDLES(CreateEvent(&sa, TRUE, FALSE, NULL));    // "nonsignaled" state, manual
    mem->CheckBugs = NOHANDLES(CreateEvent(&sa, TRUE, FALSE, NULL)); // "nonsignaled" state, manual

    // we now store bug reports in LOCAL_APPDATA where Windows WER puts minidumps by default
    // the directory is not created immediately; we handle that when a crash occurs
    if (SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, mem->BugPath) == S_OK)
    {
        int len = lstrlen(mem->BugPath);
        if (len > 0 && mem->BugPath[len - 1] == '\\') // better check the trailing backslash
            mem->BugPath[len - 1] = 0;
        lstrcat(mem->BugPath, "\\Open Salamander");
    }

    // base name for bug reports
    strcpy(mem->BugName, "AS" VERSINFO_SAL_SHORT_VERSION);

    return (mem->Process != NULL && mem->Fire != NULL && mem->Done != NULL && mem->SetSLG != NULL &&
            mem->CheckBugs != NULL && mem->BugPath[0] != 0);
}

void GetStartupSLGName(char* slgName, DWORD slgNameMax)
{
    // read from the registry the SLG name that will likely be used
    // another one can be selected later while Salamander runs
    // this is only a default; if the entry is missing we pass an empty string
    slgName[0] = 0;

    char keyName[MAX_PATH];
    sprintf(keyName, "%s\\%s", SalamanderConfigurationRoots[0], SALAMANDER_CONFIG_REG);
    HKEY hKey;
    LONG res = NOHANDLES(RegOpenKeyEx(HKEY_CURRENT_USER, keyName, 0, KEY_READ, &hKey));
    if (res == ERROR_SUCCESS)
    {
        DWORD gettedType;
        res = SalRegQueryValueEx(hKey, CONFIG_LANGUAGE_REG, 0, &gettedType, (BYTE*)slgName, &slgNameMax);
        if (res != ERROR_SUCCESS || gettedType != REG_SZ)
            slgName[0] = 0;
        RegCloseKey(hKey);
    }
}

BOOL SalmonStartProcess(const char* fileMappingName) //Configuration.LoadedSLGName
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    char cmd[2 * MAX_PATH];
    char rtlDir[MAX_PATH];
    char oldCurDir[MAX_PATH];
    char slgName[MAX_PATH];
#define MAX_ENV_PATH 32766
    char envPATH[MAX_ENV_PATH];
    BOOL ret;

    HSalmonProcess = NULL;

    ret = FALSE;
    GetModuleFileName(NULL, cmd, MAX_PATH);
    *(strrchr(cmd, '\\') + 1) = 0;
    lstrcat(cmd, "utils\\salmon.exe");
    AddDoubleQuotesIfNeeded(cmd, MAX_PATH); // CreateProcess wants the name with spaces in quotes (otherwise it tries various variants, see help)
    GetStartupSLGName(slgName, sizeof(slgName));
    wsprintf(cmd + strlen(cmd), " \"%s\" \"%s\"", fileMappingName, slgName); // slgName can be an empty string if no configuration exists
    memset(&si, 0, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.wShowWindow = SW_SHOWNORMAL;
    GetModuleFileName(NULL, rtlDir, MAX_PATH);
    *(strrchr(rtlDir, '\\') + 1) = 0;
    GetCurrentDirectory(MAX_PATH, oldCurDir);

    // another attempt to solve the problem before we split SALMON.EXE into EXE + DLL
    // we try to extend the PATH environment variable for the child process (SALMON.EXE) with the RTL path
    if (GetEnvironmentVariable("PATH", envPATH, MAX_ENV_PATH) != 0)
    {
        if (lstrlen(envPATH) + 2 + lstrlen(rtlDir) < MAX_ENV_PATH)
        {
            char newPATH[MAX_ENV_PATH];
            lstrcpy(newPATH, envPATH);
            lstrcat(newPATH, ";");
            lstrcat(newPATH, rtlDir);
            SetEnvironmentVariable("PATH", newPATH);
        }
        else
            envPATH[0] = 0;
    }
    else
        envPATH[0] = 0;

    // originally we only passed rtlDir to CreateProcess but with some UAC combinations salmon.exe could not run
    // because it could not see the RTL: https://forum.altap.cz/viewtopic.php?f=2&t=6957&p=26548#p26548
    // we additionally try to set the current directory
    // if that does not help we could pass NULL to CreateProcess instead of rtlDir so the current directory would be inherited from the launching process
    SetCurrentDirectory(rtlDir);
    // EDIT 4/2014: after several tests with Support@bluesware.ch and chr.mue@gmail.com (see emails)
    // I see two possible solutions: try extending the PATH env variable with SALRTL for the child process.
    // The second option is to split SALMON.EXE into an EXE without RTL and a DLL with implicitly linked RTL. Before loading SALMON.DLL
    // from the running SALMON.EXE we could set the current dir and load SALMON.DLL at runtime, which might work.
    // ----
    // On my machine each of the three path settings works on its own (ENV PATH, SetCurrentDirectory and rtlDir parameter in the CreateProcess call)
    if (NOHANDLES(CreateProcess(NULL, cmd, NULL, NULL, TRUE, // bInheritHandles==TRUE, needs to pass event handles!
                                CREATE_DEFAULT_ERROR_MODE | HIGH_PRIORITY_CLASS, NULL,
                                rtlDir, &si, &pi)))
    {
        HSalmonProcess = pi.hProcess;                           // we need the handle to detect whether Salmon is alive
        AllowSetForegroundWindow(SalGetProcessId(pi.hProcess)); // allow Salmon above us
        //NOHANDLES(CloseHandle(pi.hProcess)); // let it leak quietly; it would be the last handle released before process end anyway
        //NOHANDLES(CloseHandle(pi.hThread));
        ret = TRUE;
    }
    SetCurrentDirectory(oldCurDir);
    if (envPATH[0] != 0)
        SetEnvironmentVariable("PATH", envPATH);
    return ret;
}

//BOOL IsSalmonRunning()
//{
//  if (HSalmonProcess != NULL)
//  {
//    DWORD waitRet = WaitForSingleObject(HSalmonProcess, 0);
//    return waitRet == WAIT_TIMEOUT;
//  }
//  return FALSE;
//}

// We want to know about SEH exceptions even on x64 Windows 7 SP1 and later
// http://blog.paulbetts.org/index.php/2010/07/20/the-case-of-the-disappearing-onload-exception-user-mode-callback-exceptions-in-x64/
// http://connect.microsoft.com/VisualStudio/feedback/details/550944/hardware-exceptions-on-x64-machines-are-silently-caught-in-wndproc-messages
// http://support.microsoft.com/kb/976038
void EnableExceptionsOn64()
{
    typedef BOOL(WINAPI * FSetProcessUserModeExceptionPolicy)(DWORD dwFlags);
    typedef BOOL(WINAPI * FGetProcessUserModeExceptionPolicy)(LPDWORD dwFlags);
    typedef BOOL(WINAPI * FIsWow64Process)(HANDLE, PBOOL);
#define PROCESS_CALLBACK_FILTER_ENABLED 0x1

    HINSTANCE hDLL = LoadLibrary("KERNEL32.DLL");
    if (hDLL != NULL)
    {
        FIsWow64Process isWow64 = (FIsWow64Process)GetProcAddress(hDLL, "IsWow64Process");                                                      // Min: XP SP2
        FSetProcessUserModeExceptionPolicy set = (FSetProcessUserModeExceptionPolicy)GetProcAddress(hDLL, "SetProcessUserModeExceptionPolicy"); // Min: Vista with hotfix
        FGetProcessUserModeExceptionPolicy get = (FGetProcessUserModeExceptionPolicy)GetProcAddress(hDLL, "GetProcessUserModeExceptionPolicy"); // Min: Vista with hotfix
        if (isWow64 != NULL && set != NULL && get != NULL)
        {
            BOOL bIsWow64;
            if (isWow64(GetCurrentProcess(), &bIsWow64) && bIsWow64)
            {
                DWORD dwFlags;
                if (get(&dwFlags))
                    set(dwFlags & ~PROCESS_CALLBACK_FILTER_ENABLED);
            }
        }
        FreeLibrary(hDLL);
    }
}

BOOL SalmonInit()
{
    EnableExceptionsOn64();

    SalmonSharedMemory = NULL;
    char salmonFileMappingName[SALMON_FILEMAPPIN_NAME_SIZE];
    // allocate shared space in pagefile.sys
    DWORD ti = (GetTickCount() >> 3) & 0xFFF;
    while (TRUE) // looking for a unique name for the file mapping
    {
        wsprintf(salmonFileMappingName, "Salmon%X", ti++);
        SalmonFileMapping = NOHANDLES(CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, // FIXME_X64 are we passing x86/x64 incompatible data?
                                                        sizeof(CSalmonSharedMemory), salmonFileMappingName));
        if (SalmonFileMapping == NULL || GetLastError() != ERROR_ALREADY_EXISTS)
            break;
        NOHANDLES(CloseHandle(SalmonFileMapping));
    }
    if (SalmonFileMapping != NULL)
    {
        SalmonSharedMemory = (CSalmonSharedMemory*)NOHANDLES(MapViewOfFile(SalmonFileMapping, FILE_MAP_WRITE, 0, 0, 0)); // FIXME_X64 are we passing x86/x64 incompatible data?
        if (SalmonSharedMemory != NULL)
        {
            ZeroMemory(SalmonSharedMemory, sizeof(CSalmonSharedMemory));
            if (SalmonSharedMemInit(SalmonSharedMemory))
            {
                SalmonGetBugReportUID(&SalmonSharedMemory->UID);

                // if Salmon fails to start we still return TRUE - the problem will be reported later after the SLG is loaded
                SalmonStartProcess(salmonFileMappingName);
                return TRUE;
            }
        }
    }
    // a serious (and unexpected) error occurred; block Salamander startup, the message will be in English (no SLG yet)
    return FALSE;
}

// information that Salmon is not running should be shown only once
static BOOL SalmonNotRunningReported = FALSE;

void SalmonSetSLG(const char* slgName)
{
    ResetEvent(SalmonSharedMemory->Done);

    strcpy(SalmonSharedMemory->SLGName, slgName);
    SetEvent(SalmonSharedMemory->SetSLG);

    // wait for a signal from Salmon that it processed the task (event Done) or
    // for a case when someone killed Salmon
    HANDLE arr[2];
    arr[0] = HSalmonProcess;
    arr[1] = SalmonSharedMemory->Done;
    DWORD waitRet = WaitForMultipleObjects(2, arr, FALSE, INFINITE);
    if (waitRet != WAIT_OBJECT_0 + 1) // someone killed Salmon or communication failed
    {
        if (!SalmonNotRunningReported && HLanguage != NULL)
        {
            MessageBox(NULL, LoadStr(IDS_SALMON_NOT_RUNNING), SALAMANDER_TEXT_VERSION, MB_OK | MB_ICONERROR);
            SalmonNotRunningReported = TRUE;
        }
    }
    ResetEvent(SalmonSharedMemory->Done);
}

void SalmonCheckBugs()
{
    ResetEvent(SalmonSharedMemory->Done);
    SetEvent(SalmonSharedMemory->CheckBugs);

    // wait for a signal from Salmon that it processed the task (event Done) or
    // for a case when someone killed Salmon
    HANDLE arr[2];
    arr[0] = HSalmonProcess;
    arr[1] = SalmonSharedMemory->Done;
    DWORD waitRet = WaitForMultipleObjects(2, arr, FALSE, INFINITE);
    if (waitRet != WAIT_OBJECT_0 + 1) // someone killed Salmon or communication failed
    {
        if (!SalmonNotRunningReported && HLanguage != NULL)
        {
            MessageBox(NULL, LoadStr(IDS_SALMON_NOT_RUNNING), SALAMANDER_TEXT_VERSION, MB_OK | MB_ICONERROR);
            SalmonNotRunningReported = TRUE;
        }
    }
    ResetEvent(SalmonSharedMemory->Done);
}

BOOL SalmonFireAndWait(const EXCEPTION_POINTERS* e, char* bugReportPath)
{
    SalmonSharedMemory->ThreadId = GetCurrentThreadId();
    SalmonSharedMemory->ExceptionRecord = *e->ExceptionRecord;
    SalmonSharedMemory->ContextRecord = *e->ContextRecord;
    SetEvent(SalmonSharedMemory->Fire);

    // wait for a signal from Salmon that it processed the task (event Done) or
    // for a case when someone killed Salmon
    HANDLE arr[2];
    arr[0] = HSalmonProcess;
    arr[1] = SalmonSharedMemory->Done;
    WaitForMultipleObjects(2, arr, FALSE, INFINITE);
    ResetEvent(SalmonSharedMemory->Done);

    strcpy(bugReportPath, SalmonSharedMemory->BugPath);
    SalPathAppend(bugReportPath, SalmonSharedMemory->BaseName, MAX_PATH);
    strcat(bugReportPath, ".TXT");

    return TRUE;
}