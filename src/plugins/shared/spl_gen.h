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
#pragma pack(push, enter_include_spl_gen) // keep structures independent of the configured alignment
#pragma pack(4)
#endif // _MSC_VER
#ifdef __BORLANDC__
#pragma option -a4
#endif // __BORLANDC__

struct CFileData;
class CPluginDataInterfaceAbstract;

//
// ****************************************************************************
// CSalamanderGeneralAbstract
//
// generally applicable Salamander methods (for all plugin types)

// message box types
#define MSGBOX_INFO 0
#define MSGBOX_ERROR 1
#define MSGBOX_EX_ERROR 2
#define MSGBOX_QUESTION 3
#define MSGBOX_EX_QUESTION 4
#define MSGBOX_WARNING 5
#define MSGBOX_EX_WARNING 6

// constants for CSalamanderGeneralAbstract::SalMessageBoxEx
#define MSGBOXEX_OK 0x00000000                // MB_OK
#define MSGBOXEX_OKCANCEL 0x00000001          // MB_OKCANCEL
#define MSGBOXEX_ABORTRETRYIGNORE 0x00000002  // MB_ABORTRETRYIGNORE
#define MSGBOXEX_YESNOCANCEL 0x00000003       // MB_YESNOCANCEL
#define MSGBOXEX_YESNO 0x00000004             // MB_YESNO
#define MSGBOXEX_RETRYCANCEL 0x00000005       // MB_RETRYCANCEL
#define MSGBOXEX_CANCELTRYCONTINUE 0x00000006 // MB_CANCELTRYCONTINUE
#define MSGBOXEX_CONTINUEABORT 0x00000007     // MB_CONTINUEABORT
#define MSGBOXEX_YESNOOKCANCEL 0x00000008

#define MSGBOXEX_ICONHAND 0x00000010        // MB_ICONHAND / MB_ICONSTOP / MB_ICONERROR
#define MSGBOXEX_ICONQUESTION 0x00000020    // MB_ICONQUESTION
#define MSGBOXEX_ICONEXCLAMATION 0x00000030 // MB_ICONEXCLAMATION / MB_ICONWARNING
#define MSGBOXEX_ICONINFORMATION 0x00000040 // MB_ICONASTERISK / MB_ICONINFORMATION

#define MSGBOXEX_DEFBUTTON1 0x00000000 // MB_DEFBUTTON1
#define MSGBOXEX_DEFBUTTON2 0x00000100 // MB_DEFBUTTON2
#define MSGBOXEX_DEFBUTTON3 0x00000200 // MB_DEFBUTTON3
#define MSGBOXEX_DEFBUTTON4 0x00000300 // MB_DEFBUTTON4

#define MSGBOXEX_HELP 0x00004000 // MB_HELP (bit mask)

#define MSGBOXEX_SETFOREGROUND 0x00010000 // MB_SETFOREGROUND (bit mask)

// Altap-specific
#define MSGBOXEX_SILENT 0x10000000 // the message box does not play any sound when opened (bit mask)
// for an MB_YESNO message box, enables Escape (generates IDNO); for an MB_ABORTRETRYIGNORE message box
// enables Escape (generates IDCANCEL) (bit mask)
#define MSGBOXEX_ESCAPEENABLED 0x20000000
#define MSGBOXEX_HINT 0x40000000 // if CheckBoxText is used, the \t separator is searched for in it and displayed as a hint
// Vista: the default button will have the "requires elevation" state (the elevated icon will be shown)
#define MSGBOXEX_SHIELDONDEFBTN 0x80000000

#define MSGBOXEX_TYPEMASK 0x0000000F // MB_TYPEMASK
#define MSGBOXEX_ICONMASK 0x000000F0 // MB_ICONMASK
#define MSGBOXEX_DEFMASK 0x00000F00  // MB_DEFMASK
#define MSGBOXEX_MODEMASK 0x00003000 // MB_MODEMASK
#define MSGBOXEX_MISCMASK 0x0000C000 // MB_MISCMASK
#define MSGBOXEX_EXMASK 0xF0000000

// message box return values
#define DIALOG_FAIL 0x00000000 // the dialog could not be opened
// individual buttons
#define DIALOG_OK 0x00000001       // IDOK
#define DIALOG_CANCEL 0x00000002   // IDCANCEL
#define DIALOG_ABORT 0x00000003    // IDABORT
#define DIALOG_RETRY 0x00000004    // IDRETRY
#define DIALOG_IGNORE 0x00000005   // IDIGNORE
#define DIALOG_YES 0x00000006      // IDYES
#define DIALOG_NO 0x00000007       // IDNO
#define DIALOG_TRYAGAIN 0x0000000a // IDTRYAGAIN
#define DIALOG_CONTINUE 0x0000000b // IDCONTINUE
// altap specific
#define DIALOG_SKIP 0x10000000
#define DIALOG_SKIPALL 0x20000000
#define DIALOG_ALL 0x30000000

typedef void(CALLBACK* MSGBOXEX_CALLBACK)(LPHELPINFO helpInfo);

struct MSGBOXEX_PARAMS
{
    HWND HParent;
    const char* Text;
    const char* Caption;
    DWORD Flags;
    HICON HIcon;
    DWORD ContextHelpId;
    MSGBOXEX_CALLBACK HelpCallback;
    const char* CheckBoxText;
    BOOL* CheckBoxValue;
    const char* AliasBtnNames;
    const char* URL;
    const char* URLText;
};

/*
HParent
  Handle to the owner window. The message box is centered on this window.
  If this parameter is NULL, the message box has no owner window.

Text
  Pointer to a null-terminated string that contains the message to be displayed.

Caption
  Pointer to a null-terminated string that contains the message box title.
  If this member is NULL, the default title "Error" is used.

Flags
  Specifies the contents and behavior of the message box.
  This parameter can be a combination of flags from the following groups of flags.

   To indicate the buttons displayed in the message box, specify one of the following values.
    MSGBOXEX_OK                   (MB_OK)
      The message box contains one push button: OK. This is the default.
      The message box can be closed using Escape, and the return value will be DIALOG_OK (IDOK).
    MSGBOXEX_OKCANCEL             (MB_OKCANCEL)
      The message box contains two push buttons: OK and Cancel.
    MSGBOXEX_ABORTRETRYIGNORE     (MB_ABORTRETRYIGNORE)
      The message box contains three push buttons: Abort, Retry, and Ignore.
      The message box can be closed using Escape when the MSGBOXEX_ESCAPEENABLED flag is specified.
      In that case, the return value will be DIALOG_CANCEL (IDCANCEL).
    MSGBOXEX_YESNOCANCEL          (MB_YESNOCANCEL)
      The message box contains three push buttons: Yes, No, and Cancel.
    MSGBOXEX_YESNO                (MB_YESNO)
      The message box contains two push buttons: Yes and No.
      The message box can be closed using Escape when the MSGBOXEX_ESCAPEENABLED flag is specified.
      In that case, the return value will be DIALOG_NO (IDNO).
    MSGBOXEX_RETRYCANCEL          (MB_RETRYCANCEL)
      The message box contains two push buttons: Retry and Cancel.
    MSGBOXEX_CANCELTRYCONTINUE    (MB_CANCELTRYCONTINUE)
      The message box contains three push buttons: Cancel, Try Again, and Continue.

   To display an icon in the message box, specify one of the following values.
    MSGBOXEX_ICONHAND             (MB_ICONHAND / MB_ICONSTOP / MB_ICONERROR)
      A stop-sign icon appears in the message box.
    MSGBOXEX_ICONQUESTION         (MB_ICONQUESTION)
      A question-mark icon appears in the message box.
    MSGBOXEX_ICONEXCLAMATION      (MB_ICONEXCLAMATION / MB_ICONWARNING)
      An exclamation-point icon appears in the message box.
    MSGBOXEX_ICONINFORMATION      (MB_ICONASTERISK / MB_ICONINFORMATION)
      An icon consisting of a lowercase letter i in a circle appears in the message box.

   To indicate the default button, specify one of the following values.
    MSGBOXEX_DEFBUTTON1           (MB_DEFBUTTON1)
      The first button is the default button.
      MSGBOXEX_DEFBUTTON1 is the default unless MSGBOXEX_DEFBUTTON2, MSGBOXEX_DEFBUTTON3,
      or MSGBOXEX_DEFBUTTON4 is specified.
    MSGBOXEX_DEFBUTTON2           (MB_DEFBUTTON2)
      The second button is the default button.
    MSGBOXEX_DEFBUTTON3           (MB_DEFBUTTON3)
      The third button is the default button.
    MSGBOXEX_DEFBUTTON4           (MB_DEFBUTTON4)
      The fourth button is the default button.

   To specify other options, use one or more of the following values.
    MSGBOXEX_HELP                 (MB_HELP)
      Adds a Help button to the message box.
      When the user clicks the Help button or presses F1, the system sends a WM_HELP message to the owner
      or calls HelpCallback (see HelpCallback for details).
    MSGBOXEX_SETFOREGROUND        (MB_SETFOREGROUND)
      The message box becomes the foreground window. Internally, the system calls the SetForegroundWindow
      function for the message box.
    MSGBOXEX_SILENT
      No sound is played when the message box is displayed.
    MSGBOXEX_ESCAPEENABLED
      When MSGBOXEX_YESNO is specified, the user can close the message box using the Escape key and DIALOG_NO (IDNO)
      will be returned. When MSGBOXEX_ABORTRETRYIGNORE is specified, the user can close the message box using the
      Escape key and DIALOG_CANCEL (IDCANCEL) will be returned. Otherwise, this option is ignored.

HIcon
  Handle to the icon to be drawn in the message box. The icon is not destroyed when the message box is closed.
  If this parameter is NULL, the MSGBOXEX_ICONxxx style is used.

ContextHelpId
  Identifies a help context. If a help event occurs, this value is specified in
  the HELPINFO structure that the message box sends to the owner window or callback function.

HelpCallback
  Pointer to the callback function that processes help events for the message box.
  The callback function has the following form:
    VOID CALLBACK MSGBOXEX_CALLBACK(LPHELPINFO helpInfo)
  If this member is NULL, the message box sends WM_HELP messages to the owner window
  when help events occur.

CheckBoxText
  Pointer to a null-terminated string that contains the check box text.
  If the MSGBOXEX_HINT flag is specified in Flags, this text must contain HINT.
  The hint is separated from the string by a TAB character. The hint is divided into two parts
  by the second TAB character. The first part is the label displayed next to the check box.
  The second part is the text displayed when the user clicks the hint label.

  Example: "This is text for checkbox\tHint Label\tThis text will be displayed when user click the Hint Label."
  If this member is NULL, the check box is not displayed.

CheckBoxValue
  Pointer to a BOOL variable that contains the initial and returned check box state (TRUE: checked, FALSE: unchecked).
  This parameter is ignored if the CheckBoxText parameter is NULL. Otherwise, this parameter must be set.

AliasBtnNames
  Pointer to a buffer containing pairs of IDs and alias strings. The last string in the
  buffer must be terminated by a NULL character.

  The first string in each pair is a decimal number that specifies the button ID.
  The number must be one of the DIALOG_xxx values. The second string specifies the alias text
  for this button.

  The first and second strings in each pair are separated by a TAB character.
  Pairs are also separated by a TAB character.

  If this member is NULL, the normal button names are displayed.

  Example: sprintf(buffer, "%d\t%s\t%d\t%s", DIALOG_OK, "&Start", DIALOG_CANCEL, "E&xit");
           buffer: "1\t&Start\t2\tE&xit"

URL
  Pointer to a null-terminated string that contains the URL displayed below the text.
  If this member is NULL, the URL is not displayed.

URLText
  Pointer to a null-terminated string that contains the URL text displayed below the text.
  If this member is NULL, the URL itself is displayed instead.

*/

// panel identifiers
#define PANEL_SOURCE 1 // source panel (active panel)
#define PANEL_TARGET 2 // target panel (inactive panel)
#define PANEL_LEFT 3   // left panel
#define PANEL_RIGHT 4  // right panel

// path types
#define PATH_TYPE_WINDOWS 1 // Windows path ("c:\path" or UNC path)
#define PATH_TYPE_ARCHIVE 2 // path into an archive (the archive resides on a Windows path)
#define PATH_TYPE_FS 3      // path in a plugin file system

// Only one flag from the following group can be selected.
// They define the set of buttons displayed in various error messages.
#define BUTTONS_OK 0x00000000               // OK
#define BUTTONS_RETRYCANCEL 0x00000001      // Retry / Cancel
#define BUTTONS_SKIPCANCEL 0x00000002       // Skip / Skip all / Cancel
#define BUTTONS_RETRYSKIPCANCEL 0x00000003  // Retry / Skip / Skip all / Cancel
#define BUTTONS_YESALLSKIPCANCEL 0x00000004 // Yes / All / Skip / Skip all / Cancel
#define BUTTONS_YESNOCANCEL 0x00000005      // Yes / No / Cancel
#define BUTTONS_YESALLCANCEL 0x00000006     // Yes / All / Cancel
#define BUTTONS_MASK 0x000000FF             // internal mask, do not use
// I keep the detection of whether a combination has a SKIP or YES button inline so that
// when introducing new combinations it remains visible and we remember to extend it
inline BOOL ButtonsContainsSkip(DWORD btn)
{
    return (btn & BUTTONS_MASK) == BUTTONS_SKIPCANCEL ||
           (btn & BUTTONS_MASK) == BUTTONS_RETRYSKIPCANCEL ||
           (btn & BUTTONS_MASK) == BUTTONS_YESALLSKIPCANCEL;
}
inline BOOL ButtonsContainsYes(DWORD btn)
{
    return (btn & BUTTONS_MASK) == BUTTONS_YESALLSKIPCANCEL ||
           (btn & BUTTONS_MASK) == BUTTONS_YESNOCANCEL ||
           (btn & BUTTONS_MASK) == BUTTONS_YESALLCANCEL;
}

// error codes for CSalamanderGeneralAbstract::SalGetFullName
#define GFN_SERVERNAMEMISSING 1   // the server name is missing in the UNC path
#define GFN_SHARENAMEMISSING 2    // the share name is missing in the UNC path
#define GFN_TOOLONGPATH 3         // the operation would create a path that is too long
#define GFN_INVALIDDRIVE 4        // a standard path (c:\) does not contain a drive letter A-Z (or a-z)
#define GFN_INCOMLETEFILENAME 5   // relative path without a specified 'curDir' -> cannot be resolved
#define GFN_EMPTYNAMENOTALLOWED 6 // empty 'name' string
#define GFN_PATHISINVALID 7       // cannot rule out "..", e.g. "c:\.."

// error code for the case when the user terminates CSalamanderGeneralAbstract::SalCheckPath by pressing ESC
#define ERROR_USER_TERMINATED -100

#define PATH_MAX_PATH 248 // maximum path length limit (full directory name); note: the null terminator is already included (max. string length is 247 characters)

// error constants for CSalamanderGeneralAbstract::SalParsePath:
// the input was an empty path and 'curPath' was NULL (an empty path is replaced with the current path,
// but it is not known here)
#define SPP_EMPTYPATHNOTALLOWED 1
// the Windows path (normal + UNC) does not exist, is inaccessible, or the user interrupted the
// path accessibility check (this also includes an attempt to restore the network connection)
#define SPP_WINDOWSPATHERROR 2
// the Windows path starts with a file name that is not an archive (otherwise it would be a path inside an archive)
#define SPP_NOTARCHIVEFILE 3
// FS path - the plugin FS name (fs-name - before ':' in the path) is not known (no plugin
// registered this name)
#define SPP_NOTPLUGINFS 4
// this is a relative path, but the current path is unknown or it is an FS path (the root cannot be determined there
// and the structure of the fs-user-part path is not known at all, so it cannot be converted to an absolute path)
// if the current path is an FS path ('curPathIsDiskOrArchive' is FALSE), no error is reported to the user in this case
// (further processing is expected in the FS that called the SalParsePath method)
#define SPP_INCOMLETEPATH 5

// Salamander internal color constants
#define SALCOL_FOCUS_ACTIVE_NORMAL 0 // pen colors for the frame around an item
#define SALCOL_FOCUS_ACTIVE_SELECTED 1
#define SALCOL_FOCUS_FG_INACTIVE_NORMAL 2
#define SALCOL_FOCUS_FG_INACTIVE_SELECTED 3
#define SALCOL_FOCUS_BK_INACTIVE_NORMAL 4
#define SALCOL_FOCUS_BK_INACTIVE_SELECTED 5
#define SALCOL_ITEM_FG_NORMAL 6 // item text colors in the panel
#define SALCOL_ITEM_FG_SELECTED 7
#define SALCOL_ITEM_FG_FOCUSED 8
#define SALCOL_ITEM_FG_FOCSEL 9
#define SALCOL_ITEM_FG_HIGHLIGHT 10
#define SALCOL_ITEM_BK_NORMAL 11 // item background colors in the panel
#define SALCOL_ITEM_BK_SELECTED 12
#define SALCOL_ITEM_BK_FOCUSED 13
#define SALCOL_ITEM_BK_FOCSEL 14
#define SALCOL_ITEM_BK_HIGHLIGHT 15
#define SALCOL_ICON_BLEND_SELECTED 16 // colors for icon blending
#define SALCOL_ICON_BLEND_FOCUSED 17
#define SALCOL_ICON_BLEND_FOCSEL 18
#define SALCOL_PROGRESS_FG_NORMAL 19 // progress bar colors
#define SALCOL_PROGRESS_FG_SELECTED 20
#define SALCOL_PROGRESS_BK_NORMAL 21
#define SALCOL_PROGRESS_BK_SELECTED 22
#define SALCOL_HOT_PANEL 23           // color of the hot item in the panel
#define SALCOL_HOT_ACTIVE 24          //                   in the active window caption
#define SALCOL_HOT_INACTIVE 25        //                   in the inactive caption, status bar, ...
#define SALCOL_ACTIVE_CAPTION_FG 26   // text color in the active panel caption
#define SALCOL_ACTIVE_CAPTION_BK 27   // background color in the active panel caption
#define SALCOL_INACTIVE_CAPTION_FG 28 // text color in the inactive panel caption
#define SALCOL_INACTIVE_CAPTION_BK 29 // background color in the inactive panel caption
#define SALCOL_VIEWER_FG_NORMAL 30    // text color in the internal text/hex viewer
#define SALCOL_VIEWER_BK_NORMAL 31    // background color in the internal text/hex viewer
#define SALCOL_VIEWER_FG_SELECTED 32  // selected text color in the internal text/hex viewer
#define SALCOL_VIEWER_BK_SELECTED 33  // selected background color in the internal text/hex viewer
#define SALCOL_THUMBNAIL_NORMAL 34    // pen colors for the frame around a thumbnail
#define SALCOL_THUMBNAIL_SELECTED 35
#define SALCOL_THUMBNAIL_FOCUSED 36
#define SALCOL_THUMBNAIL_FOCSEL 37

// failure reason constants for CSalamanderGeneralAbstract::ChangePanelPathToXXX methods:
#define CHPPFR_SUCCESS 0 // the panel contains the new path, success (return value is TRUE)
// the new path (or archive name) cannot be converted from relative to absolute or
// the new path (or archive name) is inaccessible or
// the file system path cannot be opened (no plugin, it refuses to load, it refuses to open the FS, fatal error in ChangePath)
#define CHPPFR_INVALIDPATH 1
#define CHPPFR_INVALIDARCHIVE 2  // the file is not an archive or the archive cannot be listed
#define CHPPFR_CANNOTCLOSEPATH 4 // the current path cannot be closed
// the panel contains a shortened new path,
// clarification for FS: the panel contains either the shortened new path, the original path, or the shortened
// original path - an attempt is made to return the original path to the panel only if the new path was opened
// in the current FS (the IsOurPath method returned TRUE for it) and if the new path is inaccessible
// (and none of its subpaths is accessible)
#define CHPPFR_SHORTERPATH 5
// the panel contains a shortened new path; the path was shortened because the requested path was a file name
// - the panel contains the path to the file and the file will be focused
#define CHPPFR_FILENAMEFOCUSED 6

// types for CSalamanderGeneralAbstract::ValidateVarString() and CSalamanderGeneralAbstract::ExpandVarString()
typedef const char*(WINAPI* FSalamanderVarStrGetValue)(HWND msgParent, void* param);
struct CSalamanderVarStrEntry
{
    const char* Name;                  // variable name in the string (for example, for "$(name)" it is "name")
    FSalamanderVarStrGetValue Execute; // function that returns text representing the variable
};

class CSalamanderRegistryAbstract;

// callback type used when loading or saving configuration via
// CSalamanderGeneral::CallLoadOrSaveConfiguration; 'regKey' is NULL when loading
// the default configuration ('save' is not called when 'regKey' == NULL); 'registry' is the object used
// to work with the registry; 'param' is the user-supplied function parameter (see
// CSalamanderGeneral::CallLoadOrSaveConfiguration)
typedef void(WINAPI* FSalLoadOrSaveConfiguration)(BOOL load, HKEY regKey,
                                                  CSalamanderRegistryAbstract* registry, void* param);

// base structure for CSalamanderGeneralAbstract::ViewFileInPluginViewer (each plugin
// viewer can extend this structure with its own parameters - the structure is passed to
// CPluginInterfaceForViewerAbstract::ViewFile - the parameters can be, for example, the window caption,
// viewer mode, offset from the beginning of the file, selection position, etc.); WARNING!!! about structure packing
// (4-byte packing is required - see "#pragma pack(4)")
struct CSalamanderPluginViewerData
{
    // how many bytes from the start of the structure are valid (to distinguish structure versions)
    int Size;
    // name of the file to open in the viewer (do not use in the method
    // CPluginInterfaceForViewerAbstract::ViewFile - the file name is provided by the 'name' parameter)
    const char* FileName;
};

// extension of CSalamanderPluginViewerData for the internal text/hex viewer
struct CSalamanderPluginInternalViewerData : public CSalamanderPluginViewerData
{
    int Mode;            // 0 - text mode, 1 - hex mode
    const char* Caption; // NULL -> uses the FileName window caption, otherwise uses Caption
    BOOL WholeCaption;   // meaningful only if Caption != NULL. TRUE -> the title bar
                         // displays only the Caption string; FALSE ->
                         // the standard " - Viewer" is appended to Caption.
};

// constants for Salamander configuration parameter types (see CSalamanderGeneralAbstract::GetConfigParameter)
#define SALCFGTYPE_NOTFOUND 0 // parameter not found
#define SALCFGTYPE_BOOL 1     // TRUE/FALSE
#define SALCFGTYPE_INT 2      // 32-bit integer
#define SALCFGTYPE_STRING 3   // null-terminated multibyte string
#define SALCFGTYPE_LOGFONT 4  // Win32 LOGFONT structure

// Constants for Salamander configuration parameters (see CSalamanderGeneralAbstract::GetConfigParameter);
// the parameter type is given in the comment (BOOL, INT, STRING); for STRING, the required
// string buffer size is given in parentheses
//
// general parameters
#define SALCFG_SELOPINCLUDEDIRS 1        // BOOL, select/deselect operations (num *, num +, num -) work also with directories
#define SALCFG_SAVEONEXIT 2              // BOOL, save configuration on Salamander exit
#define SALCFG_MINBEEPWHENDONE 3         // BOOL, should it beep (play sound) when work finishes in an inactive window?
#define SALCFG_HIDEHIDDENORSYSTEMFILES 4 // BOOL, should it hide system and/or hidden files?
#define SALCFG_ALWAYSONTOP 6             // BOOL, is the main window Always On Top?
#define SALCFG_SORTUSESLOCALE 7          // BOOL, should it use regional settings when sorting?
#define SALCFG_SINGLECLICK 8             // BOOL, single click mode (single click to open file, etc.)
#define SALCFG_TOPTOOLBARVISIBLE 9       // BOOL, is top toolbar visible?
#define SALCFG_BOTTOMTOOLBARVISIBLE 10   // BOOL, is bottom toolbar visible?
#define SALCFG_USERMENUTOOLBARVISIBLE 11 // BOOL, is user-menu toolbar visible?
#define SALCFG_INFOLINECONTENT 12        // STRING (200), content of Information Line (string with parameters)
#define SALCFG_FILENAMEFORMAT 13         // INT, how to alter file name before displaying (parameter 'format' to CSalamanderGeneralAbstract::AlterFileName)
#define SALCFG_SAVEHISTORY 14            // BOOL, may history related data be stored to configuration?
#define SALCFG_ENABLECMDLINEHISTORY 15   // BOOL, is command line history enabled?
#define SALCFG_SAVECMDLINEHISTORY 16     // BOOL, may command line history be stored to configuration?
#define SALCFG_MIDDLETOOLBARVISIBLE 17   // BOOL, is middle toolbar visible?
#define SALCFG_SORTDETECTNUMBERS 18      // BOOL, should it use numerical sort for numbers contained in strings when sorting?
#define SALCFG_SORTBYEXTDIRSASFILES 19   // BOOL, should it treat dirs as files when sorting by extension? BTW, if TRUE, directory extensions are also displayed in a separate Ext column. (directories have no extensions, only files do, but many people have requested sorting by extension and displaying the extension in a separate Ext column even for directories)
#define SALCFG_SIZEFORMAT 20             // INT, units for custom size columns, 0 - Bytes, 1 - KB, 2 - short (mixed B, KB, MB, GB, ...)
#define SALCFG_SELECTWHOLENAME 21        // BOOL, should be whole name selected (including extension) when entering new filename? (for dialog boxes F2:QuickRename, Alt+F5:Pack, etc)
// recycle bin parameters
#define SALCFG_USERECYCLEBIN 50   // INT, 0 - do not use, 1 - use for all, 2 - use for files matching at \
                                  //      least one of masks (see SALCFG_RECYCLEBINMASKS)
