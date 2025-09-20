// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#define CREATE_DIR_SIZE CQuadWord(4096, 0) // operation cost estimates (measured uncached by worker thread runtime)
#define MOVE_DIR_SIZE CQuadWord(5050, 0)
#define DELETE_DIR_SIZE CQuadWord(2400, 0)
#define DELETE_DIRLINK_SIZE CQuadWord(2400, 0)
#define MOVE_FILE_SIZE CQuadWord(6500, 0)
#define COPY_MIN_FILE_SIZE CQuadWord(4096, 0) // must be at least 1 so the allocation check for copy works in DoCopyFile
#define CONVERT_MIN_FILE_SIZE CQuadWord(4096, 0)
#define COMPRESS_ENCRYPT_MIN_FILE_SIZE CQuadWord(4096, 0)
#define DELETE_FILE_SIZE CQuadWord(2300, 0)
#define CHATTRS_FILE_SIZE CQuadWord(500, 0)
#define MAX_OP_FILESIZE 6500 // WARNING: highest allowed value from this group

// 4/2012 - buffer increased tenfold; with large files over the network we reach
// transfer rates comparable to Total Commander, roughly 2-3x faster than with
// the previous one. Tested on local and network disks with no disadvantages.
#define OPERATION_BUFFER (10 * 32768)          // 320KB buffer for copy and move
#define REMOVABLE_DISK_COPY_BUFFER 65536       // 64KB buffer for copy and move on removable media (floppy, ZIP)
#define ASYNC_COPY_BUF_SIZE_512KB (128 * 1024) // 128KB buffer for files up to 512KB
#define ASYNC_COPY_BUF_SIZE_2MB (256 * 1024)   // 256KB buffer for files up to 2MB
#define ASYNC_COPY_BUF_SIZE_8MB (512 * 1024)   // 512KB buffer for files up to 8MB
#define ASYNC_COPY_BUF_SIZE (1024 * 1024)      // maximum buffer for async copy (Explorer uses up to 1MB); must be >= RETRYCOPY_TAIL_MINSIZE
#define ASYNC_SLOW_COPY_BUF_SIZE (8 * 1024)    // 8KB buffer for slow copies (mainly network disks via VPN)
#define ASYNC_SLOW_COPY_BUF_MINBLOCKS 12

// WARNING: HIGH_SPEED_LIMIT must be at least as large as the maximum of
//          OPERATION_BUFFER, REMOVABLE_DISK_COPY_BUFFER and ASYNC_COPY_BUF_SIZE
#define HIGH_SPEED_LIMIT (1024 * 1024) // if the speed limit is >= this value we throttle by inserting a sleep after transferring (speed-limit / HIGH_SPEED_LIMIT_BRAKE_DIV) bytes when needed
#define HIGH_SPEED_LIMIT_BRAKE_DIV 10  // see HIGH_SPEED_LIMIT for details

void InitWorker();
void ReleaseWorker();

struct CChangeAttrsData
{
    BOOL ChangeCompression;
    BOOL ChangeEncryption;

    BOOL ChangeTimeModified;
    FILETIME TimeModified;
    BOOL ChangeTimeCreated;
    FILETIME TimeCreated;
    BOOL ChangeTimeAccessed;
    FILETIME TimeAccessed;
};

struct CConvertData // data for ocConvert
{
    char CodeTable[256];
    int EOFType;
};

class COperations;
struct CProgressDlgArrItem;

struct CStartProgressDialogData
{
    COperations* Script;
    const char* Caption;
    CChangeAttrsData* AttrsData;
    CConvertData* ConvertData;
    CProgressDlgArrItem* NewDlg;
    BOOL OperationWasStarted;
    HANDLE ContEvent;
    RECT MainWndRectClipR; // used to center a progress dialog without a parent (background operations)
    RECT MainWndRectByR;   // used to center a progress dialog without a parent (background operations)
};

struct CProgressData
{
    const char *Operation,
        *Source,
        *Preposition,
        *Target;
};

