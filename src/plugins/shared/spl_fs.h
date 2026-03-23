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
#pragma pack(push, enter_include_spl_fs) // to keep structures independent of the alignment settings
#pragma pack(4)
#endif // _MSC_VER
#ifdef __BORLANDC__
#pragma option -a4
#endif // __BORLANDC__

struct CFileData;
class CPluginFSInterfaceAbstract;
class CSalamanderDirectoryAbstract;
class CPluginDataInterfaceAbstract;

//
// ****************************************************************************
// CSalamanderForViewFileOnFSAbstract
//
// a set of callbacks provided by Salamander to support the ViewFile operation in
// CPluginFSInterfaceAbstract; the interface instance lives only for the duration of the
// method call that received it as a parameter

class CSalamanderForViewFileOnFSAbstract
{
public:
    // finds an existing copy of the file in the disk cache or, if a copy of the file does not yet
    // exist in the disk cache, reserves a name for it (the target file, e.g. for download from FTP);
    // 'uniqueFileName' is the unique name of the original file (the disk cache is searched
    // using this name; the full file name in Salamander format should be sufficient
    // - "fs-name:fs-user-part"; CAUTION: the name is compared case-sensitively; if the
    // plugin requires case-insensitive comparison, it must convert all names, e.g. to
    // lower case - see CSalamanderGeneralAbstract::ToLowerCase); 'nameInCache' is the name
    // of the copy of the file stored in the disk cache (the last part of the original
    // file name is expected here so that it later reminds the user of the original file in the viewer
    // title bar); if 'rootTmpPath' is NULL, the disk cache is in the Windows TEMP directory;
    // otherwise, the path to the disk cache is in 'rootTmpPath'; on a system error it returns
    // NULL (this should not happen at all), otherwise it returns the full name of the copy of the file in the disk cache
    // and sets 'fileExists' to TRUE if the file in the disk cache already exists (e.g. the FTP
    // download has already taken place) or FALSE if the file still needs to be prepared (e.g.
    // downloaded); 'parent' is the parent window for error message boxes (for example, when a file name is too long)
    // CAUTION: if it did not return NULL (no system error occurred), it is necessary to call
    //          FreeFileNameInCache later (for the same 'uniqueFileName')
    // NOTE: if the FS uses the disk cache, it should at least call
    //       CSalamanderGeneralAbstract::RemoveFilesFromCache("fs-name:") when the plugin is unloaded,
    //       otherwise its file copies will unnecessarily get in the way in the disk cache
    virtual const char* WINAPI AllocFileNameInCache(HWND parent, const char* uniqueFileName, const char* nameInCache,
                                                    const char* rootTmpPath, BOOL& fileExists) = 0;

    // opens the file 'fileName' from a Windows path in the viewer requested by the user
    // (either using the viewer association or via the View With command); 'parent' is the parent of
    // error message boxes; if 'fileLock' and 'fileLockOwner' are not NULL, an association with the
    // opened viewer is returned in them (it is used later as a parameter of the FreeFileNameInCache method);
    // returns TRUE if the viewer was opened
    virtual BOOL WINAPI OpenViewer(HWND parent, const char* fileName, HANDLE* fileLock,
                                   BOOL* fileLockOwner) = 0;

    // must be paired with AllocFileNameInCache; called only after the viewer is opened (or after an error
    // while preparing the file copy or opening the viewer); 'uniqueFileName' is the unique
    // name of the original file (use the same string as when calling AllocFileNameInCache);
    // 'fileExists' is FALSE if the file copy in the disk cache did not exist and TRUE if it
    // already existed (the same value as the output parameter 'fileExists' of the AllocFileNameInCache method);
    // if 'fileExists' is TRUE, 'newFileOK' and 'newFileSize' are ignored; otherwise, 'newFileOK' is
    // TRUE if the file copy was prepared successfully (e.g. the download completed successfully), and
    // 'newFileSize' contains the size of the prepared file copy; if 'newFileOK' is FALSE,
    // 'newFileSize' is ignored; 'fileLock' and 'fileLockOwner' associate the opened viewer
    // with file copies in the disk cache (after the viewer is closed, the disk cache may remove the file
    // copy - when the copy is removed depends on the size of the disk cache on disk); both
    // of these parameters can be obtained when calling the OpenViewer method; if the viewer
    // could not be opened (or the file copy could not be prepared in the disk cache, or the viewer
    // has no link to the disk cache), 'fileLock' is set to NULL and 'fileLockOwner' to FALSE;
    // if 'fileExists' is TRUE (the file copy already existed), the value of 'removeAsSoonAsPossible'
    // is ignored; otherwise, if 'removeAsSoonAsPossible' is TRUE, the file copy will not be kept in the disk
    // cache longer than necessary (it is deleted immediately after the viewer is closed; if
    // the viewer was not opened at all ('fileLock' is NULL), the file is not inserted into the disk cache
    // but deleted)
    virtual void WINAPI FreeFileNameInCache(const char* uniqueFileName, BOOL fileExists, BOOL newFileOK,
                                            const CQuadWord& newFileSize, HANDLE fileLock,
                                            BOOL fileLockOwner, BOOL removeAsSoonAsPossible) = 0;
};

//
// ****************************************************************************
// CPluginFSInterfaceAbstract
//
// a set of plugin callbacks that Salamander needs to work with the file system

// icon types in the panel when browsing the FS (used in CPluginFSInterfaceAbstract::ListCurrentPath())
#define pitSimple 0       // simple icons for files and directories - based on extension (association)
#define pitFromRegistry 1 // icons loaded from the registry based on the file/directory extension
#define pitFromPlugin 2   // icons are provided by the plugin (icons obtained via CPluginDataInterfaceAbstract)

// event codes (and the meaning of the 'param' parameter) for the FS, received by the CPluginFSInterfaceAbstract::Event() method:
// CPluginFSInterfaceAbstract::TryCloseOrDetach returned TRUE, but the new path could not be opened,
// so we stay on the current path (the FS that receives this message);
// 'param' is the panel containing this FS (PANEL_LEFT or PANEL_RIGHT)
#define FSE_CLOSEORDETACHCANCELED 0

// successful connection of a new FS to a panel (after the path change and listing)
// 'param' is the panel containing this FS (PANEL_LEFT or PANEL_RIGHT)
#define FSE_OPENED 1

// successfully added to the list of detached FSs (end of the "panel" FS mode, start of the "detached" FS mode)
// 'param' is the panel containing this FS (PANEL_LEFT or PANEL_RIGHT)
#define FSE_DETACHED 2

// successful connection of a detached FS (end of the "detached" FS mode, start of the "panel" FS mode)
// 'param' is the panel containing this FS (PANEL_LEFT or PANEL_RIGHT)
#define FSE_ATTACHED 3

// activation of Salamander's main window (when the window is minimized it waits for restore/maximize
// and only then sends this event so that possible error windows show above Salamander),
// sent only to an FS that is in the panel (not detached); if changes on the FS are not monitored automatically,
// this event announces a suitable moment for a refresh;
// 'param' is the panel containing this FS (PANEL_LEFT or PANEL_RIGHT)
#define FSE_ACTIVATEREFRESH 4

// one of this FS's timers has expired; 'param' is that timer's parameter;
// NOTE: CPluginFSInterfaceAbstract::Event() with code FSE_TIMER is called
// from the main thread after the WM_TIMER message is delivered to the main window (so, for example,
// any modal dialog may currently be open), so the timer should be handled quietly
// (do not open any windows, etc.); CPluginFSInterfaceAbstract::Event()
// with code FSE_TIMER may be called immediately after
// calling CPluginInterfaceForFS::OpenFS (if it adds a timer for the
// newly created FS object)
#define FSE_TIMER 5

// a path change (or refresh) has just occurred in this FS in the panel, or this detached FS has just been connected to the panel
// (this event is sent after the path change and the path is listed); FSE_PATHCHANGED is sent after each successful call to ListCurrentPath
// NOTE: FSE_PATHCHANGED immediately follows every FSE_OPENED and FSE_ATTACHED
// 'param' is the panel containing this FS (PANEL_LEFT or PANEL_RIGHT)
#define FSE_PATHCHANGED 6