#define SALCFG_RECYCLEBINMASKS 51 // STRING (MAX_PATH), masks for SALCFG_USERECYCLEBIN==2
// time resolution for file comparison (used by the Compare Directories command)
#define SALCFG_COMPDIRSUSETIMERES 60 // BOOL, should it use time resolution? (FALSE==exact match)
#define SALCFG_COMPDIRTIMERES 61     // INT, time resolution for file comparison (from 0 to 3600 seconds)
// confirmations
#define SALCFG_CNFRMFILEDIRDEL 70 // BOOL, file or directory deletion
#define SALCFG_CNFRMNEDIRDEL 71   // BOOL, non-empty directory deletion
#define SALCFG_CNFRMFILEOVER 72   // BOOL, file overwrite confirmation
#define SALCFG_CNFRMSHFILEDEL 73  // BOOL, system or hidden file delete
#define SALCFG_CNFRMSHDIRDEL 74   // BOOL, system or hidden directory deletion
#define SALCFG_CNFRMSHFILEOVER 75 // BOOL, system or hidden file overwrite
#define SALCFG_CNFRMCREATEPATH 76 // BOOL, show "do you want to create target path?" in Copy/Move operations
#define SALCFG_CNFRMDIROVER 77    // BOOL, directory overwrite (copy/move selected directory: ask user if directory already exists on target path - standard behaviour is to join contents of both directories)
// drive-specific settings
#define SALCFG_DRVSPECFLOPPYMON 88         // BOOL, floppy disks - use automatic refresh (changes monitoring)
#define SALCFG_DRVSPECFLOPPYSIM 89         // BOOL, floppy disks - use simple icons
#define SALCFG_DRVSPECREMOVABLEMON 90      // BOOL, removable disks - use automatic refresh (changes monitoring)
#define SALCFG_DRVSPECREMOVABLESIM 91      // BOOL, removable disks - use simple icons
#define SALCFG_DRVSPECFIXEDMON 92          // BOOL, fixed disks - use automatic refresh (changes monitoring)
#define SALCFG_DRVSPECFIXEDSIMPLE 93       // BOOL, fixed disks - use simple icons
#define SALCFG_DRVSPECREMOTEMON 94         // BOOL, remote (network) disks - use automatic refresh (changes monitoring)
#define SALCFG_DRVSPECREMOTESIMPLE 95      // BOOL, remote (network) disks - use simple icons
#define SALCFG_DRVSPECREMOTEDONOTREF 96    // BOOL, remote (network) disks - do not refresh on activation of Salamander
#define SALCFG_DRVSPECCDROMMON 97          // BOOL, CDROM disks - use automatic refresh (changes monitoring)
#define SALCFG_DRVSPECCDROMSIMPLE 98       // BOOL, CDROM disks - use simple icons
#define SALCFG_IFPATHISINACCESSIBLEGOTO 99 // STRING (MAX_PATH), path to go to if the path in the panel is inaccessible
// internal text/hex viewer
#define SALCFG_VIEWEREOLCRLF 120          // BOOL, accept CR-LF ("\r\n") line ends?
#define SALCFG_VIEWEREOLCR 121            // BOOL, accept CR ("\r") line ends?
#define SALCFG_VIEWEREOLLF 122            // BOOL, accept LF ("\n") line endings?
#define SALCFG_VIEWEREOLNULL 123          // BOOL, accept NULL ("\0") line endings?
#define SALCFG_VIEWERTABSIZE 124          // INT, size of tab ("\t") character in spaces
#define SALCFG_VIEWERSAVEPOSITION 125     // BOOL, TRUE = save position of viewer window, FALSE = always use position of main window
#define SALCFG_VIEWERFONT 126             // LOGFONT, viewer font
#define SALCFG_VIEWERWRAPTEXT 127         // BOOL, wrap text (split a long text line into multiple lines)
#define SALCFG_AUTOCOPYSELTOCLIPBOARD 128 // BOOL, TRUE = when the user selects some text, it is instantly copied to the clipboard
// archivers
#define SALCFG_ARCOTHERPANELFORPACK 140    // BOOL, should it pack to the other panel path?
#define SALCFG_ARCOTHERPANELFORUNPACK 141  // BOOL, should it unpack to the other panel path?
#define SALCFG_ARCSUBDIRBYARCFORUNPACK 142 // BOOL, should it unpack to a subdirectory named after the archive?
#define SALCFG_ARCUSESIMPLEICONS 143       // BOOL, use simple icons in archives?

// callback type used by the CSalamanderGeneral::SalSplitGeneralPath method
typedef BOOL(WINAPI* SGP_IsTheSamePathF)(const char* path1, const char* path2);

// callback type used by the CSalamanderGeneralAbstract::CallPluginOperationFromDisk method
// 'sourcePath' is the source path on the disk (other paths are relative to it);
// selected files/directories are provided by the enumeration function 'next' whose argument is
// 'nextParam'; 'param' is the value passed to CallPluginOperationFromDisk as 'param'
typedef void(WINAPI* SalPluginOperationFromDisk)(const char* sourcePath, SalEnumSelection2 next,
                                                 void* nextParam, void* param);

// flags for text search algorithms (CSalamanderBMSearchData and CSalamanderREGEXPSearchData);
// flags can be combined with bitwise OR
#define SASF_CASESENSITIVE 0x01 // character case matters (if not set, the search ignores case)
#define SASF_FORWARD 0x02       // search forward (if not set, the search is performed backwards)

// icons for GetSalamanderIcon
#define SALICON_EXECUTABLE 1    // exe/bat/pif/com
#define SALICON_DIRECTORY 2     // dir
#define SALICON_NONASSOCIATED 3 // non-associated file
#define SALICON_ASSOCIATED 4    // associated file
#define SALICON_UPDIR 5         // up-dir ".."
#define SALICON_ARCHIVE 6       // archive

// icon sizes for GetSalamanderIcon
#define SALICONSIZE_16 1 // 16x16
#define SALICONSIZE_32 2 // 32x32
#define SALICONSIZE_48 3 // 48x48

// interface of the Boyer-Moore algorithm object for searching in text
// WARNING: each allocated object may be used only within a single thread
// (it does not have to be the main thread, and different objects can live in different threads)
class CSalamanderBMSearchData
{
public:
    // sets the pattern; 'pattern' is a null-terminated pattern string; 'flags' are
    // algorithm flags (see the SASF_XXX constants)
    virtual void WINAPI Set(const char* pattern, WORD flags) = 0;

    // sets the pattern; 'pattern' is a binary pattern of length 'length' (the 'pattern' buffer must
    // have at least ('length' + 1) characters – only to stay compatible with text patterns);
    // 'flags' are algorithm flags (see the SASF_XXX constants)
    virtual void WINAPI Set(const char* pattern, const int length, WORD flags) = 0;

    // sets the algorithm flags; 'flags' are algorithm flags (see the SASF_XXX constants)
    virtual void WINAPI SetFlags(WORD flags) = 0;

    // returns the pattern length (valid only after a successful call to Set)
    virtual int WINAPI GetLength() const = 0;

    // returns the pattern (valid only after a successful call to Set)
    virtual const char* WINAPI GetPattern() const = 0;

    // returns TRUE if searching can start (the pattern and flags were set successfully;
    // failure is only possible when the pattern is empty)
    virtual BOOL WINAPI IsGood() const = 0;

    // searches for the pattern in the 'text' buffer of length 'length' forward from the 'start' offset;
    // returns the offset of the found pattern or -1 if the pattern was not found;
    // WARNING: the algorithm must have the SASF_FORWARD flag set
    virtual int WINAPI SearchForward(const char* text, int length, int start) = 0;

    // searches for the pattern in the 'text' buffer of length 'length' backwards (starts at the end of the text);
    // returns the offset of the found pattern or -1 if the pattern was not found;
    // WARNING: the algorithm must not have the SASF_FORWARD flag set
    virtual int WINAPI SearchBackward(const char* text, int length) = 0;
};

// interface of the regular-expression search object
// WARNING: each allocated object may be used only within a single thread
// (it does not have to be the main thread, and different objects can live in different threads)
class CSalamanderREGEXPSearchData
{
public:
    // sets the regular expression; 'pattern' is a null-terminated regular-expression string; 'flags'
    // are algorithm flags (see the SASF_XXX constants); returns FALSE on error and the error description
    // can be obtained by calling the GetLastErrorText method
    virtual BOOL WINAPI Set(const char* pattern, WORD flags) = 0;

    // sets the algorithm flags; 'flags' are algorithm flags (see the SASF_XXX constants);
    // returns FALSE on error and the error description can be obtained by calling GetLastErrorText
    virtual BOOL WINAPI SetFlags(WORD flags) = 0;

    // returns the error text produced by the last call to Set or SetFlags (can be NULL)
    virtual const char* WINAPI GetLastErrorText() const = 0;

    // returns the regular expression pattern (valid only after a successful call to Set)
    virtual const char* WINAPI GetPattern() const = 0;

    // sets the text line (the line ranges from 'start' to 'end', 'end' points past the last character),
    // in which the search is performed; always returns TRUE
    virtual BOOL WINAPI SetLine(const char* start, const char* end) = 0;

    // searches for a substring matching the regular expression in the line set by SetLine;
    // searches forward from the 'start' offset; returns the offset of the found substring and its length
    // (via 'foundLen') or -1 if the substring was not found;
    // WARNING: the algorithm must have the SASF_FORWARD flag set
    virtual int WINAPI SearchForward(int start, int& foundLen) = 0;

    // searches for a substring matching the regular expression in the line set by SetLine;
    // searches backwards (starts from the end of the text of length 'length' counted from the beginning of the line);
    // returns the offset of the found substring and its length (via 'foundLen') or -1 if the substring
    // was not found;
    // WARNING: the algorithm must not have the SASF_FORWARD flag set
    virtual int WINAPI SearchBackward(int length, int& foundLen) = 0;
};

// command types used by CSalamanderGeneralAbstract::EnumSalamanderCommands
#define sctyUnknown 0
#define sctyForFocusedFile 1                 // only for the focused file (e.g. View)
#define sctyForFocusedFileOrDirectory 2      // for the focused file or directory (e.g. Open)
#define sctyForSelectedFilesAndDirectories 3 // for selected/focused files and directories (e.g. Copy)
#define sctyForCurrentPath 4                 // for the current path in the panel (e.g. Create Directory)
#define sctyForConnectedDrivesAndFS 5        // for connected drives and file systems (e.g. Disconnect)

// Salamander commands used in CSalamanderGeneralAbstract::EnumSalamanderCommands
// and CSalamanderGeneralAbstract::PostSalamanderCommand
// (WARNING: only the range <0, 499> is reserved for command numbers)
#define SALCMD_VIEW 0     // view (F3 in the panel)
#define SALCMD_ALTVIEW 1  // alternate view (Alt+F3 in the panel)
#define SALCMD_VIEWWITH 2 // view with (Ctrl+Shift+F3 in the panel)
#define SALCMD_EDIT 3     // edit (F4 in the panel)
#define SALCMD_EDITWITH 4 // edit with (Ctrl+Shift+F4 in the panel)

#define SALCMD_OPEN 20        // open (Enter in the panel)
#define SALCMD_QUICKRENAME 21 // quick rename (F2 in the panel)

#define SALCMD_COPY 40          // copy (F5 in the panel)
#define SALCMD_MOVE 41          // move/rename (F6 in the panel)
#define SALCMD_EMAIL 42         // email (Ctrl+E in the panel)
#define SALCMD_DELETE 43        // delete (Delete key in the panel)
#define SALCMD_PROPERTIES 44    // show properties (Alt+Enter in the panel)
#define SALCMD_CHANGECASE 45    // change case (Ctrl+F7 in the panel)
#define SALCMD_CHANGEATTRS 46   // change attributes (Ctrl+F2 in the panel)
#define SALCMD_OCCUPIEDSPACE 47 // calculate occupied space (Alt+F10 in the panel)

#define SALCMD_EDITNEWFILE 70     // edit new file (Shift+F4 in the panel)
#define SALCMD_REFRESH 71         // refresh (Ctrl+R in the panel)
#define SALCMD_CREATEDIRECTORY 72 // create directory (F7 in the panel)
#define SALCMD_DRIVEINFO 73       // drive info (Ctrl+F1 in the panel)
#define SALCMD_CALCDIRSIZES 74    // calculate directory sizes (Ctrl+Shift+F10 in the panel)

#define SALCMD_DISCONNECT 90 // disconnect (network drive or plugin FS) (F12 in the panel)

#define MAX_GROUPMASK 1001 // maximum number of characters (including the trailing null) in a group mask

// identifiers of shared history lists (most recently used values in combo boxes) for
// CSalamanderGeneral::GetStdHistoryValues()
#define SALHIST_QUICKRENAME 1 // names in the Quick Rename dialog (F2)
#define SALHIST_COPYMOVETGT 2 // target paths in the Copy/Move dialog (F5/F6)
#define SALHIST_CREATEDIR 3   // directory names in the Create Directory dialog (F7)
#define SALHIST_CHANGEDIR 4   // paths in the Change Directory dialog (Shift+F7)
#define SALHIST_EDITNEW 5     // names in the Edit New dialog (Shift+F4)
#define SALHIST_CONVERT 6     // names in the Convert dialog (Ctrl+K)

// interface of an object for working with a group of file masks
// WARNING: the object methods are not synchronized, so they may be used only within one thread
//          (it does not have to be the main thread) or the plugin must provide its own
//          synchronization (no "write" operation may run while another method executes;
//          "write" = SetMasksString + PrepareMasks;
//          "read" operations may be performed from multiple threads at the same time;
//          "read" = GetMasksString + AgreeMasks)
//
// Object life cycle:
//   1) Allocate by calling CSalamanderGeneralAbstract::AllocSalamanderMaskGroup
//   2) Pass the group of masks through the SetMasksString method.
//   3) Call PrepareMasks to build the internal data; on failure highlight the erroneous
//      place and, after fixing the mask, return to step (3)
//   4) Call AgreeMasks as needed to find out whether a name matches the mask group.
//   5) After another SetMasksString call continue from step (3)
//   6) Destroy the object using CSalamanderGeneralAbstract::FreeSalamanderMaskGroup
//
// Mask syntax:
//   '?' - any single character
//   '*' - any string (including empty)
//   '#' - any digit (only if 'extendedMode' == TRUE)
//
//   Examples:
//     *     - all names
//     *.*   - all names
//     *.exe - names with the "exe" extension
//     *.t?? - names whose extension starts with 't' and contains two more arbitrary characters
//     *.r## - names whose extension starts with 'r' and contains two more arbitrary digits
//
class CSalamanderMaskGroup
{
public:
    // sets the mask string (masks are separated by ';' (the escape sequence for ';' is ";;"));
    // 'masks' is the mask string (maximum length including the terminating null is MAX_GROUPMASK)
    // if 'extendedMode' is TRUE, the '#' character matches any digit ('0'-'9')
    // the '|' character can be used as a separator; the following masks (again separated by ';')
    // are evaluated inversely, so if a name matches them,
    // AgreeMasks returns FALSE; the '|' character can appear at the start of the string
    //
    //   Examples:
    //     *.txt;*.cpp - all names with the txt or cpp extension
    //     *.h*|*.html - all names with an extension starting with 'h', but not names with the "html" extension
    //     |*.txt      - all names with an extension other than "txt"
    virtual void WINAPI SetMasksString(const char* masks, BOOL extendedMode) = 0;

    // returns the mask string; 'buffer' is a buffer of at least MAX_GROUPMASK characters
    virtual void WINAPI GetMasksString(char* buffer) = 0;

    // returns the 'extendedMode' value set by SetMasksString
    virtual BOOL WINAPI GetExtendedMode() = 0;

    // working with file masks: ('?' any character, '*' any string including empty if
    //  'extendedMode' was TRUE in SetMasksString, '#' any digit '0'..'9'):
    // 1) convert the masks to a simpler format; 'errorPos' returns the position of an error in the mask string;
    //    returns TRUE if no error occurred (returns FALSE -> 'errorPos' is set)
    virtual BOOL WINAPI PrepareMasks(int& errorPos) = 0;
    // 2) use the converted masks to test whether any of them matches the file 'fileName';
    //    'fileExt' points either to the end of 'fileName' or to the extension (if it exists), 'fileExt'
    //    can be NULL (the extension is resolved according to the standard rules); returns TRUE if the file
    //    matches at least one of the masks
    virtual BOOL WINAPI AgreeMasks(const char* fileName, const char* fileExt) = 0;
};

// interface of an object for computing MD5
//
// Object life cycle:
//
//   1) Allocate using the CSalamanderGeneralAbstract::AllocSalamanderMD5 method
//   2) Call the Update() method successively for the data whose MD5 we want to compute
//   3) Call the Finalize() method
//   4) Retrieve the computed MD5 using the GetDigest() method
//   5) If we want to reuse the object, call the Init() method
//      (called automatically in step (1)) and continue with step (2)
//   6) Destroy the object using the CSalamanderGeneralAbstract::FreeSalamanderMD5 method
//
class CSalamanderMD5
{
public:
    // initializes the object; it is called automatically in the constructor
    // the method is exposed for repeated use of the allocated object
    virtual void WINAPI Init() = 0;

    // updates the object's internal state using the data block pointed to by 'input',
    // 'input_length' specifies the buffer size in bytes
    virtual void WINAPI Update(const void* input, DWORD input_length) = 0;

    // prepares the MD5 digest for retrieval using the GetDigest method
    // after calling the Finalize method, only the GetDigest() and Init() methods can be called
    virtual void WINAPI Finalize() = 0;

    // retrieves the MD5 digest; 'dest' must point to a 16-byte buffer
    // the method can be called only after calling the Finalize() method
    virtual void WINAPI GetDigest(void* dest) = 0;
};

#define SALPNG_GETALPHA 0x00000002    // when creating the DIB the alpha channel is also set (otherwise it will be zero)
#define SALPNG_PREMULTIPLE 0x00000004 // has effect if SALPNG_GETALPHA is set; pre-multiplies the RGB components so that AlphaBlend() with BLENDFUNCTION::AlphaFormat==AC_SRC_ALPHA can be called on the bitmap

class CSalamanderPNGAbstract
{
public:
    // creates a bitmap from a PNG resource; 'hInstance' and 'lpBitmapName' specify the resource,
    // 'flags' contains 0 or bits from the SALPNG_xxx family
    // on success it returns a handle to the bitmap, otherwise NULL
    // the plugin is responsible for destroying the bitmap by calling DeleteObject()
    // can be called from any thread
    virtual HBITMAP WINAPI LoadPNGBitmap(HINSTANCE hInstance, LPCTSTR lpBitmapName, DWORD flags, COLORREF unused) = 0;

    // creates a bitmap from a PNG provided in memory; 'rawPNG' points to memory containing the PNG
    // (for example loaded from a file) and 'rawPNGSize' specifies the size of the memory occupied by the PNG in bytes,
    // 'flags' contains 0 or bits from the SALPNG_xxx family
    // on success it returns a handle to the bitmap, otherwise NULL
    // the plugin is responsible for destroying the bitmap by calling DeleteObject()
    // can be called from any thread
    virtual HBITMAP WINAPI LoadRawPNGBitmap(const void* rawPNG, DWORD rawPNGSize, DWORD flags, COLORREF unused) = 0;

    // note 1: it is advisable to compress the loaded PNG using PNGSlim, see https://forum.altap.cz/viewtopic.php?f=15&t=3278
    // note 2: for an example of direct access to DIB data see Demoplugin, function AlphaBlend
    // note 3: supported are non-interlaced PNGs of type Greyscale, Greyscale with alpha, Truecolour, Truecolour with alpha, Indexed-colour
    //         with the requirement of 8 bits per channel
};

// all methods can be called only from the main thread
class CSalamanderPasswordManagerAbstract
{
public:
    // returns TRUE if the user set a master password in Salamander's configuration, otherwise returns FALSE
    // (unrelated to whether the MP was entered in this session)
    virtual BOOL WINAPI IsUsingMasterPassword() = 0;

    // returns TRUE if the user entered a correct master password during this Salamander session, otherwise returns FALSE
    virtual BOOL WINAPI IsMasterPasswordSet() = 0;

    // displays a window with parent 'hParent' prompting for the master password
    // returns TRUE if the correct master password was entered, otherwise returns FALSE
    // asks even if the master password has already been entered in this session, see IsMasterPasswordSet()
    // if the user does not use a master password, returns FALSE, see IsUsingMasterPassword()
    virtual BOOL WINAPI AskForMasterPassword(HWND hParent) = 0;

    // reads a null-terminated 'plainPassword' and, based on the 'encrypt' variable, either encrypts it with AES (if TRUE) or
    // merely scrambles it (if FALSE); it stores the allocated result in 'encryptedPassword' and returns its size in the variable
    // 'encryptedPasswordSize'; returns TRUE on success, otherwise FALSE
    // if 'encrypt'==TRUE, the caller must ensure that the master password is entered before calling the function, see AskForMasterPassword()
    // note: the returned 'encryptedPassword' is allocated on Salamander's heap; if the plugin does not use salrtl, it must release the buffer
    // using SalamanderGeneral->Free(), otherwise calling free() is sufficient;
    virtual BOOL WINAPI EncryptPassword(const char* plainPassword, BYTE** encryptedPassword, int* encryptedPasswordSize, BOOL encrypt) = 0;

    // reads 'encryptedPassword' with size 'encryptedPasswordSize' and converts it to a plaintext password, which is returned
    // in the allocated buffer 'plainPassword'; returns TRUE on success, otherwise FALSE
    // note: the returned 'plainPassword' is allocated on Salamander's heap; if the plugin does not use salrtl, it must free the buffer
    // using SalamanderGeneral->Free(), otherwise calling free() is sufficient;
    virtual BOOL WINAPI DecryptPassword(const BYTE* encryptedPassword, int encryptedPasswordSize, char** plainPassword) = 0;

    // returns TRUE if 'encyptedPassword' of length 'encyptedPasswordSize' is encrypted with AES; otherwise returns FALSE
    virtual BOOL WINAPI IsPasswordEncrypted(const BYTE* encyptedPassword, int encyptedPasswordSize) = 0;
};

// modes for CSalamanderGeneralAbstract::ExpandPluralFilesDirs
#define epfdmNormal 0   // XXX files and YYY directories
#define epfdmSelected 1 // XXX selected files and YYY selected directories
#define epfdmHidden 2   // XXX hidden files and YYY hidden directories

// commands for HTML Help: see the CSalamanderGeneralAbstract::OpenHtmlHelp method
enum CHtmlHelpCommand
{
    HHCDisplayTOC,     // see HH_DISPLAY_TOC: dwData = 0 (no topic) or: pointer to a topic within a compiled help file
    HHCDisplayIndex,   // see HH_DISPLAY_INDEX: dwData = 0 (no keyword) or: keyword to select in the index (.hhk) file
    HHCDisplaySearch,  // see HH_DISPLAY_SEARCH: dwData = 0 (empty search) or: pointer to an HH_FTS_QUERY structure
    HHCDisplayContext, // see HH_HELP_CONTEXT: dwData = numeric ID of the topic to display
};

// used as a parameter of OpenHtmlHelpForSalamander when command == HHCDisplayContext
#define HTMLHELP_SALID_PWDMANAGER 1 // displays help for the Password Manager

class CPluginFSInterfaceAbstract;

class CSalamanderZLIBAbstract;

class CSalamanderBZIP2Abstract;

class CSalamanderCryptAbstract;

class CSalamanderGeneralAbstract
{
public:
    // displays a message box with the specified text and title; the parent of the message box is the HWND
    // returned by the GetMsgBoxParent() method (see below); uses SalMessageBox (see below)
    // type = MSGBOX_INFO        - information (OK)
    // type = MSGBOX_ERROR       - error message (OK)
    // type = MSGBOX_EX_ERROR    - error message (OK/Cancel) - returns IDOK, IDCANCEL
    // type = MSGBOX_QUESTION    - question (Yes/No) - returns IDYES, IDNO
    // type = MSGBOX_EX_QUESTION - question (Yes/No/Cancel) - returns IDYES, IDNO, IDCANCEL
    // type = MSGBOX_WARNING     - warning (OK)
    // type = MSGBOX_EX_WARNING  - warning (Yes/No/Cancel) - returns IDYES, IDNO, IDCANCEL
    // returns 0 in case of failure
    // limitation: main thread
    virtual int WINAPI ShowMessageBox(const char* text, const char* title, int type) = 0;

    // SalMessageBox and SalMessageBoxEx create, display, and close the message box after one of the buttons is chosen.
    // The message box can contain a user-defined title, message, buttons, icon, and a checkbox with custom text.
    //
    // If 'hParent' is not the current foreground window (a message box in an inactive application),
    // FlashWindow(mainwnd, TRUE) is called before displaying the message box and FlashWindow(mainwnd, FALSE)
    // after it closes. Within the chain of parents of the window 'hParent', mainwnd is the one that no longer
    // has a parent (typically the main Salamander window).
    //
    // SalMessageBox fills the MSGBOXEX_PARAMS structure (hParent->HParent, lpText->Text,
    // lpCaption->Caption, and uType->Flags; the remaining fields of the structure are zeroed) and calls
    // SalMessageBoxEx, therefore only SalMessageBoxEx is described further.
    //
    // SalMessageBoxEx tries to behave as closely as possible to the Windows API functions
    // MessageBox and MessageBoxIndirect. The differences are:
    //   - the message box is centered relative to hParent (if it is a child window, a non-child parent is found)
    //   - for MB_YESNO/MB_ABORTRETRYIGNORE message boxes you can allow
    //     closing the window with the Escape key or by clicking the close button in the title bar (flag
    //     MSGBOXEX_ESCAPEENABLED); the return value is then IDNO/IDCANCEL
    //   - the beep can be suppressed (flag MSGBOXEX_SILENT)
    //
    // For notes about uType see the comment for MSGBOXEX_PARAMS::Flags
    //
    // Return Values
    //    DIALOG_FAIL       (0)            The function fails.
    //    DIALOG_OK         (IDOK)         'OK' button was selected.
    //    DIALOG_CANCEL     (IDCANCEL)     'Cancel' button was selected.
    //    DIALOG_ABORT      (IDABORT)      'Abort' button was selected.
    //    DIALOG_RETRY      (IDRETRY)      'Retry' button was selected.
    //    DIALOG_IGNORE     (IDIGNORE)     'Ignore' button was selected.
    //    DIALOG_YES        (IDYES)        'Yes' button was selected.
    //    DIALOG_NO         (IDNO)         'No' button was selected.
    //    DIALOG_TRYAGAIN   (IDTRYAGAIN)   'Try Again' button was selected.
    //    DIALOG_CONTINUE   (IDCONTINUE)   'Continue' button was selected.
    //    DIALOG_SKIP                      'Skip' button was selected.
    //    DIALOG_SKIPALL                   'Skip All' button was selected.
    //    DIALOG_ALL                       'All' button was selected.
    //
    // SalMessageBox and SalMessageBoxEx can be called from any thread
    virtual int WINAPI SalMessageBox(HWND hParent, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType) = 0;
    virtual int WINAPI SalMessageBoxEx(const MSGBOXEX_PARAMS* params) = 0;

    // returns an HWND of a suitable parent for message boxes (or other modal windows) being opened,
    // i.e. the main window, progress dialog, Plugins/Plugins dialog, or another modal window opened
    // from the main window
    // limitation: main thread, the returned HWND is always from the main thread
    virtual HWND WINAPI GetMsgBoxParent() = 0;

    // returns a handle to Salamander's main window
    // can be called from any thread
    virtual HWND WINAPI GetMainWindowHWND() = 0;

    // restores focus to the panel or the command line (whichever was active last); this call is needed
    // if a plugin disables/enables Salamander's main window (that can activate a disabled main window -
    // focus cannot be set in a disabled window - after enabling the main window the focus must be restored
    // using this method)
    virtual void WINAPI RestoreFocusInSourcePanel() = 0;

    // frequently used dialogs, the dialog parent is 'parent', return values are DIALOG_XXX;
    // if 'parent' is not currently the foreground window (a dialog in an inactive application),
    // FlashWindow(mainwnd, TRUE) is called before showing the dialog and FlashWindow(mainwnd, FALSE)
    // after it closes. Within the chain of parents of the window 'parent', mainwnd is the one without
    // a parent (typically the main Salamander window).
    // ERROR: filename+error+title (if 'title' == NULL, the standard "Error" title is used)
    //
    // The 'flags' variable determines the buttons displayed; for DialogError you can use one of the values:
    // BUTTONS_OK               // OK                                    (old DialogError3)
    // BUTTONS_RETRYCANCEL      // Retry / Cancel                        (old DialogError4)
    // BUTTONS_SKIPCANCEL       // Skip / Skip all / Cancel              (old DialogError2)
    // BUTTONS_RETRYSKIPCANCEL  // Retry / Skip / Skip all / Cancel      (old DialogError)
    //
    // all of this can be called from any thread
    virtual int WINAPI DialogError(HWND parent, DWORD flags, const char* fileName, const char* error, const char* title) = 0;