//
// ****************************************************************************
// CTransferSpeedMeter
//
// object that calculates data transfer speed (adapted from the FTP plugin)

#define TRSPMETER_ACTSPEEDSTEP 200        // transfer-speed calculation step size in milliseconds (must not be 0)
#define TRSPMETER_ACTSPEEDNUMOFSTEPS 25   // number of stored steps (more steps = smoother changes if the first step "drops" from the queue)
#define TRSPMETER_NUMOFSTOREDPACKETS 40   // how many recent "packets" to remember for speed computation at low transfer rates (must not be 0)
#define TRSPMETER_STPCKTSMININTERVAL 2000 // minimum time between the first and last stored packet required to use them for speed computation (slow transfer rates)

class CTransferSpeedMeter
{
protected:
    // WARNING: access to this object is allowed only inside COperations::StatusCS

    // data transfer speed calculation:
    DWORD TransferedBytes[TRSPMETER_ACTSPEEDNUMOFSTEPS + 1]; // circular queue holding the number of bytes transferred in the last N time steps plus one extra "working" slot accumulating the current interval
    int ActIndexInTrBytes;                                   // index of the last (current) entry in TransferedBytes
    DWORD ActIndexInTrBytesTimeLim;                          // time limit (ms) of the last entry in TransferedBytes (bytes accumulate into the last entry until this time)
    int CountOfTrBytesItems;                                 // number of steps in TransferedBytes (closed ones plus one "working")

    DWORD LastPacketsSize[TRSPMETER_NUMOFSTOREDPACKETS + 1]; // circular queue with sizes of the last N+1 "packets"
    DWORD LastPacketsTime[TRSPMETER_NUMOFSTOREDPACKETS + 1]; // circular queue with timestamps of the last N+1 "packets"
    int ActIndexInLastPackets;                               // index in LastPacketsSize/Time for storing the next received packet (also the index of the oldest packet when full)
    int CountOfLastPackets;                                  // number of packets in LastPacketsSize and LastPacketsTime (valid entries count)
    DWORD MaxPacketSize;                                     // size of the largest packet we can expect

public:
    BOOL ResetSpeed; // TRUE if the meter should probably be reset before the next measurement (call JustConnected); a huge speed drop was detected so we displayed zero

public:
    CTransferSpeedMeter();

    // clears the object (preparation for another use)
    // can be called from any thread
    void Clear();

    // returns the connection speed in bytes per second in 'speed' (must not be NULL)
    // can be called from any thread
    void GetSpeed(CQuadWord* speed);

    // called when speed measurement should start
    // may be called from any thread
    void JustConnected();

    // called after part of the data has been transferred; 'count' is how much data,
    // 'time' is the transfer duration; 'maxPacketSize' is the maximum expected
    // size of data blocks to arrive in subsequent BytesReceived() calls
    void BytesReceived(DWORD count, DWORD time, DWORD maxPacketSize);

    // adjusts 'progressBufferLimit' according to the packets received so far;
    // 'lastFileBlockCount' is the packet count we must not exceed (continuous
    // copy of a single file, protected against overflow;
    // a count > 1000000 means "a lot," the exact value is unimportant). 'lastFileStartTime'
    // is GetTickCount() when we started copying the last file
    void AdjustProgressBufferLimit(DWORD* progressBufferLimit, DWORD lastFileBlockCount,
                                   DWORD lastFileStartTime);
};

//
// ****************************************************************************
// CProgressSpeedMeter
//
// object for computing progress speed - used to estimate time remaining

#define PRSPMETER_ACTSPEEDSTEP 500         // progress-speed calculation step size in milliseconds (must not be 0)
#define PRSPMETER_ACTSPEEDNUMOFSTEPS 60    // number of steps used for progress speed (more steps = smoother changes if the first one drops out)
#define PRSPMETER_NUMOFSTOREDPACKETS 100   // how many of the most recent "packets" to remember when calculating progress speed (low transfer rates) (must not be 0)
#define PRSPMETER_STPCKTSMININTERVAL 10000 // minimum time between the first and last stored packet necessary to compute progress speed (slow transfer rates)