// constants indicating the reason for calling CPluginFSInterfaceAbstract::TryCloseOrDetach();
// the parentheses always list possible values of forceClose ("FALSE->TRUE" means "first
// try without force, if the FS refuses, ask the user and possibly retry with force") and canDetach:
//
// (FALSE, TRUE) when changing the path outside the FS opened in the panel
#define FSTRYCLOSE_CHANGEPATH 1
// (FALSE->TRUE, FALSE) for an FS opened in a panel during plugin unload (user-requested unload +
// Salamander shutdown + before removing the plugin + unload requested by the plugin)
#define FSTRYCLOSE_UNLOADCLOSEFS 2
// (FALSE, TRUE) when changing the path or refreshing (Ctrl+R) of an FS opened in a panel, it was found
// that no path on the FS is accessible anymore - Salamander tries to change the panel path
// to a fixed drive (if the FS does not allow it, the FS remains in the panel without files and directories)
#define FSTRYCLOSE_CHANGEPATHFAILURE 3
// (FALSE, FALSE) when reconnecting a detached FS back to the panel, it was found that no path
// on this FS is accessible anymore - Salamander tries to close this detached FS (if the FS refuses to close,
// it remains in the list of detached FS - e.g. in the Alt+F1/F2 menu)
#define FSTRYCLOSE_ATTACHFAILURE 4
// (FALSE->TRUE, FALSE) for a detached FS during plugin unload (user-requested unload +
// Salamander shutdown + before plugin removal + unload requested by the plugin)
#define FSTRYCLOSE_UNLOADCLOSEDETACHEDFS 5
// (FALSE, FALSE) the plugin called CSalamanderGeneral::CloseDetachedFS() for the detached FS
#define FSTRYCLOSE_PLUGINCLOSEDETACHEDFS 6

// flags indicating which file-system services the plugin provides - which
// CPluginFSInterfaceAbstract methods are defined:
// copy from FS (F5 on FS)
#define FS_SERVICE_COPYFROMFS 0x00000001
// move from FS + rename on FS (F6 on FS)
#define FS_SERVICE_MOVEFROMFS 0x00000002
// copy from disk to FS (F5 on disk)
#define FS_SERVICE_COPYFROMDISKTOFS 0x00000004
// move from disk to FS (F6 on disk)
#define FS_SERVICE_MOVEFROMDISKTOFS 0x00000008
// delete on FS (F8)
#define FS_SERVICE_DELETE 0x00000010
// quick rename on FS (F2)
#define FS_SERVICE_QUICKRENAME 0x00000020
// view from FS (F3)
#define FS_SERVICE_VIEWFILE 0x00000040
// edit from FS (F4)
#define FS_SERVICE_EDITFILE 0x00000080
// edit new file from FS (Shift+F4)
#define FS_SERVICE_EDITNEWFILE 0x00000100
// change attributes on FS (Ctrl+F2)
#define FS_SERVICE_CHANGEATTRS 0x00000200
// create directory on FS (F7)
#define FS_SERVICE_CREATEDIR 0x00000400
// show info about FS (Ctrl+F1)
#define FS_SERVICE_SHOWINFO 0x00000800
// show properties on FS (Alt+Enter)
#define FS_SERVICE_SHOWPROPERTIES 0x00001000
// calculate occupied space on FS (Alt+F10 + Ctrl+Shift+F10 + calc. needed space + spacebar key in panel)
#define FS_SERVICE_CALCULATEOCCUPIEDSPACE 0x00002000
// command line for FS (otherwise command line is disabled)
#define FS_SERVICE_COMMANDLINE 0x00008000
// get free space on FS (number in directory line)
#define FS_SERVICE_GETFREESPACE 0x00010000
// get icon of FS (icon in directory line or Disconnect dialog)
#define FS_SERVICE_GETFSICON 0x00020000
// get the next hot path for the Directory Line (used to shorten the current FS path in the panel)
#define FS_SERVICE_GETNEXTDIRLINEHOTPATH 0x00040000
// context menu on FS (Shift+F10)
#define FS_SERVICE_CONTEXTMENU 0x00080000
// get the item for the Change Drive menu or Disconnect dialog (item for active/detached FS in Alt+F1/F2 or the Disconnect dialog)
#define FS_SERVICE_GETCHANGEDRIVEORDISCONNECTITEM 0x00100000
// accepts path-change notifications from Salamander (see PostChangeOnPathNotification)
#define FS_SERVICE_ACCEPTSCHANGENOTIF 0x00200000
// get path for main-window title (text in window caption) (see Configuration/Appearance/Display current path...)
// if it's not defined, full path is displayed in window caption in all display modes
#define FS_SERVICE_GETPATHFORMAINWNDTITLE 0x00400000
// Find (Alt+F7 on FS) - if it's not defined, standard Find Files and Directories dialog
// is opened even if FS is opened in panel
#define FS_SERVICE_OPENFINDDLG 0x00800000
// open active folder (Shift+F3)
#define FS_SERVICE_OPENACTIVEFOLDER 0x01000000
// show security information (click on security icon in Directory Line, see CSalamanderGeneralAbstract::ShowSecurityIcon)
#define FS_SERVICE_SHOWSECURITYINFO 0x02000000

// missing: Change Case, Convert, Properties, Make File List

// types of context menus for the CPluginFSInterfaceAbstract::ContextMenu() method
#define fscmItemsInPanel 0 // context menu for items in the panel (selected/focused files and directories)
#define fscmPathInPanel 1  // context menu for the current path in the panel
#define fscmPanel 2        // context menu for the panel

#define SALCMDLINE_MAXLEN 8192 // maximum length of a command from Salamander's command line

class CPluginFSInterfaceAbstract
{
#ifdef INSIDE_SALAMANDER
private: // protection against incorrect direct method calls (see CPluginFSInterfaceEncapsulation)
    friend class CPluginFSInterfaceEncapsulation;
#else  // INSIDE_SALAMANDER
public:
#endif // INSIDE_SALAMANDER

    // returns the user-part of the current path in this FS, 'userPart' is a buffer of size MAX_PATH
    // for the path, returns success
    virtual BOOL WINAPI GetCurrentPath(char* userPart) = 0;

    // returns the user part of the full name of file/directory/up-dir 'file' ('isDir' is 0/1/2) on the current
    // path in this FS; for the up-dir directory (the first in the directory list and also named ".."),
    // 'isDir'==2 and the method should return the current path shortened by the last component; 'buf'
    // is a buffer of size 'bufSize' for the resulting full name; returns success
    virtual BOOL WINAPI GetFullName(CFileData& file, int isDir, char* buf, int bufSize) = 0;

    // returns the absolute path (including the fs-name) corresponding to the relative path 'path' on this FS;
    // returns FALSE if this method is not implemented (the other output values are then ignored);
    // 'parent' is the parent of any message boxes; 'fsName' is the current FS name; 'path' is a buffer
    // of size 'pathSize' characters, on input it contains the relative path on the FS, on output it contains
    // the corresponding absolute path on the FS; 'success' is set to TRUE if the path was successfully resolved
    // (the string in 'path' should be used - otherwise it is ignored) - a path change follows (if it is
    // a path on this FS, ChangePath() is called); if 'success' is set to FALSE, it is assumed
    // that the user has already seen an error message
    virtual BOOL WINAPI GetFullFSPath(HWND parent, const char* fsName, char* path, int pathSize,
                                      BOOL& success) = 0;

    // returns the user-part of the root of the current path in this FS; 'userPart' is a MAX_PATH-sized
    // buffer for the path (used by the "goto root" function); returns success
    virtual BOOL WINAPI GetRootPath(char* userPart) = 0;

    // compares the current path in this FS with the path given by 'fsNameIndex' and 'userPart'
    // (the FS name in the path belongs to this plugin and is given by the 'fsNameIndex'), returns TRUE
    // if the paths are identical; 'currentFSNameIndex' is the index of the current FS name
    virtual BOOL WINAPI IsCurrentPath(int currentFSNameIndex, int fsNameIndex, const char* userPart) = 0;

