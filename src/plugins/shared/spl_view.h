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
#pragma pack(push, enter_include_spl_view) // so that the structures are independent of the current alignment setting
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
private: // protection against incorrect direct method calls (see CPluginInterfaceForViewerEncapsulation)
    friend class CPluginInterfaceForViewerEncapsulation;
#else  // INSIDE_SALAMANDER
public:
#endif // INSIDE_SALAMANDER

    // function for the "file viewer", called when a request is made to open the viewer and load a file
    // 'name', 'left'+'top'+'width'+'height'+'showCmd'+'alwaysOnTop' specify the recommended window placement;
    // if 'returnLock' is FALSE, 'lock'+'lockOwner' have no meaning; if 'returnLock'
    // is TRUE, the viewer should return the system event 'lock' in the nonsignaled state; 'lock'
    // switches to the signaled state when viewing the file 'name' ends (the file is removed
    // from the temporary directory at that moment). It should also return TRUE in 'lockOwner' if
    // the 'lock' object is to be closed by the caller (FALSE means that the viewer releases 'lock'
    // itself - in that case the viewer must use the method CSalamanderGeneralAbstract::UnlockFileInCache
    // to switch 'lock' to the signaled state);
    // if the viewer does not set 'lock' (it remains NULL), the file 'name' is valid only until this
    // ViewFile method returns; if 'viewerData' is not NULL, extended viewer parameters are passed (see
    // CSalamanderGeneralAbstract::ViewFileInPluginViewer); 'enumFilesSourceUID' is the UID of the source (panel
    // or Find window) from which the viewer is opened; if it is -1, the source is unknown (for example archives,
    // file systems, or Alt+F11, etc.) - see for example CSalamanderGeneralAbstract::GetNextFileNameForViewer;
    // 'enumFilesCurrentIndex' is the index of the file being opened in that source (panel or Find window); if it is -1,
    // the source or index is unknown; returns TRUE on success (FALSE means failure; 'lock' and
    // 'lockOwner' have no meaning in that case)
    virtual BOOL WINAPI ViewFile(const char* name, int left, int top, int width, int height,
                                 UINT showCmd, BOOL alwaysOnTop, BOOL returnLock, HANDLE* lock,
                                 BOOL* lockOwner, CSalamanderPluginViewerData* viewerData,
                                 int enumFilesSourceUID, int enumFilesCurrentIndex) = 0;

    // function for the "file viewer", called when a request is made to open the viewer and load the file
    // 'name'; this function should not display any "invalid file format" dialogs; such
    // dialogs are displayed only when the ViewFile method of this interface is called; it determines whether
    // the file 'name' can be displayed in the viewer (e.g. the file has the appropriate signature)
    // and if so, returns TRUE; if it returns FALSE, Salamander tries to find another
    // viewer for 'name' (in the viewer priority list, see the Viewers configuration page)
    virtual BOOL WINAPI CanViewFile(const char* name) = 0;
};

#ifdef _MSC_VER
#pragma pack(pop, enter_include_spl_view)
#endif // _MSC_VER
#ifdef __BORLANDC__
#pragma option -a
#endif // __BORLANDC__