class CProgressSpeedMeter
{
protected:
    // WARNING: access to this object is allowed only inside COperations::StatusCS

    // data transfer speed calculation:
    DWORD TransferedBytes[PRSPMETER_ACTSPEEDNUMOFSTEPS + 1]; // circular queue with bytes transferred during the last N time steps plus one extra working slot accumulating the current interval
    int ActIndexInTrBytes;                                   // index of the last (current) entry in TransferedBytes
    DWORD ActIndexInTrBytesTimeLim;                          // time limit (ms) of the last entry in TransferedBytes (bytes accumulate into the last entry until this time)
    int CountOfTrBytesItems;                                 // number of steps in TransferedBytes (closed ones plus one "working")

    DWORD LastPacketsSize[PRSPMETER_NUMOFSTOREDPACKETS + 1]; // circular queue with the size of the last N+1 "packets"
    DWORD LastPacketsTime[PRSPMETER_NUMOFSTOREDPACKETS + 1]; // circular queue with the time each of the last N+1 packets was received
    int ActIndexInLastPackets;                               // index in LastPacketsSize/Time to store the next packet (also the index of the oldest packet when full)
    int CountOfLastPackets;                                  // number of packets stored in LastPacketsSize and LastPacketsTime
    DWORD MaxPacketSize;                                     // size of the largest packet we can expect

public:
    CProgressSpeedMeter();

    // clears the object (preparation for another use)
    // can be called from any thread
    void Clear();

    // stores the connection speed in bytes per second in 'speed' (must not be NULL)
    // can be called from any thread
    void GetSpeed(CQuadWord* speed);

    // called when speed measurement should begin
    // can be called from any thread
    void JustConnected();

    // called after a portion of data has been transferred; 'count' is the number of bytes and 'time'
    // is the transfer time; 'maxPacketSize' is the largest expected block size for upcoming BytesReceived() calls
};

enum COperationCode
{
    ocCopyFile,
    ocMoveFile,
    ocDeleteFile,
    ocCreateDir,
    ocMoveDir,
    ocDeleteDir,
    ocDeleteDirLink,
    ocChangeAttrs, // WARNING: desired attributes are stored in TargetName (used regardless of type, simply a DWORD)
    ocCountSize,
    ocConvert,
    ocLabelForSkipOfCreateDir, // marker to which the script jumps when skipping ocCreateDirXXX; WARNING: SourceName and TargetName contain the low and high DWORDs of the total size (including ADS) of the skipped directory; Attr stores the index of ocCreateDirXXX in the COperations array for the skipped directory
    ocCopyDirTime,             // Move/Copy: when filterCriteria->PreserveDirTime==TRUE copy directory timestamps; WARNING: lastWrite is stored in SourceName and Attr (two DWORDs used regardless of type)
};

#define OPFL_OVERWROLDERALRTESTED 0x00000001 // the skip check for "overwrite older, skip other existing" was already done
#define OPFL_AS_ENCRYPTED 0x00000002         // target file/directory should have the Encrypted attribute set
#define OPFL_COPY_ADS 0x00000004             // copy ADS of files/directories as well
#define OPFL_SRCPATH_IS_NET 0x00000008       // the source path is a network location
#define OPFL_SRCPATH_IS_FAST 0x00000010      // the source path is a disk, USB disk, flash, flash-card reader, CD, DVD or RAM disk (not a network or floppy)
#define OPFL_TGTPATH_IS_NET 0x00000020       // the target path is a network location
#define OPFL_TGTPATH_IS_FAST 0x00000040      // the target path is a disk, USB disk, flash, flash-card reader, CD, DVD or RAM disk (not a network or floppy)
#define OPFL_IGNORE_INVALID_NAME 0x00000080  // skip the name validity check (used for directories: since we didn't change the name we don't complain it's invalid)