    // returns TRUE if the path belongs to this FS (which means Salamander can pass the path
    // to ChangePath of this FS); the path always belongs to one of the FS instances of this plugin
    // (for example Windows paths and archive paths never appear here); 'fsNameIndex' is the index of the FS name
    // in the path (the index is zero for the fs-name given in CSalamanderPluginEntryAbstract::SetBasicPluginData,
    // other fs-names return the index from CSalamanderPluginEntryAbstract::AddFSName); the user-part
    // of the path is 'userPart'; 'currentFSNameIndex' is the index of the current FS name
    virtual BOOL WINAPI IsOurPath(int currentFSNameIndex, int fsNameIndex, const char* userPart) = 0;

    // changes the current path in this FS to the path specified by 'fsName' and 'userPart' (either exactly
    // or to the nearest accessible subpath of 'userPart' - see the value of 'mode'); if the path
    // is shortened because it is a path to a file (it is enough to suspect that it might be
    // a file path - after the path is listed, it is verified whether the file exists, otherwise
    // an error is shown to the user) and 'cutFileName' is not NULL (possible only in 'mode' 3),
    // the name of this file (without the path) is returned in the 'cutFileName' buffer
    // (with room for MAX_PATH characters); otherwise, an empty string is returned in
    // the 'cutFileName' buffer; 'currentFSNameIndex' is the index of the current FS name;
    // 'fsName' is a buffer with room for MAX_PATH characters; on input, it contains the FS name
    // in the path that belongs to this plugin (but it does not have to match the current FS name
    // in this object, it is enough if IsOurPath() returns TRUE for it); on output, 'fsName'
    // contains the current FS name in this object (it must belong to this plugin); 'fsNameIndex'
    // is the index of the FS name 'fsName' in the plugin (to make it easier to detect which FS name
    // it is); if 'pathWasCut' is not NULL, TRUE is returned in it if the path was shortened;
    // Salamander uses 'cutFileName' and 'pathWasCut' for the Change Directory command (Shift+F7)
    // when a file name is entered - that file then receives the focus; if 'forceRefresh' is TRUE,
    // this is a hard refresh (Ctrl+R) and the plugin should change the path without using cache
    // information (it is necessary to verify that the new path exists); 'mode' is the path-change mode:
    //   1 (refresh path) - shortens the path if needed; do not report that the path does not exist
    //                      (shorten without reporting it), report a file instead of a path,
    //                      path inaccessibility, and other errors
    //   2 (calling ChangePanelPathToPluginFS, back/forward in history, etc.) - shortens the path
    //                      if needed; report all path errors (file instead of a path,
    //                      non-existence, inaccessibility, and others)
    //   3 (change-dir command) - shortens the path only if it is a file or the path cannot be listed
    //                      (ListCurrentPath returns FALSE for it); do not report a file instead of a path
    //                      (shorten without reporting it and return the file name), report all other
    //                      path errors (non-existence, inaccessibility, and others)
    // if 'mode' is 1 or 2, FALSE is returned only if no path on this FS is accessible
    // (e.g. when the connection is lost); if 'mode' is 3, FALSE is returned if the requested
    // path or file is not accessible (the path is shortened only if it is a file);
    // if opening the FS is time-consuming (e.g. connecting to an FTP server) and 'mode'
    // is 3, the behavior may be adjusted to work like archives - shorten the path if needed and return FALSE
    // only if no path on the FS is accessible; error reporting remains unchanged
    virtual BOOL WINAPI ChangePath(int currentFSNameIndex, char* fsName, int fsNameIndex,
                                   const char* userPart, char* cutFileName, BOOL* pathWasCut,
                                   BOOL forceRefresh, int mode) = 0;

    // loads files and directories from the current path and stores them in the 'dir' object (with path NULL or
    // ""; files and directories on other paths are ignored; if a directory named
    // ".." is added, it is drawn as an "up-dir" symbol; file and directory names are entirely
    // determined by the plugin, Salamander only displays them); Salamander obtains the contents
    // of columns added by the plugin via the 'pluginData' interface (if the plugin does not add columns
    // and has no custom icons, it returns 'pluginData'==NULL); in 'iconsType' it returns the requested method
    // for obtaining file and directory icons for the panel; pitFromPlugin degrades to pitSimple if
    // 'pluginData' is NULL (pitFromPlugin cannot be provided without 'pluginData'); if 'forceRefresh'
    // is TRUE, this is a hard refresh (Ctrl+R) and the plugin should load files and directories without using
    // the cache; returns TRUE on successful loading; if it returns FALSE, this is an error and ChangePath will be called
    // for the current path; ChangePath is expected to select an accessible subpath
    // or return FALSE; after a successful call to ChangePath, ListCurrentPath will be called again;
    // if it returns FALSE, the output value 'pluginData' is ignored (the data in 'dir' must be released
    // using 'dir.Clear(pluginData)', otherwise only Salamander's part of the data is released);
    virtual BOOL WINAPI ListCurrentPath(CSalamanderDirectoryAbstract* dir,
                                        CPluginDataInterfaceAbstract*& pluginData,
                                        int& iconsType, BOOL forceRefresh) = 0;

    // prepares the FS for closing/detaching from the panel or for closing a detached FS; if 'forceClose'
    // is TRUE, the FS is closed regardless of the return values, because the action was forced by the user or a
    // critical shutdown is in progress (see CSalamanderGeneralAbstract::IsCriticalShutdown); in any case,
    // there is no point in asking the user anything, the FS should simply close immediately (do not open any windows);
    // if 'forceClose' is FALSE, the FS can either be closed or detached ('canDetach' TRUE), or only closed
    // ('canDetach' FALSE); 'detach' is set to TRUE if it only wants to detach, FALSE means close; 'reason' contains the
    // reason for calling this method (one of the FSTRYCLOSE_XXX); returns TRUE
    // if closing/detaching is possible, otherwise returns FALSE
    virtual BOOL WINAPI TryCloseOrDetach(BOOL forceClose, BOOL canDetach, BOOL& detach, int reason) = 0;

    // event received by this FS, see the FSE_XXX event codes; 'param' is the event parameter
    virtual void WINAPI Event(int event, DWORD param) = 0;

    // release all FS resources except for the listing data (during the call of this method the listing
    // may still be displayed in the panel); called right before the listing in the panel is discarded
    // (the listing is discarded only for active FS; detached FS do not have listings) and before CloseFS for this FS;
    // 'parent' is the parent of any message boxes; if a critical shutdown is in progress (see
    // CSalamanderGeneralAbstract::IsCriticalShutdown), do not show any windows
    virtual void WINAPI ReleaseObject(HWND parent) = 0;

    // obtains the set of supported FS services (see the FS_SERVICE_XXX constants); returns a combination
    // of the constants; called after opening this FS (see CPluginInterfaceForFSAbstract::OpenFS),
    // and then after each call to ChangePath and ListCurrentPath for this FS
    virtual DWORD WINAPI GetSupportedServices() = 0;

    // only if GetSupportedServices() also returns FS_SERVICE_GETCHANGEDRIVEORDISCONNECTITEM:
    // obtains the item for this FS (active or disconnected) for the Change Drive menu (Alt+F1/F2)
    // or the Disconnect dialog (hotkey: F12; any disconnect of this FS is handled by
    // CPluginInterfaceForFSAbstract::DisconnectFS; if GetChangeDriveOrDisconnectItem returns
    // FALSE and the FS is in a panel, an item with the icon obtained via GetFSIcon and the root path is added);
    // if the return value is TRUE, an item with the icon 'icon' and text 'title' is added;
    // 'fsName' is the current FS name; if 'icon' is NULL, the item has no icon; if
    // 'destroyIcon' is TRUE and 'icon' is not NULL, 'icon' is released after use via the Win32 API
    // function DestroyIcon; 'title' is text allocated on Salamander's heap and can contain
    // up to three columns separated by '\t' (see the Alt+F1/F2 menu); in the Disconnect dialog,
    // only the second column is used; if the return value is FALSE, the output values
    // 'title', 'icon', and 'destroyIcon' are ignored (the item is not added)
    virtual BOOL WINAPI GetChangeDriveOrDisconnectItem(const char* fsName, char*& title,
                                                       HICON& icon, BOOL& destroyIcon) = 0;

