// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

class CFilesWindow;

extern HANDLE RefreshFinishedEvent;
extern int SnooperSuspended;

void AddDirectory(CFilesWindow* win, const char* path, BOOL registerDevNotification);                           // add a directory for the snooper to monitor
void ChangeDirectory(CFilesWindow* win, const char* newPath, BOOL registerDevNotification);                     // switch the monitored directory
void DetachDirectory(CFilesWindow* win, BOOL waitForHandleClosure = FALSE, BOOL closeDevNotifification = TRUE); // stop monitoring the directory

BOOL InitializeThread();
void TerminateThread();

void BeginSuspendMode(BOOL debugDoNotTestCaller = FALSE);
void EndSuspendMode(BOOL debugDoNotTestCaller = FALSE);

typedef TDirectArray<CFilesWindow*> CWindowArray; // (CFilesWindow *)
typedef TDirectArray<HANDLE> CObjectArray;        // (HANDLE)

extern CWindowArray WindowArray; // arrays share the same indices
extern CObjectArray ObjectArray; // the object handle associated with MainWindow
