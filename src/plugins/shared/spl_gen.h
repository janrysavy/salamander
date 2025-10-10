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

// typy message-boxu
#define MSGBOX_INFO 0
#define MSGBOX_ERROR 1
#define MSGBOX_EX_ERROR 2
#define MSGBOX_QUESTION 3
#define MSGBOX_EX_QUESTION 4
#define MSGBOX_WARNING 5
#define MSGBOX_EX_WARNING 6

// konstanty pro CSalamanderGeneralAbstract::SalMessageBoxEx
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

// altap specific
#define MSGBOXEX_SILENT 0x10000000 // messagebox nevyda pri otevreni zadny zvuk (bit mask)
// v pripade MB_YESNO messageboxu povoli Escape (generuje IDNO); v MB_ABORTRETRYIGNORE messageboxu
// povoli Escape (generuje IDCANCEL) (bit mask)
#define MSGBOXEX_ESCAPEENABLED 0x20000000
#define MSGBOXEX_HINT 0x40000000 // pokud se pouziva CheckBoxText, bude v nem vyhledan oddelovac \t a zobrazen jako hint
// Vista: defaultni tlacitko bude mit stav "pozaduje elevaci" (zobrazi se elevated icon)
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
  Handle to the owner window. Message box is centered to this window.
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
      Message box can be closed using Escape and return value will be DIALOG_OK (IDOK).
    MSGBOXEX_OKCANCEL             (MB_OKCANCEL)
      The message box contains two push buttons: OK and Cancel.
    MSGBOXEX_ABORTRETRYIGNORE     (MB_ABORTRETRYIGNORE)
      The message box contains three push buttons: Abort, Retry, and Ignore.
      Message box can be closed using Escape when MSGBOXEX_ESCAPEENABLED flag is specified.
      In that case return value will be DIALOG_CANCEL (IDCANCEL).
    MSGBOXEX_YESNOCANCEL          (MB_YESNOCANCEL)
      The message box contains three push buttons: Yes, No, and Cancel.
    MSGBOXEX_YESNO                (MB_YESNO)
      The message box contains two push buttons: Yes and No.
      Message box can be closed using Escape when MSGBOXEX_ESCAPEENABLED flag is specified.
      In that case return value will be DIALOG_NO (IDNO).
    MSGBOXEX_RETRYCANCEL          (MB_RETRYCANCEL)
      The message box contains two push buttons: Retry and Cancel.
    MSGBOXEX_CANCELTRYCONTINUE    (MB_CANCELTRYCONTINUE)
      The message box contains three push buttons: Cancel, Try Again, Continue.

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
      No sound will be played when message box is displayed.
    MSGBOXEX_ESCAPEENABLED
      When MSGBOXEX_YESNO is specified, user can close message box using Escape key and DIALOG_NO (IDNO)
      will be returned. When MSGBOXEX_ABORTRETRYIGNORE is specified, user can close message box using
      Escape key and DIALOG_CANCEL (IDCANCEL) will be returned. Otherwise this option is ignored.

HIcon
  Handle to the icon to be drawn in the message box. Icon will not be destroyed when messagebox is closed.
  If this parameter is NULL, MSGBOXEX_ICONxxx style will be used.

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
  Pointer to a null-terminated string that contains the checkbox text.
  If the MSGBOXEX_HINT flag is specified in the Flags, this text must contain HINT.
  Hint is separated from string by the TAB character. Hint is divided by the second TAB character
  on two parts. The first part is label, that will be displayed behind the check box.
  The second part is the text displayed when user clicks the hint label.

  Example: "This is text for checkbox\tHint Label\tThis text will be displayed when user click the Hint Label."
  If this member is NULL, checkbox will not be displayed.

CheckBoxValue
  Pointer to a BOOL variable contains the checkbox initial and return state (TRUE: checked, FALSE: unchecked).
  This parameter is ignored if CheckBoxText parameter is NULL. Otherwise this parameter must be set.

AliasBtnNames
  Pointer to a buffer containing pairs of id and alias strings. The last string in the
  buffer must be terminated by NULL character.

  The first string in each pair is a decimal number that specifies button ID.
  Number must be one of the DIALOG_xxx values. The second string specifies alias text
  for this button.

  First and second string in each pair are separated by TAB character.
  Pairs are separated by TAB character too.

  If this member is NULL, normal names of buttons will displayed.

  Example: sprintf(buffer, "%d\t%s\t%d\t%s", DIALOG_OK, "&Start", DIALOG_CANCEL, "E&xit");
           buffer: "1\t&Start\t2\tE&xit"

URL
  Pointer to a null-terminated string that contains the URL displayed below text.
  If this member is NULL, the URL is not displayed.

URLText
  Pointer to a null-terminated string that contains the URL text displayed below text.
  If this member is NULL, the URL is displayed instead.

*/

// panel identification
#define PANEL_SOURCE 1 // source panel (active panel)
#define PANEL_TARGET 2 // target panel (inactive panel)
#define PANEL_LEFT 3   // left panel
#define PANEL_RIGHT 4  // right panel

// path types
#define PATH_TYPE_WINDOWS 1 // Windows path ("c:\path" or UNC path)
#define PATH_TYPE_ARCHIVE 2 // path into an archive (the archive resides on a Windows path)
#define PATH_TYPE_FS 3      // path on a plugin file system

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

// error constants for CSalamanderGeneralAbstract::SalGetFullName
#define GFN_SERVERNAMEMISSING 1   // the server name is missing in the UNC path
#define GFN_SHARENAMEMISSING 2    // the share name is missing in the UNC path
#define GFN_TOOLONGPATH 3         // the operation would create a path that is too long
#define GFN_INVALIDDRIVE 4        // a standard path (c:\) does not contain a letter A-Z (or a-z)
#define GFN_INCOMLETEFILENAME 5   // relative path without a provided 'curDir' -> unsolvable
#define GFN_EMPTYNAMENOTALLOWED 6 // empty 'name' string
#define GFN_PATHISINVALID 7       // ".." cannot be excluded, for example "c:\.."

// error code for the state when the user interrupts CSalamanderGeneralAbstract::SalCheckPath with the ESC key
#define ERROR_USER_TERMINATED -100

#define PATH_MAX_PATH 248 // maximum path length limit (full directory name); note: the null terminator is already included (max. string length is 247 characters)

// error constants for CSalamanderGeneralAbstract::SalParsePath:
// the input was an empty path and 'curPath' was NULL (an empty path is replaced with the current path,
// but it is not known here)
#define SPP_EMPTYPATHNOTALLOWED 1
// the Windows path (standard + UNC) does not exist, is inaccessible, or the user interrupted the test
// for path accessibility (this also includes an attempt to restore the network connection)
#define SPP_WINDOWSPATHERROR 2
// the Windows path starts with a file name that is not an archive (otherwise it would be an archive path)
#define SPP_NOTARCHIVEFILE 3
// FS path - the plugin FS name (fs-name - before ':' in the path) is not known (no plugin
// registered this name)
#define SPP_NOTPLUGINFS 4
// it is a relative path, but the current path is unknown or it is an FS path (there it is impossible to determine the root
// and we do not know the structure of the fs-user-part path at all, so it is impossible to convert it to an absolute path)
// if the current path is an FS path ('curPathIsDiskOrArchive' is FALSE), no error is reported to the user in this case
// (further processing is expected on the FS side that called the SalParsePath method)
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
#define SALCOL_ICON_BLEND_SELECTED 16 // colors for blending icons
#define SALCOL_ICON_BLEND_FOCUSED 17
#define SALCOL_ICON_BLEND_FOCSEL 18
#define SALCOL_PROGRESS_FG_NORMAL 19 // progress bar colors
#define SALCOL_PROGRESS_FG_SELECTED 20
#define SALCOL_PROGRESS_BK_NORMAL 21
#define SALCOL_PROGRESS_BK_SELECTED 22
#define SALCOL_HOT_PANEL 23           // color of the hot item in the panel
#define SALCOL_HOT_ACTIVE 24          //                   in the active window caption
#define SALCOL_HOT_INACTIVE 25        //                   in the inactive caption, status bar, ...
#define SALCOL_ACTIVE_CAPTION_FG 26   // caption text color in the active panel title
#define SALCOL_ACTIVE_CAPTION_BK 27   // caption background color in the active panel title
#define SALCOL_INACTIVE_CAPTION_FG 28 // caption text color in the inactive panel title
#define SALCOL_INACTIVE_CAPTION_BK 29 // caption background color in the inactive panel title
#define SALCOL_VIEWER_FG_NORMAL 30    // text color in the internal text/hex viewer
#define SALCOL_VIEWER_BK_NORMAL 31    // background color in the internal text/hex viewer
#define SALCOL_VIEWER_FG_SELECTED 32  // selected text color in the internal text/hex viewer
#define SALCOL_VIEWER_BK_SELECTED 33  // selected background color in the internal text/hex viewer
#define SALCOL_THUMBNAIL_NORMAL 34    // pen colors for the frame around a thumbnail
#define SALCOL_THUMBNAIL_SELECTED 35
#define SALCOL_THUMBNAIL_FOCUSED 36
#define SALCOL_THUMBNAIL_FOCSEL 37

// reasons why CSalamanderGeneralAbstract::ChangePanelPathToXXX methods reported failure:
#define CHPPFR_SUCCESS 0 // the panel contains the new path, success (return value is TRUE)
// the new path (or archive name) cannot be converted from relative to absolute or
// the new path (or archive name) is inaccessible or
// the FS path cannot be opened (no plugin, it refuses to load, it refuses to open the FS, fatal ChangePath error)
#define CHPPFR_INVALIDPATH 1
#define CHPPFR_INVALIDARCHIVE 2  // the file is not an archive or the archive cannot be listed
#define CHPPFR_CANNOTCLOSEPATH 4 // the current path cannot be closed
// the panel contains a shortened new path,
// clarification for FS: the panel contains either the shortened new path or the original path or the shortened
// original path - the original path is returned to the panel only if the new path was being opened
// in the current FS (method IsOurPath returned TRUE for it) and if the new path is inaccessible
// (nor any of its subpaths)
#define CHPPFR_SHORTERPATH 5
// the panel contains a shortened new path; the reason for shortening was that the requested path was a file name
// file - the panel contains the path to the file and the file will be focused
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
// the default configuration (save is not called when 'regKey' == NULL); 'registry' is the object for
// working with the registry; 'param' is the user parameter of the function (see
// CSalamanderGeneral::CallLoadOrSaveConfiguration)
typedef void(WINAPI* FSalLoadOrSaveConfiguration)(BOOL load, HKEY regKey,
                                                  CSalamanderRegistryAbstract* registry, void* param);

// base structure for CSalamanderGeneralAbstract::ViewFileInPluginViewer (each plugin
// viewer can extend this structure with its parameters - the structure is passed to
// CPluginInterfaceForViewerAbstract::ViewFile - the parameters can be for example the window caption,
// viewer mode, offset from the beginning of the file, selection position, etc.); WARNING!!! about packing
// structures (4-byte packing is required - see "#pragma pack(4)")
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

// konstanty typu parametru konfigurace Salamandera (viz CSalamanderGeneralAbstract::GetConfigParameter)
#define SALCFGTYPE_NOTFOUND 0 // parameter not found
#define SALCFGTYPE_BOOL 1     // TRUE/FALSE
#define SALCFGTYPE_INT 2      // 32-bit integer
#define SALCFGTYPE_STRING 3   // null-terminated multibyte string
#define SALCFGTYPE_LOGFONT 4  // Win32 LOGFONT structure

// konstanty parametru konfigurace Salamandera (viz CSalamanderGeneralAbstract::GetConfigParameter);
// v komentari je uveden typ parametru (BOOL, INT, STRING), za STRING je v zavorce potrebna
// velikost bufferu pro retezec
//
// general parameters
#define SALCFG_SELOPINCLUDEDIRS 1        // BOOL, select/deselect operations (num *, num +, num -) work also with directories
#define SALCFG_SAVEONEXIT 2              // BOOL, save configuration on Salamander exit
#define SALCFG_MINBEEPWHENDONE 3         // BOOL, should it beep (play sound) after end of work in inactive window?
#define SALCFG_HIDEHIDDENORSYSTEMFILES 4 // BOOL, should it hide system and/or hidden files?
#define SALCFG_ALWAYSONTOP 6             // BOOL, main window is Always On Top?
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
#define SALCFG_SORTBYEXTDIRSASFILES 19   // BOOL, should it treat dirs as files when sorting by extension? BTW, if TRUE, directories extensions are also displayed in separated Ext column. (directories have no extensions, only files have extensions, but many people have requested sort by extension and displaying extension in separated Ext column even for directories)
#define SALCFG_SIZEFORMAT 20             // INT, units for custom size columns, 0 - Bytes, 1 - KB, 2 - short (mixed B, KB, MB, GB, ...)
#define SALCFG_SELECTWHOLENAME 21        // BOOL, should be whole name selected (including extension) when entering new filename? (for dialog boxes F2:QuickRename, Alt+F5:Pack, etc)
// recycle bin parameters
#define SALCFG_USERECYCLEBIN 50   // INT, 0 - do not use, 1 - use for all, 2 - use for files matching at \
                                  //      least one of masks (see SALCFG_RECYCLEBINMASKS)