    // only if GetSupportedServices() also returns FS_SERVICE_GETFSICON:
    // obtains the FS icon for the directory-line toolbar or, if needed, for the Disconnect dialog (F12);
    // the icon for the Disconnect dialog is obtained here only if the GetChangeDriveOrDisconnectItem
    // method does not return an item for this FS (e.g. RegEdit and WMobile);
    // returns the icon or NULL if the standard icon should be used; if 'destroyIcon' is TRUE
    // and an icon (not NULL) is returned, the returned icon is released after use by the Win32 API
    // function DestroyIcon
    // Note: if the icon resource is loaded using LoadIcon at 16x16, LoadIcon returns
    //       a 32x32 icon. When it is then drawn at 16x16, colored outlines appear around the
    //       icon. This 16->32->16 conversion can be avoided by using LoadImage:
    //       (HICON)LoadImage(DLLInstance, MAKEINTRESOURCE(id), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    //
    // no windows may be shown in this method (the panel content is not consistent, messages must not
    // be dispatched - redraws, etc.)
    virtual HICON WINAPI GetFSIcon(BOOL& destroyIcon) = 0;

    // returns the requested drop-effect for a drag&drop operation from an FS (this FS may also be the source) to this FS;
    // 'srcFSPath' is the source path; 'tgtFSPath' is the target path (it belongs to this FS); 'allowedEffects'
    // contains the allowed drop-effects; 'keyState' is the state of the keys (a combination of the flags MK_CONTROL,
    // MK_SHIFT, MK_ALT, MK_BUTTON, MK_LBUTTON, MK_MBUTTON and MK_RBUTTON, see IDropTarget::Drop);
    // 'dropEffect' contains the recommended drop-effects (equal to 'allowedEffects' or limited to
    // DROPEFFECT_COPY or DROPEFFECT_MOVE if the user holds the Ctrl or Shift keys), and the selected
    // drop-effect (DROPEFFECT_COPY, DROPEFFECT_MOVE or DROPEFFECT_NONE) is returned in it;
    // if the method does not change 'dropEffect' and it contains multiple effects, the Copy operation is preferred
    virtual void WINAPI GetDropEffect(const char* srcFSPath, const char* tgtFSPath,
                                      DWORD allowedEffects, DWORD keyState,
                                      DWORD* dropEffect) = 0;

    // only if GetSupportedServices() also returns FS_SERVICE_GETFREESPACE:
    // returns in 'retValue' (must not be NULL) the amount of free space on the FS (displayed
    // on the right in the directory-line); if the free space cannot be determined, it returns
    // CQuadWord(-1, -1) (the information is not displayed)
    virtual void WINAPI GetFSFreeSpace(CQuadWord* retValue) = 0;

    // only if GetSupportedServices() also returns FS_SERVICE_GETNEXTDIRLINEHOTPATH:
    // find split points in the Directory Line text (for shortening the path using the mouse - hot-tracking);
    // 'text' is the text in the Directory Line (path + optional filter); 'pathLen' is the length of the path in 'text'
    // (the rest is the filter); 'offset' is the character offset from which the split point should be searched; returns TRUE
    // if another split point exists and returns its position in 'offset'; returns FALSE if there is no additional
    // split point (the end of the text is not considered a split point)
    virtual BOOL WINAPI GetNextDirectoryLineHotPath(const char* text, int pathLen, int& offset) = 0;

    // only if GetSupportedServices() also returns FS_SERVICE_GETNEXTDIRLINEHOTPATH:
    // adjust the text of the shortened path to be displayed in the panel (Directory Line - path shortening
    // with the mouse - hot-tracking); used when the hot text from the Directory Line does not exactly match
    // the path (for example, it is missing the closing bracket - VMS paths on FTP - "[DIR1.DIR2.DIR3]");
    // 'path' is an in/out buffer containing the path (the buffer size is 'pathBufSize')
    virtual void WINAPI CompleteDirectoryLineHotPath(char* path, int pathBufSize) = 0;

    // only if GetSupportedServices() also returns FS_SERVICE_GETPATHFORMAINWNDTITLE:
    // retrieves the text displayed in the main window title when display of the current
    // path in the main window title is enabled (see Configuration/Appearance/Display current
    // path...); 'fsName' is the current FS name; if 'mode' is 1, this is the
    // "Directory Name Only" mode (only the name of the current directory - the last
    // path component - should be displayed); if 'mode' is 2, this is the "Shortened Path"
    // mode (a shortened form of the path should be displayed - root (including the path
    // separator) + "..." + path separator + the last path component); 'buf' is a buffer
    // of size 'bufSize' for the resulting text; returns TRUE if it returns the requested
    // text; returns FALSE if the text should be created from the split point data
    // obtained via the GetNextDirectoryLineHotPath() method
    // NOTE: if GetSupportedServices() does not also return FS_SERVICE_GETPATHFORMAINWNDTITLE,
    //       the full path on the FS is displayed in the main window title in all title display
    //       modes (including "Directory Name Only" and "Shortened Path")
    virtual BOOL WINAPI GetPathForMainWindowTitle(const char* fsName, int mode, char* buf, int bufSize) = 0;

    // only if GetSupportedServices() also returns FS_SERVICE_SHOWINFO:
    // displays a dialog with information about the FS (free space, capacity, name, options, etc.);
    // 'fsName' is the current FS name; 'parent' is the suggested parent window for the displayed dialog
    virtual void WINAPI ShowInfoDialog(const char* fsName, HWND parent) = 0;

    // only if GetSupportedServices() also returns FS_SERVICE_COMMANDLINE:
    // executes a command for the FS in the active panel from the command line below the panels; returns FALSE on error
    // (the command is not added to the command line history and the other output values are ignored);
    // returns TRUE if the command was started successfully (note: the command's results do not matter - the only
    // important thing is whether it was started (e.g. for FTP, whether it was delivered to the server));
    // 'parent' is the suggested parent for any displayed dialogs; 'command' is a buffer
    // of size SALCMDLINE_MAXLEN+1 that contains the command to execute on input (the actual
    // maximum command length depends on the Windows version and the contents of the COMSPEC environment variable)
    // and the new command line contents on output (usually it is simply cleared to an empty string);
    // 'selFrom' and 'selTo' return the selection positions in the new command line contents (if they are equal,
    // only the cursor is positioned; if the output is an empty string, these values are ignored)
    // NOTE: this method should not directly change the path in the panel - the FS may be closed because of a path error
    //       (the method would then lose its this pointer)
    virtual BOOL WINAPI ExecuteCommandLine(HWND parent, char* command, int& selFrom, int& selTo) = 0;

    // only if GetSupportedServices() also returns FS_SERVICE_QUICKRENAME:
    // quick rename of the file or directory ('isDir' is FALSE/TRUE) 'file' on the FS;
    // allows opening a custom quick rename dialog (the parameter 'mode' is 1)
    // or using the standard dialog (when 'mode'==1 it returns FALSE and 'cancel' is also FALSE,
    // Salamander then opens the standard dialog and passes the obtained new name in 'newName' on
    // the next call to QuickRename with 'mode'==2); 'fsName' is the current FS name; 'parent' is
    // the suggested parent window for any displayed dialogs; 'newName' is the new name if
    // 'mode'==2; if it returns TRUE, 'newName' contains the new name (max. MAX_PATH characters;
    // not the full name, only the item name in the panel) - Salamander will try to focus it after
    // the refresh (the FS itself handles the refresh, for example by using the method
    // CSalamanderGeneralAbstract::PostRefreshPanelFS); if it returns FALSE and 'mode'==2,
    // the invalid new name is returned in 'newName' (possibly modified in some way - e.g.
    // an operation mask may already have been applied); if the user wants to cancel the operation,
    // 'cancel' returns TRUE; if 'cancel' returns FALSE, the method returns TRUE when the operation
    // completes successfully; if it returns FALSE in 'mode'==1, the standard quick rename dialog
    // should be opened; if it returns FALSE in 'mode'==2, the operation has failed (the invalid
    // new name is returned in 'newName' - the standard dialog is opened again and the user can
    // correct the invalid name there)
    virtual BOOL WINAPI QuickRename(const char* fsName, int mode, HWND parent, CFileData& file,
                                    BOOL isDir, char* newName, BOOL& cancel) = 0;

