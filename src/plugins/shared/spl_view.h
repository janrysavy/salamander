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

#ifdef _MSC_VER
#pragma pack(push, enter_include_spl_view) // so that the structures are independent of the configured alignment
#pragma pack(4)
#endif // _MSC_VER
#ifdef __BORLANDC__
#pragma option -a4
#endif // __BORLANDC__

struct CSalamanderPluginViewerData;

//
// ****************************************************************************
// CPluginInterfaceForViewerAbstract
//

class CPluginInterfaceForViewerAbstract
{
#ifdef INSIDE_SALAMANDER
private: // protection against incorrect direct calls of methods (see CPluginInterfaceForViewerEncapsulation)
    friend class CPluginInterfaceForViewerEncapsulation;
#else  // INSIDE_SALAMANDER
public:
#endif // INSIDE_SALAMANDER

    // function for "file viewer", called when there is a request to open the viewer and load a file
    // 'name', 'left'+'top'+'width'+'height'+'showCmd'+'alwaysOnTop' provide the recommended window placement
    // if 'returnLock' is FALSE, 'lock'+'lockOwner' have no meaning; if 'returnLock'
    // is TRUE, the viewer should return the system event 'lock' in the nonsignaled state; the 'lock' switches
    // to the signaled state when viewing the file 'name' ends (the file is deleted from the temporary directory
    // at that moment). It should also return TRUE in 'lockOwner' if the 'lock' object is to be closed by
    // the caller (FALSE means that the viewer releases the 'lock' itself—in that case the viewer must use
    // the method CSalamanderGeneralAbstract::UnlockFileInCache to transition 'lock' to the signaled state);
    // if the viewer does not set 'lock' (it remains NULL) the file 'name' is valid only until this ViewFile
    // method finishes; if 'viewerData' is not NULL, the viewer receives extended parameters (see
    // CSalamanderGeneralAbstract::ViewFileInPluginViewer); 'enumFilesSourceUID' is the UID of the source (panel
    // or Find window) from which the viewer is opened; if it is -1, the source is unknown (for example archives,
    // file systems, or Alt+F11, etc.)—see for instance CSalamanderGeneralAbstract::GetNextFileNameForViewer;
    // 'enumFilesCurrentIndex' is the index of the file being opened in that source (panel or Find window); if it is -1,
    // the source or index is unknown; returns TRUE on success (FALSE means failure, so ignore both 'lock' and
    // 'lockOwner' in that case)
    virtual BOOL WINAPI ViewFile(const char* name, int left, int top, int width, int height,
                                 UINT showCmd, BOOL alwaysOnTop, BOOL returnLock, HANDLE* lock,
                                 BOOL* lockOwner, CSalamanderPluginViewerData* viewerData,
                                 int enumFilesSourceUID, int enumFilesCurrentIndex) = 0;

    // function for "file viewer", called when there is a request to open the viewer and load the file
    // 'name'; this function should not show any "invalid file format" windows; those windows
    // are shown only when ViewFile of this interface is called; it checks whether
    // the file 'name' can be displayed in the viewer (for example, whether the file has a matching signature)
    // and if it can, it returns TRUE; if it returns FALSE, Salamander tries to find another
    // viewer for 'name' (in the priority list of viewers, see the Viewers configuration page)
    virtual BOOL WINAPI CanViewFile(const char* name) = 0;
};

#ifdef _MSC_VER
#pragma pack(pop, enter_include_spl_view)
#endif // _MSC_VER
#ifdef __BORLANDC__
#pragma option -a
#endif // __BORLANDC__