    // CONFIRM FILE OVERWRITE: filename1+filedata1+filename2+filedata2
    // The 'flags' variable determines the buttons displayed; for DialogOverwrite you can use one of the values:
    // BUTTONS_YESALLSKIPCANCEL // Yes / All / Skip / Skip all / Cancel  (old DialogOverwrite)
    // BUTTONS_YESNOCANCEL      // Yes / No / Cancel                     (old DialogOverwrite2)
    virtual int WINAPI DialogOverwrite(HWND parent, DWORD flags, const char* fileName1, const char* fileData1,
                                       const char* fileName2, const char* fileData2) = 0;

    // QUESTION: filename+question+title (if 'title' == NULL, the standard "Question" title is used)
    // The 'flags' variable determines the buttons displayed; for DialogQuestion you can use one of the values:
    // BUTTONS_YESALLSKIPCANCEL // Yes / All / Skip / Skip all / Cancel  (old DialogQuestion)
    // BUTTONS_YESNOCANCEL      // Yes / No / Cancel                     (old DialogQuestion2)
    // BUTTONS_YESALLCANCEL     // Yes / All / Cancel                    (old DialogQuestion3)
    virtual int WINAPI DialogQuestion(HWND parent, DWORD flags, const char* fileName,
                                      const char* question, const char* title) = 0;

    // if the path 'dir' does not exist, allows creating it (prompts the user; creates
    // multiple directories at the end of the path if needed); if the path exists or is created successfully it returns TRUE;
    // if the path does not exist and 'quiet' is TRUE, it does not ask the user whether to create
    // the path 'dir'; if 'errBuf' is NULL, errors are shown in dialogs; if 'errBuf' is not NULL,
    // the error description is placed into the buffer 'errBuf' with the size 'errBufSize' (no error dialogs are opened);
    // all opened windows use 'parent' as the parent; if 'parent' is NULL, the Salamander main window is used;
    // if 'firstCreatedDir' is not NULL, it must be a buffer of size MAX_PATH for storing the full name of the first directory
    // created on the path 'dir' (returns an empty string if the path 'dir' already exists); if 'manualCrDir' is TRUE,
    // creation of a directory with a leading space in its name is disallowed (Windows can handle it, but it is
    // potentially dangerous; for example Explorer does not allow it either)
    // can be called from any thread
    virtual BOOL WINAPI CheckAndCreateDirectory(const char* dir, HWND parent = NULL, BOOL quiet = TRUE,
                                                char* errBuf = NULL, int errBufSize = 0,
                                                char* firstCreatedDir = NULL, BOOL manualCrDir = FALSE) = 0;

    // determines free space on the path and, if it is not >= totalSize, asks whether the user wants to continue;
    // the query window has 'parent' as its parent; returns TRUE if there is enough space or the user answered
    // "continue"; if 'parent' is not currently the foreground window (a dialog in an inactive application),
    // FlashWindow(mainwnd, TRUE) is called before displaying the dialog and FlashWindow(mainwnd, FALSE)
    // after it closes. Within the chain of parents of the window 'parent', mainwnd is the one without
    // a parent (typically the main Salamander window).
    // 'messageTitle' is displayed in the title of the question message box and should be the name of the plugin
    // that called the method
    // can be called from any thread
    virtual BOOL WINAPI TestFreeSpace(HWND parent, const char* path, const CQuadWord& totalSize,
                                      const char* messageTitle) = 0;

    // returns in 'retValue' (must not be NULL) the free space for the specified path (currently the most accurate
    // value obtainable from Windows; on NT/W2K/XP/Vista it can work with reparse points
    // and SUBST drives (Salamander 2.5 handles junction points only)); 'path' is the path for
    // which the free space is determined (it does not have to be the root); if 'total' is not NULL,
    // the total disk size is returned in it; on error, CQuadWord(-1, -1) is returned
    // can be called from any thread
    virtual void WINAPI GetDiskFreeSpace(CQuadWord* retValue, const char* path, CQuadWord* total) = 0;

    // custom clone of the Windows GetDiskFreeSpace: can obtain correct data for paths containing
    // SUBST drives and reparse points under Windows 2000/XP/Vista/7 (Salamander 2.5 works with
    // junction points only); 'path' is the path for which we query free space; the remaining parameters
    // correspond to the standard Win32 API function GetDiskFreeSpace
    //
    // WARNING: do not use the return values 'lpNumberOfFreeClusters' and 'lpTotalNumberOfClusters', because
    //          on larger disks they contain nonsense (DWORD may not be sufficient for the total number of clusters);
    //          handle it via the previous GetDiskFreeSpace method, which returns 64-bit numbers
    //
    // can be called from any thread
    virtual BOOL WINAPI SalGetDiskFreeSpace(const char* path, LPDWORD lpSectorsPerCluster,
                                            LPDWORD lpBytesPerSector, LPDWORD lpNumberOfFreeClusters,
                                            LPDWORD lpTotalNumberOfClusters) = 0;

    // custom clone of the Windows GetVolumeInformation: can obtain correct data even for
    // paths containing SUBST drives and reparse points under Windows 2000/XP/Vista (Salamander 2.5
    // works with junction points only); 'path' is the path whose information is queried;
    // in 'rootOrCurReparsePoint' (if not NULL it must be a buffer with at least MAX_PATH
    // characters) it returns the root directory or the current (last) local reparse
    // point on the path 'path' (Salamander 2.5 returns the path for which it managed to obtain
    // the data or at least the root directory); the other parameters correspond to the standard Win32 API
    // function GetVolumeInformation
    // can be called from any thread
    virtual BOOL WINAPI SalGetVolumeInformation(const char* path, char* rootOrCurReparsePoint, LPTSTR lpVolumeNameBuffer,
                                                DWORD nVolumeNameSize, LPDWORD lpVolumeSerialNumber,
                                                LPDWORD lpMaximumComponentLength, LPDWORD lpFileSystemFlags,
                                                LPTSTR lpFileSystemNameBuffer, DWORD nFileSystemNameSize) = 0;

    // custom clone of the Windows GetDriveType: can obtain correct data even for paths
    // containing SUBST drives and reparse points under Windows 2000/XP/Vista (Salamander 2.5
    // works with junction points only); 'path' is the path whose type is queried
    // can be called from any thread
    virtual UINT WINAPI SalGetDriveType(const char* path) = 0;

    // because the Windows GetTempFileName does not work, we wrote our own clone:
    // creates a file/directory (depending on 'file') at 'path' (NULL -> Windows TEMP dir),
    // with the prefix 'prefix'; returns the name of the created file/directory in 'tmpName' (minimum size MAX_PATH),
    // returns success (on failure, it returns the Windows error code in 'err' if it is not NULL)
    // can be called from any thread
    virtual BOOL WINAPI SalGetTempFileName(const char* path, const char* prefix, char* tmpName, BOOL file, DWORD* err) = 0;

    // removes a directory including its contents (SHFileOperation is terribly slow)
    // can be called from any thread
    virtual void WINAPI RemoveTemporaryDir(const char* dir) = 0;

    // because the Windows MoveFile cannot rename files with the read-only attribute on Novell,
    // we wrote our own (if MoveFile fails it tries to drop the read-only attribute, perform the operation,
    // and then restore it); returns success (on failure it returns the Windows error code in 'err' if not NULL)
    // can be called from any thread
    virtual BOOL WINAPI SalMoveFile(const char* srcName, const char* destName, DWORD* err) = 0;

    // alternative to the Windows GetFileSize (with simpler error handling); 'file' is an open
    // file for calling GetFileSize(); it returns the obtained file size in 'size'; returns success,
    // on FALSE (error) 'err' contains the Windows error code and 'size' is zero;
    // NOTE: there is a variant SalGetFileSize2() that works with the full file name
    // can be called from any thread
    virtual BOOL WINAPI SalGetFileSize(HANDLE file, CQuadWord& size, DWORD& err) = 0;

    // opens the file/directory 'name' at path 'path'; it follows Windows associations and opens it
    // via the Open item in the context menu (it can also use salopen.exe, depending on configuration);
    // before launching, it sets the current directories on local drives according to the panel;
    // 'parent' is the parent of any windows (e.g. when opening an unassociated file)
    // limitation: main thread (otherwise salopen.exe would not work - it uses a single shared memory block)
    virtual void WINAPI ExecuteAssociation(HWND parent, const char* path, const char* name) = 0;

    // opens a browse dialog where the user selects a path; 'parent' is the parent of the browse dialog;
    // 'hCenterWindow' - the window the dialog will be centered to; 'title' is the title of the browse dialog;
    // 'comment' is the comment in the browse dialog; 'path' is the buffer for the resulting path (minimum MAX_PATH
    // characters); if 'onlyNet' is TRUE, only network paths can be browsed (otherwise there is no restriction);
    // if 'initDir' is not NULL, it contains the path where the browse dialog should open; returns TRUE if
    // a new path is placed in 'path'
    // WARNING: if called outside the main thread, COM must be initialized beforehand (preferably the entire
    //          OLE - see CoInitialize or OLEInitialize)
    // can be called from any thread
    virtual BOOL WINAPI GetTargetDirectory(HWND parent, HWND hCenterWindow, const char* title,
                                           const char* comment, char* path, BOOL onlyNet,
                                           const char* initDir) = 0;

    // working with file masks: ('?' any character, '*' any string - including empty)
    // all of these can be called from any thread
    // 1) convert the mask to a simpler format (src -> mask buffer - minimum size
    //    of the 'mask' buffer is (strlen(src) + 1))
    virtual void WINAPI PrepareMask(char* mask, const char* src) = 0;
    // 2) we can use the converted mask to test whether filename matches it,
    //    hasExtension = TRUE if the file has an extension
    //    returns TRUE if the file matches the mask
    virtual BOOL WINAPI AgreeMask(const char* filename, const char* mask, BOOL hasExtension) = 0;
    // 3) an unprocessed mask (do not call PrepareMask for it) can be used to create a name based on
    //    the specified name and mask ("a.txt" + "*.cpp" -> "a.cpp", etc.),
    //    the buffer should be at least strlen(name)+strlen(mask) (2*MAX_PATH is useful)
    //    returns the generated name (the pointer 'buffer')
    virtual char* WINAPI MaskName(char* buffer, int bufSize, const char* name, const char* mask) = 0;

    // working with extended file masks: ('?' any character, '*' any string - including empty,
    // '#' any digit - '0'..'9')
    // all of these can be called from any thread
    // 1) convert the mask to a simpler format (src -> mask buffer - minimum length strlen(src) + 1)
    virtual void WINAPI PrepareExtMask(char* mask, const char* src) = 0;
    // 2) we can use the converted mask to test whether filename matches it,
    //    hasExtension = TRUE if the file has an extension
    //    returns TRUE if the file matches the mask
    virtual BOOL WINAPI AgreeExtMask(const char* filename, const char* mask, BOOL hasExtension) = 0;

    // allocates a new object for working with a group of file masks
    // can be called from any thread
    virtual CSalamanderMaskGroup* WINAPI AllocSalamanderMaskGroup() = 0;

    // frees the object used to work with a group of file masks (obtained using AllocSalamanderMaskGroup)
    // can be called from any thread
    virtual void WINAPI FreeSalamanderMaskGroup(CSalamanderMaskGroup* maskGroup) = 0;

    // allocates memory on Salamander's heap (unnecessary when using salrtl9.dll - classic malloc is enough);
    // if memory is low a message is shown to the user with buttons Retry (another attempt to allocate),
    // Abort (after another prompt terminates the application), and Ignore (propagate the allocation error to the
    // application - after warning the user that the application may crash, Alloc returns NULL);
    // checking for NULL makes sense only for large memory blocks, e.g. more than 500 MB, where allocation may fail
    // due to fragmentation of the address space by loaded DLLs);
    // NOTE: Realloc() was added later and is below in this module
    // can be called from any thread
    virtual void* WINAPI Alloc(int size) = 0;
    // deallocates memory from Salamander's heap (unnecessary when using salrtl9.dll - plain free is enough)
    // can be called from any thread
    virtual void WINAPI Free(void* ptr) = 0;

    // duplicates a string - allocates memory (on Salamander's heap - the heap accessible via salrtl9.dll)
    // and copies the string; returns NULL if 'str'==NULL;
    // can be called from any thread
    virtual char* WINAPI DupStr(const char* str) = 0;

    // returns a mapping table for lower-case and upper-case letters (array of 256 characters - lower/upper case at
    // the index of the queried character); if 'lowerCase' is not NULL, it returns the table of lower-case letters there;
    // if 'upperCase' is not NULL, it returns the table of upper-case letters there
    // can be called from any thread
    virtual void WINAPI GetLowerAndUpperCase(unsigned char** lowerCase, unsigned char** upperCase) = 0;

    // converts the string 'str' to lower/upper case; unlike the ANSI C tolower/toupper functions, it works
    // directly on the string and supports more than just characters 'A' to 'Z' (conversion to lower case uses
    // the table initialized by the Win32 API function CharLower)
    virtual void WINAPI ToLowerCase(char* str) = 0;
    virtual void WINAPI ToUpperCase(char* str) = 0;

    //*****************************************************************************
    //
    // StrCmpEx
    //
    // Function compares two substrings.
    // If the two substrings are of different lengths, they are compared up to the
    // length of the shortest one. If they are equal to that point, then the return
    // value will indicate that the longer string is greater.
    //
    // Parameters
    //   s1, s2: strings to compare
    //   l1    : compared length of s1 (must be less or equal to strlen(s1))
    //   l2    : compared length of s2 (must be less or equal to strlen(s1))
    //
    // Return Values
    //   -1 if s1 < s2 (if substring pointed to by s1 is less than the substring pointed to by s2)
    //    0 if s1 = s2 (if the substrings are equal)
    //   +1 if s1 > s2 (if substring pointed to by s1 is greater than the substring pointed to by s2)
    //
    // Method can be called from any thread.
    virtual int WINAPI StrCmpEx(const char* s1, int l1, const char* s2, int l2) = 0;

    //*****************************************************************************
    //
    // StrICpy
    //
    // Function copies characters from source to destination. Upper case letters are mapped to
    // lower case using LowerCase array (filled using CharLower Win32 API call).
    //
    // Parameters
    //   dest: pointer to the destination string
    //   src: pointer to the null-terminated source string
    //
    // Return Values
    //   The StrICpy returns the number of bytes stored in buffer, not counting
    //   the terminating null character.
    //
    // Method can be called from any thread.
    virtual int WINAPI StrICpy(char* dest, const char* src) = 0;

    //*****************************************************************************
    //
    // StrICmp
    //
    // Function compares two strings. The comparison is not case sensitive and ignores
    // regional settings. For the purposes of the comparison, all characters are converted
    // to lower case using LowerCase array (filled using CharLower Win32 API call).
    //
    // Parameters
    //   s1, s2: null-terminated strings to compare
    //
    // Return Values
    //   -1 if s1 < s2 (if string pointed to by s1 is less than the string pointed to by s2)
    //    0 if s1 = s2 (if the strings are equal)
    //   +1 if s1 > s2 (if string pointed to by s1 is greater than the string pointed to by s2)
    //
    // Method can be called from any thread.
    virtual int WINAPI StrICmp(const char* s1, const char* s2) = 0;

    //*****************************************************************************
    //
    // StrICmpEx
    //
    // Function compares two substrings. The comparison is not case sensitive and ignores
    // regional settings. For the purposes of the comparison, all characters are converted
    // to lower case using LowerCase array (filled using CharLower Win32 API call).
    // If the two substrings are of different lengths, they are compared up to the
    // length of the shortest one. If they are equal to that point, then the return
    // value will indicate that the longer string is greater.
    //
    // Parameters
    //   s1, s2: strings to compare
    //   l1    : compared length of s1 (must be less or equal to strlen(s1))
    //   l2    : compared length of s2 (must be less or equal to strlen(s2))
    //
    // Return Values
    //   -1 if s1 < s2 (if substring pointed to by s1 is less than the substring pointed to by s2)
    //    0 if s1 = s2 (if the substrings are equal)
    //   +1 if s1 > s2 (if substring pointed to by s1 is greater than the substring pointed to by s2)
    //
    // Method can be called from any thread.
    virtual int WINAPI StrICmpEx(const char* s1, int l1, const char* s2, int l2) = 0;

    //*****************************************************************************
    //
    // StrNICmp
    //
    // Function compares two strings. The comparison is not case sensitive and ignores
    // regional settings. For the purposes of the comparison, all characters are converted
    // to lower case using LowerCase array (filled using CharLower Win32 API call).
    // The comparison stops after: (1) a difference between the strings is found,
    // (2) the end of the string is reached, or (3) n characters have been compared.
    //
    // Parameters
    //   s1, s2: strings to compare
    //   n:      maximum length to compare
    //
    // Return Values
    //   -1 if s1 < s2 (if substring pointed to by s1 is less than the substring pointed to by s2)
    //    0 if s1 = s2 (if the substrings are equal)
    //   +1 if s1 > s2 (if substring pointed to by s1 is greater than the substring pointed to by s2)
    //
    // Method can be called from any thread.
    virtual int WINAPI StrNICmp(const char* s1, const char* s2, int n) = 0;

    //*****************************************************************************
    //
    // MemICmp
    //
    // Compares n bytes of the two blocks of memory stored at buf1 and buf2.
    // Characters are converted to lowercase before comparing (not permanently;
    // using LowerCase array which was filled using CharLower Win32 API call),
    // so case is ignored in comparation.
    //
    // Parameters
    //   buf1, buf2: memory buffers to compare
    //   n:          maximum length to compare
    //
    // Return Values
    //   -1 if buf1 < buf2 (if buffer pointed to by buf1 is less than the buffer pointed to by buf2)
    //    0 if buf1 = buf2 (if the buffers are equal)
    //   +1 if buf1 > buf2 (if buffer pointed to by buf1 is greater than the buffer pointed to by buf2)
    //
    // Method can be called from any thread.
    virtual int WINAPI MemICmp(const void* buf1, const void* buf2, int n) = 0;

    // compares two strings 's1' and 's2' ignoring case; if SALCFG_SORTUSESLOCALE is TRUE,
    // it uses sorting according to the Windows regional settings; otherwise it compares the same way as
    // CSalamanderGeneral::StrICmp. If SALCFG_SORTDETECTNUMBERS is TRUE, it uses numeric sorting
    // for numbers contained in the strings
    // returns <0 ('s1' < 's2'), ==0 ('s1' == 's2'), >0 ('s1' > 's2')
    virtual int WINAPI RegSetStrICmp(const char* s1, const char* s2) = 0;

    // compares two strings 's1' and 's2' (with lengths 'l1' and 'l2') ignoring case; if SALCFG_SORTUSESLOCALE
    // is TRUE, sorting by the Windows regional settings is used; otherwise it compares the same way as
    // CSalamanderGeneral::StrICmp. If SALCFG_SORTDETECTNUMBERS is TRUE, it uses numerical sorting for numbers
    // contained in the strings; in 'numericalyEqual' (if not NULL) it returns TRUE if the strings are numerically
    // identical (for example "a01" and "a1"), it is automatically TRUE if the strings are equal
    // returns <0 ('s1' < 's2'), ==0 ('s1' == 's2'), >0 ('s1' > 's2')
    virtual int WINAPI RegSetStrICmpEx(const char* s1, int l1, const char* s2, int l2,
                                       BOOL* numericalyEqual) = 0;

    // compares two strings 's1' and 's2' case-sensitively; if SALCFG_SORTUSESLOCALE is TRUE,
    // sorting by the Windows regional settings is used; otherwise it compares the same way as
    // the standard C library function strcmp. If SALCFG_SORTDETECTNUMBERS is TRUE, it uses numerical
    // sorting for numbers contained in the strings
    // returns <0 ('s1' < 's2'), ==0 ('s1' == 's2'), >0 ('s1' > 's2')
    virtual int WINAPI RegSetStrCmp(const char* s1, const char* s2) = 0;

    // compares two strings 's1' and 's2' case-sensitively (with lengths 'l1' and 'l2'); if
    // SALCFG_SORTUSESLOCALE is TRUE, sorting by the Windows regional settings is used;
    // otherwise it compares the same way as the standard C library function strcmp. If
    // SALCFG_SORTDETECTNUMBERS is TRUE, it uses numerical sorting for numbers contained in the strings;
    // in 'numericalyEqual' (if not NULL) it returns TRUE if the strings are numerically identical
    // (for example "a01" and "a1"), it is automatically TRUE if the strings are equal
    // returns <0 ('s1' < 's2'), ==0 ('s1' == 's2'), >0 ('s1' > 's2')
    virtual int WINAPI RegSetStrCmpEx(const char* s1, int l1, const char* s2, int l2,
                                      BOOL* numericalyEqual) = 0;

    // returns the path in the panel; 'panel' is one of PANEL_XXX; 'buffer' is the buffer for the path (it can
    // also be NULL); 'bufferSize' is the size of the 'buffer' (if 'buffer' is NULL, this must
    // be zero); if 'type' is not NULL, it points to a variable where the path type will be stored
    // (see PATH_TYPE_XXX); if this is an archive and 'archiveOrFS' is not NULL and 'buffer' is not NULL,
    // returns 'archiveOrFS' set to 'buffer' at the position after the archive file name;
    // if this is a file system and 'archiveOrFS' is not NULL and 'buffer' is not NULL, returns
    // 'archiveOrFS' set to the ':' in 'buffer' after the file-system name (after ':' is the user part
    // of the file-system path); if 'convertFSPathToExternal' is TRUE and the panel contains an FS path,
    // the plugin to which the path belongs (according to fs-name) is found and its
    // CPluginInterfaceForFSAbstract::ConvertPathToExternal() is called; returns success (if
    // 'bufferSize'!=0, it is also considered a failure when the path does not fit into the 'buffer')
    // limitation: main thread
    virtual BOOL WINAPI GetPanelPath(int panel, char* buffer, int bufferSize, int* type,
                                     char** archiveOrFS, BOOL convertFSPathToExternal = FALSE) = 0;

    // returns the last visited Windows path for the panel, useful when returning from an FS (more convenient than
    // going straight to a fixed drive); 'panel' is one of PANEL_XXX; 'buffer' is the buffer for the path;
    // 'bufferSize' is the size of the 'buffer'; returns success
    // limitation: main thread
    virtual BOOL WINAPI GetLastWindowsPanelPath(int panel, char* buffer, int bufferSize) = 0;

    // returns the FS name assigned to the plugin "for life" by Salamander (as specified in SetBasicPluginData);
    // 'buf' is a buffer at least MAX_PATH characters long; 'fsNameIndex' is the FS-name index (zero
    // for the FS name provided in CSalamanderPluginEntryAbstract::SetBasicPluginData; for other
    // FS names, the index is returned by CSalamanderPluginEntryAbstract::AddFSName)
    // limitation: main thread only (otherwise the plugin configuration may change during the call),
    // in the entry point, it can be called only after SetBasicPluginData; earlier it may not be known
    virtual void WINAPI GetPluginFSName(char* buf, int fsNameIndex) = 0;

    // returns the interface of the plugin file system (FS) opened in the panel 'panel' (one of PANEL_XXX);
    // if no FS is open in the panel or it belongs to another plugin (not the caller), the method returns
    // NULL (objects of another plugin cannot be accessed, their structure is unknown)
    // limitation: main thread
    virtual CPluginFSInterfaceAbstract* WINAPI GetPanelPluginFS(int panel) = 0;

    // returns the plugin data interface for the panel listing (can also be NULL), 'panel' is one of PANEL_XXX;
    // if the plugin data interface exists but does not belong to this (calling) plugin, the method returns
    // NULL (you cannot work with an object of another plugin because its structure is unknown)
    // limitation: main thread
    virtual CPluginDataInterfaceAbstract* WINAPI GetPanelPluginData(int panel) = 0;

    // returns the focused item of the panel (file/directory/updir("..")), 'panel' is one of PANEL_XXX,
    // returns NULL (no item in the panel) or data of the focused item; if 'isDir' is not NULL,
    // it returns FALSE there for a file (otherwise it is a directory or updir)
    // WARNING: returned item data are read-only
    // limitation: main thread
    virtual const CFileData* WINAPI GetPanelFocusedItem(int panel, BOOL* isDir) = 0;

    // iterates through panel items (directories first, then files), 'panel' is one of PANEL_XXX,
    // 'index' is an input/output variable pointing to an int with value 0 on the first call;
    // the function stores the value for the next call when it returns (usage: reset to zero at the start,
    // then keep it unchanged); returns NULL (no more items) or data of the next (possibly first) item;
    // if 'isDir' is not NULL, it returns FALSE there for a file (otherwise it is a directory or updir)
    // WARNING: returned item data are read-only
    // limitation: main thread
    virtual const CFileData* WINAPI GetPanelItem(int panel, int* index, BOOL* isDir) = 0;

    // iterates through selected panel items (directories first, then files), 'panel' is one of
    // PANEL_XXX; 'index' is an input/output variable pointing to an int with value 0 on the first call;
    // the function stores the value for the next call when it returns (usage: reset to zero at the start,
    // then keep it unchanged); returns NULL (no more items) or data of the next (possibly first) item;
    // if 'isDir' is not NULL, it returns FALSE there for a file (otherwise it is a directory or updir)
    // WARNING: returned item data are read-only
    // limitation: main thread
    virtual const CFileData* WINAPI GetPanelSelectedItem(int panel, int* index, BOOL* isDir) = 0;

    // determines how many files and directories are selected in the panel; 'panel' is one of PANEL_XXX;
    // if 'selectedFiles' is not NULL, it returns the number of selected files there; if 'selectedDirs'
    // is not NULL, it returns the number of selected directories there; returns TRUE if at least one
    // file or directory is selected or if the focus is on a file or directory (i.e. there is something to
    // work with - the focus is not on up-dir)
    // limitation: main thread (otherwise the panel contents may change)
    virtual BOOL WINAPI GetPanelSelection(int panel, int* selectedFiles, int* selectedDirs) = 0;

    // returns the top index of the list box in the panel; 'panel' is one of PANEL_XXX
    // limitation: main thread (otherwise the panel contents may change)
    virtual int WINAPI GetPanelTopIndex(int panel) = 0;

    // informs Salamander's main window that the viewer window is being deactivated; if the main window
    // is activated immediately afterwards and there are panels with manually refreshed drives,
    // they will not be refreshed (viewers do not change disk contents); optional
    // (it may cause a possibly unnecessary refresh)
    // can be called from any thread
    virtual void WINAPI SkipOneActivateRefresh() = 0;

    // selects/deselects a panel item; 'file' is a pointer to the item to be changed, obtained by a previous
    // "get-item" call (GetPanelFocusedItem, GetPanelItem, or GetPanelSelectedItem);
    // execution must not leave the plugin after the "get-item" call and this method must run in the main
    // thread (to avoid a panel refresh that would invalidate the pointer); 'panel' must match
    // the 'panel' parameter of the corresponding "get-item" call; if 'select' is TRUE the item is selected,
    // otherwise it is deselected; after the last call, RepaintChangedItems('panel') must be used to
    // redraw the panel
    // limitation: main thread
    virtual void WINAPI SelectPanelItem(int panel, const CFileData* file, BOOL select) = 0;