    // only if GetSupportedServices() also returns FS_SERVICE_ACCEPTSCHANGENOTIF:
    // receives information about a change on the path 'path' (if 'includingSubdirs' is TRUE, it also
    // includes changes in the subdirectories of the path 'path'); this method should decide
    // whether this FS needs to be refreshed (for example using the method
    // CSalamanderGeneralAbstract::PostRefreshPanelFS); applies to both active and disconnected FS instances;
    // 'fsName' is the current FS name
    // NOTE: for the plugin as a whole there is the method
    //       CPluginInterfaceAbstract::AcceptChangeOnPathNotification()
    virtual void WINAPI AcceptChangeOnPathNotification(const char* fsName, const char* path,
                                                       BOOL includingSubdirs) = 0;

    // only if GetSupportedServices() also returns FS_SERVICE_CREATEDIR:
    // creates a new directory on the FS; can open a custom directory creation dialog
    // (when the parameter 'mode' is 1) or use the standard dialog (if it returns
    // FALSE and 'cancel' is also FALSE for 'mode'==1, Salamander opens the standard dialog and passes the obtained
    // directory name in 'newName' on the next call to CreateDir with 'mode'==2);
    // 'fsName' is the current FS name; 'parent' is the suggested parent window for any displayed
    // dialogs; 'newName' is the name of the new directory if 'mode'==2; if it returns TRUE,
    // the name of the new directory is returned in 'newName' (max. 2 * MAX_PATH characters; not the full name,
    // only the item name in the panel) - Salamander then tries to focus it after the refresh (the refresh
    // is handled by the FS itself, for example using CSalamanderGeneralAbstract::PostRefreshPanelFS);
    // if it returns FALSE and 'mode'==2, the invalid directory name is returned in 'newName' (max. 2 * MAX_PATH
    // characters, possibly converted to an absolute form); if the user wants to cancel the operation,
    // 'cancel' returns TRUE; if 'cancel' returns FALSE, the method returns TRUE when the operation completes successfully;
    // if it returns FALSE for 'mode'==1, the standard directory creation dialog should be opened; if it returns FALSE for 'mode'==2,
    // the operation has failed (the invalid directory name is returned in 'newName' - the standard dialog is opened again and the user
    // can correct the invalid name there)
    virtual BOOL WINAPI CreateDir(const char* fsName, int mode, HWND parent,
                                  char* newName, BOOL& cancel) = 0;

    // only if GetSupportedServices() also returns FS_SERVICE_VIEWFILE:
    // view file 'file' (directories cannot be viewed via the View function) on the current path
    // in the FS; 'fsName' is the current FS name; 'parent' is the parent for any error message boxes;
    // 'salamander' is the set of Salamander methods needed to implement
    // viewing with caching
    virtual void WINAPI ViewFile(const char* fsName, HWND parent,
                                 CSalamanderForViewFileOnFSAbstract* salamander,
                                 CFileData& file) = 0;

    // only if GetSupportedServices() also returns FS_SERVICE_DELETE:
    // delete the files and directories selected in the panel; allows opening a custom confirmation dialog
    // for deletion (the parameter 'mode' is 1; whether the confirmation should be displayed depends on the value of
    // SALCFG_CNFRMFILEDIRDEL - TRUE means the user wants to confirm deletions)
    // or using the standard confirmation (when 'mode'==1 it returns FALSE and 'cancelOrError' also FALSE,
    // then Salamander opens the standard confirmation (if SALCFG_CNFRMFILEDIRDEL is TRUE)
    // and, if the answer is positive, calls Delete again with 'mode'==2); 'fsName' is the current FS name;
    // 'parent' is the suggested parent of any displayed dialogs; 'panel' identifies the panel
    // (PANEL_LEFT or PANEL_RIGHT) in which the FS is opened (the files/directories to be deleted are taken from this panel);
    // 'selectedFiles' + 'selectedDirs' - the count of selected files and directories; if both values are zero, the
    // file/directory under the cursor (focus) is deleted; before calling Delete either files and directories are selected or at least
    // the focus is on a file/directory, so there is always something to work with (no additional tests are needed);
    // if it returns TRUE and 'cancelOrError' is FALSE, the operation completed successfully and the selected
    // files/directories should be unselected (if they survived the deletion); if the user wants to cancel
    // the operation or an error occurs, 'cancelOrError' returns TRUE and the files/directories remain selected;
    // if it returns FALSE when 'mode'==1 and 'cancelOrError' is FALSE, the standard delete confirmation should be opened
    virtual BOOL WINAPI Delete(const char* fsName, int mode, HWND parent, int panel,
                               int selectedFiles, int selectedDirs, BOOL& cancelOrError) = 0;

    // copy/move from the FS (the parameter 'copy' is TRUE/FALSE); the following text mentions only copying,
    // but everything applies equally to moving; 'copy' can be TRUE (copy) only if
    // GetSupportedServices() also returns FS_SERVICE_COPYFROMFS; 'copy' can be FALSE
    // (move or rename) only if GetSupportedServices() also returns FS_SERVICE_MOVEFROMFS;
    //
    // copying the files and directories (from the FS) selected in the panel; allows opening a custom dialog
    // for specifying the copy target (the parameter 'mode' is 1) or using the standard dialog (returns FALSE
    // and 'cancelOrHandlePath' also FALSE, then Salamander opens the standard dialog and passes the obtained target
    // path in 'targetPath' during the next call to CopyOrMoveFromFS with 'mode'==2); when 'mode'==2
    // 'targetPath' is exactly the string entered by the user (CopyOrMoveFromFS can analyze it
    // in its own way); if CopyOrMoveFromFS supports only Windows target paths (or cannot
    // process the path entered by the user - e.g. it leads to another FS or into an archive), it can use
    // the standard path processing in Salamander (for now it can process only Windows paths,
    // in the future it may also process FS and archive paths through the TEMP directory by a sequence of basic operations)
    // - it returns FALSE, 'cancelOrHandlePath' TRUE and 'operationMask' TRUE/FALSE
    // (supports/does not support operation masks - if it does not support them and the path contains a mask, an error message is shown),
    // then Salamander processes the path returned in 'targetPath' (for now only splitting the
    // Windows path into the existing part, non-existing part and optional mask; it also allows creating
    // subdirectories from the non-existing part) and if the path is valid, it calls CopyOrMoveFromFS again
    // with 'mode'==3 and in 'targetPath' the target path and possibly the operation mask (two strings separated by
    // a null character; no mask -> two nulls at the end of the string); if the path contains an error, it calls CopyOrMoveFromFS
    // again with 'mode'==4 and in 'targetPath' the adjusted incorrect target path (the error has already been reported to the user;
    // the user should get the opportunity to correct the path; "." and ".." may have been removed from the path, etc.);
    //
    // if the user triggers the operation via drag&drop (drops files/directories from the FS to the same panel
    // or to another drop target), 'mode'==5 and 'targetPath' contains the target path of the operation (it can be
    // a Windows path, FS path and in the future also archive paths),
    // 'targetPath' ends with two null characters (for compatibility with 'mode'==3); 'dropTarget' is
    // the drop-target window in this case (used to reactivate the drop target after opening the
    // operation progress window, see CSalamanderGeneralAbstract::ActivateDropTarget); for 'mode'==5 only the return value TRUE is meaningful;
    //
    // 'fsName' is the current FS name; 'parent' is the suggested parent of any displayed dialogs;
    // 'panel' identifies the panel (PANEL_LEFT or PANEL_RIGHT) in which the FS is opened (the files/directories to be copied are taken from this panel);
    // 'selectedFiles' + 'selectedDirs' - the count of selected files and directories; if both values are zero,
    // the file/directory under the cursor (focus) is copied; before calling CopyOrMoveFromFS either files and directories are selected
    // or at least the focus is on a file/directory, so there is always something to work with (no additional tests
    // are needed); on input 'targetPath' when 'mode'==1 contains the suggested target path
    // (only Windows paths without a mask or an empty string), when 'mode'==2 it contains the target path string
    // entered by the user in the standard dialog, when 'mode'==3 it contains the target path
    // and mask (separated by a null), when 'mode'==4 it contains the incorrect target path, when 'mode'==5
    // it contains the target path (Windows, FS or archive) terminated by two nulls; if
    // the method returns FALSE, 'targetPath' on output (buffer of 2 * MAX_PATH characters) contains, when
    // 'cancelOrHandlePath'==FALSE, the suggested target path for the standard dialog and when
    // 'cancelOrHandlePath'==TRUE, the target path string to be processed; if the method returns TRUE and
    // 'cancelOrHandlePath' is FALSE, 'targetPath' contains the name of the item that should receive the focus
    // in the source panel (buffer of 2 * MAX_PATH characters; not the full name, only the item name in the panel;
    // if it is an empty string, the focus remains unchanged); 'dropTarget' is not NULL only when
    // the path of the operation is provided via drag&drop (see the description above)
    //
    // if it returns TRUE and 'cancelOrHandlePath' is FALSE, the operation completed successfully and the selected
    // files/directories should be unselected; if the user wants to cancel the operation or an error occurred,
    // the method returns TRUE and 'cancelOrHandlePath' TRUE, in both cases the files/directories remain selected;
    // if it returns FALSE, either the standard dialog should be opened ('cancelOrHandlePath'
    // is FALSE) or the path should be processed in the standard way ('cancelOrHandlePath' is TRUE)
    //
    // NOTE: if the option to copy/move to the path of the target panel is offered,
    //       CSalamanderGeneralAbstract::SetUserWorkedOnPanelPath must be called for the target
    //       panel, otherwise the path in this panel will not be inserted into the List of Working
    //       Directories (Alt+F12)
    virtual BOOL WINAPI CopyOrMoveFromFS(BOOL copy, int mode, const char* fsName, HWND parent,
                                         int panel, int selectedFiles, int selectedDirs,
                                         char* targetPath, BOOL& operationMask,
                                         BOOL& cancelOrHandlePath, HWND dropTarget) = 0;

