// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// helper functions for convenient work with the Registry that avoid LOAD/SAVE configuration messages on errors
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
// does not check the type, so it reads REG_DWORD the same as a 4-byte REG_BINARY
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

    HANDLE Thread;           // registry worker thread
    DWORD OwnerTID;          // TID of the thread that started the worker thread (no one else can stop it)
    BOOL InUse;              // TRUE = already performing some work; further work runs without the thread (handles recursion, use from other threads is rejected, see OwnerTID)
    int StopWorkerSkipCount; // how many StopThread() calls in the OwnerTID thread to ignore (number of recursive StartThread() calls)

    HANDLE WorkReady; // signaled: the thread has data ready for processing (the main thread waits for completion + runs the message loop)
    HANDLE WorkDone;  // signaled: the thread finished the work (the main thread may continue)

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

    // start the registry worker thread; returns TRUE on success
    BOOL StartThread();

    // terminate the registry worker thread
    void StopThread();

    // clear key 'key' of all subkeys and values; returns TRUE on success
    BOOL ClearKey(HKEY key);

    // create or open the existing subkey 'name' of key 'key'; returns the handle in 'createdKey'
    // and TRUE on success. The returned key must be closed with CloseKey.
    BOOL CreateKey(HKEY key, const char* name, HKEY& createdKey);

    // open the existing subkey 'name' of key 'key'; returns the handle in 'openedKey'
    // and TRUE on success. The returned key must be closed with CloseKey.
    BOOL OpenKey(HKEY key, const char* name, HKEY& openedKey);

    // close a key previously obtained via OpenKey or CreateKey
    void CloseKey(HKEY key);

    // delete subkey 'name' of key 'key'; returns TRUE on success
    BOOL DeleteKey(HKEY key, const char* name);

    // load value 'name' of type 'type' into 'buffer' (size 'bufferSize') from key 'key'; returns TRUE on success
    BOOL GetValue(HKEY key, const char* name, DWORD type, void* buffer, DWORD bufferSize);

    // load value 'name' of type 'type1' or 'type2' into 'buffer' (size 'bufferSize') from key 'key';
    // stores the actual type in 'returnedType' and returns TRUE on success
    BOOL GetValue2(HKEY hKey, const char* name, DWORD type1, DWORD type2, DWORD* returnedType, void* buffer, DWORD bufferSize);

    // store value 'name' of type 'type' from 'data' (size 'dataSize') into key 'key'.
    // For strings you can pass 'dataSize' == -1 to derive the size from strlen(). Returns TRUE on success.
    BOOL SetValue(HKEY key, const char* name, DWORD type, const void* data, DWORD dataSize);

    // delete value 'name' from key 'key'; returns TRUE on success
    BOOL DeleteValue(HKEY key, const char* name);

    // retrieve into 'bufferSize' the required size for value 'name' of type 'type' from key 'key'; returns TRUE on success
    BOOL GetSize(HKEY key, const char* name, DWORD type, DWORD& bufferSize);

protected:
    // waits for the work to finish + runs the message loop
    void WaitForWorkDoneWithMessageLoop();

    // thread body - all work takes place here
    unsigned Body();

    static DWORD WINAPI ThreadBody(void* param); // helper function for the thread body
    static unsigned ThreadBodyFEH(void* param);  // helper function for the thread body
};

extern CRegistryWorkerThread RegistryWorkerThread;