#define SALCFG_RECYCLEBINMASKS 51 // STRING (MAX_PATH), masks for SALCFG_USERECYCLEBIN==2
// time resolution of file compare (used in command Compare Directories)
#define SALCFG_COMPDIRSUSETIMERES 60 // BOOL, should it use time resolution? (FALSE==exact match)
#define SALCFG_COMPDIRTIMERES 61     // INT, time resolution for file compare (from 0 to 3600 second)
// confirmations
#define SALCFG_CNFRMFILEDIRDEL 70 // BOOL, files or directories delete
#define SALCFG_CNFRMNEDIRDEL 71   // BOOL, non-empty directory delete
#define SALCFG_CNFRMFILEOVER 72   // BOOL, file overwrite
#define SALCFG_CNFRMSHFILEDEL 73  // BOOL, system or hidden file delete
#define SALCFG_CNFRMSHDIRDEL 74   // BOOL, system or hidden directory delete
#define SALCFG_CNFRMSHFILEOVER 75 // BOOL, system or hidden file overwrite
#define SALCFG_CNFRMCREATEPATH 76 // BOOL, show "do you want to create target path?" in Copy/Move operations
#define SALCFG_CNFRMDIROVER 77    // BOOL, directory overwrite (copy/move selected directory: ask user if directory already exists on target path - standard behaviour is to join contents of both directories)
// drive specific settings
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
#define SALCFG_IFPATHISINACCESSIBLEGOTO 99 // STRING (MAX_PATH), path where to go if path in panel is inaccessible
// internal text/hex viewer
#define SALCFG_VIEWEREOLCRLF 120          // BOOL, accept CR-LF ("\r\n") line ends?
#define SALCFG_VIEWEREOLCR 121            // BOOL, accept CR ("\r") line ends?
#define SALCFG_VIEWEREOLLF 122            // BOOL, accept LF ("\n") line ends?
#define SALCFG_VIEWEREOLNULL 123          // BOOL, accept NULL ("\0") line ends?
#define SALCFG_VIEWERTABSIZE 124          // INT, size of tab ("\t") character in spaces
#define SALCFG_VIEWERSAVEPOSITION 125     // BOOL, TRUE = save position of viewer window, FALSE = always use position of main window
#define SALCFG_VIEWERFONT 126             // LOGFONT, viewer font
#define SALCFG_VIEWERWRAPTEXT 127         // BOOL, wrap text (divide long text line to more lines)
#define SALCFG_AUTOCOPYSELTOCLIPBOARD 128 // BOOL, TRUE = when user selects some text, this text is instantly copied to the cliboard
// archivers
#define SALCFG_ARCOTHERPANELFORPACK 140    // BOOL, should it pack to other panel path?
#define SALCFG_ARCOTHERPANELFORUNPACK 141  // BOOL, should it unpack to other panel path?
#define SALCFG_ARCSUBDIRBYARCFORUNPACK 142 // BOOL, should it unpack to subdirectory named by archive?
#define SALCFG_ARCUSESIMPLEICONS 143       // BOOL, should it use simple icons in archives?

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

    // returns TRUE if searching can start (pattern and flags were set successfully;
    // failure threatens only when the pattern is empty)
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

    // returns the regular-expression text (valid only after a successful call to Set)
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
#define sctyForConnectedDrivesAndFS 5        // for connected volumes and file systems (e.g. Disconnect)

// Salamander commands used by CSalamanderGeneralAbstract::EnumSalamanderCommands
// and CSalamanderGeneralAbstract::PostSalamanderCommand
// (WARNING: the reserved command-number range is <0, 499>)
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
#define SALCMD_DELETE 43        // delete (Delete in the panel)
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