struct COperation
{
    COperationCode Opcode;
    CQuadWord Size;
    CQuadWord FileSize; // file size, valid only for ocCopyFile and ocMoveFile
    char *SourceName,
        *TargetName;
    DWORD Attr;
    DWORD OpFlags; // combination of OPFL_xxx, see above
};

class COperations : public TDirectArray<COperation>
{
public:
    CQuadWord TotalSize;      // WARNING: not the file size in bytes, only a value usable for progress estimation
    CQuadWord CompressedSize; // total size of files after compression
    CQuadWord OccupiedSpace;  // occupied disk space
    CQuadWord TotalFileSize;  // sum of file sizes on disk
    CQuadWord FreeSpace;      // free disk space (copy, move) for checks
    DWORD BytesPerCluster;    // for computing occupied space

    // sizes of individual files for estimates with the given cluster size
    TDirectArray<CQuadWord> Sizes;

    DWORD ClearReadonlyMask; // for automatic clearing of the read-only flag from CD-ROMs
    BOOL InvertRecycleBin;   // invert Recycle Bin usage

    int FilesCount;
    int DirsCount;

    const char* RemapNameFrom; // for screen output only:
    int RemapNameFromLen;      // name mapping for MoveFiles (From -> To)
    const char* RemapNameTo;
    int RemapNameToLen;

    BOOL RemovableTgtDisk;      // is the target a removable medium?
    BOOL RemovableSrcDisk;      // is the source a removable medium?
    BOOL CanUseRecycleBin;      // can we use the Recycle Bin? (local fixed drives only)
    BOOL SameRootButDiffVolume; // TRUE when moving between paths with the same root but different volumes (at least one path uses a junction point)
    BOOL TargetPathSupADS;      // TRUE when the copy/move target supports ADS (streams must be deleted before overwriting files or entire files)
                                //    BOOL TargetPathSupEFS;       // TRUE when the copy/move target supports EFS (in short: NTFS rather than FAT)

    // for Copy/Move operations
    BOOL IsCopyOrMoveOperation; // TRUE = this is a Copy/Move operation (it will be queued with disk Copy/Move operations)
    BOOL OverwriteOlder;        // overwrite older files and skip newer ones without asking
    BOOL CopySecurity;          // preserve NTFS permissions, FALSE = don't care, the outcome doesn't matter
    BOOL CopyAttrs;             // preserve Archive, Encrypt and Compress attributes, FALSE = don't care
    BOOL PreserveDirTime;       // keep directory dates/times (used by Move: we detect if times change and fix them manually, works e.g. on Samba)
    BOOL StartOnIdle;           // start only when nothing else is running
    BOOL SourcePathIsNetwork;   // TRUE when the source path is network based (UNC or mapped drive)

    // for the status line in the progress dialog (Copy/Move only)
    BOOL ShowStatus;       // show operation status under the second progress bar (copy speed, etc.)
    BOOL IsCopyOperation;  // TRUE = copy, FALSE = move
    BOOL FastMoveUsed;     // was at least one file or directory renamed? then showing total moved size is pointless
    BOOL ChangeSpeedLimit; // TRUE if the speed limit might change (worker should reach a state where this is easy)

    BOOL SkipAllCountSizeErrors; // skip all remaining count-size errors?

    char WorkPath1[MAX_PATH];  // if not empty, the first path on which work was performed (used for change reports)
    BOOL WorkPath1InclSubDirs; // TRUE/FALSE = include/exclude subdirectories (first path)
    char WorkPath2[MAX_PATH];  // if not empty, the second path on which work was performed (used for change reports)
    BOOL WorkPath2InclSubDirs; // TRUE/FALSE = include/exclude subdirectories (second path)

