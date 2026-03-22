// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

//****************************************************************************
//
// Copyright (c) 2023 Open Salamander Authors
//
// This is a part of the Open Salamander SDK library.
//
//****************************************************************************

#include "precomp.h"
//#include <windows.h>
#ifdef _MSC_VER
#include <crtdbg.h>
#endif // _MSC_VER
#include <limits.h>
#include <process.h>
//#include <commctrl.h>
#include <ostream>

#if defined(_DEBUG) && defined(_MSC_VER) // under _MSC_VER, without passing file+line to 'new' operator, the memory leak list shows only 'crtdbg.h(552)'
#define new new (_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#include "spl_com.h"
#include "spl_base.h"
#include "dbg.h"
#include "auxtools.h"

//
// ****************************************************************************
// CThreadQueue
//

CThreadQueue::CThreadQueue(const char* queueName)
{
    QueueName = queueName;
    Head = NULL;
    Continue = CreateEvent(NULL, FALSE, FALSE, NULL);
}

CThreadQueue::~CThreadQueue()
{
    ClearFinishedThreads(); // no need for a critical section; only one thread should be using it now
    if (Continue != NULL)
        CloseHandle(Continue);
    if (Head != NULL)
        TRACE_E("Some thread is still in " << QueueName << " queue!"); // after terminating a thread that waits for (or is terminating) another thread from the queue; otherwise it should not happen...
}

void CThreadQueue::ClearFinishedThreads()
{
    CThreadQueueItem* last = NULL;
    CThreadQueueItem* act = Head;
    while (act != NULL)
    {
        DWORD ec;
        if (act->Locks == 0 && (!GetExitCodeThread(act->Thread, &ec) || ec != STILL_ACTIVE))
        { // this entry is unlocked and the thread has already finished, so remove it from the list
            if (last != NULL)
                last->Next = act->Next;
            else
                Head = act->Next;
            CloseHandle(act->Thread);
            delete act;
            act = (last != NULL ? last->Next : Head);
        }
        else
        {
            last = act;
            act = act->Next;
        }
    }
}

BOOL CThreadQueue::Add(CThreadQueueItem* item)
{
    // first remove threads that have already finished
    ClearFinishedThreads();

    // add a new thread
    if (item != NULL)
    {
        item->Next = Head;
        Head = item;
        return TRUE;
    }
    return FALSE;
}

BOOL CThreadQueue::FindAndLockItem(HANDLE thread)
{
    CS.Enter();

    CThreadQueueItem* act = Head; // try to find an open handle to the thread
    while (act != NULL)
    {
        if (act->Thread == thread)
        {
            act->Locks++;
            break;
        }
        act = act->Next;
    }

    CS.Leave();

    return act != NULL; // NULL = not found
}

void CThreadQueue::UnlockItem(HANDLE thread, BOOL deleteIfUnlocked)
{
    CS.Enter();

    CThreadQueueItem* last = NULL;
    CThreadQueueItem* act = Head; // try to find the open thread handle
    while (act != NULL)
    {
        if (act->Thread == thread)
            break;
        last = act;
        act = act->Next;
    }
    if (act != NULL) // always true (it was locked, so it could not be deleted)
    {
        if (act->Locks <= 0)
            TRACE_E("CThreadQueue::UnlockItem(): thread has not locks!");
        else
        {
            if (--(act->Locks) == 0 && deleteIfUnlocked) // the thread is no longer locked and its record should be deleted if deleteIfUnlocked is set
            {
                if (last != NULL)
                    last->Next = act->Next;
                else
                    Head = act->Next;
                CloseHandle(act->Thread);
                delete act;
            }
        }
    }
    else
        TRACE_E("CThreadQueue::UnlockItem(): unable to find thread!"); // wasn't it locked, so it could not have been deleted?

    CS.Leave();
}

