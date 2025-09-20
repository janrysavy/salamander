// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// helper functions for registry access that don't display LOAD or SAVE warnings on errors
BOOL OpenKeyAux(HWND parent, HKEY hKey, const char* name, HKEY& openedKey, BOOL quiet = TRUE);
BOOL CreateKeyAux(HWND parent, HKEY hKey, const char* name, HKEY& createdKey, BOOL quiet = TRUE);
BOOL GetValueAux(HWND parent, HKEY hKey, const char* name, DWORD type, void* buffer,
                 DWORD bufferSize, BOOL quiet = TRUE);
BOOL SetValueAux(HWND parent, HKEY hKey, const char* name, DWORD type,
                 const void* data, DWORD dataSize, BOOL quiet = TRUE);
BOOL DeleteValueAux(HKEY hKey, const char* name);
BOOL ClearKeyAux(HKEY key);
void CloseKeyAux(HKEY hKey);
BOOL DeleteKeyAux(HKEY hKey, const char* name);
// does not check the type, so REG_DWORD will be loaded the same as 4-byte REG_BINARY
BOOL GetValueDontCheckTypeAux(HKEY hKey, const char* name, void* buffer, DWORD bufferSize);

enum CRegistryWorkType
{
    rwtNone,
    rwtStopWorker, // maintenance task: terminate the thread
    rwtClearKey,
    rwtCreateKey,
    rwtOpenKey,
    rwtCloseKey,
    rwtDeleteKey,
    rwtGetValue,
    rwtGetValue2,
    rwtSetValue,
    rwtDeleteValue,
    rwtGetSize,
};

class CRegistryWorkerThread
{
protected:
    class CInUseHandler
    {
    protected:
        CRegistryWorkerThread* T;

    public:
        CInUseHandler() { T = NULL; }
        ~CInUseHandler();
        BOOL CanUseThread(CRegistryWorkerThread* t);
        void ResetT() { T = NULL; }
    };

    HANDLE Thread;           // registry worker thread handle
    DWORD OwnerTID;          // TID of the thread that started the worker thread (no one else may terminate it)
    BOOL InUse;              // TRUE = already performing some work, further work runs without the thread (handles recursion; usage from other threads is refused, see OwnerTID)
    int StopWorkerSkipCount; // how many StopThread() calls in OwnerTID thread to ignore (number of recursive StartThread() calls)

    HANDLE WorkReady; // signaled: the thread has data ready to process (main thread waits for completion and runs the message loop)
    HANDLE WorkDone;  // signaled: the thread finished the work (main thread may continue)

    CRegistryWorkType WorkType;
    BOOL LastWorkSuccess;
    HKEY Key;
    const char* Name;
    HKEY OpenedKey;
    DWORD ValueType;
    DWORD ValueType2;
    DWORD* ReturnedValueType;
    void* Buffer;
    DWORD BufferSize;
    const void* Data;
    DWORD DataSize;

public:
    CRegistryWorkerThread();
    ~CRegistryWorkerThread();

    // starts the registry worker thread; returns success
    BOOL StartThread();

    // terminates the registry worker thread
    void StopThread();

    // clears key 'key' of all subkeys and values; returns success
    BOOL ClearKey(HKEY key);

    // creates or opens existing subkey 'name' of 'key', returns 'createdKey' and success;
    // the returned key ('createdKey') must be closed using CloseKey
    BOOL CreateKey(HKEY key, const char* name, HKEY& createdKey);

    // opens an existing subkey 'name' of 'key', returns 'openedKey' and success
    // the returned key ('openedKey') must be closed with CloseKey
    BOOL OpenKey(HKEY key, const char* name, HKEY& openedKey);

    // closes a key opened via OpenKey or CreateKey
    void CloseKey(HKEY key);

    // deletes subkey 'name' of 'key'; returns success
    BOOL DeleteKey(HKEY key, const char* name);

    // loads value 'name' of type 'type' into 'buffer' of size 'bufferSize' from key 'key'; returns success
    BOOL GetValue(HKEY key, const char* name, DWORD type, void* buffer, DWORD bufferSize);

    // loads value 'name' with 'type1 || type2' into 'returnedType' and 'buffer' of size 'bufferSize' from key 'key'; returns success
    BOOL GetValue2(HKEY hKey, const char* name, DWORD type1, DWORD type2, DWORD* returnedType, void* buffer, DWORD bufferSize);

    // stores value 'name' of type 'type' and data 'data' (size 'dataSize') into key 'key';
    // for strings you can pass 'dataSize' == -1 to compute the length using strlen,
    // returns success
    BOOL SetValue(HKEY key, const char* name, DWORD type, const void* data, DWORD dataSize);

    // deletes the value 'name' from key 'key'; returns success
    BOOL DeleteValue(HKEY key, const char* name);

    // retrieves into 'bufferSize' the required size for value 'name' of type 'type' in key 'key'; returns success
    BOOL GetSize(HKEY key, const char* name, DWORD type, DWORD& bufferSize);

protected:
    // waits for the work to finish while pumping the message loop
    void WaitForWorkDoneWithMessageLoop();

    // thread body where all work is performed
    unsigned Body();

    static DWORD WINAPI ThreadBody(void* param); // helper for thread body
    static unsigned ThreadBodyFEH(void* param);  // helper for thread body (FEH variant)
};

extern CRegistryWorkerThread RegistryWorkerThread;