    char* WaitInQueueSubject; // text for the "waiting in queue" state: dialog caption
    char* WaitInQueueFrom;    // text for the "waiting in queue" state: top line (From)
    char* WaitInQueueTo;      // text for the "waiting in queue" state: bottom line (To)

private:
    // for the status line in the progress dialog (Copy/Move only)
    CRITICAL_SECTION StatusCS;              // critical section guarding TransferSpeedMeter and ProgressSpeedMeter
    CTransferSpeedMeter TransferSpeedMeter; // meter measuring data transfer (Read/WriteFile)
    CProgressSpeedMeter ProgressSpeedMeter; // meter computing "time left" (also measures directory creation, copying empty files, etc.; uses same operation sizes as the progress bar)
    CQuadWord TransferredFileSize;          // number of bytes actually copied/transferred so far (final sum should match TotalFileSize unless data change on disk)
    CQuadWord ProgressSize;                 // progress expressed in copied/transferred "bytes" (uses the same operation sizes as the progress bar)

    // speed-limit data, used only inside StatusCS
    BOOL UseSpeedLimit;             // TRUE = speed limit is active
    DWORD SpeedLimit;               // speed-limit value in bytes per second, WARNING: must never be zero
    DWORD SleepAfterWrite;          // how many ms to wait after a packet of size LastBufferLimit; -1 means it must be computed (after the first packet)
    int LastBufferLimit;            // packet size, WARNING: must never be zero
    DWORD LastSetupTime;            // GetTickCount() when speed-limit parameters were last computed and possibly throttled
    CQuadWord BytesTrFromLastSetup; // bytes transferred since LastSetupTime

    // async copies only: buffer limit data (progress must move, buffer must not be too large), used only inside StatusCS
    BOOL UseProgressBufferLimit;  // TRUE if the buffer limit for async copies should be used
    DWORD ProgressBufferLimit;    // buffer size limit for copying, keeps progress updates reasonably frequent
    DWORD LastProgBufLimTestTime; // GetTickCount() when ProgressBufferLimit was last checked
    DWORD LastFileBlockCount;     // how many blocks have been copied from the last file (protected against overflow; a value > 1000000 means "a lot")
    DWORD LastFileStartTime;      // GetTickCount() when copying of the last file started

public:
    COperations(int base, int delta, char* waitInQueueSubject, char* waitInQueueFrom, char* waitInQueueTo);
    ~COperations() { HANDLES(DeleteCriticalSection(&StatusCS)); }

    void SetWorkPath1(const char* path, BOOL inclSubDirs)
    {
        lstrcpyn(WorkPath1, path, MAX_PATH);
        WorkPath1InclSubDirs = inclSubDirs;
    }

    void SetWorkPath2(const char* path, BOOL inclSubDirs)
    {
        lstrcpyn(WorkPath2, path, MAX_PATH);
        WorkPath2InclSubDirs = inclSubDirs;
    }

    void SetTFS(const CQuadWord& TFS);
    void SetTFSandProgressSize(const CQuadWord& TFS, const CQuadWord& pSize,
                               int* limitBufferSize = NULL, int bufferSize = 0);
    void AddBytesToSpeedMetersAndTFSandPS(DWORD bytesCount, BOOL onlyToProgressSpeedMeter,
                                          int bufferSize, int* limitBufferSize = NULL,
                                          DWORD maxPacketSize = 0);
    void GetNewBufSize(int* limitBufferSize, int bufferSize);
    void AddBytesToTFSandSetProgressSize(const CQuadWord& bytesCount, const CQuadWord& pSize);
    void AddBytesToTFS(const CQuadWord& bytesCount);
    void GetTFS(CQuadWord* TFS);
    void GetTFSandResetTrSpeedIfNeeded(CQuadWord* TFS);
    void SetProgressSize(const CQuadWord& pSize);
    void CalcLimitBufferSize(int* limitBufferSize, int bufferSize);

    void EnableProgressBufferLimit(BOOL useProgressBufferLimit);
    void SetFileStartParams();

    void GetStatus(CQuadWord* transferredFileSize, CQuadWord* transferSpeed,
                   CQuadWord* progressSize, CQuadWord* progressSpeed,
                   BOOL* useSpeedLimit, DWORD* speedLimit);
    void InitSpeedMeters(BOOL operInProgress);
    BOOL GetTFSandProgressSize(CQuadWord* transferredFileSize, CQuadWord* progressSize);