    // repaints the panel items that have changed (selection); 'panel' is
    // one of PANEL_XXX
    // limitation: main thread
    virtual void WINAPI RepaintChangedItems(int panel) = 0;

    // selects/deselects all items in the panel, 'panel' is one of PANEL_XXX; if 'select' is TRUE the items
    // are selected, otherwise they are deselected; if 'repaint' is TRUE all changed items in the panel are
    // repainted, otherwise they are not (RepaintChangedItems can be called later)
    // limitation: main thread
    virtual void WINAPI SelectAllPanelItems(int panel, BOOL select, BOOL repaint) = 0;

    // sets focus in the panel; 'file' is a pointer to the focused item obtained by a previous
    // "get-item" call (GetPanelFocusedItem, GetPanelItem, or GetPanelSelectedItem);
    // the plugin must not be left after the "get-item" call and this method must run in the main
    // thread (to avoid refreshing the panel and invalidating the pointer); 'panel' must match
    // the 'panel' parameter of the corresponding "get-item" call; if 'partVis' is TRUE and the item would be
    // only partially visible, the panel does not scroll on focus; when FALSE the panel scrolls so that
    // the entire item is visible
    // limitation: main thread
    virtual void WINAPI SetPanelFocusedItem(int panel, const CFileData* file, BOOL partVis) = 0;

    // finds out whether a filter is used in the panel and, if so, retrieves the string with the filter masks;
    // 'panel' identifies the panel of interest (one of PANEL_XXX);
    // 'masks' is a buffer for the filter masks with a size of at least 'masksBufSize' bytes (MAX_GROUPMASK recommended);
    // returns TRUE if a filter is used and the 'masks' buffer is large enough; returns FALSE if no filter is used
    // or the mask string did not fit into 'masks'
    // limitation: main thread
    virtual BOOL WINAPI GetFilterFromPanel(int panel, char* masks, int masksBufSize) = 0;

    // returns which side the source panel is on (left or right); returns PANEL_LEFT or PANEL_RIGHT
    // limitation: main thread
    virtual int WINAPI GetSourcePanel() = 0;

    // determines which panel has 'pluginFS' open; if it is not open in any panel,
    // returns FALSE; if it returns TRUE, the panel number is stored in 'panel' (PANEL_LEFT or PANEL_RIGHT)
    // limitation: main thread (otherwise the panel contents may change)
    virtual BOOL WINAPI GetPanelWithPluginFS(CPluginFSInterfaceAbstract* pluginFS, int& panel) = 0;

    // activates the other panel (like the TAB key); the panels denoted by PANEL_SOURCE and PANEL_TARGET
    // are naturally swapped by this
    // limitation: main thread
    virtual void WINAPI ChangePanel() = 0;

    // converts a number to a "more readable" string (space every three digits), returns the string in
    // 'buffer' (minimum size 50 bytes), returns 'buffer'
    // can be called from any thread
    virtual char* WINAPI NumberToStr(char* buffer, const CQuadWord& number) = 0;

    // prints the size of disk space into 'buf' (minimum buffer size is 100 bytes),
    // mode==0 "1.23 MB", mode==1 "1 230 000 bytes, 1.23 MB", mode==2 "1 230 000 bytes",
    // mode==3 "12 KB" (always whole kilobytes), mode==4 (same as mode==0, but always
    // at least 3 significant digits, e.g. "2.00 MB")
    // returns 'buf'
    // can be called from any thread
    virtual char* WINAPI PrintDiskSize(char* buf, const CQuadWord& size, int mode) = 0;

    // converts a number of seconds to a string ("5 sec", "1 hr 34 min", etc.); 'buf' is
    // the buffer for the resulting text and must be at least 100 characters; 'secs' is the number of seconds;
    // returns 'buf'
    // can be called from any thread
    virtual char* WINAPI PrintTimeLeft(char* buf, const CQuadWord& secs) = 0;

    // compares the root of normal (c:\path) and UNC (\\server\share\path) paths; returns TRUE if the roots match
    // can be called from any thread
    virtual BOOL WINAPI HasTheSameRootPath(const char* path1, const char* path2) = 0;

    // Returns the number of characters in the common path prefix. For a normal path the root must end with a backslash,
    // otherwise the function returns 0. ("C:\"+"C:"->0, "C:\A\B"+"C:\"->3, "C:\A\B\"+"C:\A"->4,
    // "C:\AA\BB"+"C:\AA\CC"->5)
    // Works for both normal and UNC paths.
    virtual int WINAPI CommonPrefixLength(const char* path1, const char* path2) = 0;

    // Returns TRUE if the path 'prefix' is a prefix of the path 'path'. Otherwise returns FALSE.
    // "C:\aa","C:\Aa\BB"->TRUE
    // "C:\aa","C:\aaa"->FALSE
    // "C:\aa\","C:\Aa"->TRUE
    // "\\server\share","\\server\share\aaa"->TRUE
    // Works for both normal and UNC paths.
    virtual BOOL WINAPI PathIsPrefix(const char* prefix, const char* path) = 0;

    // compares two normal (c:\path) and UNC (\\server\share\path) paths, ignores case,
    // also ignores one backslash at the beginning and end of the paths; returns TRUE if the paths are identical
    // can be called from any thread
    virtual BOOL WINAPI IsTheSamePath(const char* path1, const char* path2) = 0;

    // extracts the root path from the normal (c:\path) or UNC (\\server\share\path) path 'path';
    // returns the path in 'root' in the form 'c:\' or '\\server\share\' (minimum 'root' buffer size is MAX_PATH),
    // returns the number of characters in the root path (without the null terminator); if a UNC root path is longer
    // than MAX_PATH it is truncated to MAX_PATH-2 characters and backslashes are appended (it is not a valid root anyway)
    // can be called from any thread
    virtual int WINAPI GetRootPath(char* root, const char* path) = 0;

    // shortens a normal (c:\path) or UNC (\\server\share\path) path by the last directory
    // (cut at the last backslash - the truncated path keeps the trailing backslash only for 'c:\'); 'path' is an in/out
    // buffer (minimum size strlen(path)+2 bytes);
    // if 'cutDir' is not NULL it returns a pointer (into the 'path' buffer after the first null terminator)
    // to the last directory (the removed part); this method replaces PathRemoveFileSpec,
    // returns TRUE if shortening occurred (the path was not a root)
    // can be called from any thread
    virtual BOOL WINAPI CutDirectory(char* path, char** cutDir = NULL) = 0;

    // works with normal (c:\path) and UNC (\\server\share\path) paths,
    // joins 'path' and 'name' into 'path', ensures the joining backslash; 'path' is a buffer with at least
    // 'pathSize' characters; returns TRUE if 'name' fits after 'path'; if 'path' or 'name'
    // is empty, no joining (leading/trailing) backslash is added (e.g. "c:\" + "" -> "c:")
    // can be called from any thread
    virtual BOOL WINAPI SalPathAppend(char* path, const char* name, int pathSize) = 0;

    // works with normal (c:\path) and UNC (\\server\share\path) paths,
    // if 'path' does not end with a backslash yet, adds it to the end of 'path'; 'path' is a buffer
    // with at least 'pathSize' characters; returns TRUE if the backslash fit at the end of 'path'; if 'path'
    // is empty, the backslash is not added
    // can be called from any thread
    virtual BOOL WINAPI SalPathAddBackslash(char* path, int pathSize) = 0;

    // works with normal (c:\path) and UNC (\\server\share\path) paths,
    // removes a trailing backslash from 'path' if present
    // can be called from any thread
    virtual void WINAPI SalPathRemoveBackslash(char* path) = 0;

    // works with normal (c:\path) and UNC (\\server\share\path) paths,
    // extracts the name from the full path ("c:\path\file" -> "file")
    // can be called from any thread
    virtual void WINAPI SalPathStripPath(char* path) = 0;

    // works with normal (c:\path) and UNC (\\server\share\path) paths,
    // removes the extension from the name if present
    // can be called from any thread
    virtual void WINAPI SalPathRemoveExtension(char* path) = 0;

    // works with normal (c:\path) and UNC (\\server\share\path) paths,
    // if the name in 'path' does not have an extension yet, adds the 'extension' (e.g. ".txt");
    // 'path' is a buffer with at least 'pathSize' characters; returns FALSE if the 'path' buffer is insufficient
    // for the resulting path
    // can be called from any thread
    virtual BOOL WINAPI SalPathAddExtension(char* path, const char* extension, int pathSize) = 0;

    // works with normal (c:\path) and UNC (\\server\share\path) paths,
    // changes/adds the 'extension' extension (e.g. ".txt") in 'path'; 'path' is a buffer
    // of at least 'pathSize' characters; returns FALSE if the 'path' buffer is too small for the resulting path
    // can be called from any thread
    virtual BOOL WINAPI SalPathRenameExtension(char* path, const char* extension, int pathSize) = 0;

    // works with normal (c:\path) and UNC (\\server\share\path) paths,
    // returns a pointer within 'path' to the file/directory name (ignores a trailing backslash on 'path');
    // if the name contains no other backslashes than at the end of the string, returns 'path'
    // can be called from any thread
    virtual const char* WINAPI SalPathFindFileName(const char* path) = 0;

    // adjusts a relative or absolute normal (c:\path) and UNC (\\server\share\path) path
    // to an absolute path without '.', '..', and without a trailing backslash (except for "c:\"); if 'curDir' is NULL,
    // relative paths of the form "\path" and "path" return an error (undetermined), otherwise 'curDir' is a valid
    // normalized current path (UNC and normal); the current paths of other drives (except
    // 'curDir' + normal only, not UNC) are stored in Salamander's DefaultDir array (before use it is good
    // to call SalUpdateDefaultDir); 'name' - in/out path buffer of at least 'nameBufSize'
    // characters; if 'nextFocus' is not NULL and the specified relative path does not contain a backslash, strcpy(nextFocus, name)
    // is performed; returns TRUE if the name 'name' is ready for use, otherwise if 'errTextID' is not NULL it contains
    // the error (constants GFN_XXX - the text can be obtained via GetGFNErrorText)
    // WARNING: before using it is good to call SalUpdateDefaultDir
    // limitation: main thread (otherwise DefaultDir may change in the main thread)
    virtual BOOL WINAPI SalGetFullName(char* name, int* errTextID = NULL, const char* curDir = NULL,
                                       char* nextFocus = NULL, int nameBufSize = MAX_PATH) = 0;

    // updates Salamander's DefaultDir array according to the panel paths; if 'activePrefered' is TRUE,
    // the path in the active panel takes precedence (it is written to DefaultDir later), otherwise
    // the path in the inactive panel takes precedence
    // limitation: main thread (otherwise DefaultDir may change in the main thread)
    virtual void WINAPI SalUpdateDefaultDir(BOOL activePrefered) = 0;

    // returns a textual representation of the GFN_XXX error constant; returns 'buf' (so GetGFNErrorText
    // can be passed directly as a function parameter)
    // can be called from any thread
    virtual char* WINAPI GetGFNErrorText(int GFN, char* buf, int bufSize) = 0;

    // returns a textual representation of a system error (ERROR_XXX) in the buffer 'buf' of size 'bufSize';
    // returns 'buf' (so GetErrorText can be passed directly as a function parameter); 'buf' can be NULL or
    // 'bufSize' 0; in that case the text is returned in an internal buffer (the text may change due to modifications
    // of the internal buffer caused by subsequent GetErrorText calls from other plugins or Salamander itself;
    // the buffer is sized for at least 10 texts, only then is overwriting a risk; if you need to use the text later,
    // we recommend copying it into a local buffer of size MAX_PATH + 20)
    // can be called from any thread
    virtual char* WINAPI GetErrorText(int err, char* buf = NULL, int bufSize = 0) = 0;

    // returns Salamander's internal color, 'color' is a color constant (see SALCOL_XXX)
    // can be called from any thread
    virtual COLORREF WINAPI GetCurrentColor(int color) = 0;

    // ensures activation of Salamander's main window and focuses the file/directory 'name' on the path
    // 'path' in the panel 'panel'; changes the panel path if needed. 'panel' is one
    // of PANEL_XXX; 'path' can be any path (Windows/disk, FS, or archive);
    // 'name' can also be an empty string if nothing should be focused;
    // limitation: main thread + outside of CPluginFSInterfaceAbstract and CPluginDataInterfaceAbstract methods
    // (for example an FS open in the panel could be closed - 'this' might cease to exist for the method)
    virtual void WINAPI FocusNameInPanel(int panel, const char* path, const char* name) = 0;

    // changes the panel path – the input can be an absolute or relative UNC (\\server\share\path)
    // or a normal (c:\path) path, both Windows (disk) paths, archive paths, or FS paths
    // (absolute/relative is handled by the plugin). If the input is a file path,
    // that file becomes focused; if suggestedTopIndex is not -1, it sets the top index
    // in the panel; if suggestedFocusName is not NULL, it tries to find (ignore-case) and focus
    // the item with the same name; if 'failReason' is not NULL, it is set to one of the
    // CHPPFR_XXX constants (informing about the result of the method). If 'convertFSPathToInternal' is TRUE and this is
    // an FS path, the plugin whose path it is (according to fs-name) is located and its
    // CPluginInterfaceForFSAbstract::ConvertPathToInternal() is called; returns TRUE if the requested path
    // was listed successfully.
    // NOTE: when an FS path is specified, opening is attempted in this order: in the FS
    // in the panel, in a detached FS, or in a new FS (for panel FS and detached FS it checks whether
    // the plugin FS name matches and whether the FS method IsOurPath returns TRUE for the path);
    // limitation: main thread + outside of CPluginFSInterfaceAbstract and CPluginDataInterfaceAbstract methods
    // (for example an FS open in the panel could be closed - 'this' might cease to exist for the method)
    virtual BOOL WINAPI ChangePanelPath(int panel, const char* path, int* failReason = NULL,
                                        int suggestedTopIndex = -1,
                                        const char* suggestedFocusName = NULL,
                                        BOOL convertFSPathToInternal = TRUE) = 0;

    // changes the panel path to a relative or absolute UNC (\\server\share\path) or normal (c:\path)
    // path; if the new path is not available, it tries to succeed with shortened paths. If the change
    // stays within a single drive (including archives on that drive) and an accessible path cannot be found
    // on the drive, the path is changed to the root of the first local fixed drive (a high chance of success,
    // the panel will not remain empty). When translating a relative path to an absolute one, the path
    // in the 'panel' panel is preferred (only if it is a disk path, including archives; otherwise it is not used). 'panel' is
    // one of PANEL_XXX; 'path' is the new path; if 'suggestedTopIndex' is not -1, it is set as the top index
    // in the panel (only for the new path, not set on shortened (changed) paths); if 'suggestedFocusName' is not NULL,
    // it tries to find (ignore-case) and focus the item with the same name (only for the new path,
    // not performed on shortened (changed) paths); if 'failReason' is not NULL,
    // it is set to one of the CHPPFR_XXX constants (informing about the method result); returns TRUE if
    // the requested path was listed successfully (not shortened/unchanged)
    // limitation: main thread + outside of CPluginFSInterfaceAbstract and CPluginDataInterfaceAbstract methods
    // (for example an FS open in the panel could be closed - 'this' might cease to exist for the method)
    virtual BOOL WINAPI ChangePanelPathToDisk(int panel, const char* path, int* failReason = NULL,
                                              int suggestedTopIndex = -1,
                                              const char* suggestedFocusName = NULL) = 0;

    // changes the panel path into an archive; 'archive' is a relative or absolute UNC
    // (\\server\share\path\file) or normal (c:\path\file) archive name, 'archivePath' is the path
    // inside the archive. If the new path inside the archive is not available, it tries to succeed with shortened paths;
    // when translating a relative path to an absolute one, the path in the 'panel' panel is preferred
    // (only if it is a disk path, including an archive; otherwise it is not used). 'panel' is one of PANEL_XXX;
    // if 'suggestedTopIndex' is not -1, it will be set as the top index in the panel (only for the new
    // path, it is not set on shortened (changed) paths); if 'suggestedFocusName' is not NULL,
    // it tries to find (ignore-case) and focus the item with the same name (only for the new path,
    // not performed on shortened (changed) paths). If 'forceUpdate' is TRUE and the path change is performed
    // inside the archive 'archive' (the archive is already open in the panel), the archive file is checked for changes
    // (size & time) and, if it changed, the archive is closed (edited files might need updating)
    // and listed again or, if the file no longer exists, the path is changed to the disk
    // (the archive is closed; if the disk path is not accessible, the path is changed to the root of the first local
    // fixed drive). If 'forceUpdate' is FALSE, path changes inside the archive 'archive' are performed without
    // checking the archive file. If 'failReason' is not NULL, it is set to one of the CHPPFR_XXX constants
    // (indicating the method result); returns TRUE if the requested path was listed successfully
    // (not shortened/unchanged)
    // limitation: main thread + outside of CPluginFSInterfaceAbstract and CPluginDataInterfaceAbstract methods
    // (for example an FS open in the panel could be closed - 'this' might cease to exist for the method)
    virtual BOOL WINAPI ChangePanelPathToArchive(int panel, const char* archive, const char* archivePath,
                                                 int* failReason = NULL, int suggestedTopIndex = -1,
                                                 const char* suggestedFocusName = NULL,
                                                 BOOL forceUpdate = FALSE) = 0;

    // changes the panel path to a plugin FS; 'fsName' is the FS name (see GetPluginFSName; it does not
    // have to belong to this plugin), 'fsUserPart' is the path within the FS. If the new path in the FS
    // is not available, it tries to succeed with shortened paths (repeated ChangePath and ListCurrentPath calls,
    // see CPluginFSInterfaceAbstract). If the change remains within the current FS (see
    // CPluginFSInterfaceAbstract::IsOurPath) and an accessible path cannot be found from the new path,
    // it tries to find an accessible path from the original (current) path, and if that also fails,
    // the path is changed to the root of the first local fixed drive (high chance of success, the panel will not remain empty);
    // 'panel' is one of PANEL_XXX; if 'suggestedTopIndex' is not -1, it is set as the top index
    // in the panel (only for the new path, not set on shortened (changed) paths); if
    // 'suggestedFocusName' is not NULL, it tries to find (ignore-case) and focus the item with the same name
    // (only for the new path, not performed on shortened (changed) paths). If 'forceUpdate' is TRUE,
    // the case of changing the path to the current panel path is not optimized (the path is listed normally)
    // (either the new path matches the current path directly or it was shortened to it by the first ChangePath).
    // If 'convertPathToInternal' is TRUE, the plugin according to 'fsName' is located and its
    // method CPluginInterfaceForFSAbstract::ConvertPathToInternal() is called for 'fsUserPart';
    // if 'failReason' is not NULL, it is set to one of the CHPPFR_XXX constants (informing
    // about the method result); returns TRUE if the requested path was listed successfully
    // (not shortened/unchanged)
    // NOTE: if you need the FS path to be attempted even in detached FSs, use ChangePanelPath (ChangePanelPathToPluginFS
    // ignores detached FSs - it only works with an FS open in the panel or opens a new FS);
    // limitation: main thread + outside of CPluginFSInterfaceAbstract and CPluginDataInterfaceAbstract methods
    // (for example an FS open in the panel could be closed - 'this' might cease to exist for the method)
    virtual BOOL WINAPI ChangePanelPathToPluginFS(int panel, const char* fsName, const char* fsUserPart,
                                                  int* failReason = NULL, int suggestedTopIndex = -1,
                                                  const char* suggestedFocusName = NULL,
                                                  BOOL forceUpdate = FALSE,
                                                  BOOL convertPathToInternal = FALSE) = 0;

    // changes the panel path to a detached plugin FS (see FSE_DETACHED/FSE_ATTACHED),
    // 'detachedFS' is the detached plugin FS; if the current path in the detached FS is not available,
    // it tries to succeed with shortened paths (repeated ChangePath and ListCurrentPath calls, see
    // CPluginFSInterfaceAbstract). 'panel' is one of PANEL_XXX; if 'suggestedTopIndex' is not -1,
    // it is set as the top index in the panel (only for the new path, not set on shortened (changed) paths);
    // if 'suggestedFocusName' is not NULL, it tries to find (ignore-case) and focus the item with the same name
    // (only for the new path, not performed on shortened (changed) paths);
    // if 'failReason' is not NULL, it is set to one of the CHPPFR_XXX constants (informing about the method result);
    // returns TRUE if the requested path was listed successfully (not shortened/unchanged)
    // limitation: main thread + outside of CPluginFSInterfaceAbstract and CPluginDataInterfaceAbstract methods
    // (for example an FS open in the panel could be closed - 'this' might cease to exist for the method)
    virtual BOOL WINAPI ChangePanelPathToDetachedFS(int panel, CPluginFSInterfaceAbstract* detachedFS,
                                                    int* failReason = NULL, int suggestedTopIndex = -1,
                                                    const char* suggestedFocusName = NULL) = 0;

    // changes the panel path to the root of the first local fixed drive; this almost certainly changes
    // the current panel path. 'panel' is one of PANEL_XXX; if 'failReason' is not NULL,
    // it is set to one of the CHPPFR_XXX constants (informing about the method result); returns
    // TRUE if the root of the first local fixed drive was listed successfully
    // limitation: main thread + outside of CPluginFSInterfaceAbstract and CPluginDataInterfaceAbstract methods
    // (for example an FS open in the panel could be closed - 'this' might cease to exist for the method)
    virtual BOOL WINAPI ChangePanelPathToFixedDrive(int panel, int* failReason = NULL) = 0;

    // refreshes the path in the panel (reloads the listing and transfers the selection, icons, focus, etc.
    // to the new panel content); disk and FS paths are always reloaded, archive paths
    // are reloaded only if the archive file changed (size & time check) or if
    // 'forceRefresh' is TRUE; thumbnails on disk paths are reloaded only when the file size changes,
    // the last write time changes, or 'forceRefresh' is TRUE. 'panel' is one of PANEL_XXX; if 'focusFirstNewItem' is TRUE and
    // only a single item was added to the panel, that new item is focused (used, for example,
    // to focus a newly created file/directory)
    // limitation: main thread and only outside of CPluginFSInterfaceAbstract and
    // CPluginDataInterfaceAbstract methods (for example an FS open in the panel could be closed - 'this'
    // might cease to exist for the method)
    virtual void WINAPI RefreshPanelPath(int panel, BOOL forceRefresh = FALSE,
                                         BOOL focusFirstNewItem = FALSE) = 0;

    // posts a message to the panel that a path refresh should be performed (reloads the listing and
    // transfers the selection, icons, focus, etc. to the new panel contents); the refresh is performed only when
    // Salamander's main window becomes active (when suspend mode ends); disk
    // and FS paths are always reloaded, archive paths are reloaded only if the archive file has changed
    // (size & time check); 'panel' is one of PANEL_XXX; if 'focusFirstNewItem' is TRUE and only one item
    // was added to the panel, that new item receives focus (used, for example, to focus a newly created file/directory)
    // can be called from any thread (if the main thread is not executing code inside the plugin,
    // the refresh happens as soon as possible; otherwise the refresh waits at least until the main
    // thread leaves the plugin)
    virtual void WINAPI PostRefreshPanelPath(int panel, BOOL focusFirstNewItem = FALSE) = 0;

    // posts a message to the panel whose active FS is 'modifiedFS' that it should
    // refresh the path (reloads the listing and transfers the selection, icons, focus, etc. to
    // the new panel content); the refresh is performed when Salamander's main window becomes active
    // (when suspend mode ends); the FS path is always reloaded; if 'modifiedFS' is not in any
    // panel, nothing happens; if 'focusFirstNewItem' is TRUE and only a single item was added to the panel,
    // that new item is focused (used, for example, to focus a newly created file/directory);
    // NOTE: there is also PostRefreshPanelFS2, which returns TRUE if the refresh was performed and
    // FALSE if 'modifiedFS' was not found in any panel;
    // can be called from any thread (if the main thread is not executing code inside the plugin,
    // the refresh happens as soon as possible; otherwise it waits at least until the main
    // thread leaves the plugin)
    virtual void WINAPI PostRefreshPanelFS(CPluginFSInterfaceAbstract* modifiedFS,
                                           BOOL focusFirstNewItem = FALSE) = 0;

    // closes a detached plugin FS (if possible, see CPluginFSInterfaceAbstract::TryCloseOrDetach),
    // 'detachedFS' is the detached plugin FS; returns TRUE on success (FALSE means the detached
    // plugin FS was not closed); 'parent' is the parent of any message boxes (currently only
    // CPluginFSInterfaceAbstract::ReleaseObject can open them)
    // Note: a plugin FS opened in a panel is closed, for example, via ChangePanelPathToRescuePathOrFixedDrive
    // limitation: main thread + outside CPluginFSInterfaceAbstract methods (we are trying to close a
    // detached FS - the method's 'this' could cease to exist)
    virtual BOOL WINAPI CloseDetachedFS(HWND parent, CPluginFSInterfaceAbstract* detachedFS) = 0;

    // doubles '&' - useful for paths displayed in a menu ('&&' is displayed as '&');
    // 'buffer' is the input/output string, 'bufferSize' is the size of 'buffer' in bytes;
    // returns TRUE if doubling did not truncate characters at the end of the string (the buffer was
    // large enough)
    // can be called from any thread
    virtual BOOL WINAPI DuplicateAmpersands(char* buffer, int bufferSize) = 0;

    // removes '&' from the text; if it finds the pair "&&", it replaces it with a single '&'
    // can be called from any thread
    virtual void WINAPI RemoveAmpersands(char* text) = 0;