BOOL CThreadQueue::WaitForExit(HANDLE thread, int milliseconds)
{
    CALL_STACK_MESSAGE2("CThreadQueue::WaitForExit(, %d)", milliseconds);
    BOOL ret = TRUE;
    if (thread != NULL)
    {
        if (FindAndLockItem(thread)) // thread handle found and locked — it is safe to wait and then remove it
        {
            ret = WaitForSingleObject(thread, milliseconds) != WAIT_TIMEOUT;

            UnlockItem(thread, ret);
        }
    }
    else
        TRACE_E("CThreadQueue::WaitForExit(): Nothing to wait for (parameter 'thread'==NULL)!");
    return ret;
}

void CThreadQueue::KillThread(HANDLE thread, DWORD exitCode)
{
    CALL_STACK_MESSAGE2("CThreadQueue::KillThread(, %d)", exitCode);
    if (thread != NULL)
    {
        if (FindAndLockItem(thread)) // thread handle found and locked - we can terminate the thread and then remove the item
        {
            TerminateThread(thread, exitCode);
            WaitForSingleObject(thread, INFINITE); // wait until the thread really ends; sometimes it takes quite a while

            UnlockItem(thread, TRUE);
        }
    }
    else
        TRACE_E("CThreadQueue::KillThread(): Nothing to kill (parameter 'thread'==NULL)!");
}

BOOL CThreadQueue::KillAll(BOOL force, int waitTime, int forceWaitTime, DWORD exitCode)
{
    CALL_STACK_MESSAGE5("CThreadQueue::KillAll(%d, %d, %d, %d)", force, waitTime, forceWaitTime, exitCode);
    DWORD ti = GetTickCount();
    DWORD w = force ? forceWaitTime : waitTime;

    CS.Enter();

    // terminate every thread that is not going to exit on its own
    CThreadQueueItem* prevItem = NULL;
    CThreadQueueItem* item = Head;
    while (item != NULL)
    {
        BOOL leaveCS = FALSE;
        DWORD ec;
        if (GetExitCodeThread(item->Thread, &ec) && ec == STILL_ACTIVE)
        { // the thread is probably still running
            DWORD t = GetTickCount() - ti;
            if (w == INFINITE || t < w) // we should keep waiting
            {
                // release the queue for other threads (so they can, for example, wait for a thread from the queue to terminate and then terminate themselves)
                CS.Leave();

                if (w == INFINITE || 50 < w - t)
                    Sleep(50);
                else
                {
                    Sleep(w - t);
                    ti -= w; // skip the next wait-interval check
                }

                CS.Enter();
                item = Head;
                prevItem = NULL;
                continue; // restart the iteration (re-evaluates the loop condition)
            }
            if (force) // terminate the thread forcibly
            {
                TRACE_E("Thread has not ended itself, we must terminate it (" << QueueName << " queue).");
                TerminateThread(item->Thread, exitCode);
                WaitForSingleObject(item->Thread, INFINITE); // wait until the thread really ends; sometimes it takes a while
                // if another thread is waiting for the one we just terminated, release the queue
                // for a moment so it does not get stuck inside UnlockItem()
                leaveCS = item->Locks > 0;
            }
            else // without 'force' we merely report that something is still running
            {
                TRACE_I("KillAll(): At least one thread is still running in " << QueueName << " queue.");
                ClearFinishedThreads(); // keep the debugging output tidy
                CS.Leave();
                return FALSE;
            }
        }
        CThreadQueueItem* delItem = item;
        item = item->Next;
        if (delItem->Locks == 0) // the handle can be closed, delete the item
        {
            if (Head == delItem)
                Head = item;
            else
                prevItem->Next = item;
            CloseHandle(delItem->Thread);
            delete delItem;
        }
        else
            prevItem = delItem; // we must leave the handle alone, so the list item stays

        if (leaveCS)
        {
            // release the queue so other threads can, for example, wait for a thread from the queue to finish and then exit
            CS.Leave();

            Sleep(50); // give it a moment for the queue to take over and possibly for the thread to finish before we terminate it like all the others

            CS.Enter();
            item = Head;
            prevItem = NULL;
            continue; // restart the iteration (re-evaluates the loop condition)
        }
    }

    CS.Leave();
    return TRUE;
}