    void SetSpeedLimit(BOOL useSpeedLimit, DWORD speedLimit);
    void GetSpeedLimit(BOOL* useSpeedLimit, DWORD* speedLimit);
};

class COperationsQueue // queue of disk Copy/Move operations
{
protected:
    CRITICAL_SECTION QueueCritSect; // object's critical section

    // arrays OperDlgs and OperPaused have the same number of items and indices (one index per operation)
    TDirectArray<HWND> OperDlgs;    // HWND array: operation dialogs in the queue
    TDirectArray<DWORD> OperPaused; // int array: state of each operation: 2/1/0 = "manually-paused"/"auto-paused"/"running"

public:
    COperationsQueue() : OperDlgs(5, 10), OperPaused(5, 10)
    {
        HANDLES(InitializeCriticalSection(&QueueCritSect));
    }
    ~COperationsQueue()
    {
        if (OperDlgs.Count > 0 || OperPaused.Count > 0)
            TRACE_E("~COperationsQueue(): unexpected situation: operation queue is not empty!");
        HANDLES(DeleteCriticalSection(&QueueCritSect));
    }

    // adds an operation to the queue; returns TRUE on success, otherwise adding failed (out of memory)
    // 'dlg' is the handle of the operation dialog; 'startOnIdle' is TRUE when it should start
    // once nothing else is running; 'startPaused' (must not be NULL) returns TRUE if
    // the added operation should start in "paused" mode, otherwise it starts in "running" mode
    BOOL AddOperation(HWND dlg, BOOL startOnIdle, BOOL* startPaused);

    // removes an operation from the queue once it finishes; if 'doNotResume' is FALSE it posts
    // a "resume" to the first operation in the queue when all queued operations are paused;
    // if 'foregroundWnd' is not NULL it receives the handle of the dialog that should be
    // activated (value remains unchanged when no activation is needed)
    void OperationEnded(HWND dlg, BOOL doNotResume, HWND* foregroundWnd);

    // sets operation 'dlg' state to 'paused' (2/1/0 = "manually-paused"/"auto-paused"/"running")
    void SetPaused(HWND dlg, BOOL paused);

    // moves operation 'dlg' to the end of the list and sets it to "auto-paused"
    void AutoPauseOperation(HWND dlg, HWND* foregroundWnd);

    // returns TRUE when no operation is in the queue
    BOOL IsEmpty();

    // returns the current number of operations in the queue
    int GetNumOfOperations();
};

extern COperationsQueue OperationsQueue; // queue of disk Copy/Move operations

HANDLE StartWorker(COperations* script, HWND hDlg, CChangeAttrsData* attrsData,
                   CConvertData* convertData, HANDLE wContinue, HANDLE workerNotSuspended,
                   BOOL* cancelWorker, int* operationProgress, int* summaryProgress);

void FreeScript(COperations* script);

//
// File information classes and Io Status block (see NTDDK.H)
//
typedef enum _FILE_INFORMATION_CLASS
{
    FileDirectoryInformation = 1,
    FileFullDirectoryInformation, // 2
    FileBothDirectoryInformation, // 3
    FileBasicInformation,         // 4  wdm
    FileStandardInformation,      // 5  wdm
    FileInternalInformation,      // 6
    FileEaInformation,            // 7
    FileAccessInformation,        // 8
    FileNameInformation,          // 9
    FileRenameInformation,        // 10
    FileLinkInformation,          // 11
    FileNamesInformation,         // 12
    FileDispositionInformation,   // 13
    FilePositionInformation,      // 14 wdm
    FileFullEaInformation,        // 15
    FileModeInformation,          // 16
    FileAlignmentInformation,     // 17
    FileAllInformation,           // 18
    FileAllocationInformation,    // 19
    FileEndOfFileInformation,     // 20 wdm
    FileAlternateNameInformation, // 21
    FileStreamInformation,        // 22
    FilePipeInformation,          // 23
    FilePipeLocalInformation,     // 24
    FilePipeRemoteInformation,    // 25
    FileMailslotQueryInformation, // 26
    FileMailslotSetInformation,   // 27
    FileCompressionInformation,   // 28
    FileObjectIdInformation,      // 29
    FileCompletionInformation,    // 30
    FileMoveClusterInformation,   // 31
    FileQuotaInformation,         // 32
    FileReparsePointInformation,  // 33
    FileNetworkOpenInformation,   // 34
    FileAttributeTagInformation,  // 35
    FileTrackingInformation,      // 36
    FileMaximumInformation
} FILE_INFORMATION_CLASS,
    *PFILE_INFORMATION_CLASS;

