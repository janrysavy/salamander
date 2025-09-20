// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

class CFilesWindow;

extern HANDLE RefreshFinishedEvent;
extern int SnooperSuspended;

void AddDirectory(CFilesWindow* win, const char* path, BOOL registerDevNotification);                           // new directory for the snooper
void ChangeDirectory(CFilesWindow* win, const char* newPath, BOOL registerDevNotification);                     // change the watched directory
void DetachDirectory(CFilesWindow* win, BOOL waitForHandleClosure = FALSE, BOOL closeDevNotifification = TRUE); // stop watching the directory

BOOL InitializeThread();
void TerminateThread();

void BeginSuspendMode(BOOL debugDoNotTestCaller = FALSE);
void EndSuspendMode(BOOL debugDoNotTestCaller = FALSE);

typedef TDirectArray<CFilesWindow*> CWindowArray; // (CFilesWindow *)
typedef TDirectArray<HANDLE> CObjectArray;        // (HANDLE)

extern CWindowArray WindowArray; // arrays with matching indices
extern CObjectArray ObjectArray; // MainWindow belongs to ObjectHandle