    // ValidateVarString and ExpandVarString:
    // methods for validating and expanding strings with variables in the form "$(var_name)", "$(var_name:num)"
    // (num is the variable width, a numeric value from 1 to 9999), "$(var_name:max)" ("max" is a
    // symbol indicating that the variable width is determined by the value in the 'maxVarWidths' array; see
    // ExpandVarString for details) and "$[env_var]" (expands the value of an environment variable); used when the
    // user can specify a format string (for example, in the info line). Example of a string with variables:
    // "$(files) files and $(dirs) directories" - variables 'files' and 'dirs';
    // source code for use in the info line (without variables in the form "$(varname:max)") is in DEMOPLUG
    //
    // checks the syntax of 'varText' (a string with variables), returns FALSE if it finds an error; the
    // position is stored in 'errorPos1' (offset of the start of the erroneous part) and 'errorPos2' (offset of the end of the
    // erroneous part); 'variables' is an array of CSalamanderVarStrEntry structures terminated by a structure with
    // Name==NULL; 'msgParent' is the parent window for error message boxes; if it is NULL, errors are not displayed
    virtual BOOL WINAPI ValidateVarString(HWND msgParent, const char* varText, int& errorPos1, int& errorPos2,
                                          const CSalamanderVarStrEntry* variables) = 0;
    //
    // fills 'buffer' with the expansion of 'varText' (the string with variables); returns FALSE if
    // 'buffer' is too small (it expects the variable string to be validated by ValidateVarString,
    // otherwise it also returns FALSE on a syntax error) or if the user clicked Cancel when an
    // environment-variable error occurred (variable not found or too long); 'bufferLen' is the size of 'buffer';
    // 'variables' is an array of CSalamanderVarStrEntry structures terminated by an entry with
    // Name == NULL; 'param' is the pointer passed to CSalamanderVarStrEntry::Execute when expanding
    // the found variable; 'msgParent' is the parent of error message boxes; if it is NULL, errors are not shown;
    // if 'ignoreEnvVarNotFoundOrTooLong' is TRUE, environment-variable errors (not found or too long) are ignored;
    // if it is FALSE, a message box with the error is displayed; if 'varPlacements' is not NULL, it points to an array
    // of DWORD items with '*varPlacementsCount' entries that will be filled with DWORD values composed of the
    // variable position in the output buffer (lower WORD) and the variable length (upper WORD); if 'varPlacementsCount'
    // is not NULL, it returns the number of filled entries in the 'varPlacements' array (effectively the number of variables
    // in the input string);
    // if this method is used only to expand the string for a single 'param' value, set 'detectMaxVarWidths' to FALSE,
    // 'maxVarWidths' to NULL, and 'maxVarWidthsCount' to 0; if the method is used repeatedly to expand the string for a
    // specific set of 'param' values (for example Make File List expands a line for all selected files and directories),
    // it makes sense to use variables in the form "$(varname:max)" whose width is determined as the largest width of the
    // expanded variable within the entire set; measuring the maximum width of the expanded variable is done in the first
    // cycle (for all values in the set) of calling ExpandVarString; in the first cycle the 'detectMaxVarWidths' parameter is
    // TRUE and the 'maxVarWidths' array with 'maxVarWidthsCount' entries is cleared in advance (used to store the maxima
    // between individual calls to ExpandVarString);
    // the actual expansion then takes place in the second cycle (for all values in the set) of calling ExpandVarString; in the
    // second cycle the 'detectMaxVarWidths' parameter is FALSE and the 'maxVarWidths' array with 'maxVarWidthsCount'
    // entries contains the precomputed maximum widths (from the first cycle)
    virtual BOOL WINAPI ExpandVarString(HWND msgParent, const char* varText, char* buffer, int bufferLen,
                                        const CSalamanderVarStrEntry* variables, void* param,
                                        BOOL ignoreEnvVarNotFoundOrTooLong = FALSE,
                                        DWORD* varPlacements = NULL, int* varPlacementsCount = NULL,
                                        BOOL detectMaxVarWidths = FALSE, int* maxVarWidths = NULL,
                                        int maxVarWidthsCount = 0) = 0;

    // sets the plugin's load-on-Salamander-start flag (should the plugin be loaded when Salamander starts?);
    // 'start' is the new flag value; returns the previous value; if SetFlagLoadOnSalamanderStart
    // has never been called, the flag is FALSE (the plugin is not loaded at startup, only when needed)
    // limitation: main thread (otherwise the plugin configuration could change during the call)
    virtual BOOL WINAPI SetFlagLoadOnSalamanderStart(BOOL start) = 0;

    // marks the calling plugin to be unloaded at the earliest opportunity
    // (once all posted menu commands are executed (see PostMenuExtCommand), there are no messages
    // in the main thread's message queue, and Salamander is not "busy");
    // WARNING: when called from a thread other than the main thread, the unload request (processed
    // in the main thread) may arrive even before PostUnloadThisPlugin finishes (see
    // CPluginInterfaceAbstract::Release for more information about unloading)
    // can be called from any thread (but only after the plugin entry point finishes; while the entry point
    // is running, the method may be called from the main thread only)
    virtual void WINAPI PostUnloadThisPlugin() = 0;

    // iterates over Salamander modules (the executable and .spl files of installed plugins, all with versions);
    // 'index' is an input/output variable pointing to an int that must contain 0 on the first call; the function stores
    // the value for the next call when it returns (usage: reset to zero at start, then leave unchanged);
    // 'module' is the buffer for the module name (minimum size MAX_PATH characters);
    // 'version' is the buffer for the module version (minimum size MAX_PATH characters);
    // returns FALSE if 'module' + 'version' are not filled because there are no more modules;
    // returns TRUE if 'module' + 'version' contain another module
    // limitation: main thread (otherwise the plugin configuration might change during the call - add/remove)
    virtual BOOL WINAPI EnumInstalledModules(int* index, char* module, char* version) = 0;

    // calls 'loadOrSaveFunc' to load or save the configuration; if 'load' is TRUE, the configuration is loaded.
    // If the plugin supports "load/save configuration" and its private registry key exists at the time of the call,
    // 'loadOrSaveFunc' is called for that key, otherwise the default configuration is loaded (the 'regKey' parameter of
    // 'loadOrSaveFunc' is NULL);
    // if 'load' is FALSE, the configuration is saved; 'loadOrSaveFunc' is called only when the plugin supports
    // "load/save configuration" and Salamander's key exists at the time of the call;
    // 'param' is a user parameter forwarded to 'loadOrSaveFunc'
    // limitation: main thread; inside the entry point it can be called only after SetBasicPluginData,
    // earlier it might not be known whether "load/save configuration" is supported or what the private registry key is
    virtual void WINAPI CallLoadOrSaveConfiguration(BOOL load, FSalLoadOrSaveConfiguration loadOrSaveFunc,
                                                    void* param) = 0;

    // stores 'text' of length 'textLen' (-1 means "use strlen") on the clipboard both as multibyte
    // and Unicode text (otherwise, for example, Notepad cannot handle Czech); if successful it can display (if 'showEcho' is TRUE)
    // the message "Text was successfully copied to clipboard." (the parent of the message box is 'echoParent');
    // returns TRUE on success
    // can be called from any thread
    virtual BOOL WINAPI CopyTextToClipboard(const char* text, int textLen, BOOL showEcho, HWND echoParent) = 0;

    // stores Unicode 'text' of length 'textLen' (-1 means "use wcslen") on the clipboard both as Unicode
    // and multibyte text (otherwise, for example, MSVC 6.0 cannot handle Czech); if successful it can display (if 'showEcho' is TRUE)
    // the message "Text was successfully copied to clipboard." (the parent of the message box is 'echoParent');
    // returns TRUE on success
    // can be called from any thread
    virtual BOOL WINAPI CopyTextToClipboardW(const wchar_t* text, int textLen, BOOL showEcho, HWND echoParent) = 0;

    // runs the menu command with the identifier 'id' in the main thread (calls
    // CPluginInterfaceForMenuExtAbstract::ExecuteMenuItem(salamander, main-window-hwnd, 'id', 0);
    // 'salamander' is NULL if 'waitForSalIdle' is FALSE, otherwise it contains a pointer to a valid
    // set of Salamander methods for performing operations; the return value is ignored);
    // if 'waitForSalIdle' is FALSE, the command is triggered by posting a message to the main window
    // (the message is delivered by any running message loop in the main thread – including modal dialogs/message boxes,
    // even those opened by the plugin), so reentering the plugin is possible; if 'waitForSalIdle' is TRUE,
    // 'id' is limited to the range <0, 999999> and the command runs once the main-thread message queue is empty and
    // Salamander is not "busy" (no modal dialog is open and no message is being processed);
    // WARNING: when called from a thread other than the main thread, the menu command (executed in the main thread)
    // may start even before PostMenuExtCommand finishes
    // can be called from any thread, and if 'waitForSalIdle' is FALSE, you must wait until after
    // CPluginInterfaceAbstract::GetInterfaceForMenuExt is called (invoked from the main thread after the entry point)
    virtual void WINAPI PostMenuExtCommand(int id, BOOL waitForSalIdle) = 0;

    // determines whether there is a high chance (this cannot be determined with certainty) that Salamander will not be "busy"
    // in the next few moments (no modal dialog is open and no message is being processed) - returns TRUE in that case
    // (otherwise FALSE); if 'lastIdleTime' is not NULL, it receives the GetTickCount() value from the moment of the last
    // transition from the "idle" to the "busy" state; this can be used, for example, to predict delivery of a command posted via
    // CSalamanderGeneralAbstract::PostMenuExtCommand with 'waitForSalIdle' == TRUE;
    // can be called from any thread
    virtual BOOL WINAPI SalamanderIsNotBusy(DWORD* lastIdleTime) = 0;

    // sets the message that the Bug Report dialog should display if a crash occurs inside the plugin
    // (inside the plugin = at least one call-stack message stored from the plugin) and allows overriding the
    // default bug-report e-mail address (support@altap.cz); 'message' is the new message (NULL means "no message");
    // 'email' is the new e-mail address (NULL means "use the default"; maximum e-mail length is 100 characters);
    // this method can be called repeatedly; the previous message and e-mail are overwritten; Salamander does not
    // remember either the message or the e-mail for the next run, so this method must always be called again when
    // the plugin is loaded (preferably in the entry point)
    // limitation: main thread (otherwise changes to the plugin configuration may occur during the call)
    virtual void WINAPI SetPluginBugReportInfo(const char* message, const char* email) = 0;

    // checks whether a plugin is installed (however, it does not determine whether it can be loaded - for example, whether the
    // user has merely deleted it from disk); 'pluginSPL' identifies the plugin - it is the required trailing part of the
    // full path to the plugin's .SPL file (for example, "ieviewer\\ieviewer.spl" identifies the
    // IEViewer shipped with Salamander); returns TRUE if the plugin is installed
    // limitation: main thread (otherwise the plugin configuration may change during the call)
    virtual BOOL WINAPI IsPluginInstalled(const char* pluginSPL) = 0;

    // opens a file in a viewer implemented by a plugin or in the internal text/hex viewer;
    // if 'pluginSPL' is NULL, the internal text/hex viewer should be used, otherwise it identifies the plugin
    // viewer – it is the required trailing part of the full path to the plugin's .SPL file (e.g.
    // "ieviewer\\ieviewer.spl" identifies the IEViewer shipped with Salamander); 'pluginData'
    // is a data structure containing the viewed file name and may optionally hold extended
    // viewer parameters (see CSalamanderPluginViewerData); if 'useCache' is FALSE, 'rootTmpPath'
    // and 'fileNameInCache' are ignored and the file is simply opened in the viewer;
    // if 'useCache' is TRUE, the file is first moved to the disk cache under the file name
    // 'fileNameInCache' (name without path), then opened in the viewer, and after the viewer closes it is
    // removed from the disk cache; if 'pluginData->FileName' is located on the same drive as the
    // disk cache, the move is instant; otherwise the file is copied between volumes, which can take
    // longer, but no progress dialog is shown (if 'rootTmpPath' is NULL, the disk cache resides in the Windows TEMP
    // directory, otherwise the disk cache path is in 'rootTmpPath'; SalMoveFile is used for the move to the disk cache); using
    // SalGetTempFileName with 'path' equal to 'rootTmpPath' is ideal;
    // returns TRUE when the file is opened in the viewer successfully; returns FALSE if opening fails (the specific
    // reason is stored in 'error' – 0 - success, 1 - plugin cannot be loaded, 2 - the plugin's ViewFile returned an
    // error, 3 - the file cannot be moved to the disk cache); if 'useCache' is TRUE, the file is deleted from the disk
    // (as if the viewer was closed)
    // limitation: main thread (otherwise the plugin configuration could change during the call),
    // and it cannot be called from the entry point (plugin loading is not reentrant)
    virtual BOOL WINAPI ViewFileInPluginViewer(const char* pluginSPL,
                                               CSalamanderPluginViewerData* pluginData,
                                               BOOL useCache, const char* rootTmpPath,
                                               const char* fileNameInCache, int& error) = 0;

    // notifies Salamander, then all loaded plugins, and then all open FS instances (in panels and detached)
    // about a change on the 'path' (disk or FS path) as soon as possible; this is important for paths where
    // changes cannot be monitored automatically (see FindFirstChangeNotification) or where the user disabled
    // such monitoring (auto-refresh); FS plugins handle change monitoring themselves; the notification happens as
    // soon as possible (if the main thread is not executing code inside the plugin, the refresh takes place after the
    // message is delivered to the main window and after refresh is re-enabled, e.g. when a dialog closes; otherwise
    // the refresh waits at least until the main thread leaves the plugin); 'includingSubdirs' is TRUE if the change may also
    // affect subdirectories of 'path';
    // WARNING: when called from a thread other than the main thread, the change notification (processed in the main thread)
    // may occur even before PostChangeOnPathNotification finishes
    // can be called from any thread
    virtual void WINAPI PostChangeOnPathNotification(const char* path, BOOL includingSubdirs) = 0;

    // tries to access the Windows path 'path' (normal or UNC); the test runs in a worker thread, so it
    // can be interrupted by pressing ESC (after some time, a dialog appears reporting ESC)
    // 'echo' TRUE means an error message may be displayed if the path is inaccessible;
    // if 'err' differs from ERROR_SUCCESS and 'echo' is TRUE, it only displays the error (the path
    // is no longer accessed); 'parent' is the parent of the message box; returns ERROR_SUCCESS if
    // the path is OK, otherwise it returns the standard Windows error code or ERROR_USER_TERMINATED
    // if the user used the ESC key to abort the test
    // restriction: main thread only (repeated calls are not possible and the main thread uses this method)
    virtual DWORD WINAPI SalCheckPath(BOOL echo, const char* path, DWORD err, HWND parent) = 0;

    // checks whether the Windows path 'path' is accessible and, if necessary, restores network connections (for
    // a normal path, it tries to reconnect remembered network connections; for a UNC path, it allows login
    // with a new user name and password); returns TRUE if the path is accessible; 'parent' is the parent of
    // message boxes and dialogs; 'tryNet' is TRUE if it makes sense to try restoring network connections
    // (if FALSE, it falls back to SalCheckPath; this is only for optimization)
    // limitation: main thread only (repeated calls are not possible and the main thread uses this method)
    virtual BOOL WINAPI SalCheckAndRestorePath(HWND parent, const char* path, BOOL tryNet) = 0;

    // a more complex variant of SalCheckAndRestorePath; checks whether the Windows path 'path' is accessible and,
    // if necessary, shortens it; if 'tryNet' is TRUE, it also attempts to restore the network connection and sets
    // 'tryNet' to FALSE (for a normal path, it tries to re-establish the remembered network connection; for a UNC
    // path, it allows login with a new user name and password); if 'donotReconnect' is TRUE, it only detects the
    // error and no longer attempts to restore the connection; returns 'err' (the Windows error code of the current
    // path), 'lastErr' (the error code that led to the path being shortened), 'pathInvalid' (TRUE if an attempt
    // to restore the network connection was made without success), 'cut' (TRUE if the resulting path is shortened);
    // 'parent' is the parent message box; returns TRUE if the resulting 'path' is accessible
    // limitation: main thread (repeated calls are not possible and the main thread uses this method)
    virtual BOOL WINAPI SalCheckAndRestorePathWithCut(HWND parent, char* path, BOOL& tryNet, DWORD& err,
                                                      DWORD& lastErr, BOOL& pathInvalid, BOOL& cut,
                                                      BOOL donotReconnect) = 0;

    // detects what type of path (FS/Windows/archive) is provided and splits it into its parts
    // (for FS paths into fs-name and fs-user-part, for archive paths into path-to-archive and
    // path-in-archive, for Windows paths into the existing part and the remainder); FS paths
    // are not validated; Windows (normal + UNC) paths are checked to determine how far the path exists
    // (and optionally restore the network connection); archive paths check the existence of the archive file
    // (archive detection is based on the extension);
    // 'path' is the full or relative path (buffer of at least 'pathBufSize' characters; for relative paths the
    // current path 'curPath' (if not NULL) is used as the base for resolving the full path;
    // 'curPathIsDiskOrArchive' is TRUE if 'curPath' is a Windows or archive path;
    // if the current path is an archive, 'curArchivePath' contains the archive name, otherwise it is NULL);
    // the resulting full path is stored back into 'path' (must be at least 'pathBufSize' characters); returns TRUE when
    // recognition succeeds; in that case 'type' is the path type (see PATH_TYPE_XXX) and 'secondPart' is set as follows:
    // - to the position within 'path' after the existing part (after '\\' or at the end of the string; if a file is present
    //   in the path, it points right after its path) (Windows path type). WARNING: the length of the returned part is not
    //   handled (the full path may exceed MAX_PATH)
    // - to the character after the archive file (archive path type). WARNING: the path length inside the archive is not
    //   handled (it may exceed MAX_PATH)
    // - to the character after ':' following the file-system name – the user part of the FS path (FS path type). WARNING:
    //   the length of the user part is not handled (it may exceed MAX_PATH);
    // if TRUE is returned, 'isDir' is also set to:
    // - TRUE if the existing part of the path is a directory, FALSE if it is a file (Windows path type)
    // - FALSE for archive and FS paths;
    // if FALSE is returned, an error has already been reported to the user (with one exception – see SPP_INCOMLETEPATH)
    // describing what failed during recognition (if 'error' is not NULL, one of the SPP_XXX constants is stored there);
    // 'errorTitle' is the title of the error message box; if 'nextFocus' != NULL and the Windows/archive path does not contain
    // '\\' or ends right after '\\', the path is copied into 'nextFocus' (see SalGetFullName);
    // WARNING: uses SalGetFullName, therefore it is advisable to call CSalamanderGeneralAbstract::SalUpdateDefaultDir first
    // limitation: main thread (repeated calls are not allowed and the main thread uses this method)
    virtual BOOL WINAPI SalParsePath(HWND parent, char* path, int& type, BOOL& isDir, char*& secondPart,
                                     const char* errorTitle, char* nextFocus, BOOL curPathIsDiskOrArchive,
                                     const char* curPath, const char* curArchivePath, int* error,
                                     int pathBufSize) = 0;

    // obtains the existing part and the operation mask from a Windows target path; allows creating any missing part;
    // on success returns TRUE and stores the existing Windows target path in 'path' and the detected operation mask in 'mask'
    // (points inside the 'path' buffer, but the path and mask are separated by a null character; if the path lacks a mask,
    // a "*.*" mask is created automatically). 'parent' is the parent of any message boxes; 'title' and 'errorTitle' are the
    // titles of informational and error message boxes; 'selCount' is the number of selected files and directories; 'path' is
    // the target path to process on input and, on output (at least 2 * MAX_PATH characters), the existing target path;
    // 'secondPart' points within 'path' behind the existing part (after '\\' or at the end – if the path contains a file, it
    // points after that file's path); 'pathIsDir' is TRUE/FALSE depending on whether the existing part is a directory/file;
    // 'backslashAtEnd' is TRUE if 'path' ended with a backslash before parsing (e.g. SalParsePath removes that backslash);
    // 'dirName' and 'curDiskPath' are not NULL if at most one file/directory is selected (its name without the path is in
    // 'dirName'; if nothing is selected, the focus is used) and the current path is a Windows path (stored in 'curDiskPath');
    // 'mask' is the output pointer to the operation mask within the 'path' buffer; if an error occurs, the method returns
    // FALSE because the problem has already been reported to the user
    // can be called from any thread
    virtual BOOL WINAPI SalSplitWindowsPath(HWND parent, const char* title, const char* errorTitle,
                                            int selCount, char* path, char* secondPart, BOOL pathIsDir,
                                            BOOL backslashAtEnd, const char* dirName,
                                            const char* curDiskPath, char*& mask) = 0;

    // obtains the existing part of the target path and the operation mask; detects any non-existing part; on
    // success returns TRUE, the relative path to create (in 'newDirs'), the existing target path (in 'path';
    // existing only if the relative path 'newDirs' is created), and the detected operation mask
    // (in 'mask' - points into the 'path' buffer, but the path and mask are separated by a null; if the path contains no
    // mask, it automatically creates the mask "*.*"); 'parent' is the parent of any message boxes;
    // 'title' + 'errorTitle' are the titles of the information + error message boxes; 'selCount' is the number of selected
    // files and directories; 'path' is the target path to process on input and, on output (at least 2 * MAX_PATH
    // characters), the existing target path (always ending with a backslash); 'afterRoot' points into 'path' past the root
    // of the path (past '\\' or to the end of the string); 'secondPart' points into 'path' to the position after the existing
    // path (past '\\' or to the end of the string; if the path contains a file, it points past the path to that file);
    // 'pathIsDir' is TRUE/FALSE if the existing part of the path is a directory/file; 'backslashAtEnd' is
    // TRUE if there was a backslash at the end of 'path' before "parse" was performed (e.g. SalParsePath removes such
    // a backslash); 'dirName' + 'curPath' are not NULL if at most one file/directory is selected
    // (its name without the path is in 'dirName'; its path is in 'curPath'; if nothing is selected, the focus is used);
    // 'mask' is the output pointer to the operation mask in the 'path' buffer; if 'newDirs' is not NULL,
    // then it is a buffer (of size at least MAX_PATH) for the relative path (relative to the existing path
    // in 'path') that must be created (the user agreed to create it, the same prompt as for copying from disk to disk
    // was used; empty string = create nothing); if 'newDirs' is NULL and
    // some relative path needs to be created, only an error is reported; 'isTheSamePathF' is a function for
    // comparing two paths (needed only if 'curPath' is not NULL); if it is NULL, IsTheSamePath is used;
    // if there is an error in the path, the method returns FALSE; the problem has already been reported to the user
    // can be called from any thread
    virtual BOOL WINAPI SalSplitGeneralPath(HWND parent, const char* title, const char* errorTitle,
                                            int selCount, char* path, char* afterRoot, char* secondPart,
                                            BOOL pathIsDir, BOOL backslashAtEnd, const char* dirName,
                                            const char* curPath, char*& mask, char* newDirs,
                                            SGP_IsTheSamePathF isTheSamePathF) = 0;

    // removes ".." (removes ".." together with one preceding subdirectory) and "." (removes only ".")
    // from a path; requires backslash as the subdirectory separator; 'afterRoot' points past the root
    // of the processed path (path changes are made only after 'afterRoot'); returns TRUE if the changes
    // succeeded, FALSE if ".." cannot be removed (the root is already on the left)
    // can be called from any thread
    virtual BOOL WINAPI SalRemovePointsFromPath(char* afterRoot) = 0;

    // returns a parameter from Salamander's configuration; 'paramID' identifies the parameter
    // (see the SALCFG_XXX constants); 'buffer' points to the buffer where the parameter data will be copied;
    // its size is 'bufferSize'; if 'type' is not NULL, it returns one of the SALCFGTYPE_XXX constants or
    // SALCFGTYPE_NOTFOUND (if the parameter with 'paramID' was not found); returns TRUE if 'paramID' is valid
    // and the configuration value fits into 'buffer'
    // note: configuration changes are reported via the PLUGINEVENT_CONFIGURATIONCHANGED event
    //       (see CPluginInterfaceAbstract::Event)
    // limitation: main thread – configuration changes happen only in the main thread (no additional synchronization)
    virtual BOOL WINAPI GetConfigParameter(int paramID, void* buffer, int bufferSize, int* type) = 0;

    // changes the character case in a file name (name without path); 'tgtName' is the output buffer
    // (its size must be at least enough for the 'srcName' string); 'srcName' is the file name (it is modified,
    // but restored before returning); 'format' selects the output format (1 - capitalize words,
    // 2 - all lowercase, 3 - all uppercase, 4 - unchanged, 5 - for DOS names (8.3) -> capitalize words,
    // 6 - file lowercase, directory uppercase, 7 - capitalize the name and lowercase the extension);
    // 'changedParts' determines which parts of the name should change (0 - change name and extension,
    // 1 - change name only (valid only with format 1, 2, 3, 4), 2 - change extension only (valid only with
    // format 1, 2, 3, 4)); 'isDir' is TRUE if the name refers to a directory
    // can be called from any thread
    virtual void WINAPI AlterFileName(char* tgtName, char* srcName, int format, int changedParts,
                                      BOOL isDir) = 0;

    // shows/hides a message in a window running in its own thread (it does not drain the message queue);
    // it shows only one message at a time, repeated calls report an error to TRACE (not fatal);
    // NOTE: it is used in SalCheckPath and other routines, so collisions between
    //       requests to open windows may occur (not fatal, the window is simply not shown)
    // everything can be called from any thread (but the window must be handled only
    // from a single thread - it cannot be shown from one thread and hidden from another)
    //
    // opens a window with the text 'message' after a 'delay' (in ms), only if 'hForegroundWnd' is NULL
    // or identifies the foreground window
    // 'message' can be multi-line; individual lines are separated by '\n'
    // 'caption' can be NULL: "Open Salamander" is then used
    // 'showCloseButton' specifies whether the window will contain a Close button; equivalent to the Escape key
    virtual void WINAPI CreateSafeWaitWindow(const char* message, const char* caption,
                                             int delay, BOOL showCloseButton, HWND hForegroundWnd) = 0;
    // close the window
    virtual void WINAPI DestroySafeWaitWindow() = 0;
    // hides/shows the window (if it is open); call in response to WM_ACTIVATE from the hForegroundWnd window:
    //    case WM_ACTIVATE:
    //    {
    //      ShowSafeWaitWindow(LOWORD(wParam) != WA_INACTIVE);
    //      break;
    //    }
    // If the thread (from which the window was created) is busy, messages are not dispatched,
    // so WM_ACTIVATE is not delivered when the user clicks another application.
    // The messages are delivered only when a message box is shown,
    // which is exactly what we need: temporarily hide the window and later (after the
    // message box is closed and the hForegroundWnd window is activated) show it again.
    virtual void WINAPI ShowSafeWaitWindow(BOOL show) = 0;
    // after calling CreateSafeWaitWindow or ShowSafeWaitWindow, the function returns FALSE until
    // the user clicks the Close button with the mouse (if it is shown); it then returns TRUE
    virtual BOOL WINAPI GetSafeWaitWindowClosePressed() = 0;
    // used for subsequent changes to the text in the window
    // WARNING: the window is not laid out again, and if the text becomes longer,
    // it will be clipped; use for example for a countdown: 60s, 55s, 50s, ...
    virtual void WINAPI SetSafeWaitWindowText(const char* message) = 0;

    // najde v disk-cache existujici kopii souboru a zamkne ji (zakaze jeji zruseni); 'uniqueFileName'
    // je unikatni nazev originalniho souboru (podle tohoto nazvu se prohledava disk-cache; melo by
    // stacit plne jmeno souboru v salamanderovske forme - "fs-name:fs-user-part"; POZOR: nazev se
    // porovnava "case-sensitive", pokud plugin vyzaduje "case-insensitive", musi vsechny nazvy
    // prevadet napr. na mala pismena - viz CSalamanderGeneralAbstract::ToLowerCase); v 'tmpName'
    // se vraci ukazatel (platny az do zruseni kopie souboru v disk-cache) na plne jmeno kopie souboru,
    // ktera je umistena v docasnem adresari; 'fileLock' je zamek kopie souboru, jde o systemovy event
    // ve stavu nonsignaled, ktery po zpracovani kopie souboru prejde do stavu signaled (je nutne
    // pouzit metodu UnlockFileInCache; plugin dava signal, ze kopii v disk-cache jiz lze zrusit);
    // pokud nebyla kopie nalezena vraci FALSE a 'tmpName' NULL (jinak vraci TRUE)
    // mozne volat z libovolneho threadu
    virtual BOOL WINAPI GetFileFromCache(const char* uniqueFileName, const char*& tmpName, HANDLE fileLock) = 0;

    // unlocks the lock for a file copy in the disk-cache (sets 'fileLock' to the signaled state, prompts
    // the disk-cache to check the lock, and then sets 'fileLock' back to the nonsignaled state);
    // if this was the last lock, the copy may be deleted; when the copy is deleted depends
    // on the size of the disk-cache on disk; the lock may also be used for multiple file copies (the lock
    // must be of the "manual reset" type, otherwise after unlocking the first copy the lock is set to the
    // nonsignaled state and unlocking stops); in this case, all copies are unlocked
    // can be called from any thread
    virtual void WINAPI UnlockFileInCache(HANDLE fileLock) = 0;