typedef struct _IO_STATUS_BLOCK
{
    union
    {
        NTSTATUS Status;
        PVOID Pointer;
    };
    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef NTSTATUS(__stdcall* NTQUERYINFORMATIONFILE)(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass);

extern NTQUERYINFORMATIONFILE DynNtQueryInformationFile;

typedef VOID(__stdcall* PIO_APC_ROUTINE)(
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved);

typedef NTSTATUS(__stdcall* NTFSCONTROLFILE)(
    _In_ HANDLE FileHandle,
    _In_opt_ HANDLE Event,
    _In_opt_ PIO_APC_ROUTINE ApcRoutine,
    _In_opt_ PVOID ApcContext,
    _Out_ PIO_STATUS_BLOCK IoStatusBlock,
    _In_ ULONG FsControlCode,
    _In_opt_ PVOID InputBuffer,
    _In_ ULONG InputBufferLength,
    _Out_opt_ PVOID OutputBuffer,
    _In_ ULONG OutputBufferLength);

extern NTFSCONTROLFILE DynNtFsControlFile;

//
// MessageId: STATUS_BUFFER_OVERFLOW
//
// MessageText:
//
//  {Buffer Overflow}
//  The data was too large to fit into the specified buffer.
//
#define STATUS_BUFFER_OVERFLOW ((NTSTATUS)0x80000005L)

#pragma pack(4)
typedef struct
{
    ULONG NextEntry;
    ULONG NameLength;
    LARGE_INTEGER Size;
    LARGE_INTEGER AllocationSize;
    USHORT Name[1];
} FILE_STREAM_INFORMATION, *PFILE_STREAM_INFORMATION;
#pragma pack()

// queries alternate data streams (ADS) of a file or directory ('isDir' is FALSE/TRUE)
// 'fileName' should only be used on NTFS volumes; if 'adsSize' is not NULL it returns
// the total size of all ADS; if 'streamNames' is not NULL it returns an allocated array
// of Unicode names of all ADS (except the default ADS) - the caller must free each
// element and the array itself. The array is returned only when no error occurred
// (see 'lowMemory' and 'winError') and ADS were found (the function returns TRUE).
// If 'streamNamesCount' is not NULL it receives the number of items in 'streamNames'.
// If 'lowMemory' is not NULL it becomes TRUE when a memory shortage occurs (it can
// return TRUE only if 'streamNames' is not NULL). If 'winError' is not NULL it
// receives the Windows error code (NO_ERROR if none; if a Windows error occurs the
// function always returns FALSE). The function returns TRUE if the file/directory
// contains any ADS, otherwise FALSE. 'bytesPerCluster' is the cluster size used to
// compute disk space occupied by the ADS (0 = unknown size). If 'adsOccupiedSpace'
// is not NULL it returns the disk space used by the ADS. If 'onlyDiscardableStreams'
// is not NULL it returns TRUE when only discardable streams were found (currently
// thumbnails from Windows 2000).
BOOL CheckFileOrDirADS(const char* fileName, BOOL isDir, CQuadWord* adsSize, wchar_t*** streamNames,
                       int* streamNamesCount, BOOL* lowMemory, DWORD* winError,
                       DWORD bytesPerCluster, CQuadWord* adsOccupiedSpace,
                       BOOL* onlyDiscardableStreams);