// identifiers of shared histories (most recently used values in combo boxes) for
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
    // the '|' character can be used as a separator; following masks (again separated by ';')
    // are evaluated inversely, so if a name matches them,
    // AgreeMasks returns FALSE; the '|' character may stand at the start of the string
    //
    //   Examples:
    //     *.txt;*.cpp - all names with the txt or cpp extension
    //     *.h*|*.html - all names whose extension starts with 'h', but not with the "html" extension
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
    // initializes the object, it is called automatically in the constructor
    // the method is published for multiple use of the allocated object
    virtual void WINAPI Init() = 0;

    // updates the internal state of the object based on the data block specified by the 'input' variable,
    // 'input_length' specifies the size of the buffer in bytes
    virtual void WINAPI Update(const void* input, DWORD input_length) = 0;

    // prepares the MD5 value for retrieval using the GetDigest method
    // after calling the Finalize method you can call only the GetDigest() and Init() methods
    virtual void WINAPI Finalize() = 0;

    // retrieves the MD5, 'dest' must point to a buffer with a size of 16 bytes
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

    // displays a window with the parent 'hParent' that prompts for entering the master password
    // returns TRUE if the correct MP was entered, otherwise returns FALSE
    // asks even if the master password was already entered in this session, see IsMasterPasswordSet()
    // if the user does not use a master password, it returns FALSE, see IsUsingMasterPassword()
    virtual BOOL WINAPI AskForMasterPassword(HWND hParent) = 0;

    // reads a null-terminated 'plainPassword' and, based on the 'encrypt' variable, either encrypts it with AES (if TRUE) or
    // merely scrambles it (if FALSE); it stores the allocated result in 'encryptedPassword' and returns its size in the variable
    // 'encryptedPasswordSize'; returns TRUE on success, otherwise FALSE
    // if 'encrypt'==TRUE, the caller must ensure that the master password is entered before calling the function, see AskForMasterPassword()
    // note: the returned 'encryptedPassword' is allocated on Salamander's heap; if the plugin does not use salrtl, it must release the buffer
    // using SalamanderGeneral->Free(), otherwise calling free() is sufficient;
    virtual BOOL WINAPI EncryptPassword(const char* plainPassword, BYTE** encryptedPassword, int* encryptedPasswordSize, BOOL encrypt) = 0;

    // reads 'encryptedPassword' of the size 'encryptedPasswordSize' and converts it to a plaintext password returned
    // in the allocated buffer 'plainPassword'; returns TRUE on success, otherwise FALSE
    // note: the returned 'plainPassword' is allocated on Salamander's heap; if the plugin does not use salrtl, it must release the buffer
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

    // returns in 'retValue' (must not be NULL) the free space on the given path (so far the most accurate
    // value obtainable from Windows; on NT/W2K/XP/Vista it can work with reparse points
    // and SUBST drives (Salamander 2.5 handles junction points only)); 'path' is the path for which the free space
    // is queried (it does not have to be the root); if 'total' is not NULL, it returns the total disk size there;
    // if an error occurs, it returns CQuadWord(-1, -1)
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
    // mozne volat z libovolneho threadu
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
    // creates a file/directory (depending on 'file') on the path 'path' (NULL -> Windows TEMP dir),
    // with the prefix 'prefix'; returns the name of the created file in 'tmpName' (minimum size MAX_PATH),
    // returns success (on failure it returns the Windows error code in 'err' if not NULL)
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

    // opens the file/directory 'name' on the path 'path'; it follows Windows associations and opens
    // via the Open item in the context menu (it can also use salopen.exe depending on configuration);
    // before starting it sets the current directories on local drives according to the panel;
    // 'parent' is the parent of any dialogs (for example when opening an unassociated file)
    // limitation: main thread (otherwise salopen.exe would not work - it uses one shared memory)
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
    // 2) we can use the converted mask to test whether the file filename matches it,
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
    // 2) we can use the converted mask to test whether the file filename matches it,
    //    hasExtension = TRUE if the file has an extension
    //    returns TRUE if the file matches the mask
    virtual BOOL WINAPI AgreeExtMask(const char* filename, const char* mask, BOOL hasExtension) = 0;

    // allocates a new object for working with a group of file masks
    // can be called from any thread
    virtual CSalamanderMaskGroup* WINAPI AllocSalamanderMaskGroup() = 0;

    // releases an object for working with a group of file masks (obtained using AllocSalamanderMaskGroup)
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
    // deallocates memory from Salamander's heap (unnecessary when using salrtl9.dll - classic free is enough)
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

    // converts the string 'str' to lower/upper case; unlike the ANSI C tolower/toupper it works
    // directly with the string and supports not only characters 'A' to 'Z' (conversion to lower case is performed via
    // the array initialized by the Win32 API function CharLower)
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
    // sorting by the Windows regional settings is used; otherwise it compares the same way as
    // CSalamanderGeneral::StrICmp. If SALCFG_SORTDETECTNUMBERS is TRUE, it uses numerical sorting
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
    // also be NULL); 'bufferSize' is the size of the 'buffer' (if 'buffer' is NULL, this must be zero);
    // if 'type' is not NULL it points to a variable where the path type is stored (see PATH_TYPE_XXX);
    // if it is an archive and 'archiveOrFS' is not NULL and 'buffer' is not NULL, 'archiveOrFS' returns a pointer
    // into 'buffer' positioned after the archive file name;
    // if it is a file system and 'archiveOrFS' is not NULL and 'buffer' is not NULL, 'archiveOrFS' returns a pointer
    // into 'buffer' positioned at the ':' after the file-system name (the user part of the file-system path follows ':');
    // if 'convertFSPathToExternal' is TRUE and the panel contains a file-system path, the plugin whose path
    // (by fs-name) it is gets located and its CPluginInterfaceForFSAbstract::ConvertPathToExternal() is called;
    // returns success (if 'bufferSize'==0, it is also considered a failure when the path does not fit into the 'buffer')
    // limitation: main thread
    virtual BOOL WINAPI GetPanelPath(int panel, char* buffer, int bufferSize, int* type,
                                     char** archiveOrFS, BOOL convertFSPathToExternal = FALSE) = 0;

    // returns the last visited Windows path in the panel, useful for returning from an FS (more pleasant than
    // jumping straight to the fixed drive); 'panel' is one of PANEL_XXX; 'buffer' is the buffer for the path;
    // 'bufferSize' is the size of the 'buffer'; returns success
    // limitation: main thread
    virtual BOOL WINAPI GetLastWindowsPanelPath(int panel, char* buffer, int bufferSize) = 0;

    // returns the FS name assigned to the plugin "for life" by Salamander (according to SetBasicPluginData);
    // 'buf' is a buffer at least MAX_PATH characters long; 'fsNameIndex' is the FS-name index (index zero
    // for the FS-name provided in CSalamanderPluginEntryAbstract::SetBasicPluginData, the indices of other
    // FS-names are returned by CSalamanderPluginEntryAbstract::AddFSName)
    // limitation: main thread (otherwise the plugin configuration may change during the call),
    // can be called in the entry point only after SetBasicPluginData, earlier it may not be known
    virtual void WINAPI GetPluginFSName(char* buf, int fsNameIndex) = 0;

    // returns the interface of the plugin file system (FS) opened in the panel 'panel' (one of PANEL_XXX);
    // if no FS is open in the panel or it belongs to another plugin (not the caller), the method returns
    // NULL (objects of another plugin cannot be accessed, their structure is unknown)
    // limitation: main thread
    virtual CPluginFSInterfaceAbstract* WINAPI GetPanelPluginFS(int panel) = 0;

    // returns the plugin data interface of the panel listing (can also be NULL), 'panel' is one of PANEL_XXX;
    // if the plugin data interface exists but does not belong to this (calling) plugin, the method returns
    // NULL (objects of another plugin cannot be accessed, their structure is unknown)
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
    // they will not be refreshed (viewers do not change disk contents). Optional (may cause unnecessary refresh)
    // can be called from any thread
    virtual void WINAPI SkipOneActivateRefresh() = 0;

    // selects/deselects a panel item; 'file' is a pointer to the item being changed obtained by a previous
    // "get-item" call (GetPanelFocusedItem, GetPanelItem, or GetPanelSelectedItem);
    // the plugin must not be left after the "get-item" call and this method must run in the main
    // thread (to avoid refreshing the panel and invalidating the pointer); 'panel' must match
    // the 'panel' parameter of the corresponding "get-item" call; if 'select' is TRUE the item is selected,
    // otherwise it is deselected; after the last call RepaintChangedItems('panel') must be used to
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

    // returns the position of the source panel (is it on the left or on the right?), returns PANEL_LEFT or PANEL_RIGHT
    // limitation: main thread
    virtual int WINAPI GetSourcePanel() = 0;

    // determines which panel has 'pluginFS' open; if it is not open in any panel,
    // returns FALSE; if it returns TRUE, the panel number is stored in 'panel' (PANEL_LEFT or PANEL_RIGHT)
    // limitation: main thread (otherwise the panel contents may change)
    virtual BOOL WINAPI GetPanelWithPluginFS(CPluginFSInterfaceAbstract* pluginFS, int& panel) = 0;

    // aktivuje druhy panel (ala klavesa TAB); panely oznacene pres PANEL_SOURCE a PANEL_TARGET
    // se tim prirozene prohazuji
    // omezeni: hlavni thread
    virtual void WINAPI ChangePanel() = 0;

    // converts a number to a "more readable" string (space every three digits), returns the string in
    // 'buffer' (minimum size 50 bytes), returns 'buffer'
    // can be called from any thread
    virtual char* WINAPI NumberToStr(char* buffer, const CQuadWord& number) = 0;

    // prints disk size into 'buf' (minimum buffer size is 100 bytes),
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

    // Returns TRUE if the path 'prefix' is the base of the path 'path'. Otherwise returns FALSE.
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
    // if 'path' does not end with a backslash yet, adds it at the end of 'path'; 'path' is a buffer
    // with at least 'pathSize' characters; returns TRUE if the backslash fit after 'path'; if 'path'
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
    // changes/adds the extension 'extension' (e.g. ".txt") in the name 'path'; 'path' is a buffer
    // with at least 'pathSize' characters; returns FALSE if the 'path' buffer is insufficient for the resulting path
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

    // refreshes Salamander's DefaultDir array based on the panel paths; if 'activePrefered' is TRUE,
    // the path in the active panel takes precedence (written later to DefaultDir), otherwise
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

    // posts a message to the panel that it should refresh the path (reloads the listing and
    // transfers the selection, icons, focus, etc. to the new panel content); the refresh is performed when
    // Salamander's main window becomes active (when suspend mode ends); disk
    // and FS paths are always reloaded, archive paths are reloaded only if the archive file changed
    // (size & time check); 'panel' is one of PANEL_XXX; if 'focusFirstNewItem' is TRUE and only one item
    // was added to the panel, that new item is focused (used, for example, to focus a newly created file/directory)
    // can be called from any thread (if the main thread is not executing code inside the plugin,
    // the refresh happens as soon as possible; otherwise it waits at least until the main
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

    // closes a detached plugin FS (if allowed, see CPluginFSInterfaceAbstract::TryCloseOrDetach),
    // 'detachedFS' is the detached plugin FS; returns TRUE on success (FALSE means the detached
    // plugin FS was not closed); 'parent' is the parent of any message boxes (currently only
    // CPluginFSInterfaceAbstract::ReleaseObject can open them)
    // Note: a plugin FS opened in the panel is closed for example via ChangePanelPathToRescuePathOrFixedDrive
    // limitation: main thread + outside of CPluginFSInterfaceAbstract methods (we attempt to close a
    // detached FS - 'this' might cease to exist for the method)
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
    // (num is the width of the variable, a numeric value from 1 to 9999), "$(var_name:max)" ("max" is a
    // symbol indicating that the variable width is governed by the value in the 'maxVarWidths' array, see
    // ExpandVarString for details) and "$[env_var]" (expands the value of an environment variable); used when the
    // user can specify a string format (such as in the info line). Example of a string with variables:
    // "$(files) files and $(dirs) directories" - variables 'files' and 'dirs';
    // source code for use in the info line (without variables of the form "$(varname:max)") is in DEMOPLUG
    //
    // checks the syntax of 'varText' (string with variables), returns FALSE if it finds an error; its
    // position is stored in 'errorPos1' (offset of the start of the erroneous part) and 'errorPos2' (offset of the end of the
    // erroneous part); 'variables' is an array of CSalamanderVarStrEntry structures terminated by a structure with
    // Name==NULL; 'msgParent' is the parent of the error message box; if it is NULL, errors are not displayed
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

    // determines whether there is a high chance (cannot be guaranteed) that Salamander will not be "busy"
    // in the next few moments (no modal dialog open and no message being processed) – returns TRUE in that case
    // (otherwise FALSE); if 'lastIdleTime' is not NULL, it returns the GetTickCount() value from the moment of the last
    // transition from the "idle" to the "busy" state; it can serve as a prediction for delivering a command posted via
    // CSalamanderGeneralAbstract::PostMenuExtCommand with 'waitForSalIdle' == TRUE;
    // can be called from any thread
    virtual BOOL WINAPI SalamanderIsNotBusy(DWORD* lastIdleTime) = 0;

    // sets the message that should be displayed by the Bug Report dialog if a crash occurs inside the plugin
    // (inside the plugin = at least one call-stack message stored from the plugin) and allows redefining the default
    // bug-report e-mail address (support@altap.cz); 'message' is the new message (NULL means "no message");
    // 'email' is the new e-mail address (NULL means "use the default"; maximum length is 100 characters);
    // the method can be called repeatedly – the previous message and e-mail are overwritten;
    // Salamander does not remember the message or e-mail between runs
    // e-mail, takze je nutne tuto metodu vzdy pri loadu pluginu (nejlepe v entry-pointu) znovu
    // zavolat
    // omezeni: hlavni thread (jinak muze dojit ke zmenam v konfiguraci pluginu behem volani)
    virtual void WINAPI SetPluginBugReportInfo(const char* message, const char* email) = 0;

    // zjisti jestli je plugin nainstalovan (ovsem nezjisti, jestli se da naloadit - jestli ho
    // user napr. nesmazal jen z disku); 'pluginSPL' identifikuje plugin - jde o pozadovanou
    // koncovou cast plne cesty .SPL souboru pluginu (napr. "ieviewer\\ieviewer.spl" identifikuje
    // IEViewer shipped with Salamander); returns TRUE if the plugin is installed
    // limitation: main thread (otherwise the plugin configuration could change during the call)
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

    // tries to access the Windows path 'path' (normal or UNC); the test runs in a worker thread so it
    // can be interrupted by pressing ESC (after some time a window appears informing about ESC)
    // 'echo' TRUE enables displaying an error message if the path is inaccessible;
    // when 'err' is different from ERROR_SUCCESS in combination with 'echo' TRUE, it only displays the error (the path
    // is no longer accessed); 'parent' is the parent of the message box; returns ERROR_SUCCESS when the path is fine,
    // otherwise returns the standard Windows error code or ERROR_USER_TERMINATED if the user pressed ESC to abort the test
    // limitation: main thread (repeated calls are not allowed and the main thread uses this method)
    virtual DWORD WINAPI SalCheckPath(BOOL echo, const char* path, DWORD err, HWND parent) = 0;

    // checks whether the Windows path 'path' is accessible and, if needed, restores network connections (for
    // a normal path it tries to revive remembered network connections; for a UNC path it allows login with a new user name
    // and password); returns TRUE if the path is accessible; 'parent' is the parent of message boxes and dialogs;
    // 'tryNet' is TRUE if attempting to restore network connections makes sense (if FALSE it degrades to SalCheckPath;
    // provided only for optimization)
    // limitation: main thread (repeated calls are not allowed and the main thread uses this method)
    virtual BOOL WINAPI SalCheckAndRestorePath(HWND parent, const char* path, BOOL tryNet) = 0;

    // a more complex variant of SalCheckAndRestorePath; tests whether the Windows path 'path' is accessible and optionally
    // shortens it; if 'tryNet' is TRUE, it also attempts to restore the network connection and sets 'tryNet' to FALSE
    // (for a normal path it tries to revive remembered network connections; for a UNC path it allows login with a new
    // user name and password); if 'donotReconnect' is TRUE, it only determines the error and does not perform the reconnection;
    // returns 'err' (the Windows error code for the current path), 'lastErr' (the error code that led to the path shortening),
    // 'pathInvalid' (TRUE if an attempt to restore the network connection failed), 'cut' (TRUE if the resulting path is shortened);
    // 'parent' is the parent of the message box; returns TRUE if the resulting 'path' is accessible
    // limitation: main thread (repeated calls are not allowed and the main thread uses this method)
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

    // obtains the existing part and the operation mask from a target path; recognizes any missing part; on success returns TRUE,
    // the relative path to create (in 'newDirs'), the existing target path (in 'path'; it exists only if the relative path 'newDirs'
    // is created), and the detected operation mask (in 'mask' – points inside the 'path' buffer, but the path and mask are
    // separated by a null character; if the path does not contain a mask, a "*.*" mask is created automatically); 'parent' is
    // the parent of any message boxes; 'title' and 'errorTitle' are the titles of informational and error message boxes;
    // 'selCount' is the number of selected files and directories; 'path' is the target path to process on input and, on output
    // (at least 2 * MAX_PATH characters), the existing target path (always ending with a backslash); 'afterRoot' points within
    // 'path' just after the root (after '\\' or at the end); 'secondPart' points within 'path' behind the existing part (after '\\'
    // or at the end; if the path contains a file, it points after that file's path); 'pathIsDir' is TRUE/FALSE depending on
    // whether the existing part is a directory/file; 'backslashAtEnd' is TRUE if 'path' ended with a backslash before parsing
    // (e.g. SalParsePath removes that backslash); 'dirName' and 'curPath' are not NULL if at most one file/directory is selected
    // (its name without the path is in 'dirName'; its path is in 'curPath'; if nothing is selected, the focus is used);
    // 'mask' is the output pointer to the operation mask within the 'path' buffer; if 'newDirs' is not NULL, it points to a buffer
    // (at least MAX_PATH in size) for the relative path (relative to the existing path in 'path') that needs to be created (the user
    // agreed to create it; the same prompt as for disk-to-disk copying is used; empty string = create nothing); if 'newDirs' is NULL
    // and a relative path needs to be created, only an error is displayed; 'isTheSamePathF' compares two paths (needed only when
    // 'curPath' is not NULL); if NULL, IsTheSamePath is used; if the path contains an error, the method returns FALSE – the problem
    // has already been reported to the user
    // can be called from any thread
    virtual BOOL WINAPI SalSplitGeneralPath(HWND parent, const char* title, const char* errorTitle,
                                            int selCount, char* path, char* afterRoot, char* secondPart,
                                            BOOL pathIsDir, BOOL backslashAtEnd, const char* dirName,
                                            const char* curPath, char*& mask, char* newDirs,
                                            SGP_IsTheSamePathF isTheSamePathF) = 0;

    // removes ".." (skips ".." together with the preceding subdirectory) and "." (skips just ".")
    // from a path; the condition is that backslash is used as the subdirectory separator; 'afterRoot'
    // points behind the root of the processed path (modifications are performed only after 'afterRoot');
    // returns TRUE if the adjustments succeeded, FALSE if ".." cannot be removed (the root is on the left)
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

    // zobrazi/schova zpravu v okenku ve vlastnim threadu (neodcerpa message-queue); zobrazi
    // najednou jen jednu zpravu, opakovane volani ohlasi chybu do TRACE (neni fatalni);
    // POZN.: pouziva se v SalCheckPath a dalsich rutinach, muze tedy dojit ke kolizi mezi
    //        pozadavky na otevreni okenek (neni fatalni, jen se nezobrazi)
    // vse je mozne volat z libovolneho threadu (ale okenko se musi obsluhovat jen
    // z jednoho threadu - nelze jej ukazat z jednoho threadu a schovat z druheho)
    //
    // otevreni okenka s textem 'message' se zpodenim 'delay' (v ms), jen pokud je 'hForegroundWnd' NULL
    // nebo identifikuje okno na popredi (foreground)
    // 'message' muze byt vicerakova; jednotlive radky se oddeluji znakem '\n'
    // 'caption' muze byt NULL: pouzije se pak "Open Salamander"
    // 'showCloseButton' udava, zda bude okenko obsahovat tlacitko Close; ekvivalent ke klavese Escape
    virtual void WINAPI CreateSafeWaitWindow(const char* message, const char* caption,
                                             int delay, BOOL showCloseButton, HWND hForegroundWnd) = 0;
    // zavreni okenka
    virtual void WINAPI DestroySafeWaitWindow() = 0;
    // schovani/zobrazeni okenka (je-li otevrene); volat jako reakci na WM_ACTIVATE z okna hForegroundWnd:
    //    case WM_ACTIVATE:
    //    {
    //      ShowSafeWaitWindow(LOWORD(wParam) != WA_INACTIVE);
    //      break;
    //    }
    // Pokud je thread (ze ktereho bylo okenko vytvoreno) zamestnan, nedochazi
    // k distribuci zprav, takze nedojde ani k doruceni WM_ACTIVATE pri kliknuti
    // na jinou aplikaci. Zpravy se doruci az ve chvili zobrazeni messageboxu,
    // coz presne potrebujeme: docasne se nechame schovat a pozdeji (po zavreni
    // messageboxu a aktivaci okna hForegroundWnd) zase zobrazit.
    virtual void WINAPI ShowSafeWaitWindow(BOOL show) = 0;
    // po zavolani CreateSafeWaitWindow nebo ShowSafeWaitWindow vraci funkce FALSE az do doby,
    // kdy user kliknul mysi na Close tlacitko (pokud je zobrazeno); pak vraci TRUE
    virtual BOOL WINAPI GetSafeWaitWindowClosePressed() = 0;
    // slouzi pro dodatecnou zmenu textu v okenku
    // POZOR: nedochazi k novemu layoutovani okna a pokud dojde k vetsimu natazeni
    // textu, bude orezan; pouzivat napriklad pro countdown: 60s, 55s, 50s, ...
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

    // odemkne zamek kopie souboru v disk-cache (nastavi 'fileLock' do stavu signaled, vyzve
    // disk-cache k provedeni kontroly zamku, a pak nastavi 'fileLock' zpet do stavu nonsignaled);
    // pokud slo o posledni zamek, muze dojit ke zruseni kopie, kdy dojde ke zruseni kopie zalezi
    // na velikosti disk-cache na disku; zamek muze byt pouzity i pro vice kopii souboru (zamek
    // musi byt typu "manual reset", jinak se po odemknuti prvni kopie zamek nastavi do stavu
    // nonsignaled a odemykani skonci), v tomto pripade probehne odemknuti u vsech kopii
    // mozne volat z libovolneho threadu
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

    // odstrani z disk-cache kopii souboru, jejiz unikatni nazev je 'uniqueFileName' (POZOR: nazev
    // se porovnava "case-sensitive", pokud plugin vyzaduje "case-insensitive", musi vsechny nazvy
    // prevadet napr. na mala pismena - viz CSalamanderGeneralAbstract::ToLowerCase); pokud se kopie
    // souboru jeste pouziva, odstrani se az to bude mozne (az se zavrou viewery), kazdopadne uz ji
    // disk-cache nikomu neposkytne jako platnou kopii souboru (je oznacena jako out-of-date)
    // mozne volat z libovolneho threadu
    virtual void WINAPI RemoveOneFileFromCache(const char* uniqueFileName) = 0;

    // odstrani z disk-cache vsechny kopie souboru, jejichz unikatni nazvy zacinaji na 'fileNamesRoot'
    // (pouziva se pri uzavreni file-systemu, kdyz uz je dale nezadouci cachovat downloadene kopie
    // souboru; POZOR: nazvy se porovnavaji "case-sensitive", pokud plugin vyzaduje "case-insensitive",
    // musi vsechny nazvy prevadet napr. na mala pismena - viz CSalamanderGeneralAbstract::ToLowerCase);
    // pokud se kopie souboru jeste pouzivaji, odstrani se az to bude mozne (az se odemknou
    // napr. po zavreni vieweru), kazdopadne uz je disk-cache nikomu neposkytne jako platne kopie
    // souboru (jsou oznacene jako out-of-date)
    // mozne volat z libovolneho threadu
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

    // vraci jmeno kodove stranky pouzivane ve Windows v tomto regionu (cerpa v convert\XXX\convert.cfg
    // v instalaci Salamandera); jde o normalne zobrazitelne kodovani, proto se pouziva v pripade,
    // ze je potreba zobrazit text, ktery byl vytvoren v jine kodove strance (zde se zada jako
    // "cilove" kodovani pri hledani konverzni tabulky, viz metoda GetConversionTable);
    // 'parent' je parent messageboxu (je-li NULL, je parent hlavni okno); 'codePage' je buffer
    // (min. 101 znaku) pro jmeno kodove stranky (pokud neni v souboru convert\XXX\convert.cfg toto jmeno
    // definovane, vraci se v bufferu prazdny retezec)
    // mozne volat z libovolneho threadu
    virtual void WINAPI GetWindowsCodePage(HWND parent, char* codePage) = 0;

    // zjisti z bufferu 'pattern' o delce 'patternLen' (napr. prvnich 10000 znaku) jestli jde
    // o text (existuje kodova stranka, ve ktere obsahuje jen povolene znaky - zobrazitelne
    // a ridici) a pokud jde o text, zjisti take jeho kodovou stranku (nejpravdepodobnejsi);
    // 'parent' je parent messageboxu (je-li NULL, je parent hlavni okno); je-li 'forceText'
    // TRUE, neprovadi se kontrola na nepovolene znaky (pouziva se, pokud 'pattern' obsahuje
    // text); neni-li 'isText' NULL, vraci se v nem TRUE pokud jde o text; neni-li 'codePage'
    // NULL, jde o buffer (min. 101 znaku) pro jmeno kodove stranky (nejpravdepodobnejsi)
    // mozne volat z libovolneho threadu
    virtual void WINAPI RecognizeFileType(HWND parent, const char* pattern, int patternLen, BOOL forceText,
                                          BOOL* isText, char* codePage) = 0;

    // zjisti z bufferu 'text' o delce 'textLen' jestli jde o ANSI text (obsahuje (v ANSI
    // znakove sade) jen povolene znaky - zobrazitelne a ridici); rozhoduje bez kontextu
    // (nezalezi na poctu znaku ani jak jdou po sobe - testovany text je mozne rozdelit
    // na libovolne casti a testovat je postupne); vraci TRUE pokud jde o ANSI text (jinak
    // je obsah bufferu 'text' binarni)
    // mozne volat z libovolneho threadu
    virtual BOOL WINAPI IsANSIText(const char* text, int textLen) = 0;

    // zavola funkci 'callback' s parametry 'param' a funkci pro ziskavani oznacenych
    // souboru/adresaru (viz definice typu SalPluginOperationFromDisk) z panelu 'panel'
    // (v panelu musi byt otevrena windowsova cesta); 'panel' je jeden z PANEL_XXX
    // omezeni: hlavni thread
    virtual void WINAPI CallPluginOperationFromDisk(int panel, SalPluginOperationFromDisk callback,
                                                    void* param) = 0;

    // vraci standardni charset, ktery ma uzivatel nastaveny (soucast regionalniho
    // nastaveni); fonty je nutne konstruovat s timto charsetem, jinak nemusi byt
    // texty citelne (je-li text ve standardni kodove strance, viz Win32 API funkce
    // GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDEFAULTANSICODEPAGE, ...))
    // mozne volat z libovolneho threadu
    virtual BYTE WINAPI GetUserDefaultCharset() = 0;

    // alokuje novy objekt Boyer-Moorova vyhledavaciho algoritmu
    // mozne volat z libovolneho threadu
    virtual CSalamanderBMSearchData* WINAPI AllocSalamanderBMSearchData() = 0;

    // uvolni objekt Boyer-Moorova vyhledavaciho algoritmu (ziskany metodou AllocSalamanderBMSearchData)
    // mozne volat z libovolneho threadu
    virtual void WINAPI FreeSalamanderBMSearchData(CSalamanderBMSearchData* data) = 0;

    // alokuje novy objekt algoritmu pro vyhledavani pomoci regularnich vyrazu
    // mozne volat z libovolneho threadu
    virtual CSalamanderREGEXPSearchData* WINAPI AllocSalamanderREGEXPSearchData() = 0;

    // uvolni objekt algoritmu pro vyhledavani pomoci regularnich vyrazu (ziskany metodou
    // AllocSalamanderREGEXPSearchData)
    // mozne volat z libovolneho threadu
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

    // nastavi volajicimu pluginu priznak, ze se ma pri nejblizsi mozne prilezitosti spustit
    // prikaz Salamandera s cislem 'salCmd' (jakmile nebudou v message-queue hl. threadu zadne
    // message a Salamander nebude "busy" (nebude otevreny zadny modalni dialog a nebude se
    // zpracovavat zadna zprava));
    // POZOR: pokud se vola z jineho nez hlavniho threadu, muze dojit ke spusteni prikazu Salamandera
    // (probiha v hlavnim threadu) dokonce drive nez skonci PostSalamanderCommand
    // mozne volat z libovolneho threadu
    virtual void WINAPI PostSalamanderCommand(int salCmd) = 0;

    // nastavi priznak "uzivatel pracoval s aktualni cestou" v panelu 'panel' (tento priznak
    // se vyuziva pri plneni seznamu pracovnich adresaru - List Of Working Directories (Alt+F12));
    // 'panel' je jeden z PANEL_XXX
    // omezeni: hlavni thread
    virtual void WINAPI SetUserWorkedOnPanelPath(int panel) = 0;

    // v panelu 'panel' (jedna z konstant PANEL_XXX) ulozi vybrana (Selected) jmena
    // do zvlastniho pole, ze ktereho muze uzivatel vyber obnovit prikazem Edit/Restore Selection
    // vyuziva se pro prikazy, ktere zrusi aktualni vyber, aby mel uzivatel moznost
    // se k nemu vratit a provest jeste dalsi operaci
    // omezeni: hlavni thread
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

    // alokuje novy objekt pro vypocet MD5
    // mozne volat z libovolneho threadu
    virtual CSalamanderMD5* WINAPI AllocSalamanderMD5() = 0;

    // uvolni objekt pro vypocet MD5 (ziskany metodou AllocSalamanderMD5)
    // mozne volat z libovolneho threadu
    virtual void WINAPI FreeSalamanderMD5(CSalamanderMD5* md5) = 0;

    // V textu nalezne pary '<' '>', vyradi je z bufferu a prida odkazy na
    // jejich obsah do 'varPlacements'. 'varPlacements' je pole DWORDu o '*varPlacementsCount'
    // polozkach, DWORDy jsou slozene vzdy z pozice odkazu ve vystupnim bufferu (spodni WORD)
    // a poctu znaku odkazu (horni WORD). Retezce "\<", "\>", "\\" jsou chapany
    // jako escape sekvence a budou nahrazeny znaky '<', '>' a '\\'.
    // Vraci TRUE v pripade uspechu, jinak FALSE; vzdy nastavi 'varPlacementsCount' na
    // pocet zpracovanych promennych.
    // mozne volat z libovolneho threadu
    virtual BOOL WINAPI LookForSubTexts(char* text, DWORD* varPlacements, int* varPlacementsCount) = 0;

    // cekani (maximalne 0.2 sekundy) na pusteni klavesy ESC; pouziva se pokud plugin obsahuje
    // akce, ktere se prerusuji klavesou ESC (monitorovani klavesy ESC pres
    // GetAsyncKeyState(VK_ESCAPE)) - zabranuje tomu, aby se po stisknuti ESC v dialogu/messageboxu
    // hned prerusila i nasledujici akce monitorujici klavesu ESC
    // mozne volat z libovolneho threadu
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
    //   If the default window position lies on the primary monitor or some error occured,
    //   the return value is FALSE and you should use CreateWindow with CW_USEDEFAULT in
    //   the 'x' parameter.
    //
    //   Otherwise the return value is TRUE and coordinates from 'p' structure should be used
    //   in the CreateWindow 'x' and 'y' parameters.
    //
    // Remarks
    //   Method can be called from any thread.
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
    //      [in] Handle to the window of the interest. If this parameter is NULL,
    //      or window is not visible or is iconic, monitor with the currently active window
    //      from the same application will be used. Otherwise primary monitor will be used.
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
    //   Centers the window against a specified window or monitor.
    //
    // Parameters
    //   'hWindow'
    //      [in] Handle to the window whose parent window handle is to be retrieved.
    //
    //   'hByWnd'
    //      [in] Handle to the window against which to center. If this parameter is NULL,
    //      or window is not visible or is iconic, the method will center 'hWindow' against
    //      the working area of the monitor. Monitor with the currently active window
    //      from the same application will be used. Otherwise primary monitor will be used.
    //
    //   'findTopWindow'
    //      [in] If this parameter is TRUE, non child visible window will be used by walking
    //      the chain of parent windows of 'hByWnd' as the window against which to center.
    //
    //      If this parameter is FALSE, 'hByWnd' will be to the window against which to center.
    //
    // Remarks
    //   If centered window gets over working area of the monitor, the method positions
    //   the window to be whole visible.
    //
    //   Method can be called from any thread.
    //
    virtual void WINAPI MultiMonCenterWindow(HWND hWindow, HWND hByWnd, BOOL findTopWindow) = 0;

    //
    // MultiMonEnsureRectVisible
    //   Ensures that specified rectangle is either entirely or partially visible,
    //   adjusting the coordinates if necessary. All monitors are considered.
    //
    // Parameters
    //   'rect'
    //      [in/out] Pointer to the RECT structure that contain the coordinated to be
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
    //   Installs special word break procedure to the specified window. This procedure
    //   is inteded for easier cursor movevement in the single line edit controls.
    //   Delimiters '\\', '/', ' ', ';', ',', and '.' are used as cursor stops when user
    //   navigates using Ctrl+Left or Ctrl+Right keys.
    //   You can use Ctrl+Backspace to delete one word.
    //
    // Parameters
    //   'hWindow'
    //      [in] Handle to the window or control where word break proc is to be isntalled.
    //      Window may be either edit or combo box with edit control.
    //
    // Return Values
    //   The return value is TRUE if the word break proc is installed. It is FALSE if the
    //   window is neither edit nor combo box with edit control, some error occured, or
    //   this special word break proc is not supported on your OS.
    //
    // Remarks
    //   You needn't uninstall word break procedure before window is destroyed.
    //
    //   Method can be called from any thread.
    //
    virtual BOOL WINAPI InstallWordBreakProc(HWND hWindow) = 0;

    // Salamander 3 nebo novejsi: vraci TRUE, pokud byla tato instance Altap
    // Salamandera prvni spustena (v dobe startu instance se hledaji dalsi bezici
    // instance verze 3 nebo novejsi);
    //
    // Poznamky k ruznym SID / Session / Integrity Level (netyka se Salamandera 2.5 a 2.51):
    // funkce vrati TRUE i v pripade, ze jiz bezi instance Salamandera spustena
    // pod jinym SID; na session a integrity level nezalezi, takze pokud jiz bezi
    // instance Salamandera na jine session, pripadne s jinym integrity level, ale
    // se shodnym SID, vrati nove spustena instance FALSE
    //
    // mozne volat z libovolneho threadu
    virtual BOOL WINAPI IsFirstInstance3OrLater() = 0;

    // support for parameter dependent strings (dealing with singles/plurals);
    // 'format' is format string for resulting string - its description follows;
    // resulting string is copied to 'buffer' buffer which size is 'bufferSize' bytes;
    // 'parametersArray' is array of parameters; 'parametersCount' is count of
    // these parameters; returns length of the resulting string
    //
    // format string description:
    //   - each format string starts with signature "{!}"
    //   - format string can contain following escape sequences (it allows to use special
    //     character without its special meaning): "\\" = "\", "\{" = "{", "\}" = "}",
    //     "\:" = ":", and "\|" = "|" (do not forget to double backslashes when writing C++
    //     strings, this applies only to format strings placed directly in C++ source code)
    //   - text which is not placed in curly brackets goes directly to resulting string
    //     (only escape sequences are handled)
    //   - parameter dependent text is placed in curly brackets
    //   - each parameter dependent text uses one parameter from 'parametersArray'
    //     (it is 64-bit unsigned int)
    //   - parameter dependent text contains more variants of resulting text, which variant
    //     is used depends on parameter value, more precisely to which defined interval the
    //     value belongs
    //   - variants of resulting text and interval bounds are separated by "|" character
    //   - first interval is from 0 to first interval bound
    //   - last interval is from last interval bound plus one to infinity (2^64-1)
    //   - parameter dependent text "{}" is used to skip one parameter from 'parametersArray'
    //     (nothing goes to resulting string)
    //   - you can also specify index of parameter to use for parameter dependent text,
    //     just place its index (from one to number of parameters) to the beginning of
    //     parameter dependent text and follow it by ':' character
    //   - if you don't specify index of parameter to use, it is assigned automatically
    //     (starting from one to number of parameters)
    //   - if you specify index of parameter to use, the next index which is assigned
    //     automatically is not affected,
    //     e.g. in "{!}%d file{2:s|0||1|s} and %d director{y|1|ies}" the first parameter
    //     dependent text uses parameter with index 2 and second uses parameter with index 1
    //   - you can use any number of parameter dependent texts with specified index
    //     of parameter to use
    //
    // examples of format strings:
    //   - "{!}director{y|1|ies}": for parameter values from 0 to 1 resulting string will be
    //     "directory" and for parameter values from 2 to infinity (2^64-1) resulting string
    //     will be "directories"
    //   - "{!}%d soubor{u|0||1|y|4|u} a %d adresar{u|0||1|e|4|u}": it needs two parameters
    //     because there are two dependent texts in curly brackets, resulting string for
    //     choosen pairs of parameters (I believe it is not needed to show all possible variants):
    //       0, 0: "%d souboru a %d adresaru"
    //       1, 12: "%d soubor a %d adresaru"
    //       3, 4: "%d soubory a %d adresare"
    //       13, 1: "%d souboru a %d adresar"
    //
    // method can be called from any thread
    virtual int WINAPI ExpandPluralString(char* buffer, int bufferSize, const char* format,
                                          int parametersCount, const CQuadWord* parametersArray) = 0;

    // v aktualni jazykove verzi Salamandera pripravi retezec "XXX (selected/hidden)
    // files and YYY (selected/hidden) directories"; je-li XXX (hodnota parametru 'files')
    // nebo YYY (hodnota parametru 'dirs') nula, prislusna cast retezce se vypousti (oba
    // parametry zaroven nulove se neuvazuji); pouziti "selected" a "hidden" zavisi
    // na rezimu 'mode' - viz popis konstant epfdmXXX; vysledny text
    // se vraci v bufferu 'buffer' o velikosti 'bufferSize' bytu; vraci delku vysledneho
    // textu; 'forDlgCaption' je TRUE/FALSE pokud je/neni text urceny pro titulek dialogu
    // (v anglictine nutna velka pocatecni pismena)
    // mozne volat z libovolneho threadu
    virtual int WINAPI ExpandPluralFilesDirs(char* buffer, int bufferSize, int files, int dirs,
                                             int mode, BOOL forDlgCaption) = 0;

    // v aktualni jazykove verzi Salamandera pripravi retezec "BBB bytes in XXX selected
    // files and YYY selected directories"; BBB je hodnota parametru 'selectedBytes';
    // je-li XXX (hodnota parametru 'files') nebo YYY (hodnota parametru 'dirs') nula,
    // prislusna cast retezce se vypousti (oba parametry zaroven nulove se neuvazuji);
    // je-li 'useSubTexts' TRUE, uzavorkuje se BBB do '<' a '>', aby se s BBB dalo
    // dale pracovat na info-line (viz metoda CSalamanderGeneralAbstract::LookForSubTexts a
    // CPluginDataInterfaceAbstract::GetInfoLineContent); vysledny text se vraci v bufferu
    // 'buffer' o velikosti 'bufferSize' bytu; vraci delku vysledneho textu
    // mozne volat z libovolneho threadu
    virtual int WINAPI ExpandPluralBytesFilesDirs(char* buffer, int bufferSize,
                                                  const CQuadWord& selectedBytes, int files, int dirs,
                                                  BOOL useSubTexts) = 0;

    // vraci retezec popisujici s cim se pracuje (napr. "file "test.txt"" nebo "directory "test""
    // nebo "3 files and 1 directory"); 'sourceDescr' je buffer pro vysledek o velikosti
    // minimalne 'sourceDescrSize'; 'panel' popisuje zdrojovy panel operace (jedna z PANEL_XXX nebo -1
    // pokud operace nema zdrojovy panel (napr. CPluginFSInterfaceAbstract::CopyOrMoveFromDiskToFS));
    // 'selectedFiles'+'selectedDirs' - pokud ma operace zdrojovy panel, je zde pocet oznacenych
    // souboru a adresaru ve zdrojovem panelu, pokud jsou obe hodnoty nulove, pracuje se se
    // souborem/adresarem pod kurzorem (fokusem); 'selectedFiles'+'selectedDirs' - pokud operace nema
    // zdrojovy panel, je zde pocet souboru/adresaru, se kterymi operace pracuje;
    // 'fileOrDirName'+'isDir' - pouziva se jen pokud operace nema zdrojovy panel a pokud
    // 'selectedFiles + selectedDirs == 1', je zde jmeno souboru/adresare a jestli jde o soubor
    // nebo adresar ('isDir' je FALSE nebo TRUE); 'forDlgCaption' je TRUE/FALSE pokud je/neni
    // text urceny pro titulek dialogu (v anglictine nutna velka pocatecni pismena)
    // omezeni: hlavni thread (muze pracovat s panelem)
    virtual void WINAPI GetCommonFSOperSourceDescr(char* sourceDescr, int sourceDescrSize,
                                                   int panel, int selectedFiles, int selectedDirs,
                                                   const char* fileOrDirName, BOOL isDir,
                                                   BOOL forDlgCaption) = 0;

    // nakopiruje retezec 'srcStr' za retezec 'dstStr' (za jeho koncovou nulu);
    // 'dstStr' je buffer o velikosti 'dstBufSize' (musi byt nejmene rovno 2);
    // pokud se do bufferu oba retezce nevejdou, jsou zkraceny (vzdy tak, aby se
    // veslo co nejvice znaku z obou retezu)
    // mozne volat z libovolneho threadu
    virtual void WINAPI AddStrToStr(char* dstStr, int dstBufSize, const char* srcStr) = 0;

    // zjisti jestli je mozne retezec 'fileNameComponent' pouzit jako komponentu
    // jmena na Windows filesystemu (resi retezce delsi nez MAX_PATH-4 (4 = "C:\"
    // + null-terminator), prazdny retezec, retezce znaku '.', retezce white-spaces,
    // znaky "*?\\/<>|\":" a jednoducha jmena typu "prn" a "prn  .txt")
    // mozne volat z libovolneho threadu
    virtual BOOL WINAPI SalIsValidFileNameComponent(const char* fileNameComponent) = 0;

    // pretvori retezec 'fileNameComponent' tak, aby mohl byt pouzity jako komponenta
    // jmena na Windows filesystemu (resi retezce delsi nez MAX_PATH-4 (4 = "C:\"
    // + null-terminator), resi prazdny retezec, retezce znaku '.', retezce
    // white-spaces, znaky "*?\\/<>|\":" nahradi '_' + jednoducha jmena typu "prn"
    // a "prn  .txt" doplni o '_' na konci nazvu); 'fileNameComponent' musi jit
    // rozsirit alespon o jeden znak (maximalne se vsak z 'fileNameComponent'
    // pouzije MAX_PATH bytu)
    // mozne volat z libovolneho threadu
    virtual void WINAPI SalMakeValidFileNameComponent(char* fileNameComponent) = 0;

    // vraci TRUE pokud je zdroj enumerace panel, v 'panel' pak vraci PANEL_LEFT nebo
    // PANEL_RIGHT; pokud nebyl zdroj enumerace nalezen nebo jde o Find okno, vraci FALSE;
    // 'srcUID' je unikatni identifikator zdroje (predava se jako parametr pri otevirani
    // vieweru nebo jej lze ziskat volanim GetPanelEnumFilesParams)
    // mozne volat z libovolneho threadu
    virtual BOOL WINAPI IsFileEnumSourcePanel(int srcUID, int* panel) = 0;

    // vraci dalsi jmeno souboru pro viewer ze zdroje (levy/pravy panel nebo Findy);
    // 'srcUID' je unikatni identifikator zdroje (predava se jako parametr pri otevirani
    // vieweru nebo jej lze ziskat volanim GetPanelEnumFilesParams); 'lastFileIndex'
    // (nesmi byt NULL) je IN/OUT parametr, ktery by plugin mel menit jen pokud chce vratit
    // jmeno prvniho souboru, v tomto pripade nastavit 'lastFileIndex' na -1; pocatecni
    // hodnota 'lastFileIndex' se predava jako parametr jak pri otevirani vieweru, tak
    // pri volani GetPanelEnumFilesParams; 'lastFileName' je plne jmeno aktualniho souboru
    // (prazdny retezec pokud neni zname, napr. je-li 'lastFileIndex' -1); je-li
    // 'preferSelected' TRUE a aspon jedno jmeno oznacene, budou se vracet oznacena jmena;
    // je-li 'onlyAssociatedExtensions' TRUE, vraci jen soubory s priponou asociovanou s
    // viewerem tohoto pluginu (F3 na tomto souboru by se pokusilo otevrit viewer tohoto
    // pluginu + ignoruje pripadne zastineni viewerem jineho pluginu); 'fileName' je buffer
    // pro ziskane jmeno (velikost alespon MAX_PATH); vraci TRUE pokud se podari jmeno
    // ziskat; vraci FALSE pri chybe: zadne dalsi jmeno souboru ve zdroji neni (neni-li
    // 'noMoreFiles' NULL, vraci se v nem TRUE), zdroj je zaneprazdnen (nezpracovava zpravy;
    // neni-li 'srcBusy' NULL, vraci se v nem TRUE), jinak zdroj prestal existovat (zmena
    // cesty v panelu, atp.);
    // mozne volat z libovolneho threadu; POZOR: pouziti z hlavniho threadu nedava smysl
    // (Salamander je pri volani metody pluginu zaneprazdeny, takze vzdy vrati FALSE + TRUE
    // v 'srcBusy')
    virtual BOOL WINAPI GetNextFileNameForViewer(int srcUID, int* lastFileIndex, const char* lastFileName,
                                                 BOOL preferSelected, BOOL onlyAssociatedExtensions,
                                                 char* fileName, BOOL* noMoreFiles, BOOL* srcBusy) = 0;

    // vraci predchozi jmeno souboru pro viewer ze zdroje (levy/pravy panel nebo Findy);
    // 'srcUID' je unikatni identifikator zdroje (predava se jako parametr pri otevirani
    // vieweru nebo jej lze ziskat volanim GetPanelEnumFilesParams); 'lastFileIndex' (nesmi
    // byt NULL) je IN/OUT parametr, ktery by plugin mel menit jen pokud chce vratit jmeno
    // posledniho souboru, v tomto pripade nastavit 'lastFileIndex' na -1; pocatecni hodnota
    // 'lastFileIndex' se predava jako parametr jak pri otevirani vieweru, tak pri volani
    // GetPanelEnumFilesParams; 'lastFileName' je plne jmeno aktualniho souboru (prazdny
    // retezec pokud neni zname, napr. je-li 'lastFileIndex' -1); je-li 'preferSelected'
    // TRUE a aspon jedno jmeno oznacene, budou se vracet oznacena jmena; je-li
    // 'onlyAssociatedExtensions' TRUE, vraci jen soubory s priponou asociovanou s viewerem
    // tohoto pluginu (F3 na tomto souboru by se pokusilo otevrit viewer tohoto
    // pluginu + ignoruje pripadne zastineni viewerem jineho pluginu); 'fileName' je buffer
    // pro ziskane jmeno (velikost alespon MAX_PATH); vraci TRUE pokud se podari jmeno
    // ziskat; vraci FALSE pri chybe: zadne predchozi jmeno souboru ve zdroji neni (neni-li
    // 'noMoreFiles' NULL, vraci se v nem TRUE), zdroj je zaneprazdnen (nezpracovava zpravy;
    // neni-li 'srcBusy' NULL, vraci se v nem TRUE), jinak zdroj prestal existovat (zmena
    // cesty v panelu, atp.)
    // mozne volat z libovolneho threadu; POZOR: pouziti z hlavniho threadu nedava smysl
    // (Salamander je pri volani metody pluginu zaneprazdeny, takze vzdy vrati FALSE + TRUE
    // v 'srcBusy')
    virtual BOOL WINAPI GetPreviousFileNameForViewer(int srcUID, int* lastFileIndex, const char* lastFileName,
                                                     BOOL preferSelected, BOOL onlyAssociatedExtensions,
                                                     char* fileName, BOOL* noMoreFiles, BOOL* srcBusy) = 0;

    // zjisti, jestli je aktualni soubor z vieweru oznaceny (selected) ve zdroji (levy/pravy
    // panel nebo Findy); 'srcUID' je unikatni identifikator zdroje (predava se jako parametr
    // pri otevirani vieweru nebo jej lze ziskat volanim GetPanelEnumFilesParams); 'lastFileIndex'
    // je parametr, ktery by plugin nemel menit, pocatecni hodnota 'lastFileIndex' se predava
    // jako parametr jak pri otevirani vieweru, tak pri volani GetPanelEnumFilesParams;
    // 'lastFileName' je plne jmeno aktualniho souboru; vraci TRUE pokud se podarilo zjistit,
    // jestli je aktualni soubor oznaceny, vysledek je v 'isFileSelected' (nesmi byt NULL);
    // vraci FALSE pri chybe: zdroj prestal existovat (zmena cesty v panelu, atp.) nebo soubor
    // 'lastFileName' uz ve zdroji neni (pri techto dvou chybach, neni-li 'srcBusy' NULL,
    // vraci se v nem FALSE), zdroj je zaneprazdnen (nezpracovava zpravy; pri teto chybe,
    // neni-li 'srcBusy' NULL, vraci se v nem TRUE)
    // mozne volat z libovolneho threadu; POZOR: pouziti z hlavniho threadu nedava smysl
    // (Salamander je pri volani metody pluginu zaneprazdeny, takze vzdy vrati FALSE + TRUE
    // v 'srcBusy')
    virtual BOOL WINAPI IsFileNameForViewerSelected(int srcUID, int lastFileIndex,
                                                    const char* lastFileName,
                                                    BOOL* isFileSelected, BOOL* srcBusy) = 0;

    // nastavi oznaceni (selectionu) na aktualnim souboru z vieweru ve zdroji (levy/pravy
    // panel nebo Findy); 'srcUID' je unikatni identifikator zdroje (predava se jako parametr
    // pri otevirani vieweru nebo jej lze ziskat volanim GetPanelEnumFilesParams);
    // 'lastFileIndex' je parametr, ktery by plugin nemel menit, pocatecni hodnota
    // 'lastFileIndex' se predava jako parametr jak pri otevirani vieweru, tak pri volani
    // GetPanelEnumFilesParams; 'lastFileName' je plne jmeno aktualniho souboru; 'select'
    // je TRUE/FALSE pokud se ma aktualni soubor oznacit/odznacit; vraci TRUE pri uspechu;
    // vraci FALSE pri chybe: zdroj prestal existovat (zmena cesty v panelu, atp.) nebo
    // soubor 'lastFileName' uz ve zdroji neni (pri techto dvou chybach, neni-li 'srcBusy'
    // NULL, vraci se v nem FALSE), zdroj je zaneprazdnen (nezpracovava zpravy; pri teto
    // chybe, neni-li 'srcBusy' NULL, vraci se v nem TRUE)
    // mozne volat z libovolneho threadu; POZOR: pouziti z hlavniho threadu nedava smysl
    // (Salamander je pri volani metody pluginu zaneprazdeny, takze vzdy vrati FALSE + TRUE
    // v 'srcBusy')
    virtual BOOL WINAPI SetSelectionOnFileNameForViewer(int srcUID, int lastFileIndex,
                                                        const char* lastFileName, BOOL select,
                                                        BOOL* srcBusy) = 0;

    // vraci odkaz na sdilenou historii (posledne pouzite hodnoty) zvoleneho comboboxu;
    // jde o pole alokovanych retezcu; pole ma pevny pocet retezcu, ten se vraci
    // v 'historyItemsCount' (nesmi byt NULL); odkaz na pole se vraci v 'historyArr'
    // (nesmi byt NULL); 'historyID' (jedna v SALHIST_XXX) urcuje, na kterou sdilenou historii se ma vracet
    // odkaz
    // omezeni: hlavni thread (sdilene historie se v jinem threadu nedaji pouzivat, pristup
    // do nich neni nijak synchronizovany)
    virtual BOOL WINAPI GetStdHistoryValues(int historyID, char*** historyArr, int* historyItemsCount) = 0;

    // prida do sdilene historie ('historyArr'+'historyItemsCount') naalokovanou kopii
    // nove hodnoty 'value'; je-li 'caseSensitiveValue' TRUE, hleda se hodnota (retezec)
    // v poli historie pomoci case-sensitive porovnani (FALSE = case-insensitive porovnani),
    // nalezena hodnota se pouze presouva na prvni misto v poli historie
    // omezeni: hlavni thread (sdilene historie se v jinem threadu nedaji pouzivat, pristup
    // do nich neni nijak synchronizovany)
    // POZNAMKA: pokud se pouziva pro jine nez sdilene historie, je mozne volat v libovolnem threadu
    virtual void WINAPI AddValueToStdHistoryValues(char** historyArr, int historyItemsCount,
                                                   const char* value, BOOL caseSensitiveValue) = 0;

    // prida do comboboxu ('combo') texty ze sdilene historie ('historyArr'+'historyItemsCount');
    // pred pridavanim provede reset obsahu comboboxu (viz CB_RESETCONTENT)
    // omezeni: hlavni thread (sdilene historie se v jinem threadu nedaji pouzivat, pristup
    // do nich neni nijak synchronizovany)
    // POZNAMKA: pokud se pouziva pro jine nez sdilene historie, je mozne volat v libovolnem threadu
    virtual void WINAPI LoadComboFromStdHistoryValues(HWND combo, char** historyArr, int historyItemsCount) = 0;

    // zjisti barevnou hloubku aktualniho zobrazeni a je-li vice nez 8-bit (256 barev), vraci TRUE
    // mozne volat z libovolneho threadu
    virtual BOOL WINAPI CanUse256ColorsBitmap() = 0;

    // zkontroluje jestli je enablovany-root-parent okna 'parent' foreground window, pokud ne,
    // udela se FlashWindow(root-parent okna 'parent', TRUE) a vrati root-parent okna 'parent',
    // jinak se vraci NULL
    // POUZITI:
    //    HWND mainWnd = GetWndToFlash(parent);
    //    CDlg(parent).Execute();
    //    if (mainWnd != NULL) FlashWindow(mainWnd, FALSE);  // pod W2K+ uz asi neni potreba: flashovani se musi odstranit rucne
    // mozne volat z libovolneho threadu
    virtual HWND WINAPI GetWndToFlash(HWND parent) = 0;

    // provede reaktivaci drop-targetu (po dropnuti pri drag&dropu) po otevreni naseho progress-
    // -okna (to se pri otevreni aktivuje, cimz deaktivuje drop-target); neni-li 'dropTarget'
    // NULL a zaroven nejde o panel v tomto Salamanderovi, provede aktivaci 'progressWnd' a nasledne
    // aktivaci nejvzdalenejsiho enablovaneho predka 'dropTarget' (tato kombinace nas zbavi aktivovaneho
    // stavu bez aktivni aplikace, ktery jinak obcas vznika)
    // mozne volat z libovolneho threadu
    virtual void WINAPI ActivateDropTarget(HWND dropTarget, HWND progressWnd) = 0;

    // naplanuje otevreni Pack dialogu s vybranym packerem z tohoho pluginu (viz
    // CSalamanderConnectAbstract::AddCustomPacker), pokud packer z tohoto pluginu
    // neexistuje (napr. protoze ho uzivatel smazal), vypise se userovi chybova
    // hlaska; dialog se otevre jakmile nebudou v message-queue hl. threadu zadne
    // zpravy a Salamander nebude "busy" (nebude otevreny zadny modalni dialog
    // a nebude se zpracovavat zadna zprava); opakovane volani teto metody pred
    // otevrenim Pack dialogu vede jen ke zmene parametru 'delFilesAfterPacking';
    // 'delFilesAfterPacking' ovlivnuje checkbox "Delete files after packing"
    // v Pack dialogu: 0=default, 1=zapnuty, 2=vypnuty
    // omezeni: hlavni thread
    virtual void WINAPI PostOpenPackDlgForThisPlugin(int delFilesAfterPacking) = 0;

    // naplanuje otevreni Unpack dialogu s vybranym unpackerem z tohoho pluginu (viz
    // CSalamanderConnectAbstract::AddCustomUnpacker), pokud unpacker z tohoto pluginu
    // neexistuje (napr. protoze ho uzivatel smazal), vypise se userovi chybova
    // hlaska; dialog se otevre jakmile nebudou v message-queue hl. threadu zadne
    // zpravy a Salamander nebude "busy" (nebude otevreny zadny modalni dialog
    // a nebude se zpracovavat zadna zprava); opakovane volani teto metody pred
    // otevrenim Unpack dialogu vede jen ke zmene parametru 'unpackMask';
    // 'unpackMask' ovlivnuje masku "Unpack files": NULL=default, jinak text masky
    // omezeni: hlavni thread
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

    // umozni odpojit/pripojit sledovani zmen (jen pro windows cesty a cesty do archivu)
    // na cestach prohlizenych v jednom z panelu; ucel: pokud vasemu kodu (formatovani
    // disku, shredovani disku, atp.) prekazi to, ze panel ma pro cestu otevreny handle
    // "ChangeNotification", touto metodou jej muzete docasne odpojit (po pripojeni se
    // vyvolava refresh pro cestu v panelu); 'panel' je jeden z PANEL_XXX; 'stopMonitoring'
    // je TRUE/FALSE (odpojeni/pripojeni)
    // omezeni: hlavni thread
    virtual void WINAPI PanelStopMonitoring(int panel, BOOL stopMonitoring) = 0;

    // alokuje novy objekt CSalamanderDirectory pro praci se soubory/adresari v archivu nebo
    // file-systemu; je-li 'isForFS' TRUE, je objekt prednastaven pro pouziti pro file-system,
    // jinak je objekt prednastaven pro pouziti pro archiv (defaultni flagy objektu se
    // lisi pro archiv a file-system, viz metoda CSalamanderDirectoryAbstract::SetFlags)
    // mozne volat z libovolneho threadu
    virtual CSalamanderDirectoryAbstract* WINAPI AllocSalamanderDirectory(BOOL isForFS) = 0;

    // uvolni objekt CSalamanderDirectory (ziskany metodou AllocSalamanderDirectory,
    // POZOR: nesmi se volat pro zadny jiny ukazatel CSalamanderDirectoryAbstract)
    // mozne volat z libovolneho threadu
    virtual void WINAPI FreeSalamanderDirectory(CSalamanderDirectoryAbstract* salDir) = 0;

    // prida novy timer pro objekt pluginoveho FS; az dojde k timeoutu timeru, zavola se metoda
    // CPluginFSInterfaceAbstract::Event() objektu pluginoveho FS 'timerOwner' s parametry
    // FSE_TIMER a 'timerParam'; 'timeout' je timeout timeru od jeho pridani (v milisekundach,
    // musi byt >= 0); timer se zrusi v okamziku sveho timeoutu (pred volanim
    // CPluginFSInterfaceAbstract::Event()) nebo pri zavreni objektu pluginoveho FS;
    // vraci TRUE, pokud byl timer uspesne pridan
    // omezeni: hlavni thread
    virtual BOOL WINAPI AddPluginFSTimer(int timeout, CPluginFSInterfaceAbstract* timerOwner,
                                         DWORD timerParam) = 0;

    // zrusi bud vsechny timery objektu pluginoveho FS 'timerOwner' (je-li 'allTimers' TRUE)
    // nebo jen vsechny timery s parametrem rovnym 'timerParam' (je-li 'allTimers' FALSE);
    // vraci pocet zrusenych timeru
    // omezeni: hlavni thread
    virtual int WINAPI KillPluginFSTimer(CPluginFSInterfaceAbstract* timerOwner, BOOL allTimers,
                                         DWORD timerParam) = 0;

    // zjistuje viditelnost polozky pro FS v Change Drive menu a v Drive barach; vraci TRUE,
    // pokud je polozka viditelna, jinak vraci FALSE
    // omezeni: hlavni thread (jinak muze dojit ke zmenam v konfiguraci pluginu behem volani)
    virtual BOOL WINAPI GetChangeDriveMenuItemVisibility() = 0;

    // nastavuje viditelnost polozky pro FS v Change Drive menu a v Drive barach; pouzivat
    // jen pri instalaci pluginu (jinak hrozi prenastaveni uzivatelem zvolene viditelnosti);
    // 'visible' je TRUE v pripade, ze polozka ma byt viditelna
    // omezeni: hlavni thread (jinak muze dojit ke zmenam v konfiguraci pluginu behem volani)
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
    //   Function retrieves handle to large or small icon from the specified object,
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
    //      [in] Indicate that 'path' is the address of an ITEMIDLIST structure rather
    //      than a path name.
    //
    //   'hIcon'
    //      [out] Pointer to icon handle that receive handle to the icon extracted
    //      from the object.
    //
    //   'iconSize'
    //      [in] Required size of icon. SALICONSIZE_xxx
    //
    //   'fallbackToDefIcon'
    //      [in] Value specifying whether the default (simple) icon should be used if
    //      the icon of the specified object is not available. If this parameter is
    //      TRUE, function tries to return the default (simple) icon in this situation.
    //      Otherwise, it returns no icon (return value is FALSE).
    //
    //   'defIconIsDir'
    //      [in] Specifies whether the default (simple) icon for 'path' is icon of
    //      directory. This parameter is ignored unless 'fallbackToDefIcon' is TRUE.
    //
    // Return Values
    //   Returns TRUE if successful, or FALSE otherwise.
    //
    // Remarks
    //   You are responsible for freeing returned icons with DestroyIcon when you
    //   no longer need them.
    //
    //   You must initialize COM with CoInitialize or OLEInitialize prior to
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
    // lze volat z libovolneho threadu
    virtual BOOL WINAPI FileExists(const char* fileName) = 0;

    // provede zmenu cesty v panelu na posledni znamou diskovou cestu, pokud neni pristupna,
    // tak se provede zmena na uzivatelem zvolenou "zachranou" cestu (viz
    // SALCFG_IFPATHISINACCESSIBLEGOTO) a pokud i ta selze, tak na root prvniho lokalniho
    // fixed drivu (Salamander 2.5 a 2.51 dela jen zmenu na root prvniho lokalniho fixed drivu);
    // pouziva se pro zavreni file-systemu v panelu (disconnect); 'parent' je parent pripadnych
    // messageboxu; 'panel' je jeden z PANEL_XXX
    // omezeni: hlavni thread + mimo metody CPluginFSInterfaceAbstract a CPluginDataInterfaceAbstract
    // (hrozi napr. zavreni FS otevreneho v panelu - metode by mohl prestat existovat 'this')
    virtual void WINAPI DisconnectFSFromPanel(HWND parent, int panel) = 0;

    // vraci TRUE, pokud je nazev souboru 'name' asociovan v Archives Associations in Panels
    // k volajicimu pluginu
    // 'name' musi byt pouze nazev souboru, ne s plnou nebo relativni cestou
    // omezeni: hlavni thread
    virtual BOOL WINAPI IsArchiveHandledByThisPlugin(const char* name) = 0;

    // slouzi jako LR_xxx parametr pro API funkci LoadImage()
    // pokud uzivatel nema zapnute hi-color ikony v konfiguraci desktopu,
    // vraci LR_VGACOLOR, aby nedoslo k chybnemu nacteni vice barevne verze ikony
    // jinak vraci 0 (LR_DEFAULTCOLOR); vysledek funkce lze orovat s dalsimi LR_xxx flagy
    // lze volat z libovolneho threadu
    virtual DWORD WINAPI GetIconLRFlags() = 0;

    // zjisti podle pripony souboru, jestli jde o link ("lnk", "pif" nebo "url"); 'fileExtension'
    // je pripona souboru (ukazatel za tecku), nesmi byt NULL; vraci 1 pokud jde o link, jinak
    // vraci 0; POZNAMKA: pouziva se pro plneni CFileData::IsLink
    // lze volat z libovolneho threadu
    virtual int WINAPI IsFileLink(const char* fileExtension) = 0;

    // vrati ILC_COLOR??? podle verze Windows - odladene pro pouziti imagelistu v listviewech
    // typicke pouziti: ImageList_Create(16, 16, ILC_MASK | GetImageListColorFlags(), ???, ???)
    // lze volat z libovolneho threadu
    virtual DWORD WINAPI GetImageListColorFlags() = 0;

    // "bezpecna" verze GetOpenFileName()/GetSaveFileName() resi situaci, kdy podana cesta
    // v OPENFILENAME::lpstrFile neni platna (napriklad z:\); v tomto pripade std. API verze
    // funkce neotevre okenko a tise se vrati z FALSE a CommDlgExtendedError() vraci FNERR_INVALIDFILENAME.
    // Nasledujici dve funkce v tomto pripade zavolaji API jeste jednou, ale s "bezpecne"
    // existujici cestou (Documents, pripadne Desktop).
    virtual BOOL WINAPI SafeGetOpenFileName(LPOPENFILENAME lpofn) = 0;
    virtual BOOL WINAPI SafeGetSaveFileName(LPOPENFILENAME lpofn) = 0;

    // plugin musi pred pouzitim OpenHtmlHelp() zadat Salamanderovi jmeno sveho .chm souboru
    // bez cesty (napr. "demoplug.chm")
    // lze volat z libovolneho threadu, ale je potreba vyloucit soucasne volani s OpenHtmlHelp()
    virtual void WINAPI SetHelpFileName(const char* chmName) = 0;

    // otevre HTML help pluginu, jazyk helpu (adresar s .chm soubory) vybira takto:
    // -adresar ziskany z aktualniho .slg souboru Salamandera (viz SLGHelpDir v shared\versinfo.rc)
    // -HELP\ENGLISH\*.chm
    // -prvni nalezeny podadresar v podadresari HELP
    // plugin musi pred pouzitim OpenHtmlHelp() zavolat SetHelpFileName(); 'parent' je parent
    // messageboxu s chybou; 'command' je prikaz HTML helpu, viz HHCDisplayXXX; 'dwData' je parametr
    // prikazu HTML helpu, viz HHCDisplayXXX
    // lze volat z libovolneho threadu
    // poznamka: zobrazeni helpu Salamandera viz OpenHtmlHelpForSalamander
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

    // postne panelu s aktivnim FS 'modifiedFS' zpravu o tom, ze by se mel
    // provest refresh cesty (znovu nacte listing a prenese oznaceni, ikony, fokus, atd. do
    // noveho obsahu panelu); refresh se provede az dojde k aktivaci hlavniho okna Salamandera
    // (az skonci suspend-mode); FS cesta se vzdycky nacte znovu; pokud 'modifiedFS' neni v zadnem
    // panelu, neprovede se nic; je-li 'focusFirstNewItem' TRUE a v panelu pribyla jen jedina
    // polozka, dojde k fokusu teto nove polozky (pouziva se napr. pro fokus nove vytvoreneho
    // souboru/adresare); vraci TRUE pokud se provedl refresh, FALSE pokud nebyl 'modifiedFS'
    // nalezen ani v jednom panelu
    // mozne volat z libovolneho threadu (pokud hlavni thread nespousti kod uvnitr pluginu,
    // probehne refresh co nejdrive, jinak refresh pocka minimalne do okamziku, kdy hlavni
    // thread opusti plugin)
    virtual BOOL WINAPI PostRefreshPanelFS2(CPluginFSInterfaceAbstract* modifiedFS,
                                            BOOL focusFirstNewItem = FALSE) = 0;

    // nacte z resourcu modulu 'module' text s ID 'resID'; vraci text v internim bufferu (hrozi
    // zmena textu diky zmene interniho bufferu zpusobene dalsimi volanimi LoadStr i z jinych
    // pluginu nebo Salamandera; buffer je velky 10000 znaku, prepis hrozi teprve po jeho
    // zaplneni (pouziva se cyklicky); pokud potrebujete text pouzit az pozdeji, doporucujeme
    // jej zkopirovat do lokalniho bufferu); je-li 'module' NULL nebo 'resID' neni v modulu,
    // vraci se text "ERROR LOADING STRING" (a debug/SDK verze vypise TRACE_E)
    // mozne volat z libovolneho threadu
    virtual char* WINAPI LoadStr(HINSTANCE module, int resID) = 0;

    // nacte z resourcu modulu 'module' text s ID 'resID'; vraci text v internim bufferu (hrozi
    // zmena textu diky zmene interniho bufferu zpusobene dalsimi volanimi LoadStrW i z jinych
    // pluginu nebo Salamandera; buffer je velky 10000 znaku, prepis hrozi teprve po jeho
    // zaplneni (pouziva se cyklicky); pokud potrebujete text pouzit az pozdeji, doporucujeme
    // jej zkopirovat do lokalniho bufferu); je-li 'module' NULL nebo 'resID' neni v modulu,
    // vraci se text L"ERROR LOADING WIDE STRING" (a debug/SDK verze vypise TRACE_E)
    // mozne volat z libovolneho threadu
    virtual WCHAR* WINAPI LoadStrW(HINSTANCE module, int resID) = 0;

    // zmena cesty v panelu na uzivatelem zvolenou "zachranou" cestu (viz
    // SALCFG_IFPATHISINACCESSIBLEGOTO) a pokud i ta selze, tak na root prvniho lokalniho fixed
    // drivu, jde o temer jistou zmenu aktualni cesty v panelu; 'panel' je jeden z PANEL_XXX;
    // neni-li 'failReason' NULL, nastavuje se na jednu z konstant CHPPFR_XXX (informuje o vysledku
    // metody); vraci TRUE pokud se zmena cesty podarila (na "zachranou" nebo fixed drive)
    // omezeni: hlavni thread + mimo metody CPluginFSInterfaceAbstract a CPluginDataInterfaceAbstract
    // (hrozi napr. zavreni FS otevreneho v panelu - metode by mohl prestat existovat 'this')
    virtual BOOL WINAPI ChangePanelPathToRescuePathOrFixedDrive(int panel, int* failReason = NULL) = 0;

    // prihlasi plugin jako nahradu za Network polozku v Change Drive menu a v Drive barach,
    // plugin musi pridavat do Salamandera file-system, na kterem se pak oteviraji nekompletni
    // UNC cesty ("\\" a "\\server") z prikazu Change Directory a na ktery se odchazi
    // pres symbol up-diru ("..") z rootu UNC cest;
    // omezeni: volat jen z entry-pointu pluginu a to az po SetBasicPluginData
    virtual void WINAPI SetPluginIsNethood() = 0;

    // otevre systemove kontextove menu pro oznacene polozky nebo fokuslou polozku na sitove ceste
    // ('forItems' je TRUE) nebo pro sitovou cestu ('forItems' je FALSE), vybrany prikaz z menu
    // take provede; menu se ziskava prochazenim slozky CSIDL_NETWORK; 'parent' je navrzeny parent
    // kontextoveho menu; 'panel' identifikuje panel (PANEL_LEFT nebo PANEL_RIGHT), pro ktery se
    // ma kontextove menu otevrit (z tohoto panelu se ziskavaji fokusle/oznacene soubory/adresare,
    // se kterymi se pracuje); 'menuX' + 'menuY' jsou navrzene souradnice leveho horniho rohu
    // kontextoveho menu; 'netPath' je sitova cesta, povolene jsou jen "\\" a "\\server"; neni-li
    // 'newlyMappedDrive' NULL, vraci se v nem pismenko ('A' az 'Z') nove namapovaneho disku (pres
    // prikaz Map Network Drive z kontextoveho menu), pokud se v nem vraci nula, k zadnemu novemu
    // mapovani nedoslo
    // omezeni: hlavni thread
    virtual void WINAPI OpenNetworkContextMenu(HWND parent, int panel, BOOL forItems, int menuX,
                                               int menuY, const char* netPath,
                                               char* newlyMappedDrive) = 0;

    // zdvojuje '\\' - hodi se pro texty, ktere posilame do LookForSubTexts, ktera '\\\\'
    // zase zredukuje na '\\'; 'buffer' je vstupne/vystupni retezec, 'bufferSize' je velikost
    // 'buffer' v bytech; vraci TRUE pokud zdvojenim nedoslo ke ztrate znaku z konce retezce
    // (buffer byl dost veliky)
    // mozne volat z libovolneho threadu
    virtual BOOL WINAPI DuplicateBackslashes(char* buffer, int bufferSize) = 0;

    // ukaze v panelu 'panel' throbber (animace informujici uzivatele o aktivite souvisejici
    // s panelem, napr. "nacitam data ze site") se zpozdenim 'delay' (v ms); 'panel' je jeden
    // z PANEL_XXX; neni-li 'tooltip' NULL, jde o text, ktery se ukaze po najeti mysi na
    // throbber (je-li NULL, zadny text se neukazuje); pokud je uz v panelu throbber zobrazeny
    // nebo ceka na zobrazeni, zmeni se jeho identifikacni cislo a tooltip (je-li zobrazeny,
    // 'delay' se ignoruje, ceka-li na zobrazeni, nastavi se nove zpozdeni podle 'delay');
    // vraci identifikacni cislo throbberu (nikdy neni -1, tedy -1 je mozne pouzit jako
    // prazdnou hodnotu + vsechna vracena cisla jsou unikatni, presneji receno opakovat se
    // zacnou po nerealnych 2^32 zobrazeni throbberu);
    // POZNAMKA: vhodne misto pro zobrazeni throbberu pro FS je prijem udalosti FSE_PATHCHANGED,
    // to uz je FS v panelu (jestli se ma nebo nema throbber zobrazit se muze urcit predem
    // v ChangePath nebo ListCurrentPath)
    // omezeni: hlavni thread
    virtual int WINAPI StartThrobber(int panel, const char* tooltip, int delay) = 0;

    // schova throbber s identifikacnim cislem 'id'; vraci TRUE pokud dojde ke schovani
    // throbberu; vraci FALSE pokud se jiz tento throbber schoval nebo se pres nej ukazal
    // jiny throbber;
    // POZNAMKA: throbber se automaticky schovava tesne pred zmenou cesty v panelu nebo
    // pred refreshem (u FS to znamena tesne po uspesnem volani ListCurrentPath, u archivu
    // je to po otevreni a vylistovani archivu, u disku je to po overeni pristupnosti cesty)
    // omezeni: hlavni thread
    virtual BOOL WINAPI StopThrobber(int id) = 0;

    // ukaze v panelu 'panel' ikonu zabezpeceni (zamknuty nebo odemknuty zamek, napr. u FTPS informuje
    // uzivatele o tom, ze je spojeni se serverem zabezpecene pomoci SSL a identita serveru je
    // overena (zamknuty zamek) nebo overena neni (odemknuty zamek)); 'panel' je jeden z PANEL_XXX;
    // je-li 'showIcon' TRUE, ikona se ukaze, jinak se schova; 'isLocked' urcuje, jestli jde
    // o zamknuty (TRUE) nebo odemknuty (FALSE) zamek; neni-li 'tooltip' NULL, jde o text, ktery se
    // ukaze po najeti mysi na ikonu (je-li NULL, zadny text se neukazuje); pokud se ma po kliknuti
    // na ikone zabezpeceni provest nejaka akce (napr. u FTPS se zobrazuje dialog s certifikatem
    // serveru), je nutne ji pridat do metody CPluginFSInterfaceAbstract::ShowSecurityInfo file-systemu
    // zobrazeneho v panelu;
    // POZNAMKA: vhodne misto pro zobrazeni ikony zabezpeceni pro FS je prijem udalosti
    // FSE_PATHCHANGED, to uz je FS v panelu (jestli se ma nebo nema ikona zobrazit se muze urcit
    // predem v ChangePath nebo ListCurrentPath)
    // POZNAMKA: ikona zabezpeceni se automaticky schovava tesne pred zmenou cesty v panelu nebo
    // pred refreshem (u FS to znamena tesne po uspesnem volani ListCurrentPath, u archivu
    // je to po otevreni a vylistovani archivu, u disku je to po overeni pristupnosti cesty)
    // omezeni: hlavni thread
    virtual void WINAPI ShowSecurityIcon(int panel, BOOL showIcon, BOOL isLocked,
                                         const char* tooltip) = 0;

    // odstrani aktualni cestu v panelu z historie adresaru zobrazenych v panelu (Alt+Left/Right)
    // a ze seznamu pracovnich cest (Alt+F12); pouziva se pro zneviditelneni prechodnych cest,
    // napr. "net:\Entire Network\Microsoft Windows Network\WORKGROUP\server\share" automaticky
    // prechazi na "\\server\share" a je nezadouci, aby se tento prechod delal pri pohybu v historii
    // omezeni: hlavni thread
    virtual void WINAPI RemoveCurrentPathFromHistory(int panel) = 0;

    // vracit TRUE, pokud je aktualni uzivatel clenem skupiny Administrators, jinak vraci FALSE
    // mozne volat z libovolneho threadu
    virtual BOOL WINAPI IsUserAdmin() = 0;

    // vraci TRUE, pokud Salamander bezi na vzdalene plose (RemoteDesktop), jinak vraci FALSE
    // mozne volat z libovolneho threadu
    virtual BOOL WINAPI IsRemoteSession() = 0;

    // ekvivalent volani WNetAddConnection2(lpNetResource, NULL, NULL, CONNECT_INTERACTIVE);
    // vyhodou je podrobnejsi zobrazeni chybovych stavu (napr. ze expirovalo heslo,
    // ze je spatne zadane heslo nebo jmeno, ze je potreba zmenit heslo, atd.)
    // mozne volat z libovolneho threadu
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

    // informuje Salamandera o tom, ze plugin pouziva Password Manager a tedy Salamander ma
    // pluginu hlasit zavedeni/zmenu/zruseni master passwordu (viz
    // CPluginInterfaceAbstract::PasswordManagerEvent)
    // omezeni: volat jen z entry-pointu pluginu a to az po SetBasicPluginData
    virtual void WINAPI SetPluginUsesPasswordManager() = 0;

    //
    // GetSalamanderPasswordManager
    //   Provides interface to Password Manager provided by Salamander.
    //
    // Remarks
    //   Method can be called from main thread only.
    virtual CSalamanderPasswordManagerAbstract* WINAPI GetSalamanderPasswordManager() = 0;

    // otevre HTML help samotneho Salamanadera (misto help pluginu, ktery se otevira pomoci OpenHtmlHelp()),
    // jazyk helpu (adresar s .chm soubory) vybira takto:
    // -adresar ziskany z aktualniho .slg souboru Salamandera (viz SLGHelpDir v shared\versinfo.rc)
    // -HELP\ENGLISH\*.chm
    // -prvni nalezeny podadresar v podadresari HELP
    // 'parent' je parent messageboxu s chybou; 'command' je prikaz HTML helpu, viz HHCDisplayXXX;
    // 'dwData' je parametr prikazu HTML helpu, viz HHCDisplayXXX; pokud je command==HHCDisplayContext,
    // musi byt hodnota 'dwData' z rodiny konstant HTMLHELP_SALID_XXX
    // lze volat z libovolneho threadu
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
    //   Locks main window to pretend it is disabled. Main windows is still able to receive focus
    //   in the locked state. Set 'lock' to TRUE to lock main window and to FALSE to revert it back
    //   to normal state. 'hToolWnd' is reserverd parameter, set it to NULL. 'lockReason' is (optional,
    //   can be NULL) describes the reason for main window locked state. It will be displayed during
    //   attempt to close locked main window; content of string is copied to internal structure
    //   so buffer can be deallocated after return from LockMainWindow().
    //
    // Remarks
    //   Method can be called from main thread only.
    virtual void WINAPI LockMainWindow(BOOL lock, HWND hToolWnd, const char* lockReason) = 0;

    // jen pro pluginy "dynamic menu extension" (viz FUNCTION_DYNAMICMENUEXT):
    // nastavi volajicimu pluginu priznak, ze se ma pri nejblizsi mozne prilezitosti
    // (jakmile nebudou v message-queue hl. threadu zadne message a Salamander nebude
    // "busy" (nebude otevreny zadny modalni dialog a nebude se zpracovavat zadna zprava))
    // znovu sestavit menu volanim metody CPluginInterfaceForMenuExtAbstract::BuildMenu;
    // POZOR: pokud se vola z jineho nez hlavniho threadu, muze dojit k volani BuildMenu
    // (probiha v hlavnim threadu) dokonce drive nez skonci PostPluginMenuChanged
    // mozne volat z libovolneho threadu
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

    // nase varianty funkci RegQueryValue a RegQueryValueEx, narozdil od API variant
    // zajistuji pridani null-terminatoru pro typy REG_SZ, REG_MULTI_SZ a REG_EXPAND_SZ
    // POZOR: pri zjistovani potrebne velikosti bufferu vraci o jeden nebo dva (dva
    //        jen u REG_MULTI_SZ) znaky vic pro pripad, ze by string bylo potreba
    //        zakoncit nulou/nulami
    // mozne volat z libovolneho threadu
    virtual LONG WINAPI SalRegQueryValue(HKEY hKey, LPCSTR lpSubKey, LPSTR lpData, PLONG lpcbData) = 0;
    virtual LONG WINAPI SalRegQueryValueEx(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved,
                                           LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData) = 0;

    // protoze windowsova verze GetFileAttributes neumi pracovat se jmeny koncicimi mezerou,
    // napsali jsme si vlastni (u techto jmen pridava backslash na konec, cimz uz pak
    // GetFileAttributes funguje spravne, ovsem jen pro adresare, pro soubory s mezerou na
    // konci reseni nemame, ale aspon se to nezjistuje od jineho souboru - windowsova verze
    // orizne mezery a pracuje tak s jinym souborem/adresarem)
    // mozne volat z libovolneho threadu
    virtual DWORD WINAPI SalGetFileAttributes(const char* fileName) = 0;

    // zatim neexistuje Win32 API pro detekci SSD, takze se jejich detekovani provadi heuristikou
    // na zaklade dotazu na podporu pro TRIM, StorageDeviceSeekPenaltyProperty, atd
    // funkce vraci TRUE, pokud disk na ceste 'path' vypada jako SSD; FALSE jindy
    // vysledek neni 100%, lide hlasi nefunkcnost algoritmu napriklad na SSD PCIe kartach:
    // http://stackoverflow.com/questions/23363115/detecting-ssd-in-windows/33359142#33359142
    // umi zjistit korektni udaje i pro cesty obsahujici substy a reparse pointy pod Windows
    // 2000/XP/Vista (Salamander 2.5 pracuje jen s junction-pointy); 'path' je cesta, pro kterou
    // zjistujeme informace; pokud cesta vede pres sitovou cestu, tise vraci FALSE
    // mozne volat z libovolneho threadu
    virtual BOOL WINAPI IsPathOnSSD(const char* path) = 0;

    // vraci TRUE, pokud jde o UNC cestu (detekuje oba formaty: \\server\share i \\?\UNC\server\share)
    // mozne volat z libovolneho threadu
    virtual BOOL WINAPI IsUNCPath(const char* path) = 0;

    // nahradi substy v ceste 'resPath' jejich cilovymi cestami (prevod na cestu bez SUBST drive-letters);
    // 'resPath' musi ukazovat na buffer o minimalne 'MAX_PATH' znacich
    // vraci TRUE pri uspechu, FALSE pri chybe
    // mozne volat z libovolneho threadu
    virtual BOOL WINAPI ResolveSubsts(char* resPath) = 0;

    // volat jen pro cesty 'path', jejichz root (po odstraneni substu) je DRIVE_FIXED (jinde nema smysl hledat
    // reparse pointy); hledame cestu bez reparse pointu, vedouci na stejny svazek jako 'path'; pro cestu
    // obsahujici symlink vedouci na sitovou cestu (UNC nebo mapovanou) vracime jen root teto sitove cesty
    // (ani Vista neumi delat s reparse pointy na sitovych cestach, takze to je nejspis zbytecne drazdit);
    // pokud takova cesta neexistuje z duvodu, ze aktualni (posledni) lokalni reparse point je volume mount
    // point (nebo neznamy typ reparse pointu), vracime cestu k tomuto volume mount pointu (nebo reparse
    // pointu neznameho typu); pokud cesta obsahuje vice nez 50 reparse pointu (nejspis nekonecny cyklus),
    // vracime puvodni cestu;
    //
    // 'resPath' je buffer pro vysledek o velikosti MAX_PATH; 'path' je puvodni cesta; v 'cutResPathIsPossible'
    // (nesmi byt NULL) vracime FALSE pokud vysledna cesta v 'resPath' obsahuje na konci reparse point (volume
    // mount point nebo neznamy typ reparse pointu) a tudiz ji nesmime zkracovat (dostali bysme se tim nejspis
    // na jiny svazek); je-li 'rootOrCurReparsePointSet' neNULLove a obsahuje-li FALSE a na puvodni ceste je
    // aspon jeden lokalni reparse point (reparse pointy na sitove casti cesty ignorujeme), vracime v teto
    // promenne TRUE + v 'rootOrCurReparsePoint' (neni-li NULL) vracime plnou cestu k aktualnimu (poslednimu
    // lokalnimu) reparse pointu (pozor, ne kam vede); cilovou cestu aktualniho reparse pointu (jen je-li to
    // junction nebo symlink) vracime v 'junctionOrSymlinkTgt' (neni-li NULL) + typ vracime v 'linkType':
    // 2 (JUNCTION POINT), 3 (SYMBOLIC LINK); v 'netPath' (neni-li NULL) vracime sitovou cestu, na kterou
    // vede aktualni (posledni) lokalni symlink v ceste - v teto situaci se root sitove cesty vraci v 'resPath'
    // mozne volat z libovolneho threadu
    virtual void WINAPI ResolveLocalPathWithReparsePoints(char* resPath, const char* path,
                                                          BOOL* cutResPathIsPossible,
                                                          BOOL* rootOrCurReparsePointSet,
                                                          char* rootOrCurReparsePoint,
                                                          char* junctionOrSymlinkTgt, int* linkType,
                                                          char* netPath) = 0;

    // Provede resolve substu a reparse pointu pro cestu 'path', nasledne se pro mount-point cesty
    // (pokud chybi tak pro root cesty) pokusi ziskat GUID path. Pri neuspechu vrati FALSE. Pri
    // uspechu, vrati TRUE a nastavi 'mountPoint' a 'guidPath' (pokud jsou ruzne od NULL, musi
    // odkazovat na buffery o velikosti minimalne MAX_PATH; retezce budou zakonceny zpetnym lomitkem).
    // mozne volat z libovolneho threadu
    virtual BOOL WINAPI GetResolvedPathMountPointAndGUID(const char* path, char* mountPoint, char* guidPath) = 0;

    // nahradi v retezci posledni znak '.' decimalnim separatorem ziskanym ze systemu LOCALE_SDECIMAL
    // delka retezce muze narust, protoze separator muze mit podle MSDN az 4 znaky
    // vraci TRUE, pokud byl buffer dostatecne veliky a operaci se povedlo dokoncit, jinak vraci FALSE
    // mozne volat z libovolneho threadu
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

    // pokud ma soubor/adresar 'name' read-only atribut, pokusime se ho vypnout
    // (duvod: napr. aby sel smazat pres DeleteFile); pokud uz mame atributy 'name'
    // nactene, predame je v 'attr', je-li 'attr' -1, ctou se atributy 'name' z disku;
    // vraci TRUE pokud se provede pokus o zmenu atributu (uspech se nekontroluje);
    // POZNAMKA: vypina jen read-only atribut, aby v pripade vice hardlinku nedoslo
    // k zbytecne velke zmene atributu na zbyvajicich hardlinkach souboru (atributy
    // vsechny hardlinky sdili)
    // mozne volat z libovolneho threadu
    virtual BOOL WINAPI ClearReadOnlyAttr(const char* name, DWORD attr = -1) = 0;

    // zjisti, jestli prave probiha critical shutdown (nebo log off), pokud ano, vraci TRUE;
    // pri tomto shutdownu mame cas jen 5s na ulozeni konfigurace celeho programu
    // vcetne pluginu, takze casove narocnejsi operace musime vynechat, po uplynuti
    // 5s system nas process nasilne ukonci, vice viz WM_ENDSESSION, flag ENDSESSION_CRITICAL,
    // je to Vista+
    virtual BOOL WINAPI IsCriticalShutdown() = 0;

    // projde v threadu 'tid' (0 = aktualni) vsechna okna (EnumThreadWindows) a vsem enablovanym
    // a viditelnym dialogum (class name "#32770") vlastnenym oknem 'parent' postne WM_CLOSE;
    // pouziva se pri critical shutdown k odblokovani okna/dialogu, nad kterym jsou otevrene
    // modalni dialogy, hrozi-li vice vrstev, je nutne volat opakovane
    virtual void WINAPI CloseAllOwnedEnabledDialogs(HWND parent, DWORD tid = 0) = 0;
};

#ifdef _MSC_VER
#pragma pack(pop, enter_include_spl_gen)
#endif // _MSC_VER
#ifdef __BORLANDC__
#pragma option -a
#endif // __BORLANDC__
