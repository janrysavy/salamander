// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

//****************************************************************************
//
// Copyright (c) 2023 Open Salamander Authors
//
// This is a part of the Open Salamander SDK library.
//
//****************************************************************************

#pragma once

//
// ****************************************************************************
// CThreadQueue
//

struct CThreadQueueItem
{
    HANDLE Thread;
    DWORD ThreadID; // for debugging purposes only (finding the thread in the debugger's thread list)
    int Locks;      // number of locks; if > 0 we must not close 'Thread'
    CThreadQueueItem* Next;

    CThreadQueueItem(HANDLE thread, DWORD tid)
    {
        Thread = thread;
        ThreadID = tid;
        Next = NULL;
        Locks = 0;
    }
};

class CThreadQueue
{
protected:
    const char* QueueName; // queue name (for debugging purposes only)
    CThreadQueueItem* Head;
    HANDLE Continue; // we must wait until the started thread takes the handed-off data

    struct CCS // access from multiple threads -> synchronization required
    {
        CRITICAL_SECTION cs;

        CCS() { InitializeCriticalSection(&cs); }
        ~CCS() { DeleteCriticalSection(&cs); }

        void Enter() { EnterCriticalSection(&cs); }
        void Leave() { LeaveCriticalSection(&cs); }
    } CS;

public:
    CThreadQueue(const char* queueName /* e.g. "DemoPlug Viewers" */);
    ~CThreadQueue();

    // starts function 'body' with parameter 'param' in a newly created thread with a stack
    // of size 'stack_size' (0 = default); returns the thread handle or NULL on error,
    // and also stores the result in 'threadHandle' before the thread is started (resumed)
    // (if it is not NULL); use the returned thread handle only for NULL checks and for calling
    // CThreadQueue methods WaitForExit() and KillThread(); this queue object
    // closes the thread handle
    // WARNING: - the thread may start with a delay, only after StartThread() returns
    //          (if 'param' is a pointer to a structure stored on the stack, it is necessary
    //           to synchronize passing the data from 'param' - the main thread must wait
    //           until the new thread takes over the data)
    //         - the returned thread handle may already be closed if the thread finishes before
    //          StartThread() returns and StartThread() or KillAll() is called from another
    //          thread
    // can be called from any thread
    HANDLE StartThread(unsigned(WINAPI* body)(void*), void* param, unsigned stack_size = 0,
                       HANDLE* threadHandle = NULL, DWORD* threadID = NULL);

    // waits for a thread from this queue to finish; 'thread' is a thread handle that may already
    // be closed (this object closes it when StartThread and KillAll are called); if the thread does
    // finish, it removes it from the queue and closes its handle
    BOOL WaitForExit(HANDLE thread, int milliseconds = INFINITE);

    // kills a thread from this queue (via TerminateThread()); 'thread' is a thread handle
    // that may already be closed (this object closes it when StartThread or KillAll is called);
    // if it finds the thread, it terminates it, removes it from the queue, and closes its handle (the thread object
    // is not deallocated because its state is unknown and possibly inconsistent)
    void KillThread(HANDLE thread, DWORD exitCode = 666);

    // checks whether all threads have finished; if 'force' is TRUE and some thread is still running,
    // it waits 'forceWaitTime' (in ms) for all threads to finish, then terminates the threads still running
    // (their objects are not deallocated because their state is unknown and may be inconsistent);
    // returns TRUE if all threads have finished; with 'force' TRUE it always returns TRUE;
    // if 'force' is FALSE and some thread is still running, it waits 'waitTime' (in ms) for all threads to finish;
    // if something is still running afterwards, it returns FALSE; INFINITE means waiting for an unlimited
    // time
    // can be called from any thread
    BOOL KillAll(BOOL force, int waitTime = 1000, int forceWaitTime = 200, DWORD exitCode = 666);

protected:                                                 // internal non-synchronized methods
    BOOL Add(CThreadQueueItem* item);                      // adds an item to the queue, returns success
    BOOL FindAndLockItem(HANDLE thread);                   // finds the item for 'thread' in the queue and locks it
    void UnlockItem(HANDLE thread, BOOL deleteIfUnlocked); // unlocks the item for 'thread' in the queue, optionally deletes it
    void ClearFinishedThreads();                           // removes threads that have already finished from the queue
    static DWORD WINAPI ThreadBase(void* param);           // generic thread entry point
};

//
// ****************************************************************************
// CThread
//
// WARNING: must be allocated (it is not possible to have CThread on the stack); it deletes itself
//        only when the thread is successfully created by the Create() method

class CThread
{
public:
    // thread handle (NULL = the thread has not started yet / has not run yet), WARNING: after the thread terminates it
    // is closed automatically (becomes invalid); moreover, this object has already been deallocated
    HANDLE Thread;

protected:
    char Name[101]; // buffer for the thread name (TRACE and CALL-STACK use it to identify the thread)
                    // WARNING: if the thread data contains references to the stack or other temporary objects,
                    //         ensure these references are used only while they remain valid

public:
    CThread(const char* name = NULL);
    virtual ~CThread() {} // to ensure destructors of derived classes are called correctly

    // creates (starts) a thread in the thread queue 'queue'; 'stack_size' is the stack size
    // of the new thread in bytes (0 = default); returns the new thread handle or NULL on error;
    // the 'queue' object closes the handle; if the thread is created, this object
    // is deallocated when the thread finishes; if starting the thread fails, the caller must deallocate the object
    // WARNING: without extra synchronization the thread may finish before Create() returns ->
    //         therefore once Create() succeeds the pointer 'this' must be considered invalid,
    //         and the same applies to the returned thread handle (use it only for NULL checks and for calling
    //         the CThreadQueue helpers WaitForExit() and KillThread())
    // can be called from any thread
    HANDLE Create(CThreadQueue& queue, unsigned stack_size = 0, DWORD* threadID = NULL);

    // returns the 'Thread' member as described above
    HANDLE GetHandle() { return Thread; }

    // returns the thread name
    const char* GetName() { return Name; }

    // this method implements the thread body
    virtual unsigned Body() = 0;

protected:
    static unsigned WINAPI UniversalBody(void* param); // helper method for starting threads
};