    // vlozi (presune) kopii souboru do disk-cache (vkladana kopie neni zamcena, takze kdykoliv muze dojit
    // k jejimu zruseni); 'uniqueFileName' je unikatni nazev originalniho souboru (podle tohoto
    // nazvu se prohledava disk-cache; melo by stacit plne jmeno souboru v salamanderovske
    // forme - "fs-name:fs-user-part"; POZOR: nazev se porovnava "case-sensitive", pokud plugin
    // vyzaduje "case-insensitive", musi vsechny nazvy prevadet napr. na mala pismena - viz
    // CSalamanderGeneralAbstract::ToLowerCase); 'nameInCache' je jmeno kopie souboru, ktera bude umistena
    // v docasnem adresari (ocekava se zde posledni cast jmena originalniho souboru, aby pozdeji
    // uzivateli pripominala originalni soubor); 'newFileName' je plne jmeno ukladane kopie souboru,
    // ktera bude presunuta do disk-cache pod jmenem 'nameInCache', musi byt umistena na stejnem disku
    // jako diskova cache (je-li 'rootTmpPath' NULL, je diskova cache ve Windows TEMP adresari, jinak
    // je cesta do disk-cache v 'rootTmpPath'; pro prejmenovani do disk cache pres Win32 API funkci
    // MoveFile); 'newFileName' je idealni ziskat volanim SalGetTempFileName s parametrem 'path' rovnym
    // 'rootTmpPath'); v 'newFileSize' je velikost ukladane kopie souboru; vraci TRUE pri uspechu
    // (soubor byl presunut do disk-cache - zmizel z puvodniho mista na disku), vraci FALSE pri
    // interni chybe nebo pokud soubor jiz je v disk-cache (neni-li 'alreadyExists' NULL, vraci
    // se v nem TRUE pokud soubor jiz je v disk-cache)
    // POZN.: pokud plugin vyuziva disk-cache, mel by alespon pri unloadu pluginu zavolat
    //        CSalamanderGeneralAbstract::RemoveFilesFromCache("fs-name:"), jinak budou
    //        jeho kopie souboru zbytecne prekazet v disk-cache
    // mozne volat z libovolneho threadu
    virtual BOOL WINAPI MoveFileToCache(const char* uniqueFileName, const char* nameInCache,
                                        const char* rootTmpPath, const char* newFileName,
                                        const CQuadWord& newFileSize, BOOL* alreadyExists) = 0;

    // removes from disk-cache the copy of the file whose unique name is 'uniqueFileName' (NOTE: the name
    // is compared "case-sensitive"; if the plugin requires "case-insensitive", it must convert all names
    // for example to lowercase - see CSalamanderGeneralAbstract::ToLowerCase); if the file copy
    // is still being used, it is removed as soon as possible (when the viewers are closed), but in any case it
    // will no longer be provided by disk-cache to anyone as a valid copy of the file (it is marked as out-of-date)
    // can be called from any thread
    virtual void WINAPI RemoveOneFileFromCache(const char* uniqueFileName) = 0;

    // removes from the disk cache all copies of files whose unique names start with 'fileNamesRoot'
    // (used when closing a file system, when it is no longer desirable to cache downloaded copies of
    // files; WARNING: names are compared "case-sensitive"; if the plugin requires "case-insensitive",
    // it must convert all names, for example, to lowercase - see CSalamanderGeneralAbstract::ToLowerCase);
    // if the file copies are still in use, they are removed as soon as possible (after they are unlocked,
    // e.g. after closing the viewer); in any case, the disk cache will no longer provide them to anyone as valid copies
    // of files (they are marked as out-of-date)
    // can be called from any thread
    virtual void WINAPI RemoveFilesFromCache(const char* fileNamesRoot) = 0;

    // vraci postupne konverzni tabulky (nactene ze souboru convert\XXX\convert.cfg
    // v instalaci Salamandera - XXX je prave pouzivany adresar konverznich tabulek);
    // 'parent' je parent messageboxu (je-li NULL, je parent hlavni okno);
    // 'index' je vstupne/vystupni promenna, ukazuje na int, ve kterem je pri prvnim volani 0,
    // hodnotu pro dalsi volani si funkce ulozi pri navratu (pouziti: na zacatku vynulovat, pak
    // nemenit); vraci FALSE, pokud jiz neexistuje zadna dalsi tabulka; pokud vraci TRUE, obsahuje
    // 'name' (neni-li NULL) odkaz na jmeno konverze (muze obsahovat '&' - podtrzeny znak v menu) nebo NULL
    // jde-li o separator a 'table' (neni-li NULL) odkaz na 256-bytovou konverzni tabulku nebo NULL
    // jde-li o separator; odkazy 'name' a 'table' jsou platne po celou dobu behu Salamandera (neni
    // nutne kopirovat obsah)
    // POZOR: ukazatel 'table' pouzivat timto zpusobem (nutne pretypovani na "unsigned"):
    //        *s = table[(unsigned char)*s]
    // mozne volat z libovolneho threadu
    virtual BOOL WINAPI EnumConversionTables(HWND parent, int* index, const char** name, const char** table) = 0;

    // vraci konverzni tabulku 'table' (buffer min. 256 znaku) pro konverzi 'conversion' (jmeno
    // konverze viz soubor convert\XXX\convert.cfg v instalaci Salamandera, napr. "ISO-8859-2 - CP1250";
    // znaky <= ' ' a '-' a '&' ve jmene pri hledani nehraji roli; hleda se bez ohledu na velka a mala
    // pismena); 'parent' je parent messageboxu (je-li NULL, je parent hlavni okno); vraci TRUE
    // pokud byla konverze nalezena (jinak neni obsah 'table' platny);
    // POZOR: pouzivat timto zpusobem (nutne pretypovani na "unsigned"): *s = table[(unsigned char)*s]
    // mozne volat z libovolneho threadu
    virtual BOOL WINAPI GetConversionTable(HWND parent, char* table, const char* conversion) = 0;

    // returns the name of the code page used by Windows in this region (read from convert\XXX\convert.cfg
    // in the Salamander installation); this is a normally displayable encoding, so it is used when
    // text created in a different code page needs to be displayed (here it is specified as the
    // "target" encoding when looking up the conversion table, see the GetConversionTable method);
    // 'parent' is the parent of the message box (if it is NULL, the main window is used); 'codePage' is a buffer
    // (min. 101 characters) for the code page name (if this name is not defined in convert\XXX\convert.cfg,
    // an empty string is returned in the buffer)
    // can be called from any thread
    virtual void WINAPI GetWindowsCodePage(HWND parent, char* codePage) = 0;

    // determines from the 'pattern' buffer of length 'patternLen' (e.g. the first 10000 characters) whether it is
    // text (i.e. whether there is a code page in which it contains only allowed characters - printable
    // and control), and if it is text, also determines its code page (the most probable one);
    // 'parent' is the parent of the message box (if it is NULL, the main window is the parent); if 'forceText' is
    // TRUE, the check for disallowed characters is not performed (used when 'pattern' contains
    // text); if 'isText' is not NULL, TRUE is returned in it if the buffer is text; if 'codePage' is not
    // NULL, it is a buffer (min. 101 characters) for the code page name (the most probable one)
    // can be called from any thread
    virtual void WINAPI RecognizeFileType(HWND parent, const char* pattern, int patternLen, BOOL forceText,
                                          BOOL* isText, char* codePage) = 0;

    // determines from the buffer 'text' of length 'textLen' whether it is ANSI text (it contains,
    // in the ANSI character set, only allowed characters - printable and control); it decides without context
    // (it does not depend on the number of characters or their order - the tested text can be split
    // into arbitrary parts and tested progressively); returns TRUE if it is ANSI text (otherwise
    // the contents of buffer 'text' are binary)
    // can be called from any thread
    virtual BOOL WINAPI IsANSIText(const char* text, int textLen) = 0;

    // calls the 'callback' function with the 'param' parameter and a function for obtaining selected
    // files/directories (see the definition of type SalPluginOperationFromDisk) from panel 'panel'
    // (a Windows path must be open in the panel); 'panel' is one of PANEL_XXX
    // limitation: main thread
    virtual void WINAPI CallPluginOperationFromDisk(int panel, SalPluginOperationFromDisk callback,
                                                    void* param) = 0;

    // returns the default charset set by the user (part of the regional
    // settings); fonts must be created with this charset, otherwise the
    // texts may not be readable (if the text is in the default code page, see
    // the Win32 API function GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDEFAULTANSICODEPAGE, ...))
    // can be called from any thread
    virtual BYTE WINAPI GetUserDefaultCharset() = 0;

    // allocates a new object for the Boyer-Moore search algorithm
    // can be called from any thread
    virtual CSalamanderBMSearchData* WINAPI AllocSalamanderBMSearchData() = 0;

    // frees the Boyer-Moore search algorithm object (obtained by calling AllocSalamanderBMSearchData)
    // can be called from any thread
    virtual void WINAPI FreeSalamanderBMSearchData(CSalamanderBMSearchData* data) = 0;

    // allocates a new regular-expression search algorithm object
    // can be called from any thread
    virtual CSalamanderREGEXPSearchData* WINAPI AllocSalamanderREGEXPSearchData() = 0;

    // frees the regular-expression search algorithm object (obtained by calling
    // AllocSalamanderREGEXPSearchData)
    // can be called from any thread
    virtual void WINAPI FreeSalamanderREGEXPSearchData(CSalamanderREGEXPSearchData* data) = 0;

    // vraci postupne prikazy Salamandera (postupuje v poradi definice konstant SALCMD_XXX);
    // 'index' je vstupne/vystupni promenna, ukazuje na int, ve kterem je pri prvnim volani 0,
    // hodnotu pro dalsi volani si funkce ulozi pri navratu (pouziti: na zacatku vynulovat, pak
    // nemenit); vraci FALSE, pokud jiz neexistuje zadny dalsi prikaz; pokud vraci TRUE, obsahuje
    // 'salCmd' (neni-li NULL) cislo prikazu Salamandera (viz konstanty SALCMD_XXX; cisla maji
    // rezervovany interval 0 az 499, pokud tedy maji v menu byt prikazy Salamandera spolecne s
    // jinymi prikazy, neni problem vytvorit vzajemne se neprekryvajici mnoziny hodnot prikazu
    // napr. posunutim vsech hodnot o zvolene cislo - priklad viz DEMOPLUGin -
    // CPluginFSInterface::ContextMenu), 'nameBuf' (buffer o velikosti 'nameBufSize' bytu)
    // obsahuje jmeno prikazu (jmeno je pripraveno pro pouziti v menu - ma zdvojene ampersandy,
    // podtrzene znaky oznacene ampersandy a za '\t' ma popisy klavesovych zkratek), 'enabled'
    // (neni-li NULL) obsahuje stav prikazu (TRUE/FALSE pokud je enabled/disabled), 'type'
    // (neni-li NULL) obsahuje typ prikazu (viz popis konstant sctyXXX)
    // mozne volat z libovolneho threadu
    virtual BOOL WINAPI EnumSalamanderCommands(int* index, int* salCmd, char* nameBuf, int nameBufSize,
                                               BOOL* enabled, int* type) = 0;

    // vraci prikaz Salamandera s cislem 'salCmd' (viz konstanty SALCMD_XXX);
    // vraci FALSE, pokud takovy prikaz neexistuje; pokud vraci TRUE, obsahuje
    // 'nameBuf' (buffer o velikosti 'nameBufSize' bytu) jmeno prikazu (jmeno je pripraveno pro
    // pouziti v menu - ma zdvojene ampersandy, podtrzene znaky oznacene ampersandy a za '\t' ma
    // popisy klavesovych zkratek), 'enabled' (neni-li NULL) obsahuje stav prikazu (TRUE/FALSE
    // pokud je enabled/disabled), 'type' (neni-li NULL) obsahuje typ prikazu (viz popis konstant
    // sctyXXX)
    // mozne volat z libovolneho threadu
    virtual BOOL WINAPI GetSalamanderCommand(int salCmd, char* nameBuf, int nameBufSize, BOOL* enabled,
                                             int* type) = 0;

    // sets a flag for the calling plugin that Salamander command number 'salCmd' should be run
    // at the next possible opportunity (as soon as there are no messages in the main thread's message queue
    // and Salamander is not "busy" (no modal dialog is open and no message
    // is being processed));
    // WARNING: if called from a thread other than the main thread, the Salamander command may run
    // (it runs in the main thread) even before PostSalamanderCommand returns
    // can be called from any thread
    virtual void WINAPI PostSalamanderCommand(int salCmd) = 0;

    // sets the "user worked with the current path" flag in panel 'panel' (this flag
    // is used when populating the List Of Working Directories list (Alt+F12));
    // 'panel' is one of PANEL_XXX
    // restriction: main thread
    virtual void WINAPI SetUserWorkedOnPanelPath(int panel) = 0;

    // in panel 'panel' (one of the PANEL_XXX constants), stores the selected names
    // into a special array from which the user can restore the selection using the Edit/Restore Selection command
    // this is used for commands that cancel the current selection so that the user has the option
    // to return to it and perform another operation
    // limitation: main thread
    virtual void WINAPI StoreSelectionOnPanelPath(int panel) = 0;

    //
    // UpdateCrc32
    //   Updates CRC-32 (32-bit Cyclic Redundancy Check) with specified array of bytes.
    //
    // Parameters
    //   'buffer'
    //      [in] Pointer to the starting address of the block of memory to update 'crcVal' with.
    //
    //   'count'
    //      [in] Size, in bytes, of the block of memory to update 'crcVal' with.
    //
    //   'crcVal'
    //      [in] Initial crc value. Set this value to zero to calculate CRC-32 of the 'buffer'.
    //
    // Return Values
    //   Returns updated CRC-32 value.
    //
    // Remarks
    //   Method can be called from any thread.
    //
    virtual DWORD WINAPI UpdateCrc32(const void* buffer, DWORD count, DWORD crcVal) = 0;

    // allocates a new object for MD5 calculation
    // can be called from any thread
    virtual CSalamanderMD5* WINAPI AllocSalamanderMD5() = 0;

    // releases the object used to compute MD5 (obtained using AllocSalamanderMD5)
    // can be called from any thread
    virtual void WINAPI FreeSalamanderMD5(CSalamanderMD5* md5) = 0;

    // Finds '<' '>' pairs in the text, removes them from the buffer, and records
    // their positions in 'varPlacements'. 'varPlacements' is an array of DWORDs with '*varPlacementsCount'
    // items; each DWORD always consists of the position of the variable in the output buffer (low WORD)
    // and the number of characters in the variable (high WORD). The strings "\\<", "\\>", "\\\\" are treated
    // as escape sequences and are replaced with the characters '<', '>' and '\\'.
    // Returns TRUE on success, otherwise FALSE; always sets 'varPlacementsCount' to
    // the number of processed variables.
    // can be called from any thread
    virtual BOOL WINAPI LookForSubTexts(char* text, DWORD* varPlacements, int* varPlacementsCount) = 0;

    // waits (for at most 0.2 seconds) for the ESC key to be released; used if the plugin contains
    // actions that are interrupted by the ESC key (monitoring the ESC key via
    // GetAsyncKeyState(VK_ESCAPE)) - prevents the next action monitoring the ESC key
    // from being interrupted immediately after ESC is pressed in a dialog/message box
    // can be called from any thread
    virtual void WINAPI WaitForESCRelease() = 0;

    //
    // GetMouseWheelScrollLines
    //   An OS independent method to retrieve the number of wheel scroll lines.
    //
    // Return Values
    //   Number of scroll lines where WHEEL_PAGESCROLL (0xffffffff) indicates to scroll a page at a time.
    //
    // Remarks
    //   Method can be called from any thread.
    //
    virtual DWORD WINAPI GetMouseWheelScrollLines() = 0;

    //
    // GetTopVisibleParent
    //   Retrieves the visible root window by walking the chain of parent windows
    //   returned by GetParent.
    //
    // Parameters
    //   'hParent'
    //      [in] Handle to the window whose parent window handle is to be retrieved.
    //
    // Return Values
    //   The return value is the handle to the top Popup or Overlapped visible parent window.
    //
    // Remarks
    //   Method can be called from any thread.
    //
    virtual HWND WINAPI GetTopVisibleParent(HWND hParent) = 0;

    //
    // MultiMonGetDefaultWindowPos
    //   Retrieves the default position of the upper-left corner for a newly created window
    //   on the display monitor that has the largest area of intersection with the bounding
    //   rectangle of a specified window.
    //
    // Parameters
    //   'hByWnd'
    //      [in] Handle to the window of interest.
    //
    //   'p'
    //      [out] Pointer to a POINT structure that receives the virtual-screen coordinates
    //      of the upper-left corner for the window that would be created with CreateWindow
    //      with CW_USEDEFAULT in the 'x' parameter. Note that if the monitor is not the
    //      primary display monitor, some of the point's coordinates may be negative values.
    //
    // Return Values
    //   If the default window position lies on the primary monitor or some error occurred,
    //   the return value is FALSE and you should use CreateWindow with CW_USEDEFAULT in
    //   the 'x' parameter.
    //
    //   Otherwise the return value is TRUE and coordinates from the 'p' structure should be used
    //   in the CreateWindow 'x' and 'y' parameters.
    //
    // Remarks
    //   This method can be called from any thread.
    //
    virtual BOOL WINAPI MultiMonGetDefaultWindowPos(HWND hByWnd, POINT* p) = 0;

    //
    // MultiMonGetClipRectByRect
    //   Retrieves the bounding rectangle of the display monitor that has the largest
    //   area of intersection with a specified rectangle.
    //
    // Parameters
    //   'rect'
    //      [in] Pointer to a RECT structure that specifies the rectangle of interest
    //      in virtual-screen coordinates.
    //
    //   'workClipRect'
    //      [out] A RECT structure that specifies the work area rectangle of the
    //      display monitor, expressed in virtual-screen coordinates. Note that
    //      if the monitor is not the primary display monitor, some of the rectangle's
    //      coordinates may be negative values.
    //
    //   'monitorClipRect'
    //      [out] A RECT structure that specifies the display monitor rectangle,
    //      expressed in virtual-screen coordinates. Note that if the monitor is
    //      not the primary display monitor, some of the rectangle's coordinates
    //      may be negative values. This parameter can be NULL.
    //
    // Remarks
    //   Method can be called from any thread.
    //
    virtual void WINAPI MultiMonGetClipRectByRect(const RECT* rect, RECT* workClipRect, RECT* monitorClipRect) = 0;

    //
    // MultiMonGetClipRectByWindow
    //   Retrieves the bounding rectangle of the display monitor that has the largest
    //   area of intersection with the bounding rectangle of a specified window.
    //
    // Parameters
    //   'hByWnd'
    //      [in] Handle to the window of interest. If this parameter is NULL,
    //      or the window is not visible or is iconic, the monitor with the currently active window
    //      from the same application will be used. Otherwise, the primary monitor will be used.
    //
    //   'workClipRect'
    //      [out] A RECT structure that specifies the work area rectangle of the
    //      display monitor, expressed in virtual-screen coordinates. Note that
    //      if the monitor is not the primary display monitor, some of the rectangle's
    //      coordinates may be negative values.
    //
    //   'monitorClipRect'
    //      [out] A RECT structure that specifies the display monitor rectangle,
    //      expressed in virtual-screen coordinates. Note that if the monitor is
    //      not the primary display monitor, some of the rectangle's coordinates
    //      may be negative values. This parameter can be NULL.
    //
    // Remarks
    //   Method can be called from any thread.
    //
    virtual void WINAPI MultiMonGetClipRectByWindow(HWND hByWnd, RECT* workClipRect, RECT* monitorClipRect) = 0;

    //
    // MultiMonCenterWindow
    //   Centers a window relative to a specified window or monitor.
    //
    // Parameters
    //   'hWindow'
    //      [in] Handle to the window to center.
    //
    //   'hByWnd'
    //      [in] Handle to the window relative to which to center. If this parameter is NULL,
    //      or the window is not visible or is iconic, the method will center 'hWindow' within
    //      the working area of the monitor. The monitor with the currently active window
    //      from the same application will be used. Otherwise, the primary monitor will be used.
    //
    //   'findTopWindow'
    //      [in] If this parameter is TRUE, a visible non-child window found by walking
    //      the chain of parent windows of 'hByWnd' will be used as the window relative to
    //      which to center.
    //
    //      If this parameter is FALSE, 'hByWnd' will be the window relative to which to center.
    //
    // Remarks
    //   If the centered window extends beyond the working area of the monitor, the method
    //   positions the window so that it remains fully visible.
    //
    //   This method can be called from any thread.
    //
    virtual void WINAPI MultiMonCenterWindow(HWND hWindow, HWND hByWnd, BOOL findTopWindow) = 0;

    //
    // MultiMonEnsureRectVisible
    //   Ensures that the specified rectangle is either entirely or partially visible,
    //   adjusting the coordinates if necessary. All monitors are considered.
    //
    // Parameters
    //   'rect'
    //      [in/out] Pointer to the RECT structure that contains the coordinates to be
    //      adjusted. The rectangle is presumed to be in virtual-screen coordinates.
    //
    //   'partialOK'
    //      [in] Value specifying whether the rectangle must be entirely visible.
    //      If this parameter is TRUE, no moving occurs if the item is at least
    //      partially visible.
    //
    // Return Values
    //   If the rectangle is adjusted, the return value is TRUE.
    //
    //   If the rectangle is not adjusted, the return value is FALSE.
    //
    // Remarks
    //   Method can be called from any thread.
    //
    virtual BOOL WINAPI MultiMonEnsureRectVisible(RECT* rect, BOOL partialOK) = 0;

    //
    // InstallWordBreakProc
    //   Installs a special word-break procedure for the specified window. This procedure
    //   is intended to make cursor movement easier in single-line edit controls.
    //   Delimiters '\\', '/', ' ', ';', ',', and '.' are used as cursor stops when the user
    //   navigates with the Ctrl+Left or Ctrl+Right keys.
    //   You can use Ctrl+Backspace to delete one word.
    //
    // Parameters
    //   'hWindow'
    //      [in] Handle to the window or control where the word-break proc is to be installed.
    //      The window may be either an edit control or a combo box with an edit control.
    //
    // Return Values
    //   The return value is TRUE if the word-break proc is installed. It is FALSE if the
    //   window is neither an edit control nor a combo box with an edit control, if some
    //   error occurred, or if this special word-break proc is not supported on your OS.
    //
    // Remarks
    //   You do not need to uninstall the word-break procedure before the window is destroyed.
    //
    //   This method can be called from any thread.
    //
    virtual BOOL WINAPI InstallWordBreakProc(HWND hWindow) = 0;

    // Salamander 3 or later: returns TRUE if this Altap
    // Salamander instance was the first one started (when an instance starts, other running
    // instances of version 3 or later are searched for);
    //
    // Notes on different SID / Session / Integrity Level (does not apply to Salamander 2.5 and 2.51):
    // the function also returns TRUE if a Salamander instance is already running
    // under a different SID; session and integrity level do not matter, so if a
    // Salamander instance is already running in another session, or with a different
    // integrity level, but with the same SID, the newly started instance returns FALSE
    //
    // can be called from any thread
    virtual BOOL WINAPI IsFirstInstance3OrLater() = 0;

    // support for parameter-dependent strings (handling singular/plural forms);
    // 'format' is the format string for the resulting string - its description follows;
    // the resulting string is copied to buffer 'buffer', whose size is 'bufferSize' bytes;
    // 'parametersArray' is the array of parameters; 'parametersCount' is the number of
    // these parameters; returns the length of the resulting string
    //
    // format string description:
    //   - each format string starts with the signature "{!}"
    //   - the format string can contain the following escape sequences (allowing a special
    //     character to be used without its special meaning): "\\" = "\", "\{" = "{", "\}" = "}",
    //     "\:" = ":", and "\|" = "|" (do not forget to double backslashes when writing C++
    //     strings; this applies only to format strings placed directly in C++ source code)
    //   - text not enclosed in curly brackets is copied directly to the resulting string
    //     (only escape sequences are handled)
    //   - parameter-dependent text is enclosed in curly brackets
    //   - each parameter-dependent text uses one parameter from 'parametersArray'
    //     (it is a 64-bit unsigned int)
    //   - parameter-dependent text contains multiple variants of the resulting text; the variant
    //     used depends on the parameter value, more precisely on which defined interval the
    //     value belongs to
    //   - variants of the resulting text and interval bounds are separated by the "|" character
    //   - the first interval is from 0 to the first interval bound
    //   - the last interval is from the last interval bound plus one to infinity (2^64-1)
    //   - parameter-dependent text "{}" is used to skip one parameter from 'parametersArray'
    //     (nothing is added to the resulting string)
    //   - you can also specify the index of the parameter to use for parameter-dependent text;
    //     just place its index (from one to the number of parameters) at the beginning of
    //     the parameter-dependent text and follow it with the ':' character
    //   - if you do not specify the index of the parameter to use, it is assigned automatically
    //     (starting from one up to the number of parameters)
    //   - if you specify the index of the parameter to use, the next index assigned
    //     automatically is not affected,
    //     e.g. in "{!}%d file{2:s|0||1|s} and %d director{y|1|ies}" the first parameter-
    //     dependent text uses the parameter with index 2 and the second uses the parameter with index 1
    //   - you can use any number of parameter-dependent texts with a specified index
    //     of the parameter to use
    //
    // examples of format strings:
    //   - "{!}director{y|1|ies}": for parameter values from 0 to 1 the resulting string will be
    //     "directory", and for parameter values from 2 to infinity (2^64-1) the resulting string
    //     will be "directories"
    //   - "{!}%d soubor{u|0||1|y|4|u} a %d adresar{u|0||1|e|4|u}": it needs two parameters
    //     because there are two dependent texts in curly brackets; the resulting string for
    //     selected pairs of parameters is (it is probably not necessary to show all possible variants):
    //       0, 0: "%d souboru a %d adresaru"
    //       1, 12: "%d soubor a %d adresaru"
    //       3, 4: "%d soubory a %d adresare"
    //       13, 1: "%d souboru a %d adresar"
    //
    // method can be called from any thread
    virtual int WINAPI ExpandPluralString(char* buffer, int bufferSize, const char* format,
                                          int parametersCount, const CQuadWord* parametersArray) = 0;

    // in the current Salamander language version, prepares the string "XXX (selected/hidden)
    // files and YYY (selected/hidden) directories"; if XXX (the value of parameter 'files')
    // or YYY (the value of parameter 'dirs') is zero, the corresponding part of the string is omitted (both
    // parameters being zero at the same time is not considered); use of "selected" and "hidden" depends
    // on the 'mode' setting - see the description of constants epfdmXXX; the resulting text
    // is returned in buffer 'buffer' with size 'bufferSize' bytes; returns the length of the resulting
    // text; 'forDlgCaption' is TRUE/FALSE if the text is/is not intended for a dialog caption
    // (in English, initial capital letters are required)
    // can be called from any thread
    virtual int WINAPI ExpandPluralFilesDirs(char* buffer, int bufferSize, int files, int dirs,
                                             int mode, BOOL forDlgCaption) = 0;

