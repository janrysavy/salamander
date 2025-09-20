// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// SalmonClient
// The SALMON.EXE module is used for out-of-process generation of minidumps, packing them and uploading to a server
// SALMON must run from Salamander startup to react to crashes. Crashes before SALMON is started
// happen silently and SALMON will process them "next time"
//
// this header is shared between the SALMON and SALAMAND projects because they communicate through memory
//
// out of process minidumps
// http://www.nynaeve.net/?p=128
// http://social.msdn.microsoft.com/Forums/en-US/windbg/thread/2dfd711f-e81e-466f-a566-4605e78075f6
//
// http://www.voyce.com/index.php/2008/06/11/creating-a-featherweight-debugger/
// http://social.msdn.microsoft.com/Forums/en-US/windbg/thread/2dfd711f-e81e-466f-a566-4605e78075f6
// http://social.msdn.microsoft.com/Forums/en-US/vsdebug/thread/b290b7bd-1ec8-4302-8e3a-8ee0dc134683/
// http://www.ms-news.info/f3682/minidumpwritedump-fails-after-writing-partial-dump-access-denied-1843614.html
//
// debugging handles
// http://www.codeproject.com/Articles/6988/Debug-Tutorial-Part-5-Handle-Leaks

#define SALMON_FILEMAPPIN_NAME_SIZE 20

// x64 and x86 versions of Salamander/Salmon are incompatible
#ifdef _WIN64
#define SALMON_SHARED_MEMORY_VERSION_PLATFORM 0x10000000
#else
#define SALMON_SHARED_MEMORY_VERSION_PLATFORM 0x00000000
#endif
#define SALMON_SHARED_MEMORY_VERSION (SALMON_SHARED_MEMORY_VERSION_PLATFORM | 4)

#pragma pack(push)
#pragma pack(4)
struct CSalmonSharedMemory
{
    DWORD Version;           // SALMON_SHARED_MEMORY_VERSION (if it differs for SALAM/SALMON, scream and refuse to communicate)
    HANDLE Process;          // handle to the parent process so we can wait for its termination; this value intentionally leaks
    DWORD ProcessId;         // ID of the crashed parent process
    DWORD ThreadId;          // ID of the crashed thread
    HANDLE Fire;             // AS signals Salmon to send the reports
    HANDLE Done;             // Salmon reports back to AS when it is done
    HANDLE SetSLG;           // AS signals Salmon to load the SLG according to the SLGName buffer set before signaling
    HANDLE CheckBugs;        // AS signals SALMON to check the bug-report directory and offer uploads of any reports from previous crashes
    char SLGName[MAX_PATH];  // meaningful when AS sets SetSLG, telling which SLG to load
    char BugPath[MAX_PATH];  // set by Salamander; path where bug reports will be stored (may not exist yet and is created on crash)
    char BugName[MAX_PATH];  // set by Salamander; internal name of the minidump/bug report
    char BaseName[MAX_PATH]; // set by Salmon; composed as "UID-BugName-DATE-TIME"; ".DMP" is appended for minidumps
    DWORD64 UID;             // unique machine ID created by XORing the GUID; stored in the Bug Reporter key; Salamander sets it, Salmon only reads it and inserts it into the report name

    // EXCEPTION_POINTERS passed component by component; filled before setting the Fire event
    EXCEPTION_RECORD ExceptionRecord;
    CONTEXT ContextRecord;
};
#pragma pack(pop)

//-----------------------------------------------------------------------
#ifdef INSIDE_SALAMANDER

BOOL SalmonInit();
void SalmonSetSLG(const char* slgName); // tell Salmon which language to load
void SalmonCheckBugs();

// stores exception info in shared memory and asks Salmon to create a minidump; waits until it finishes
// returns TRUE on success, FALSE if Salmon could not be executed for some reason
BOOL SalmonFireAndWait(const EXCEPTION_POINTERS* e, char* bugReportPath);

#endif //INSIDE_SALAMANDER