    // copy/move from a Windows path to the FS (the parameter 'copy' is TRUE/FALSE); the text below
    // refers only to copying, but everything applies equally to moving; 'copy' can be TRUE (copying)
    // only if GetSupportedServices() also returns FS_SERVICE_COPYFROMDISKTOFS; 'copy' can be FALSE
    // (move or rename) only if GetSupportedServices() also returns FS_SERVICE_MOVEFROMDISKTOFS;
    //
    // copying selected (in the panel or elsewhere) files and directories to the FS; when 'mode'==1 it allows
    // preparing the target path text for the user for the standard copy dialog; this is the situation
    // where the source panel (the panel where the Copy command (F5) is launched) contains a Windows path
    // and the target panel contains this FS; when 'mode'==2 and 'mode'==3 the plugin can perform the copy operation or
    // report one of two errors: "path error" (e.g. it contains invalid characters or does not exist)
    // and "the requested operation cannot be performed on this FS" (for example, it may be FTP, but the path
    // currently open in this FS differs from the target path (e.g. a different FTP server) - a different/new FS
    // needs to be opened; this error cannot be reported by a newly opened FS);
    // WARNING: this method can be called for any target FS path of this plugin (so it may
    //          also be a path with a different FS name of this plugin)
    //
    // 'fsName' is the current FS name; 'parent' is the suggested parent for any displayed
    // dialogs; 'sourcePath' is the source Windows path (all selected files and directories
    // are addressed relative to it), for 'mode'==1 it is NULL; the selected files and directories
    // are specified by the enumeration function 'next', whose parameter is 'nextParam'; for 'mode'==1
    // they are NULL; 'sourceFiles' + 'sourceDirs' - the number of selected files and directories (the sum
    // is always non-zero); 'targetPath' is an in/out buffer of at least 2 * MAX_PATH characters for the target
    // path; for 'mode'==1, on input 'targetPath' is the current path on this FS and on output it is the target
    // path for the standard copy dialog; for 'mode'==2, on input 'targetPath' is the target path entered by the user
    // (without modifications, including a mask, etc.) and on output it is ignored except when the
    // method returns FALSE (error) and 'invalidPathOrCancel' is TRUE (path error); in that case, on output it contains
    // the adjusted target path (e.g. with "." and ".." removed), which the user will correct
    // in the standard copy dialog; for 'mode'==3, on input 'targetPath' is the target path entered via drag&drop
    // and on output it is ignored; if 'invalidPathOrCancel' is not NULL (only for 'mode'==2
    // and 'mode'==3), TRUE is returned in it if the path is invalid (contains invalid characters or
    // does not exist, etc.) or if the operation was canceled - the error/cancel message is displayed
    // before this method returns
    //
    // for 'mode'==1 the method returns TRUE on success; if it returns FALSE, an empty string is used as the target path
    // for the standard copy dialog; if the method returns FALSE for 'mode'==2 and 'mode'==3,
    // another FS should be sought to handle the operation (if 'invalidPathOrCancel' is FALSE), or the user should
    // correct the target path (if 'invalidPathOrCancel' is TRUE); if the method returns TRUE
    // for 'mode'==2 or 'mode'==3, the operation was completed and the selected files and directories should be deselected
    // (if 'invalidPathOrCancel' is FALSE), or an error/cancel occurred and the selected files and directories should not be
    // deselected (if 'invalidPathOrCancel' is TRUE)
    //
    // WARNING: The CopyOrMoveFromDiskToFS method can be called in three situations:
    //          - this FS is active in the panel
    //          - this FS is disconnected
    //          - this FS has just been created (by calling OpenFS) and after the method returns it immediately ceases to exist
    //            again (by calling CloseFS) - no other method was called on it (not even ChangePath)
    virtual BOOL WINAPI CopyOrMoveFromDiskToFS(BOOL copy, int mode, const char* fsName, HWND parent,
                                               const char* sourcePath, SalEnumSelection2 next,
                                               void* nextParam, int sourceFiles, int sourceDirs,
                                               char* targetPath, BOOL* invalidPathOrCancel) = 0;

    // only if GetSupportedServices() also returns FS_SERVICE_CHANGEATTRS:
    // changes the attributes of the files and directories selected in the panel; each plugin has its own dialog for specifying attribute changes;
    // 'fsName' is the current FS name; 'parent' is the suggested parent of the plugin's own dialog; 'panel'
    // identifies the panel (PANEL_LEFT or PANEL_RIGHT) in which the FS is open (the files/directories to work with are taken from this
    // panel);
    // 'selectedFiles' + 'selectedDirs' - the number of selected files and directories;
    // if both values are zero, the file/directory under the cursor is used
    // (focus); before calling ChangeAttributes, either files and directories are selected or at least
    // the focus is on a file/directory, so there is always something to work with (no additional tests
    // are needed); if the method returns TRUE, the operation completed successfully and the selected files/directories
    // should be unselected; if the user cancels the operation or an error occurs, the method returns
    // FALSE and the files/directories remain selected
    virtual BOOL WINAPI ChangeAttributes(const char* fsName, HWND parent, int panel,
                                         int selectedFiles, int selectedDirs) = 0;

    // only if GetSupportedServices() also returns FS_SERVICE_SHOWPROPERTIES:
    // display a properties window for the files and directories selected in the panel; each plugin has its own properties window;
    // 'fsName' is the current FS name; 'parent' is the suggested parent of its window
    // (the Windows properties window is modeless - note: a modeless window must have its own thread); 'panel' identifies the panel (PANEL_LEFT or PANEL_RIGHT)
    // in which the FS is open (the files/directories to work with are taken from this panel); 'selectedFiles' + 'selectedDirs' - the number of selected
    // files and directories; if both values are zero, work with the file/directory under the cursor
    // (focus); before calling ShowProperties either files and directories are selected or at least the focus is on a file/directory, so there is always
    // something to work with (no additional tests are needed)
    virtual void WINAPI ShowProperties(const char* fsName, HWND parent, int panel,
                                       int selectedFiles, int selectedDirs) = 0;