    // in the current Salamander language version, prepares the string "BBB bytes in XXX selected
    // files and YYY selected directories"; BBB is the value of the 'selectedBytes' parameter;
    // if XXX (the value of the 'files' parameter) or YYY (the value of the 'dirs' parameter) is zero,
    // the corresponding part of the string is omitted (both parameters being zero at the same time is not considered);
    // if 'useSubTexts' is TRUE, BBB is enclosed in '<' and '>' so that BBB can be further processed on the info-line (see methods CSalamanderGeneralAbstract::LookForSubTexts and
    // CPluginDataInterfaceAbstract::GetInfoLineContent); the resulting text is returned in the
    // 'buffer' buffer with size 'bufferSize' bytes; returns the length of the resulting text
    // can be called from any thread
    virtual int WINAPI ExpandPluralBytesFilesDirs(char* buffer, int bufferSize,
                                                  const CQuadWord& selectedBytes, int files, int dirs,
                                                  BOOL useSubTexts) = 0;

    // returns a string describing what is being worked with (e.g. "file \"test.txt\"" or "directory \"test\""
    // or "3 files and 1 directory"); 'sourceDescr' is the output buffer with size
    // at least 'sourceDescrSize'; 'panel' describes the source panel of the operation (one of PANEL_XXX or -1
    // if the operation has no source panel (e.g. CPluginFSInterfaceAbstract::CopyOrMoveFromDiskToFS));
    // 'selectedFiles'+'selectedDirs' - if the operation has a source panel, this is the number of selected
    // files and directories in the source panel; if both values are zero, the operation works with the
    // file/directory under the cursor (focus); 'selectedFiles'+'selectedDirs' - if the operation has no
    // source panel, this is the number of files/directories the operation works with;
    // 'fileOrDirName'+'isDir' - used only if the operation has no source panel and if
    // 'selectedFiles + selectedDirs == 1'; this contains the file/directory name and whether it is a file
    // or directory ('isDir' is FALSE or TRUE); 'forDlgCaption' is TRUE/FALSE if the text is/is not
    // intended for a dialog caption (in English, initial capitals are required)
    // limitation: main thread (it may work with the panel)
    virtual void WINAPI GetCommonFSOperSourceDescr(char* sourceDescr, int sourceDescrSize,
                                                   int panel, int selectedFiles, int selectedDirs,
                                                   const char* fileOrDirName, BOOL isDir,
                                                   BOOL forDlgCaption) = 0;

    // copies the string 'srcStr' after the string 'dstStr' (after its terminating null);
    // 'dstStr' is a buffer of size 'dstBufSize' (must be at least 2);
    // if both strings do not fit into the buffer, they are truncated (always so that
    // as many characters as possible from both strings fit)
    // can be called from any thread
    virtual void WINAPI AddStrToStr(char* dstStr, int dstBufSize, const char* srcStr) = 0;

    // checks whether the string 'fileNameComponent' can be used as a component
    // of a name on a Windows filesystem (handles strings longer than MAX_PATH-4 (4 = "C:\"
    // + null terminator), an empty string, strings of '.' characters, whitespace-only strings,
    // the characters "*?\\/<>|\":" and simple names such as "prn" and "prn  .txt")
    // can be called from any thread
    virtual BOOL WINAPI SalIsValidFileNameComponent(const char* fileNameComponent) = 0;

    // transforms the string 'fileNameComponent' so that it can be used as a component
    // of a name on the Windows filesystem (handles strings longer than MAX_PATH-4 (4 = "C:\"
    // + null-terminator), handles an empty string, strings of '.' characters, strings of
    // white spaces, replaces the characters "*?\\/<>|\":" with '_' + appends '_' to simple names such as "prn"
    // and "prn  .txt" at the end of the name); 'fileNameComponent' must be possible
    // to extend by at least one character (however, at most MAX_PATH bytes from 'fileNameComponent'
    // are used)
    // can be called from any thread
    virtual void WINAPI SalMakeValidFileNameComponent(char* fileNameComponent) = 0;

    // returns TRUE if the enumeration source is a panel; in 'panel' it then returns PANEL_LEFT or
    // PANEL_RIGHT; if the enumeration source was not found or it is a Find window, returns FALSE;
    // 'srcUID' is the unique identifier of the source (it is passed as a parameter when opening the
    // viewer or can be obtained by calling GetPanelEnumFilesParams)
    // can be called from any thread
    virtual BOOL WINAPI IsFileEnumSourcePanel(int srcUID, int* panel) = 0;

    // returns the next file name for the viewer from the source (left/right panel or Find results);
    // 'srcUID' is the unique identifier of the source (it is passed as a parameter when opening
    // the viewer or can be obtained by calling GetPanelEnumFilesParams); 'lastFileIndex'
    // (must not be NULL) is an IN/OUT parameter that the plugin should modify only if it wants to return
    // the name of the first file, in that case set 'lastFileIndex' to -1; the initial
    // value of 'lastFileIndex' is passed as a parameter both when opening the viewer and
    // when calling GetPanelEnumFilesParams; 'lastFileName' is the full name of the current file
    // (an empty string if it is not known, e.g. if 'lastFileIndex' is -1); if
    // 'preferSelected' is TRUE and at least one name is selected, the selected names will be returned;
    // if 'onlyAssociatedExtensions' is TRUE, it returns only files with an extension associated with
    // this plugin's viewer (pressing F3 on this file would try to open this plugin's viewer
    // + ignores any possible shadowing by another plugin's viewer); 'fileName' is the buffer
    // for the obtained name (size at least MAX_PATH); returns TRUE if the name is
    // obtained successfully; returns FALSE on error: there is no next file name in the source
    // (if 'noMoreFiles' is not NULL, TRUE is returned in it), the source is busy (not processing messages;
    // if 'srcBusy' is not NULL, TRUE is returned in it), otherwise the source no longer exists (panel
    // path changed, etc.);
    // can be called from any thread; WARNING: using it from the main thread makes no sense
    // (Salamander is busy while calling the plugin method, so it always returns FALSE + TRUE
    // in 'srcBusy')
    virtual BOOL WINAPI GetNextFileNameForViewer(int srcUID, int* lastFileIndex, const char* lastFileName,
                                                 BOOL preferSelected, BOOL onlyAssociatedExtensions,
                                                 char* fileName, BOOL* noMoreFiles, BOOL* srcBusy) = 0;

    // returns the previous file name for the viewer from the source (left/right panel or Find);
    // 'srcUID' is the unique identifier of the source (it is passed as a parameter when opening
    // the viewer, or it can be obtained by calling GetPanelEnumFilesParams); 'lastFileIndex' (must
    // not be NULL) is an IN/OUT parameter that the plugin should change only if it wants to return
    // the name of the last file, in which case set 'lastFileIndex' to -1; the initial value of
    // 'lastFileIndex' is passed as a parameter both when opening the viewer and when calling
    // GetPanelEnumFilesParams; 'lastFileName' is the full name of the current file (an empty
    // string if it is unknown, e.g. if 'lastFileIndex' is -1); if 'preferSelected' is TRUE and at
    // least one name is selected, selected names are returned; if 'onlyAssociatedExtensions' is
    // TRUE, returns only files with an extension associated with this plugin's viewer (pressing F3
    // on this file would attempt to open this plugin's viewer + ignores possible overriding by
    // another plugin's viewer); 'fileName' is a buffer for the obtained name (size at least
    // MAX_PATH); returns TRUE if the name is obtained; returns FALSE on error: there is no previous
    // file name in the source (if 'noMoreFiles' is not NULL, TRUE is returned in it), the source
    // is busy (not processing messages; if 'srcBusy' is not NULL, TRUE is returned in it),
    // otherwise the source no longer exists (path change in the panel, etc.)
    // can be called from any thread; WARNING: using it from the main thread makes no sense
    // (Salamander is busy while calling the plugin method, so it always returns FALSE + TRUE
    // in 'srcBusy')
    virtual BOOL WINAPI GetPreviousFileNameForViewer(int srcUID, int* lastFileIndex, const char* lastFileName,
                                                     BOOL preferSelected, BOOL onlyAssociatedExtensions,
                                                     char* fileName, BOOL* noMoreFiles, BOOL* srcBusy) = 0;

    // determines whether the current file in the viewer is selected in the source (left/right
    // panel or Find); 'srcUID' is the unique source identifier (it is passed as a parameter
    // when opening the viewer or can be obtained by calling GetPanelEnumFilesParams); 'lastFileIndex'
    // is a parameter that the plugin should not modify, the initial value of 'lastFileIndex' is passed
    // as a parameter both when opening the viewer and when calling GetPanelEnumFilesParams;
    // 'lastFileName' is the full name of the current file; returns TRUE if it was possible to determine
    // whether the current file is selected, the result is returned in 'isFileSelected' (must not be NULL);
    // returns FALSE on error: the source no longer exists (path changed in the panel, etc.) or the file
    // 'lastFileName' is no longer in the source (for these two errors, if 'srcBusy' is not NULL,
    // FALSE is returned in it), or the source is busy (not processing messages; for this error,
    // if 'srcBusy' is not NULL, TRUE is returned in it)
    // can be called from any thread; WARNING: using it from the main thread makes no sense
    // (Salamander is busy while calling the plugin method, so it always returns FALSE + TRUE
    // in 'srcBusy')
    virtual BOOL WINAPI IsFileNameForViewerSelected(int srcUID, int lastFileIndex,
                                                    const char* lastFileName,
                                                    BOOL* isFileSelected, BOOL* srcBusy) = 0;

    // sets the selection on the current file from the viewer in the source (left/right
    // panel or Find); 'srcUID' is a unique source identifier (it is passed as a parameter
    // when opening the viewer or can be obtained by calling GetPanelEnumFilesParams);
    // 'lastFileIndex' is a parameter that the plugin should not modify; the initial value
    // of 'lastFileIndex' is passed as a parameter both when opening the viewer and when
    // calling GetPanelEnumFilesParams; 'lastFileName' is the full name of the current file;
    // 'select' is TRUE/FALSE depending on whether the current file should be
    // selected/deselected; returns TRUE on success;
    // returns FALSE on error: the source no longer exists (path changed in the panel, etc.)
    // or the file 'lastFileName' is no longer in the source (for these two errors, if
    // 'srcBusy' is not NULL, FALSE is returned in it), the source is busy (not processing
    // messages; for this error, if 'srcBusy' is not NULL, TRUE is returned in it)
    // can be called from any thread; WARNING: using it from the main thread does not make
    // sense (Salamander is busy while calling the plugin method, so it always returns FALSE
    // + TRUE in 'srcBusy')
    virtual BOOL WINAPI SetSelectionOnFileNameForViewer(int srcUID, int lastFileIndex,
                                                        const char* lastFileName, BOOL select,
                                                        BOOL* srcBusy) = 0;

    // returns a pointer to the shared history (last used values) of the selected combo box;
    // it is an array of allocated strings; the array has a fixed number of strings, which is returned
    // in 'historyItemsCount' (must not be NULL); a pointer to the array is returned in 'historyArr'
    // (must not be NULL); 'historyID' (one of SALHIST_XXX) specifies which shared history a
    // pointer should be returned to
    // restriction: main thread (shared histories cannot be used from another thread, access
    // to them is not synchronized in any way)
    virtual BOOL WINAPI GetStdHistoryValues(int historyID, char*** historyArr, int* historyItemsCount) = 0;

    // adds an allocated copy of the new value 'value' to the shared history ('historyArr'+'historyItemsCount'); if
    // 'caseSensitiveValue' is TRUE, the value (string) is searched for in the history array using a
    // case-sensitive comparison (FALSE = case-insensitive comparison); a found value is only moved
    // to the first position in the history array
    // limitation: main thread (shared histories cannot be used in another thread; access
    // to them is not synchronized in any way)
    // NOTE: if used for something other than shared histories, it can be called from any thread
    virtual void WINAPI AddValueToStdHistoryValues(char** historyArr, int historyItemsCount,
                                                   const char* value, BOOL caseSensitiveValue) = 0;

    // adds texts from the shared history ('historyArr'+'historyItemsCount') to the combo box ('combo');
    // resets the combo box contents before adding (see CB_RESETCONTENT)
    // limitation: main thread only (shared histories cannot be used from another thread; access
    // to them is not synchronized in any way)
    // NOTE: if used for something other than shared histories, it can be called from any thread
    virtual void WINAPI LoadComboFromStdHistoryValues(HWND combo, char** historyArr, int historyItemsCount) = 0;

    // determines the color depth of the current display and returns TRUE if it is more than 8-bit (256 colors)
    // can be called from any thread
    virtual BOOL WINAPI CanUse256ColorsBitmap() = 0;

    // checks whether the enabled root parent of window 'parent' is the foreground window; if not,
    // FlashWindow(root parent of window 'parent', TRUE) is called and the root parent of window 'parent' is returned,
    // otherwise NULL is returned
    // USAGE:
    //    HWND mainWnd = GetWndToFlash(parent);
    //    CDlg(parent).Execute();
    //    if (mainWnd != NULL) FlashWindow(mainWnd, FALSE);  // under W2K+ this is probably no longer needed: flashing must be removed manually
    // can be called from any thread
    virtual HWND WINAPI GetWndToFlash(HWND parent) = 0;

    // reactivates the drop target (after a drop during drag&drop) after opening our progress-
    // -window (it becomes active when opened, which deactivates the drop target); if 'dropTarget'
    // is not NULL and is not a panel in this Salamander, it activates 'progressWnd' and then
    // activates the topmost enabled ancestor of 'dropTarget' (this combination gets rid of the
    // active state without an active application, which otherwise occasionally occurs)
    // can be called from any thread
    virtual void WINAPI ActivateDropTarget(HWND dropTarget, HWND progressWnd) = 0;

    // schedules opening the Pack dialog with the selected packer from this plugin (see
    // CSalamanderConnectAbstract::AddCustomPacker); if a packer from this plugin
    // does not exist (for example because the user deleted it), an error
    // message is shown to the user; the dialog opens as soon as there are no
    // messages in the main thread's message queue and Salamander is not "busy" (no modal dialog
    // is open and no message is being processed); repeated calls to this method before
    // the Pack dialog is opened only change the value of the 'delFilesAfterPacking' parameter;
    // 'delFilesAfterPacking' controls the "Delete files after packing" checkbox
    // in the Pack dialog: 0=default, 1=checked, 2=unchecked
    // limitation: main thread only
    virtual void WINAPI PostOpenPackDlgForThisPlugin(int delFilesAfterPacking) = 0;

    // schedules opening of the Unpack dialog with the selected unpacker from this plugin (see
    // CSalamanderConnectAbstract::AddCustomUnpacker); if the unpacker from this plugin does not
    // exist (e.g. because the user deleted it), an error message is shown to the user; the dialog
    // opens as soon as there are no messages in the main thread's message queue and Salamander is not "busy"
    // (no modal dialog is open and no message is being processed); repeated calls to this method before
    // the Unpack dialog is opened only change the 'unpackMask' parameter;
    // 'unpackMask' affects the "Unpack files" mask: NULL=default, otherwise the mask text
    // limitation: main thread
    virtual void WINAPI PostOpenUnpackDlgForThisPlugin(const char* unpackMask) = 0;

    // vytvoreni souboru se jmenem 'fileName' pres klasicke volani Win32 API
    // CreateFile (lpSecurityAttributes==NULL, dwCreationDisposition==CREATE_NEW,
    // hTemplateFile==NULL); tato metoda resi kolizi 'fileName' s dosovym nazvem
    // jiz existujiciho souboru/adresare (jen pokud nejde i o kolizi s dlouhym
    // jmenem souboru/adresare) - zajisti zmenu dosoveho jmena tak, aby se soubor se
    // jmenem 'fileName' mohl vytvorit (zpusob: docasne prejmenuje konfliktni
    // soubor/adresar na jine jmeno a po vytvoreni 'fileName' ho prejmenuje zpet);
    // vraci handle souboru nebo pri chybe INVALID_HANDLE_VALUE (vraci v 'err'
    // (neni-li NULL) kod Windows chyby)
    // mozne volat z libovolneho threadu
    virtual HANDLE WINAPI SalCreateFileEx(const char* fileName, DWORD desiredAccess, DWORD shareMode,
                                          DWORD flagsAndAttributes, DWORD* err) = 0;

    // vytvoreni adresare se jmenem 'name' pres klasicke volani Win32 API
    // CreateDirectory(lpSecurityAttributes==NULL); tato metoda resi kolizi 'name'
    // s dosovym nazvem jiz existujiciho souboru/adresare (jen pokud nejde i o
    // kolizi s dlouhym jmenem souboru/adresare) - zajisti zmenu dosoveho jmena
    // tak, aby se adresar se jmenem 'name' mohl vytvorit (zpusob: docasne prejmenuje
    // konfliktni soubor/adresar na jine jmeno a po vytvoreni 'name' ho
    // prejmenuje zpet); dale resi jmena koncici na mezery (umi je vytvorit, narozdil
    // od CreateDirectory, ktera mezery bez varovani orizne a vytvori tak vlastne
    // jiny adresar); vraci TRUE pri uspechu, pri chybe FALSE (vraci v 'err'
    // (neni-li NULL) kod Windows chyby)
    // mozne volat z libovolneho threadu
    virtual BOOL WINAPI SalCreateDirectoryEx(const char* name, DWORD* err) = 0;

    // allows disconnecting/reconnecting change monitoring (only for Windows paths and archive paths)
    // on paths viewed in one of the panels; purpose: if your code (disk formatting,
    // disk shredding, etc.) is hindered by the panel having an open
    // "ChangeNotification" handle for the path, you can temporarily disconnect it with
    // this method (after reconnecting, a refresh is triggered for the path in the panel);
    // 'panel' is one of PANEL_XXX; 'stopMonitoring' is TRUE/FALSE (disconnect/reconnect)
    // limitation: main thread
    virtual void WINAPI PanelStopMonitoring(int panel, BOOL stopMonitoring) = 0;

    // allocates a new CSalamanderDirectory object for working with files/directories in an archive or
    // the file system; if 'isForFS' is TRUE, the object is preset for use with the file system,
    // otherwise the object is preset for use with an archive (the object's default flags
    // differ for an archive and the file system, see method CSalamanderDirectoryAbstract::SetFlags)
    // can be called from any thread
    virtual CSalamanderDirectoryAbstract* WINAPI AllocSalamanderDirectory(BOOL isForFS) = 0;

    // frees the CSalamanderDirectory object (obtained by calling AllocSalamanderDirectory,
    // WARNING: must not be called for any other CSalamanderDirectoryAbstract pointer)
    // can be called from any thread
    virtual void WINAPI FreeSalamanderDirectory(CSalamanderDirectoryAbstract* salDir) = 0;

    // adds a new timer for a plugin FS object; when the timer expires, the method
    // CPluginFSInterfaceAbstract::Event() is called on the plugin FS object 'timerOwner' with parameters
    // FSE_TIMER and 'timerParam'; 'timeout' is the timer timeout from the moment it is added (in milliseconds,
    // must be >= 0); the timer is canceled when it expires (before calling
    // CPluginFSInterfaceAbstract::Event()) or when the plugin FS object is closed;
    // returns TRUE if the timer was added successfully
    // limitation: main thread
    virtual BOOL WINAPI AddPluginFSTimer(int timeout, CPluginFSInterfaceAbstract* timerOwner,
                                         DWORD timerParam) = 0;

    // cancels either all timers of the plugin FS object 'timerOwner' (if 'allTimers' is TRUE)
    // or only all timers whose parameter equals 'timerParam' (if 'allTimers' is FALSE);
    // returns the number of cancelled timers
    // limitation: main thread
    virtual int WINAPI KillPluginFSTimer(CPluginFSInterfaceAbstract* timerOwner, BOOL allTimers,
                                         DWORD timerParam) = 0;

    // zjistuje viditelnost polozky pro FS v Change Drive menu a v Drive barach; vraci TRUE,
    // pokud je polozka viditelna, jinak vraci FALSE
    // omezeni: hlavni thread (jinak muze dojit ke zmenam v konfiguraci pluginu behem volani)
    virtual BOOL WINAPI GetChangeDriveMenuItemVisibility() = 0;

    // sets the visibility of the FS item in the Change Drive menu and in the drive bars; use
    // only when installing the plugin (otherwise the user-selected visibility may be overwritten);
    // 'visible' is TRUE if the item should be visible
    // limitation: main thread (otherwise the plugin configuration may change during the call)
    virtual void WINAPI SetChangeDriveMenuItemVisibility(BOOL visible) = 0;

    // Nastavuje breakpoint na x-tou COM/OLE alokaci. Slouzi k dohledani COM/OLE leaku.
    // V release verzi Salamandera nedela nic. Debug verze Salamandera pri svem ukonceni
    // zobrazuje do Debug okna debuggeru a do Trace Serveru seznam COM/OLE leaku.
    // V hranatych zavorkach je poradi alokace, kterou predame jako 'alloc' do volani
    // OleSpySetBreak. Lze zavolat z libovolneho threadu.
    virtual void WINAPI OleSpySetBreak(int alloc) = 0;

    // Vraci kopie ikon, ktere Salamander pouziva v panelech. 'icon' urcuje ikonu a jde
    // o jednu z hodnot SALICON_xxx. 'iconSize' urcuje jakou ma mit vracena ikona velikost
    // a jde o jednu z hodnot SALICONSIZE_xxx.
    // V pripade uspechu vraci handle vytvorene ikony. Destrukci ikony musi zajistit
    // plugin pomoci volani API DestroyIcon. V pripade neuspechu vraci NULL.
    // omezeni: hlavni thread
    virtual HICON WINAPI GetSalamanderIcon(int icon, int iconSize) = 0;

    // GetFileIcon
    //   Retrieves a handle to a large or small icon for the specified object,
    //   such as a file, a folder, a directory, or a drive root.
    //
    // Parameters
    //   'path'
    //      [in] Pointer to a null-terminated string that contains the path and file
    //      name. If the 'pathIsPIDL' parameter is TRUE, this parameter must be the
    //      address of an ITEMIDLIST (PIDL) structure that contains the list of item
    //      identifiers that uniquely identify the file within the Shell's namespace.
    //      The PIDL must be a fully qualified PIDL. Relative PIDLs are not allowed.
    //
    //   'pathIsPIDL'
    //      [in] Indicates that 'path' is the address of an ITEMIDLIST structure rather
    //      than a path name.
    //
    //   'hIcon'
    //      [out] Pointer to an icon handle that receives the handle to the icon extracted
    //      from the object.
    //
    //   'iconSize'
    //      [in] Required icon size. SALICONSIZE_xxx
    //
    //   'fallbackToDefIcon'
    //      [in] Specifies whether the default (simple) icon should be used if the
    //      icon of the specified object is not available. If this parameter is
    //      TRUE, the function tries to return the default (simple) icon in that
    //      situation. Otherwise, it returns no icon (the return value is FALSE).
    //
    //   'defIconIsDir'
    //      [in] Specifies whether the default (simple) icon for 'path' is a
    //      directory icon. This parameter is ignored unless 'fallbackToDefIcon' is TRUE.
    //
    // Return Values
    //   Returns TRUE if successful, or FALSE otherwise.
    //
    // Remarks
    //   You are responsible for freeing returned icons with DestroyIcon when you
    //   no longer need them.
    //
    //   You must initialize COM with CoInitialize or OleInitialize prior to
    //   calling GetFileIcon.
    //
    //   Method can be called from any thread.
    //
    virtual BOOL WINAPI GetFileIcon(const char* path, BOOL pathIsPIDL,
                                    HICON* hIcon, int iconSize, BOOL fallbackToDefIcon,
                                    BOOL defIconIsDir) = 0;

    // FileExists
    //   Function checks the existence of a file. It returns TRUE if the specified
    //   file exists. If the file does not exist, it returns 0. FileExists only checks
    //   the existence of files, directories are ignored.
    // can be called from any thread
    virtual BOOL WINAPI FileExists(const char* fileName) = 0;

    // changes the path in the panel to the last known disk path; if it is not accessible,
    // the path is changed to the user-selected "rescue" path (see
    // SALCFG_IFPATHISINACCESSIBLEGOTO), and if that also fails, then to the root of the first local
    // fixed drive (Salamander 2.5 and 2.51 only change it to the root of the first local fixed drive);
    // used when closing a file system in a panel (disconnect); 'parent' is the parent of any
    // message boxes; 'panel' is one of PANEL_XXX
    // limitation: main thread only + outside the methods of CPluginFSInterfaceAbstract and CPluginDataInterfaceAbstract
    // (for example, an FS open in the panel could be closed - the method could stop having a valid 'this')
    virtual void WINAPI DisconnectFSFromPanel(HWND parent, int panel) = 0;

    // returns TRUE if the file name 'name' is associated with the calling plugin in
    // Archives Associations in Panels
    // 'name' must be only the file name, not a full or relative path
    // limitation: main thread
    virtual BOOL WINAPI IsArchiveHandledByThisPlugin(const char* name) = 0;

    // serves as an LR_xxx parameter for the API function LoadImage()
    // if the user does not have hi-color icons enabled in the desktop configuration,
    // it returns LR_VGACOLOR to avoid incorrectly loading the multi-color version of the icon
    // otherwise it returns 0 (LR_DEFAULTCOLOR); the function result can be ORed with other LR_xxx flags
    // can be called from any thread
    virtual DWORD WINAPI GetIconLRFlags() = 0;

    // determines from the file extension whether it is a link ("lnk", "pif" or "url"); 'fileExtension'
    // is the file extension (a pointer past the dot), must not be NULL; returns 1 if it is a link,
    // otherwise returns 0; NOTE: used to populate CFileData::IsLink
    // can be called from any thread
    virtual int WINAPI IsFileLink(const char* fileExtension) = 0;

    // returns ILC_COLOR??? according to the Windows version - tuned for using image lists in list views
    // typical use: ImageList_Create(16, 16, ILC_MASK | GetImageListColorFlags(), ???, ???)
    // can be called from any thread
    virtual DWORD WINAPI GetImageListColorFlags() = 0;

    // the "safe" version of GetOpenFileName()/GetSaveFileName() handles the situation when the path
    // passed in OPENFILENAME::lpstrFile is not valid (for example z:\); in this case the standard API version
    // of the function does not open the dialog box and silently returns FALSE, and CommDlgExtendedError() returns FNERR_INVALIDFILENAME.
    // In this case, the following two functions call the API once more, but with a "safely"
    // existing path (Documents, or Desktop respectively).
    virtual BOOL WINAPI SafeGetOpenFileName(LPOPENFILENAME lpofn) = 0;
    virtual BOOL WINAPI SafeGetSaveFileName(LPOPENFILENAME lpofn) = 0;

    // before using OpenHtmlHelp(), the plugin must provide Salamander with the name of its .chm file
    // without a path (e.g. "demoplug.chm")
    // can be called from any thread, but concurrent calls with OpenHtmlHelp() must be prevented
    virtual void WINAPI SetHelpFileName(const char* chmName) = 0;

    // opens the plugin's HTML help; the help language (the directory with .chm files) is selected as follows:
    // -directory obtained from Salamander's current .slg file (see SLGHelpDir in shared\versinfo.rc)
    // -HELP\ENGLISH\*.chm
    // -the first subdirectory found in the HELP subdirectory
    // before using OpenHtmlHelp(), the plugin must call SetHelpFileName(); 'parent' is the parent
    // of the error message box; 'command' is the HTML Help command, see HHCDisplayXXX; 'dwData' is the parameter
    // of the HTML Help command, see HHCDisplayXXX
    // can be called from any thread
    // note: for displaying Salamander help, see OpenHtmlHelpForSalamander
    virtual BOOL WINAPI OpenHtmlHelp(HWND parent, CHtmlHelpCommand command, DWORD_PTR dwData,
                                     BOOL quiet) = 0;