struct CThreadBaseData
{
    unsigned(WINAPI* Body)(void*);
    void* Param;
    HANDLE Continue;
};

DWORD WINAPI
CThreadQueue::ThreadBase(void* param)
{
    CThreadBaseData* d = (CThreadBaseData*)param;

    // copy the WINAPI threadBody pointer and threadParam to the stack ('d' stops being valid after 'Continue')
    unsigned(WINAPI * threadBody)(void*) = d->Body;
    void* threadParam = d->Param;

    SetEvent(d->Continue); // let the main thread continue
    d = NULL;

    // start our thread
    return SalamanderDebug->CallWithCallStack(threadBody, threadParam);
}

HANDLE
CThreadQueue::StartThread(unsigned(WINAPI* body)(void*), void* param, unsigned stack_size,
                          HANDLE* threadHandle, DWORD* threadID)
{
    CALL_STACK_MESSAGE2("CThreadQueue::StartThread(, , %d, ,)", stack_size);
    if (threadHandle != NULL)
        *threadHandle = NULL;
    if (threadID != NULL)
        *threadID = 0;
    if (Continue == NULL)
    {
        TRACE_E("Unable to start thread, because Continue event was not created.");
        return NULL;
    }

    CS.Enter();

    CThreadBaseData data;
    data.Body = body;
    data.Param = param;
    data.Continue = Continue;

    // start the thread; we do not call _beginthreadex(), because starting with VC2015 it loads this module (plugin) again
    // and releases it only during a normal shutdown; when we call TerminateThread(), the module stays loaded until the
    // Salamander process terminates, and only then are the destructors of global objects run, which can cause unexpected
    // crashes because all plugin interfaces are already released (for example,
    // SalamanderDebug)
    DWORD tid;
    HANDLE thread = CreateThread(NULL, stack_size, CThreadQueue::ThreadBase, &data, CREATE_SUSPENDED, &tid);
    if (thread == NULL)
    {
        TRACE_E("Unable to start thread.");

        CS.Leave();

        return NULL;
    }
    else
    {
        // add the thread to this plugin's queue
        if (!Add(new CThreadQueueItem(thread, tid)))
        {
            TRACE_E("Unable to add thread to the queue.");
            TerminateThread(thread, 666);          // it is suspended, so it will not be in any critical section, etc.
            WaitForSingleObject(thread, INFINITE); // wait until the thread really ends; sometimes it takes quite a while
            CloseHandle(thread);

            CS.Leave();

            return NULL;
        }

        // assign while the thread is still suspended (ensures it has not finished and its queue entry has not been deleted yet)
        if (threadHandle != NULL)
            *threadHandle = thread;
        if (threadID != NULL)
            *threadID = tid;

        SalamanderDebug->TraceAttachThread(thread, tid);
        ResumeThread(thread);

        WaitForSingleObject(Continue, INFINITE); // wait until CThreadQueue::ThreadBase receives the data

        CS.Leave();

        return thread;
    }
}

//
// ****************************************************************************
// CThread
//

CThread::CThread(const char* name)
{
    if (name != NULL)
        lstrcpyn(Name, name, 101);
    else
        Name[0] = 0;
    Thread = NULL;
}

unsigned WINAPI
CThread::UniversalBody(void* param)
{
    CThread* thread = (CThread*)param;
    CALL_STACK_MESSAGE2("CThread::UniversalBody(thread name = \"%s\")", thread->Name);
    SalamanderDebug->SetThreadNameInVCAndTrace(thread->Name);

    unsigned ret = thread->Body(); // run the actual thread body

    delete thread; // delete the thread object
    return ret;
}

HANDLE
CThread::Create(CThreadQueue& queue, unsigned stack_size, DWORD* threadID)
{
    // WARNING: StartThread() may destroy 'this', so the assignment to 'Thread' happens inside that call
    return queue.StartThread(UniversalBody, this, stack_size, &Thread, threadID);
}