    // only if GetSupportedServices() also returns FS_SERVICE_CONTEXTMENU:
    // display the context menu for the files and directories selected in the panel (right-clicking
    // the items in the panel) or for the current path in the panel (right-clicking
    // the change-drive button in the panel toolbar) or for the panel itself (right-clicking
    // behind the items in the panel); each plugin has its own context menu;
    //
    // 'fsName' is the current FS name; 'parent' is the suggested parent of the context menu;
    // 'menuX' + 'menuY' are the suggested coordinates of the top-left corner of the context menu;
    // 'type' is the context menu type (see the descriptions of the fscmXXX constants); 'panel'
    // identifies the panel (PANEL_LEFT or PANEL_RIGHT) for which the context menu should be opened (the files/directories/path to work with are taken from this panel);
    // when 'type'==fscmItemsInPanel, 'selectedFiles' + 'selectedDirs'
    // is the count of selected files and directories; if both values are zero, work with the
    // file/directory under the cursor (focus); before calling ContextMenu either files and directories are selected (and the click occurred on them)
    // or at least the focus is on a file/directory (not on the up-dir), so there is always something to work with (no additional tests are needed);
    // if 'type'!=fscmItemsInPanel, 'selectedFiles' + 'selectedDirs'
    // are always set to zero (ignored)
    virtual void WINAPI ContextMenu(const char* fsName, HWND parent, int menuX, int menuY, int type,
                                    int panel, int selectedFiles, int selectedDirs) = 0;

    // only if GetSupportedServices() also returns FS_SERVICE_CONTEXTMENU:
    // if an FS is open in the panel and one of the messages WM_INITPOPUP, WM_DRAWITEM,
    // WM_MENUCHAR or WM_MEASUREITEM arrives, Salamander calls HandleMenuMsg so that the plugin
    // can work with IContextMenu2 and IContextMenu3
    // the plugin returns TRUE if it handled the message and FALSE otherwise
    virtual BOOL WINAPI HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plResult) = 0;

    // only if GetSupportedServices() also returns FS_SERVICE_OPENFINDDLG:
    // open the Find dialog for the FS in the panel; 'fsName' is the current FS name; 'panel' identifies
    // the panel (PANEL_LEFT or PANEL_RIGHT) for which the Find dialog should be opened (the search path is typically taken from this panel);
    // returns TRUE if the Find dialog was opened successfully;
    // if it returns FALSE, Salamander opens the standard Find Files and Directories dialog
    virtual BOOL WINAPI OpenFindDialog(const char* fsName, int panel) = 0;

    // only if GetSupportedServices() also returns FS_SERVICE_OPENACTIVEFOLDER:
    // opens an Explorer window for the current path in the panel
    // 'fsName' is the current FS name; 'parent' is the suggested parent for the displayed dialog
    virtual void WINAPI OpenActiveFolder(const char* fsName, HWND parent) = 0;

    // only if GetSupportedServices() returns FS_SERVICE_MOVEFROMFS or FS_SERVICE_COPYFROMFS:
    // allows influencing the allowed drop-effects when dragging from this FS; if 'allowedEffects'
    // is not NULL, it contains on input the drop-effects allowed so far (a combination of DROPEFFECT_MOVE and DROPEFFECT_COPY),
    // on output it contains the drop-effects allowed by this FS (effects should only be removed);
    // 'mode' is 0 for the call that immediately precedes starting the drag&drop operation; the effects returned
    // in 'allowedEffects' are used for the DoDragDrop call (applies to the entire drag&drop operation);
    // 'mode' is 1 while the mouse is dragged over an FS from this process (it can be this FS or the FS from the other
    // panel); when 'mode' is 1, 'tgtFSPath' contains the target path that will be used if the drop occurs,
    // otherwise 'tgtFSPath' is NULL; 'mode' is 2 for the call that immediately follows the completion
    // of the drag&drop operation (successful or not)
    virtual void WINAPI GetAllowedDropEffects(int mode, const char* tgtFSPath, DWORD* allowedEffects) = 0;

    // allows the plugin to change the standard message "There are no items in this panel." displayed
    // when the panel contains no items (file/directory/up-dir); returns FALSE if
    // the standard message should be used (the contents of 'textBuf' are then ignored); returns TRUE
    // if the plugin returns its own alternative to this message in 'textBuf' (a buffer of 'textBufSize' characters)
    virtual BOOL WINAPI GetNoItemsInPanelText(char* textBuf, int textBufSize) = 0;

    // only if GetSupportedServices() returns FS_SERVICE_SHOWSECURITYINFO:
    // the user clicked the security icon (see CSalamanderGeneralAbstract::ShowSecurityIcon;
    // for example FTPS shows a dialog with the server certificate); 'parent' is the suggested parent of the dialog
    virtual void WINAPI ShowSecurityInfo(HWND parent) = 0;

    /* still to be finished:
    // calculate occupied space on FS (Alt+F10 + Ctrl+Shift+F10 + calc. needed space + spacebar key in panel)
    #define FS_SERVICE_CALCULATEOCCUPIEDSPACE
    // edit from FS (F4)
    #define FS_SERVICE_EDITFILE
    // edit new file from FS (Shift+F4)
    #define FS_SERVICE_EDITNEWFILE
    */
};

//
// ****************************************************************************
// CPluginInterfaceForFSAbstract
//

class CPluginInterfaceForFSAbstract
{
#ifdef INSIDE_SALAMANDER
private: // protection against incorrect direct method calls (see CPluginInterfaceForFSEncapsulation)
    friend class CPluginInterfaceForFSEncapsulation;
#else  // INSIDE_SALAMANDER
public:
#endif // INSIDE_SALAMANDER

    // function for the "file system"; called to open an FS; 'fsName' is the name of the FS
    // to be opened; 'fsNameIndex' is the index of the FS name to be opened
    // (the index is zero for the fs-name given in CSalamanderPluginEntryAbstract::SetBasicPluginData,
    // other fs-names return the index from CSalamanderPluginEntryAbstract::AddFSName);
    // returns a pointer to the opened FS interface CPluginFSInterfaceAbstract or
    // NULL on error
    virtual CPluginFSInterfaceAbstract* WINAPI OpenFS(const char* fsName, int fsNameIndex) = 0;

    // function for the "file system", called to close an FS; 'fs' is a pointer to
    // the interface of the opened FS; after this call, the 'fs' interface is considered invalid in Salamander
    // and will no longer be used (this function pairs with OpenFS)
    // WARNING: no window or dialog may be opened in this method
    //          (windows can be opened in CPluginFSInterfaceAbstract::ReleaseObject)
    virtual void WINAPI CloseFS(CPluginFSInterfaceAbstract* fs) = 0;

    // execute a command on the item for the FS in the Change Drive menu or in the Drive bars
    // (for adding it, see CSalamanderConnectAbstract::SetChangeDriveMenuItem);
    // 'panel' identifies the panel we should work with - for a command from the Change Drive
    // menu, 'panel' is always PANEL_SOURCE (this menu can be expanded only for the active
    // panel); for a command from the Drive bars, it can be PANEL_LEFT or PANEL_RIGHT (if
    // two Drive bars are enabled, we can also work with the inactive panel)
    virtual void WINAPI ExecuteChangeDriveMenuItem(int panel) = 0;

    // opens the context menu for the FS item in the Change Drive menu or in the Drive
    // bars, or for the active/detached FS in the Change Drive menu; 'parent' is the parent
    // of the context menu; 'x' and 'y' are the coordinates at which to open the context menu
    // (the right-click position or the suggested coordinates for Shift+F10);
    // if 'pluginFS' is NULL, this is an item for an FS; otherwise, 'pluginFS' is the interface of the
    // active/detached FS ('isDetachedFS' is FALSE/TRUE); if 'pluginFS' is not
    // NULL, 'pluginFSName' contains the name of the FS opened in 'pluginFS' (otherwise
    // 'pluginFSName' is NULL) and 'pluginFSNameIndex' contains the index of the name of the FS opened
    // in 'pluginFS' (to make it easier to determine which FS name it is; otherwise
    // 'pluginFSNameIndex' is -1); if it returns FALSE, the other return values are
    // ignored; otherwise, they have the following meaning: 'refreshMenu' returns TRUE if the
    // Change Drive menu should be refreshed (ignored for Drive bars, because active/detached FSs are not
    // shown there); 'closeMenu' returns TRUE if the Change Drive menu should be
    // closed (there is nothing to close for Drive bars); if 'closeMenu' returns
    // TRUE and 'postCmd' is non-zero, ExecuteChangeDrivePostCommand is called with parameters 'postCmd'
    // and 'postCmdParam' after the Change Drive menu is closed (immediately for Drive bars);
    // 'panel' identifies the panel we should work with - for the context menu in the Change Drive menu,
    // 'panel' is always PANEL_SOURCE (this menu can be opened only for the active panel); for the
    // context menu in the Drive bars, it can be PANEL_LEFT or PANEL_RIGHT (if two Drive bars are enabled,
    // we can also work with the inactive panel)
    virtual BOOL WINAPI ChangeDriveMenuItemContextMenu(HWND parent, int panel, int x, int y,
                                                       CPluginFSInterfaceAbstract* pluginFS,
                                                       const char* pluginFSName, int pluginFSNameIndex,
                                                       BOOL isDetachedFS, BOOL& refreshMenu,
                                                       BOOL& closeMenu, int& postCmd, void*& postCmdParam) = 0;