    // vraci TRUE, pokud jsou cesty 'path1' a 'path2' ze stejneho svazku; v 'resIsOnlyEstimation'
    // (neni-li NULL) vraci TRUE, pokud neni vysledek jisty (jisty je jen v pripade shody cest nebo
    // pokud se podari ziskat "volume name" (GUID svazku) u obou cest, coz pripada v uvahu jen pro
    // lokalni cesty pod W2K nebo novejsimi z rady NT)
    // mozne volat z libovolneho threadu
    virtual BOOL WINAPI PathsAreOnTheSameVolume(const char* path1, const char* path2,
                                                BOOL* resIsOnlyEstimation) = 0;

    // realokace pameti na heapu Salamandera (pri pouziti salrtl9.dll zbytecne - staci klasicky realloc);
    // pri nedostatku pameti zobrazi uzivateli hlaseni s tlacitky Retry a Cancel (po dalsim dotazu
    // terminuje aplikaci)
    // mozne volat z libovolneho threadu
    virtual void* WINAPI Realloc(void* ptr, int size) = 0;

    // vraci v 'enumFilesSourceUID' (nesmi byt NULL) unikatni identifikator zdroje pro panel
    // 'panel' (jeden z PANEL_XXX), pouziva se ve viewerech pri enumeraci souboru
    // z panelu (viz parametr 'srcUID' napr. v metode GetNextFileNameForViewer), tento
    // identifikator se meni napr. pri zmene cesty v panelu; neni-li 'enumFilesCurrentIndex'
    // NULL, vraci se v nem index fokusleho souboru (pokud neni fokusly soubor, vraci se -1);
    // omezeni: hlavni thread (jinak se muze obsah panelu menit)
    virtual void WINAPI GetPanelEnumFilesParams(int panel, int* enumFilesSourceUID,
                                                int* enumFilesCurrentIndex) = 0;

    // posts a message to the panel with active FS 'modifiedFS' that the path should be
    // refreshed (it reloads the listing and transfers selection, icons, focus, etc. to the
    // new panel contents); the refresh is performed only after the Salamander main window is
    // activated (after suspend mode ends); the FS path is always reloaded; if 'modifiedFS' is
    // not in any panel, nothing is done; if 'focusFirstNewItem' is TRUE and only a single item
    // was added to the panel, that new item gets focus (used for example to focus a newly created
    // file/directory); returns TRUE if the refresh was performed, FALSE if 'modifiedFS' was not
    // found in either panel
    // can be called from any thread (if the main thread is not running code inside the plugin,
    // the refresh is performed as soon as possible; otherwise it waits at least until the main
    // thread leaves the plugin)
    virtual BOOL WINAPI PostRefreshPanelFS2(CPluginFSInterfaceAbstract* modifiedFS,
                                            BOOL focusFirstNewItem = FALSE) = 0;

    // loads from module 'module' the text with ID 'resID'; returns the text in an internal buffer (the text may
    // change due to changes in the internal buffer caused by subsequent calls to LoadStr, even from other
    // plugins or Salamander; the buffer is 10000 characters long, overwriting does not occur until it
    // becomes full (it is used cyclically); if you need to use the text later, we recommend
    // copying it to a local buffer); if 'module' is NULL or 'resID' is not in the module,
    // the text "ERROR LOADING STRING" is returned (and the debug/SDK version outputs TRACE_E)
    // can be called from any thread
    virtual char* WINAPI LoadStr(HINSTANCE module, int resID) = 0;

    // nacte z resourcu modulu 'module' text s ID 'resID'; vraci text v internim bufferu (hrozi
    // zmena textu diky zmene interniho bufferu zpusobene dalsimi volanimi LoadStrW i z jinych
    // pluginu nebo Salamandera; buffer je velky 10000 znaku, prepis hrozi teprve po jeho
    // zaplneni (pouziva se cyklicky); pokud potrebujete text pouzit az pozdeji, doporucujeme
    // jej zkopirovat do lokalniho bufferu); je-li 'module' NULL nebo 'resID' neni v modulu,
    // vraci se text L"ERROR LOADING WIDE STRING" (a debug/SDK verze vypise TRACE_E)
    // mozne volat z libovolneho threadu
    virtual WCHAR* WINAPI LoadStrW(HINSTANCE module, int resID) = 0;

    // changes the panel path to the user-selected "rescue" path (see
    // SALCFG_IFPATHISINACCESSIBLEGOTO) and if even that fails, then to the root of the first local fixed
    // drive; this is an almost certain way to change the current panel path; 'panel' is one of PANEL_XXX;
    // if 'failReason' is not NULL, it is set to one of the CHPPFR_XXX constants (indicating the result
    // of the method); returns TRUE if the path change succeeded (to the "rescue" path or fixed drive)
    // limitation: main thread only + outside methods of CPluginFSInterfaceAbstract and CPluginDataInterfaceAbstract
    // (for example, the FS opened in the panel may be closed - the method could stop having a valid 'this')
    virtual BOOL WINAPI ChangePanelPathToRescuePathOrFixedDrive(int panel, int* failReason = NULL) = 0;

    // registers the plugin as a replacement for the Network item in the Change Drive menu and in the drive bars,
    // the plugin must add a file system to Salamander on which incomplete
    // UNC paths ("\\" and "\\server") from the Change Directory command are then opened and which is entered
    // via the up-dir symbol ("..") from the root of UNC paths;
    // limitation: call only from the plugin entry point and only after SetBasicPluginData
    virtual void WINAPI SetPluginIsNethood() = 0;

    // opens the system context menu for selected items or the focused item on a network path
    // ('forItems' is TRUE) or for a network path ('forItems' is FALSE), and also executes the selected
    // command from the menu; the menu is obtained by traversing the CSIDL_NETWORK folder; 'parent' is the suggested parent
    // of the context menu; 'panel' identifies the panel (PANEL_LEFT or PANEL_RIGHT) for which the
    // context menu should be opened (the focused/selected files/directories to work with are taken
    // from this panel); 'menuX' + 'menuY' are the suggested coordinates of the upper-left corner
    // of the context menu; 'netPath' is a network path, only "\\" and "\\server" are allowed; if
    // 'newlyMappedDrive' is not NULL, it receives the letter ('A' to 'Z') of the newly mapped drive (via
    // the Map Network Drive command from the context menu); if it receives zero, no new
    // mapping occurred
    // limitation: main thread
    virtual void WINAPI OpenNetworkContextMenu(HWND parent, int panel, BOOL forItems, int menuX,
                                               int menuY, const char* netPath,
                                               char* newlyMappedDrive) = 0;

    // duplicates '\\' - useful for texts that we send to LookForSubTexts, which reduces '\\\\'
    // back to '\\'; 'buffer' is an input/output string, 'bufferSize' is the size of
    // 'buffer' in bytes; returns TRUE if duplicating did not cause characters at the end of the string
    // to be lost (the buffer was large enough)
    // can be called from any thread
    virtual BOOL WINAPI DuplicateBackslashes(char* buffer, int bufferSize) = 0;

    // shows a throbber in panel 'panel' (an animation informing the user about activity related
    // to the panel, e.g. "loading data from the network") with a delay of 'delay' (in ms); 'panel' is one
    // of PANEL_XXX; if 'tooltip' is not NULL, it is the text shown when the mouse hovers over the
    // throbber (if it is NULL, no text is shown); if a throbber is already displayed in the panel
    // or is waiting to be displayed, its identifier and tooltip are changed (if it is already displayed,
    // 'delay' is ignored; if it is waiting to be displayed, the new delay is set according to 'delay');
    // returns the throbber identifier (it is never -1, so -1 can be used as
    // an empty value + all returned identifiers are unique; more precisely, they would start to repeat
    // only after an unrealistic 2^32 throbber displays);
    // NOTE: a suitable place to show the throbber for FS is when receiving the FSE_PATHCHANGED event,
    // because the FS is already in the panel at that point (whether the throbber should or should not be shown can be determined in advance
    // in ChangePath or ListCurrentPath)
    // limitation: main thread
    virtual int WINAPI StartThrobber(int panel, const char* tooltip, int delay) = 0;

    // hides the throbber with identifier 'id'; returns TRUE if the throbber is hidden
    // successfully; returns FALSE if this throbber has already been hidden or another
    // throbber has been shown over it;
    // NOTE: the throbber is automatically hidden just before the path changes in the panel or
    // before a refresh (for FS this means just after a successful call to ListCurrentPath, for archives
    // it is after the archive is opened and listed, for drives it is after verifying that the path is accessible)
    // limitation: main thread
    virtual BOOL WINAPI StopThrobber(int id) = 0;

    // shows a security icon in panel 'panel' (a locked or unlocked padlock; for example, in FTPS it informs
    // the user that the connection to the server is secured using SSL and that the server's identity has
    // been verified (locked padlock) or has not been verified (unlocked padlock)); 'panel' is one of PANEL_XXX;
    // if 'showIcon' is TRUE, the icon is shown, otherwise it is hidden; 'isLocked' specifies whether it is a
    // locked (TRUE) or unlocked (FALSE) padlock; if 'tooltip' is not NULL, it is the text that is
    // shown when the mouse hovers over the icon (if it is NULL, no text is shown); if some action is to be performed
    // when the security icon is clicked (for example, in FTPS a dialog with the server certificate is displayed),
    // it must be added to the CPluginFSInterfaceAbstract::ShowSecurityInfo method of the file system
    // displayed in the panel;
    // NOTE: a suitable place to display the security icon for an FS is when handling the
    // FSE_PATHCHANGED event, because the FS is already in the panel by then (whether the icon should or should not be shown can be determined
    // in advance in ChangePath or ListCurrentPath)
    // NOTE: the security icon is automatically hidden just before the path in the panel changes or
    // before a refresh (for an FS this means immediately after a successful call to ListCurrentPath; for an archive
    // it is after the archive is opened and listed; for a disk it is after the path accessibility is verified)
    // limitation: main thread
    virtual void WINAPI ShowSecurityIcon(int panel, BOOL showIcon, BOOL isLocked,
                                         const char* tooltip) = 0;

    // removes the current path in the panel from the history of directories displayed in the panel (Alt+Left/Right)
    // and from the list of working paths (Alt+F12); used to hide transitional paths,
    // e.g. "net:\Entire Network\Microsoft Windows Network\WORKGROUP\server\share" automatically
    // changes to "\\server\share", and it is undesirable for this transition to occur when moving through the history
    // limitation: main thread
    virtual void WINAPI RemoveCurrentPathFromHistory(int panel) = 0;

    // returns TRUE if the current user is a member of the Administrators group, otherwise returns FALSE
    // can be called from any thread
    virtual BOOL WINAPI IsUserAdmin() = 0;

    // returns TRUE if Salamander is running in a Remote Desktop session, otherwise returns FALSE
    // can be called from any thread
    virtual BOOL WINAPI IsRemoteSession() = 0;

    // equivalent to calling WNetAddConnection2(lpNetResource, NULL, NULL, CONNECT_INTERACTIVE);
    // the advantage is more detailed reporting of error conditions (e.g. that the password expired,
    // that the password or user name was entered incorrectly, that the password needs to be changed, etc.)
    // can be called from any thread
    virtual DWORD WINAPI SalWNetAddConnection2Interactive(LPNETRESOURCE lpNetResource) = 0;

    //
    // GetMouseWheelScrollChars
    //   An OS independent method to retrieve the number of wheel scroll chars.
    //
    // Return Values
    //   Number of scroll characters where WHEEL_PAGESCROLL (0xffffffff) indicates to scroll a page at a time.
    //
    // Remarks
    //   Method can be called from any thread.
    virtual DWORD WINAPI GetMouseWheelScrollChars() = 0;

    //
    // GetSalamanderZLIB
    //   Provides simplified interface to the ZLIB library provided by Salamander,
    //   for details see spl_zlib.h.
    //
    // Remarks
    //   Method can be called from any thread.
    virtual CSalamanderZLIBAbstract* WINAPI GetSalamanderZLIB() = 0;

    //
    // GetSalamanderPNG
    //   Provides interface to the PNG library provided by Salamander.
    //
    // Remarks
    //   Method can be called from any thread.
    virtual CSalamanderPNGAbstract* WINAPI GetSalamanderPNG() = 0;

    //
    // GetSalamanderCrypt
    //   Provides interface to encryption libraries provided by Salamander,
    //   for details see spl_crypt.h.
    //
    // Remarks
    //   Method can be called from any thread.
    virtual CSalamanderCryptAbstract* WINAPI GetSalamanderCrypt() = 0;

    // informs Salamander that the plugin uses Password Manager and therefore Salamander should
    // notify the plugin about the master password being set/changed/cleared (see
    // CPluginInterfaceAbstract::PasswordManagerEvent)
    // restriction: call only from the plugin entry point, and only after SetBasicPluginData
    virtual void WINAPI SetPluginUsesPasswordManager() = 0;

    //
    // GetSalamanderPasswordManager
    //   Provides interface to Password Manager provided by Salamander.
    //
    // Remarks
    //   Method can be called from main thread only.
    virtual CSalamanderPasswordManagerAbstract* WINAPI GetSalamanderPasswordManager() = 0;

    // opens Salamander's own HTML help (instead of plugin help, which is opened using OpenHtmlHelp()),
    // the help language (the directory with .chm files) is selected as follows:
    // -directory obtained from Salamander's current .slg file (see SLGHelpDir in shared\versinfo.rc)
    // -HELP\ENGLISH\*.chm
    // -the first subdirectory found in the HELP subdirectory
    // 'parent' is the parent of the error message box; 'command' is the HTML Help command, see HHCDisplayXXX;
    // 'dwData' is the parameter of the HTML Help command, see HHCDisplayXXX; if command==HHCDisplayContext,
    // the value of 'dwData' must be from the HTMLHELP_SALID_XXX family of constants
    // can be called from any thread
    virtual BOOL WINAPI OpenHtmlHelpForSalamander(HWND parent, CHtmlHelpCommand command, DWORD_PTR dwData, BOOL quiet) = 0;

    //
    // GetSalamanderBZIP2
    //   Provides simplified interface to the BZIP2 library provided by Salamander,
    //   for details see spl_bzip2.h.
    //
    // Remarks
    //   Method can be called from any thread.
    virtual CSalamanderBZIP2Abstract* WINAPI GetSalamanderBZIP2() = 0;

    //
    // GetFocusedItemMenuPos
    //   Returns point (in screen coordinates) where the context menu for focused item in the
    //   active panel should be displayed. The upper left corner of the panel is returned when
    //   focused item is not visible
    //
    // Remarks
    //   Method can be called from main thread only.
    virtual void WINAPI GetFocusedItemMenuPos(POINT* pos) = 0;

    //
    // LockMainWindow
    //   Locks the main window so that it appears disabled. The main window is still able to receive focus
    //   in the locked state. Set 'lock' to TRUE to lock the main window and to FALSE to return it
    //   to the normal state. 'hToolWnd' is a reserved parameter; set it to NULL. 'lockReason'
    //   optionally describes the reason for the main window being locked (it can be NULL). It is displayed
    //   when attempting to close the locked main window; the string content is copied to an internal
    //   structure, so the buffer can be deallocated after LockMainWindow() returns.
    //
    // Remarks
    //   This method can be called only from the main thread.
    virtual void WINAPI LockMainWindow(BOOL lock, HWND hToolWnd, const char* lockReason) = 0;

    // only for "dynamic menu extension" plugins (see FUNCTION_DYNAMICMENUEXT):
    // sets a flag for the calling plugin that at the next possible opportunity
    // (as soon as there are no messages in the main thread's message queue and Salamander is not
    // "busy" (no modal dialog is open and no message is being processed))
    // the menu should be rebuilt by calling the CPluginInterfaceForMenuExtAbstract::BuildMenu method;
    // WARNING: if called from a thread other than the main thread, BuildMenu may be called
    // (it runs in the main thread) even before PostPluginMenuChanged returns
    // can be called from any thread
    virtual void WINAPI PostPluginMenuChanged() = 0;

    //
    // GetMenuItemHotKey
    //   Search through plugin's menu items added with AddMenuItem() for item with 'id'.
    //   When such item is found, its 'hotKey' and 'hotKeyText' (up to 'hotKeyTextSize' characters)
    //   is set. Both 'hotKey' and 'hotKeyText' could be NULL.
    //   Returns TRUE when item with 'id' is found, otherwise returns FALSE.
    //
    //   Remarks
    //   Method can be called from main thread only.
    virtual BOOL WINAPI GetMenuItemHotKey(int id, WORD* hotKey, char* hotKeyText, int hotKeyTextSize) = 0;

    // our variants of the RegQueryValue and RegQueryValueEx functions, unlike the API variants,
    // ensure that a null terminator is added for REG_SZ, REG_MULTI_SZ, and REG_EXPAND_SZ types
    // WARNING: when determining the required buffer size, they return one or two extra characters (two
    //          only for REG_MULTI_SZ) in case the string needs to be terminated with a null/nulls
    // can be called from any thread
    virtual LONG WINAPI SalRegQueryValue(HKEY hKey, LPCSTR lpSubKey, LPSTR lpData, PLONG lpcbData) = 0;
    virtual LONG WINAPI SalRegQueryValueEx(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved,
                                           LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData) = 0;

    // because the Windows version of GetFileAttributes cannot work with names ending in a space,
    // we wrote our own (for such names it appends a backslash at the end, after which
    // GetFileAttributes works correctly, but only for directories; for files with a space at
    // the end we have no solution, but at least it does not read attributes from a different file -
    // the Windows version trims trailing spaces and thus works with a different file/directory)
    // can be called from any thread
    virtual DWORD WINAPI SalGetFileAttributes(const char* fileName) = 0;

    // there is currently no Win32 API for SSD detection, so it is done heuristically
    // based on queries for support for TRIM, StorageDeviceSeekPenaltyProperty, etc.
    // the function returns TRUE if the disk at 'path' appears to be an SSD; FALSE otherwise
    // the result is not 100%; people report the algorithm does not work, for example, on SSD PCIe cards:
    // http://stackoverflow.com/questions/23363115/detecting-ssd-in-windows/33359142#33359142
    // it can determine correct information even for paths containing SUBST drives and reparse points under Windows
    // 2000/XP/Vista (Salamander 2.5 works only with junction points); 'path' is the path for which
    // we determine the information; if the path goes through a network path, it silently returns FALSE
    // can be called from any thread
    virtual BOOL WINAPI IsPathOnSSD(const char* path) = 0;

    // returns TRUE if this is a UNC path (detects both formats: \\server\share and \\?\UNC\server\share)
    // can be called from any thread
    virtual BOOL WINAPI IsUNCPath(const char* path) = 0;

    // replaces SUBSTs in the path 'resPath' with their target paths (converts it to a path without SUBST drive letters);
    // 'resPath' must point to a buffer of at least 'MAX_PATH' characters
    // returns TRUE on success, FALSE on error
    // can be called from any thread
    virtual BOOL WINAPI ResolveSubsts(char* resPath) = 0;

    // call only for 'path' paths whose root (after resolving substs) is DRIVE_FIXED (elsewhere there is no point in looking
    // for reparse points); we look for a path without reparse points that leads to the same volume as 'path'; for a path
    // containing a symlink leading to a network path (UNC or mapped), we return only the root of that network path
    // (even Vista cannot handle reparse points on network paths, so it is probably pointless to push it);
    // if no such path exists because the current (last) local reparse point is a volume mount point
    // (or an unknown type of reparse point), we return the path to this volume mount point (or reparse
    // point of an unknown type); if the path contains more than 50 reparse points (most likely an infinite loop),
    // we return the original path;
    //
    // 'resPath' is an output buffer of size MAX_PATH; 'path' is the original path; in 'cutResPathIsPossible'
    // (must not be NULL) we return FALSE if the resulting path in 'resPath' ends with a reparse point (volume
    // mount point or an unknown reparse point type) and therefore must not be shortened (we would most likely end up
    // on a different volume); if 'rootOrCurReparsePointSet' is non-NULL and contains FALSE, and the original path
    // has at least one local reparse point (reparse points in the network part of the path are ignored), we return
    // TRUE in this variable and return the full path to the current (last local) reparse point in
    // 'rootOrCurReparsePoint' (if it is not NULL) (note: not where it leads); the target path of the current reparse
    // point (only if it is a junction or symlink) is returned in 'junctionOrSymlinkTgt' (if not NULL), and the type
    // is returned in 'linkType': 2 (JUNCTION POINT), 3 (SYMBOLIC LINK); in 'netPath' (if not NULL) we return the
    // network path to which the current (last) local symlink in the path leads - in this case the root of the
    // network path is returned in 'resPath'
    // can be called from any thread
    virtual void WINAPI ResolveLocalPathWithReparsePoints(char* resPath, const char* path,
                                                          BOOL* cutResPathIsPossible,
                                                          BOOL* rootOrCurReparsePointSet,
                                                          char* rootOrCurReparsePoint,
                                                          char* junctionOrSymlinkTgt, int* linkType,
                                                          char* netPath) = 0;

    // Resolves SUBSTs and reparse points for the path 'path', then tries to obtain the GUID path for the path's mount point
    // (if it is missing, then for the path root). On failure, returns FALSE. On success,
    // returns TRUE and sets 'mountPoint' and 'guidPath' (if they are not NULL, they must
    // point to buffers at least MAX_PATH in size; the strings will be terminated with a backslash).
    // Can be called from any thread
    virtual BOOL WINAPI GetResolvedPathMountPointAndGUID(const char* path, char* mountPoint, char* guidPath) = 0;

    // replaces the last '.' character in the string with the decimal separator obtained from the system LOCALE_SDECIMAL
    // the string length may increase because according to MSDN the separator can be up to 4 characters long
    // returns TRUE if the buffer was large enough and the operation completed successfully, otherwise returns FALSE
    // can be called from any thread
    virtual BOOL WINAPI PointToLocalDecimalSeparator(char* buffer, int bufferSize) = 0;

    // nastavi pro tento plugin pole icon-overlays; po nastaveni muze plugin v listingach vracet
    // index icon-overlaye (viz CFileData::IconOverlayIndex), ktery se ma zobrazit pres ikonu
    // polozky listingu, takto je mozne pouzit az 15 icon-overlays (indexy 0 az 14, protoze
    // index 15=ICONOVERLAYINDEX_NOTUSED aneb: nezobrazuj icon-overlay); 'iconOverlaysCount'
    // je pocet icon-overlays pro plugin; pole 'iconOverlays' obsahuje pro kazdy icon-overlay
    // postupne vsechny velikosti ikon: SALICONSIZE_16, SALICONSIZE_32 a SALICONSIZE_48 - tedy
    // v poli 'iconOverlays' je 3 * 'iconOverlaysCount' ikon; uvolneni ikon v poli 'iconOverlays'
    // zajisti Salamander (volani DestroyIcon()), samotne pole je vec volajiciho, pokud v poli
    // budou nejake NULL (napr. nezdaril se load ikony), funkce selze, ale platne ikony z pole
    // uvolni; pri zmene barev v systemu by mel plugin icon-overlays znovu nacist a znovu nastavit
    // touto funkci, idealni je reakce na PLUGINEVENT_COLORSCHANGED ve funkci
    // CPluginInterfaceAbstract::Event()
    // POZOR: pred Windows XP (ve W2K) je velikost ikony SALICONSIZE_48 jen 32 bodu!
    // omezeni: hlavni thread
    virtual void WINAPI SetPluginIconOverlays(int iconOverlaysCount, HICON* iconOverlays) = 0;

    // popis viz SalGetFileSize(), prvni rozdil je, ze se soubor zadava plnou cestou;
    // druhy je, ze 'err' muze byt NULL, pokud nestojime o kod chyby;
    virtual BOOL WINAPI SalGetFileSize2(const char* fileName, CQuadWord& size, DWORD* err) = 0;

    // zjisti velikost souboru, na ktery vede symlink 'fileName'; velikost vraci v 'size';
    // 'ignoreAll' je in + out, je-li TRUE vsechny chyby se ignoruji (pred akci je treba
    // priradit FALSE, jinak se okno s chybou vubec nezobrazi, pak uz nemenit);
    // pri chybe zobrazi standardni okno s dotazem Retry / Ignore / Ignore All / Cancel
    // s parentem 'parent'; pokud velikost uspesne zjisti, vraci TRUE; pri chybe a stisku
    // tlacitka Ignore / Igore All v okne s chybou, vraci FALSE a v 'cancel' vraci FALSE;
    // je-li 'ignoreAll' TRUE, okno se neukaze, na tlacitko se neceka, chova se jako by
    // uzivatel stiskl Ignore; pri chybe a stisku Cancel v okne s chybou vraci FALSE a
    // v 'cancel' vraci TRUE;
    // mozne volat z libovolneho threadu
    virtual BOOL WINAPI GetLinkTgtFileSize(HWND parent, const char* fileName, CQuadWord* size,
                                           BOOL* cancel, BOOL* ignoreAll) = 0;

    // smaze link na adresar (junction point, symbolic link, mount point); pri uspechu
    // vraci TRUE; pri chybe vraci FALSE a neni-li 'err' NULL, vraci kod chyby v 'err'
    // mozne volat z libovolneho threadu
    virtual BOOL WINAPI DeleteDirLink(const char* name, DWORD* err) = 0;

    // if file/directory 'name' has the read-only attribute, we try to clear it
    // (reason: e.g. so it can be deleted via DeleteFile); if we already have the attributes of 'name'
    // loaded, we pass them in 'attr'; if 'attr' is -1, the attributes of 'name' are read from disk;
    // returns TRUE if an attempt to change the attribute is made (success is not checked);
    // NOTE: only clears the read-only attribute, so that in the case of multiple hard links there is no
    // unnecessarily large attribute change on the remaining hard links of the file (the attributes
    // are shared by all hard links)
    // can be called from any thread
    virtual BOOL WINAPI ClearReadOnlyAttr(const char* name, DWORD attr = -1) = 0;

    // checks whether a critical shutdown (or logoff) is currently in progress; returns TRUE if so;
    // during this shutdown we have only 5 seconds to save the configuration of the entire program,
    // including plugins, so we must skip more time-consuming operations; after 5 seconds
    // the system forcibly terminates our process; see `WM_ENDSESSION` and the `ENDSESSION_CRITICAL` flag;
    // Vista+
    virtual BOOL WINAPI IsCriticalShutdown() = 0;

    // enumerates all windows (EnumThreadWindows) in thread 'tid' (0 = current) and posts WM_CLOSE
    // to all enabled and visible dialogs (class name "#32770") owned by window 'parent';
    // used during critical shutdown to unblock the window/dialog over which modal dialogs
    // are open; if multiple layers are possible, it must be called repeatedly
    virtual void WINAPI CloseAllOwnedEnabledDialogs(HWND parent, DWORD tid = 0) = 0;
};

#ifdef _MSC_VER
#pragma pack(pop, enter_include_spl_gen)
#endif // _MSC_VER
#ifdef __BORLANDC__
#pragma option -a
#endif // __BORLANDC__