    // executes a command from the context menu on the FS item or on the active/detached FS in
    // the Change Drive menu after the Change Drive menu is closed, or executes a command from the
    // context menu on the FS item in the Drive bars (only for compatibility with the Change Drive
    // menu); called in response to the return values 'closeMenu' (TRUE), 'postCmd', and
    // 'postCmdParam' of ChangeDriveMenuItemContextMenu after the Change Drive menu is closed (for
    // Drive bars, immediately); 'panel' identifies the panel we should work with - for the context
    // menu in the Change Drive menu, 'panel' is always PANEL_SOURCE (this menu can be opened only
    // for the active panel), for the context menu in the Drive bars it can be PANEL_LEFT or
    // PANEL_RIGHT (if two Drive bars are enabled, we can work with the inactive panel too)
    virtual void WINAPI ExecuteChangeDrivePostCommand(int panel, int postCmd, void* postCmdParam) = 0;

    // executes an item in the panel with an open FS (e.g. in response to the Enter key in the panel;
    // for subdirectories/up-dirs (it is an up-dir if the name is ".." and it is also the first directory)
    // a path change is assumed; for files, a copy of the file is opened on disk so that any
    // changes can be loaded back to the FS); execution cannot be done in an FS interface method because
    // path-changing methods cannot be called there (they may even close the FS);
    // 'panel' specifies the panel in which the execution takes place (PANEL_LEFT or PANEL_RIGHT);
    // 'pluginFS' is the interface of the FS opened in the panel; 'pluginFSName' is the name of the FS opened
    // in the panel; 'pluginFSNameIndex' is the index of the name of the FS opened in the panel (for easier detection
    // of which FS name it is); 'file' is the file/directory/up-dir to execute ('isDir' is 0/1/2);
    // WARNING: calling a method that changes the path in the panel may invalidate 'pluginFS' (after the FS is closed)
    //          and 'file'+'isDir' (changing the listing in the panel -> discarding the items from the original listing)
    // NOTE: if a file is opened or otherwise processed (e.g. downloaded),
    //       CSalamanderGeneralAbstract::SetUserWorkedOnPanelPath must be called for panel
    //       'panel', otherwise the path in this panel will not be added to the List of Working
    //       Directories (Alt+F12)
    virtual void WINAPI ExecuteOnFS(int panel, CPluginFSInterfaceAbstract* pluginFS,
                                    const char* pluginFSName, int pluginFSNameIndex,
                                    CFileData& file, int isDir) = 0;

    // performs the disconnect of the FS requested by the user in the Disconnect dialog; 'parent' is the
    // parent of any message boxes (the Disconnect dialog is still open);
    // the disconnect cannot be performed in an FS interface method because the FS is supposed to cease to exist;
    // 'isInPanel' is TRUE if the FS is in a panel; in that case 'panel' specifies which panel
    // (PANEL_LEFT or PANEL_RIGHT); 'isInPanel' is FALSE if the FS is disconnected, then
    // 'panel' is 0; 'pluginFS' is the FS interface; 'pluginFSName' is the FS name; 'pluginFSNameIndex'
    // is the index of the FS name (to make it easier to detect which FS name it is); the method returns FALSE
    // if the disconnect fails and the Disconnect dialog should remain open (its contents are refreshed so that previous successful disconnects are shown)
    virtual BOOL WINAPI DisconnectFS(HWND parent, BOOL isInPanel, int panel,
                                     CPluginFSInterfaceAbstract* pluginFS,
                                     const char* pluginFSName, int pluginFSNameIndex) = 0;

    // convert the user-part of a path in the buffer 'fsUserPart' (size MAX_PATH characters) from the external
    // to the internal format (for example for FTP: internal format = paths as handled by the server,
    // external format = URL format = paths containing hex-escape sequences (e.g. "%20" = " "))
    virtual void WINAPI ConvertPathToInternal(const char* fsName, int fsNameIndex,
                                              char* fsUserPart) = 0;

    // converts the user-part of the path in the 'fsUserPart' buffer (MAX_PATH characters) from the internal
    // to the external format
    virtual void WINAPI ConvertPathToExternal(const char* fsName, int fsNameIndex,
                                              char* fsUserPart) = 0;

    // this method is called only for plugins that act as a replacement for the Network item
    // in the Change Drive menu and in the Drive bars (see CSalamanderGeneralAbstract::SetPluginIsNethood()):
    // by calling this method Salamander informs the plugin that the user is changing the path from the root of the UNC
    // path "\\server\share" through the up-dir symbol ("..") into the plugin FS to the path with
    // the user-part "\\server" in the panel 'panel' (PANEL_LEFT or PANEL_RIGHT); the purpose of this method:
    // the plugin should immediately list at least this one share on that path so that it can
    // be focused in the panel (which is the usual behavior when changing the path via an up-dir)
    virtual void WINAPI EnsureShareExistsOnServer(int panel, const char* server, const char* share) = 0;
};

#ifdef _MSC_VER
#pragma pack(pop, enter_include_spl_fs)
#endif // _MSC_VER
#ifdef __BORLANDC__
#pragma option -a
#endif // __BORLANDC__

/*   Preliminary version of the help for the plugin interface

  Opening, changing, listing and refreshing the path:
    - ChangePath is called to open a path in a new FS (the first call to ChangePath is always for opening a path)
    - ChangePath is called to change a path (the second and all subsequent calls to ChangePath are path changes)
    - on a fatal error ChangePath returns FALSE (the FS path is not opened in the panel; if it was
      a path change, ChangePath for the original path is attempted next; if that fails as well,
      the path switches to a fixed drive)
    - if ChangePath returns TRUE (success) and the path was not shortened back to the original one (whose
      listing is currently loaded), ListCurrentPath is called to obtain the new listing
    - after a successful listing ListCurrentPath returns TRUE
    - on a fatal error ListCurrentPath returns FALSE and the subsequent call to ChangePath
      must also return FALSE
    - if the current path cannot be listed, ListCurrentPath returns FALSE and the subsequent call to
      ChangePath must change the path and return TRUE (ListCurrentPath is called again); if the path
      can no longer be changed (root, etc.), ChangePath also returns FALSE (the FS path is not opened in the panel;
      if it was a path change, ChangePath for the original path is attempted next; if that fails as well,
      the path switches to a fixed drive)
    - refreshing the path (Ctrl+R) behaves the same as changing the path to the current path (the path
      may remain unchanged or it may be shortened or, in case of a fatal error, changed to a fixed drive);
      when the path is refreshed the parameter 'forceRefresh' is TRUE for all calls of
      ChangePath and ListCurrentPath (the FS must not use any cache for changing the path or loading the listing - the user does not want to use the cache);

  When traversing the history (back/forward) the FS interface in which the listing of the FS path
  ('fsName':'fsUserPart') will occur is obtained using the first possible method from the following:
    - the FS interface in which the path was last opened has not been closed yet
      and is among the detached interfaces or is active in the panel (not active in the other panel)
    - the active FS interface in the panel ('currentFSName') belongs to the same plugin as
      'fsName' and IsOurPath('currentFSName', 'fsName', 'fsUserPart') returns TRUE
    - the first of the detached FS interfaces ('currentFSName') that belongs to the same
      plugin as 'fsName' and for which IsOurPath('currentFSName', 'fsName', 'fsUserPart') returns TRUE
    - a new FS interface
*/
