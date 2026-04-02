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
#pragma pack(push, enter_include_spl_base) // keep the structures independent of the current packing alignment
#pragma pack(4)
#pragma warning(3 : 4706) // warning C4706: assignment within conditional expression
#endif                    // _MSC_VER
#ifdef __BORLANDC__
#pragma option -a4
#endif // __BORLANDC__

// In debug builds we test whether the source and destination memory blocks overlap (memcpy must not be used on overlapping blocks)
#if defined(_DEBUG) && defined(TRACE_ENABLE)
#define memcpy _sal_safe_memcpy
#ifdef __cplusplus
extern "C"
{
#endif
    void* _sal_safe_memcpy(void* dest, const void* src, size_t count);
#ifdef __cplusplus
}
#endif
#endif // defined(_DEBUG) && defined(TRACE_ENABLE)

// The following functions do not crash when working with invalid memory (including NULL):
// lstrcpy, lstrcpyn, lstrlen and lstrcat (they are defined with the A or W suffix, so
// we do not redefine them directly). To make bugs easier to debug, we need them to crash,
// because otherwise the error would only be discovered later, at a point where it might no longer be clear
// what caused it
#define lstrcpyA _sal_lstrcpyA
#define lstrcpyW _sal_lstrcpyW
#define lstrcpynA _sal_lstrcpynA
#define lstrcpynW _sal_lstrcpynW
#define lstrlenA _sal_lstrlenA
#define lstrlenW _sal_lstrlenW
#define lstrcatA _sal_lstrcatA
#define lstrcatW _sal_lstrcatW
#ifdef __cplusplus
extern "C"
{
#endif
    LPSTR _sal_lstrcpyA(LPSTR lpString1, LPCSTR lpString2);
    LPWSTR _sal_lstrcpyW(LPWSTR lpString1, LPCWSTR lpString2);
    LPSTR _sal_lstrcpynA(LPSTR lpString1, LPCSTR lpString2, int iMaxLength);
    LPWSTR _sal_lstrcpynW(LPWSTR lpString1, LPCWSTR lpString2, int iMaxLength);
    int _sal_lstrlenA(LPCSTR lpString);
    int _sal_lstrlenW(LPCWSTR lpString);
    LPSTR _sal_lstrcatA(LPSTR lpString1, LPCSTR lpString2);
    LPWSTR _sal_lstrcatW(LPWSTR lpString1, LPCWSTR lpString2);
#ifdef __cplusplus
}
#endif

// The original SDK shipped with VC6 defined the value as 0x00000040 (in 1998 when the attribute was not used yet, it appeared with Windows 2000)
#if (FILE_ATTRIBUTE_ENCRYPTED != 0x00004000)
#pragma message(__FILE__ " ERROR: FILE_ATTRIBUTE_ENCRYPTED != 0x00004000. You have to install latest version of Microsoft SDK. This value has changed!")
#endif

class CSalamanderGeneralAbstract;
class CPluginDataInterfaceAbstract;
class CPluginInterfaceForArchiverAbstract;
class CPluginInterfaceForViewerAbstract;
class CPluginInterfaceForMenuExtAbstract;
class CPluginInterfaceForFSAbstract;
class CPluginInterfaceForThumbLoaderAbstract;
class CSalamanderGUIAbstract;
class CSalamanderSafeFileAbstract;
class CGUIIconListAbstract;

//
// ****************************************************************************
// CSalamanderDebugAbstract
//
// a set of Salamander methods used for debugging in both debug and release builds

// macro CALLSTK_MEASURETIMES - enables timing of the time spent preparing call-stack messages (measured as a ratio
//                              to the total time spent running the functions)
//                              NOTE: each plugin has to enable it separately
// macro CALLSTK_DISABLEMEASURETIMES - suppresses timing of the time spent preparing call-stack messages in the DEBUG/SDK/PB build

#if (defined(_DEBUG) || defined(CALLSTK_MEASURETIMES)) && !defined(CALLSTK_DISABLEMEASURETIMES)
struct CCallStackMsgContext
{
    DWORD PushesCounterStart;                      // starting value of the counter for Push calls made in this thread
    LARGE_INTEGER PushPerfTimeCounterStart;        // starting value of the counter for time spent in Push methods called in this thread
    LARGE_INTEGER IgnoredPushPerfTimeCounterStart; // starting value of the counter for time spent in unmeasured (ignored) Push methods called in this thread
    LARGE_INTEGER StartTime;                       // "time" of Push for this call-stack macro
    DWORD_PTR PushCallerAddress;                   // address of the CALL_STACK_MESSAGE macro (address of the Push call)
};
#else  // (defined(_DEBUG) || defined(CALLSTK_MEASURETIMES)) && !defined(CALLSTK_DISABLEMEASURETIMES)
struct CCallStackMsgContext;
#endif // (defined(_DEBUG) || defined(CALLSTK_MEASURETIMES)) && !defined(CALLSTK_DISABLEMEASURETIMES)

class CSalamanderDebugAbstract
{
public:
    // prints 'file'+'line'+'str' as TRACE_I to TRACE SERVER - only in the DEBUG/SDK/PB build of Salamander
    virtual void WINAPI TraceI(const char* file, int line, const char* str) = 0;
    virtual void WINAPI TraceIW(const WCHAR* file, int line, const WCHAR* str) = 0;

    // prints 'file'+'line'+'str' as TRACE_E to TRACE SERVER - only in the DEBUG/SDK/PB build of Salamander
    virtual void WINAPI TraceE(const char* file, int line, const char* str) = 0;
    virtual void WINAPI TraceEW(const WCHAR* file, int line, const WCHAR* str) = 0;

    // registers a new thread with TRACE (assigns a Unique ID); 'thread'+'tid' are returned by
    // _beginthreadex or CreateThread; calling this is optional (the UID is then -1)
    virtual void WINAPI TraceAttachThread(HANDLE thread, unsigned tid) = 0;

    // sets the name of the active thread for TRACE, optional (the thread is otherwise marked as "unknown")
    // NOTE: requires registering the thread with TRACE (see TraceAttachThread), otherwise it does nothing
    virtual void WINAPI TraceSetThreadName(const char* name) = 0;
    virtual void WINAPI TraceSetThreadNameW(const WCHAR* name) = 0;

    // installs everything needed for CALL-STACK methods in the thread (see Push and Pop below),
    // all plugin methods invoked afterwards can call CALL_STACK methods directly,
    // this method is used only for new plugin threads,
    // it runs the 'threadBody' function with the 'param' parameter and returns the result of 'threadBody'
    virtual unsigned WINAPI CallWithCallStack(unsigned(WINAPI* threadBody)(void*), void* param) = 0;

    // pushes a message onto the CALL-STACK ('format'+'args' same as vsprintf). If the application crashes
    // the CALL-STACK contents are written into the Bug Report window reporting the crash
    virtual void WINAPI Push(const char* format, va_list args, CCallStackMsgContext* callStackMsgContext,
                             BOOL doNotMeasureTimes) = 0;

    // removes the last message from the CALL-STACK; calls must be paired with Push
    virtual void WINAPI Pop(CCallStackMsgContext* callStackMsgContext) = 0;

    // sets the active thread name for the VC debugger
    virtual void WINAPI SetThreadNameInVC(const char* name) = 0;

    // calls TraceSetThreadName and SetThreadNameInVC for 'name' (see both methods for details)
    virtual void WINAPI SetThreadNameInVCAndTrace(const char* name) = 0;

    // If not yet connected to the Trace Server, tries to establish the connection (the server
    // must be running). SDK builds of Salamander only (including Preview Builds): if auto-start of the
    // server is enabled and the server is not running (for example, the user terminated it), it tries to start it
    // before connecting.
    virtual void WINAPI TraceConnectToServer() = 0;

    // Called for modules that may report memory leaks. If memory leaks are detected,
    // Salamander loads all registered modules "as image" (without module initialization)
    // because they have already been unloaded when checking for leaks, and only then prints
    // the memory leak report = the .cpp module names are visible instead of "#File Error#"
    // Can be called from any thread
    virtual void WINAPI AddModuleWithPossibleMemoryLeaks(const char* fileName) = 0;
};

//
// ****************************************************************************
// CSalamanderRegistryAbstract
//
// a set of Salamander methods for working with the system registry,
// used in CPluginInterfaceAbstract::LoadConfiguration
// and CPluginInterfaceAbstract::SaveConfiguration

class CSalamanderRegistryAbstract
{
public:
    // clears the 'key' of all subkeys and values, returns success
    virtual BOOL WINAPI ClearKey(HKEY key) = 0;

    // creates the subkey 'name' of the key 'key', or opens it if it already exists; returns success and the key in 'createdKey'
    // the obtained key ('createdKey') must be closed by calling CloseKey
    virtual BOOL WINAPI CreateKey(HKEY key, const char* name, HKEY& createdKey) = 0;

    // opens the existing subkey 'name' of the key 'key'; returns success and the key in 'openedKey'
    // the obtained key ('openedKey') must be closed by calling CloseKey
    virtual BOOL WINAPI OpenKey(HKEY key, const char* name, HKEY& openedKey) = 0;

    // closes a key opened via OpenKey or CreateKey
    virtual void WINAPI CloseKey(HKEY key) = 0;

    // deletes the subkey 'name' of the key 'key'; returns whether it succeeded
    virtual BOOL WINAPI DeleteKey(HKEY key, const char* name) = 0;

    // reads the 'name' value of type 'type' from key 'key' into 'buffer' of size 'bufferSize', returns success
    virtual BOOL WINAPI GetValue(HKEY key, const char* name, DWORD type, void* buffer, DWORD bufferSize) = 0;

    // stores the value specified by 'name'+'type'+'data'+'dataSize' in the key 'key'. For strings, it is possible
    // to pass 'dataSize' == -1 -> the string length is computed using strlen,
    // returns success
    virtual BOOL WINAPI SetValue(HKEY key, const char* name, DWORD type, const void* data, DWORD dataSize) = 0;

    // deletes the value 'name' of the key 'key'; returns whether it succeeded
    virtual BOOL WINAPI DeleteValue(HKEY key, const char* name) = 0;

    // retrieves into 'bufferSize' the required size of the 'name' value of type 'type' from key 'key', returns success
    virtual BOOL WINAPI GetSize(HKEY key, const char* name, DWORD type, DWORD& bufferSize) = 0;
};

//
// ****************************************************************************
// CSalamanderConnectAbstract
//
// a set of Salamander methods for connecting a plugin into Salamander
// (custom pack/unpack + panel archiver view/edit + file viewer + menu items)

// constants for CSalamanderConnectAbstract::AddMenuItem
#define MENU_EVENT_TRUE 0x0001                    // always occurs
#define MENU_EVENT_DISK 0x0002                    // the source is a Windows directory ("c:\path" or UNC)
#define MENU_EVENT_THIS_PLUGIN_ARCH 0x0004        // the source is this plugin's archive
#define MENU_EVENT_THIS_PLUGIN_FS 0x0008          // the source is this plugin's file system
#define MENU_EVENT_FILE_FOCUSED 0x0010            // focus is on a file
#define MENU_EVENT_DIR_FOCUSED 0x0020             // focus is on a directory
#define MENU_EVENT_UPDIR_FOCUSED 0x0040           // focus is on ".."
#define MENU_EVENT_FILES_SELECTED 0x0080          // files are selected
#define MENU_EVENT_DIRS_SELECTED 0x0100           // directories are selected
#define MENU_EVENT_TARGET_DISK 0x0200             // the target is a Windows directory ("c:\path" or UNC)
#define MENU_EVENT_TARGET_THIS_PLUGIN_ARCH 0x0400 // the target is this plugin's archive
#define MENU_EVENT_TARGET_THIS_PLUGIN_FS 0x0800   // the target is this plugin's file system
#define MENU_EVENT_SUBDIR 0x1000                  // the directory is not the root (contains "..")
// focus is on a file for which this plugin provides "panel archiver view" or "panel archiver edit"
#define MENU_EVENT_ARCHIVE_FOCUSED 0x2000
// only 0x4000 is still available (both masks are combined into a DWORD and are masked with 0x7FFF beforehand)

// determines which user level the item is intended for
#define MENU_SKILLLEVEL_BEGINNER 0x0001     // intended for the most important menu items for beginners
#define MENU_SKILLLEVEL_INTERMEDIATE 0x0002 // also assign to less frequently used commands; for more advanced users
#define MENU_SKILLLEVEL_ADVANCED 0x0004     // assign to all commands (advanced users should have everything in the menu)
#define MENU_SKILLLEVEL_ALL 0x0007          // helper constant combining all of the above

// macro to prepare a 'HotKey' for AddMenuItem()
// LOWORD - the hot key (virtual key + modifiers) (LOBYTE - virtual key, HIBYTE - modifiers)
// mods: a combination of HOTKEYF_CONTROL, HOTKEYF_SHIFT, HOTKEYF_ALT
// examples: SALHOTKEY('A', HOTKEYF_CONTROL | HOTKEYF_SHIFT), SALHOTKEY(VK_F1, HOTKEYF_CONTROL | HOTKEYF_ALT | HOTKEYF_EXT)
//#define SALHOTKEY(vk,mods,cst) ((DWORD)(((BYTE)(vk)|((WORD)((BYTE)(mods))<<8))|(((DWORD)(BYTE)(cst))<<16)))
#define SALHOTKEY(vk, mods) ((DWORD)(((BYTE)(vk) | ((WORD)((BYTE)(mods)) << 8))))

// macro to prepare 'hotKey' for AddMenuItem()
// tells Salamander that the menu item will contain a hotkey (separated by '\t')
// Salamander will not complain via TRACE_E and will display the hotkey in the Plugins menu
// NOTE: this is not a hotkey that Salamander would deliver to the plugin; it really is just a label
// if the user assigns their own hotkey to this command in Plugin Manager, the hint will be suppressed
#define SALHOTKEY_HINT ((DWORD)0x00020000)

class CSalamanderConnectAbstract
{
public:
    // adds the plugin to the list for "custom archiver pack",
    // 'title' is the custom packer name shown to the user, 'defaultExtension' is the default extension
    // for new archives; if this is not an upgrade of "custom pack" (or adding the whole plugin) and
    // 'update' is FALSE, the call is ignored; if 'update' is TRUE, the settings are overwritten with the new
    // values 'title' and 'defaultExtension' - it is necessary to prevent repeated 'update'==TRUE
    // (continuous overwriting of the settings)
    virtual void WINAPI AddCustomPacker(const char* title, const char* defaultExtension, BOOL update) = 0;

    // adding the plugin to the list for "custom archiver unpack",
    // 'title' is the name of the custom unpacker shown to the user, 'masks' are archive file masks (they are used
    // to determine which unpacker should unpack a given archive, the separator is ';' (the escape sequence for ';' is
    // ";;") and the usual wildcards '*' and '?' plus '#' for '0'..'9' are supported); if this is not an upgrade
    // of "custom unpack" (or adding the whole plugin) and 'update' is FALSE, the call is ignored;
    // if 'update' is TRUE, the settings are overwritten with the new values of 'title' and 'masks' - it is necessary to prevent
    // repeated 'update'==TRUE calls (continuous overwriting of the settings)
    virtual void WINAPI AddCustomUnpacker(const char* title, const char* masks, BOOL update) = 0;

    // adds the plugin to the list for "panel archiver view/edit",
    // 'extensions' are archive extensions that should be handled by this plugin
    // (the separator is ';' (';' has no escape sequence here) and wildcard '#' is used for
    // '0'..'9'); if 'edit' is TRUE, this plugin handles "panel archiver view/edit", otherwise only
    // "panel archiver view"; if this is not an upgrade of "panel archiver view/edit" (or adding
    // the whole plugin) and 'updateExts' is FALSE, the call is ignored; if 'updateExts' is TRUE,
    // new archive extensions are added (ensuring that all extensions from 'extensions' are present) -
    // repeated 'updateExts'==TRUE must be prevented (to avoid constantly reviving extensions from 'extensions')
    virtual void WINAPI AddPanelArchiver(const char* extensions, BOOL edit, BOOL updateExts) = 0;

    // removes an extension from the "panel archiver view/edit" list (only from items belonging to
    // this plugin); 'extension' is the archive extension (a single one; wildcard '#' is used for '0'..'9'),
    // protection against repeated calls is necessary (to avoid repeatedly deleting 'extension')
    virtual void WINAPI ForceRemovePanelArchiver(const char* extension) = 0;

    // adds the plugin to the list for "file viewer",
    // 'masks' are viewer extensions that should be handled by this plugin
    // (the separator is ';' (the escape sequence for ';' is ";;"), and the wildcards '*' and '?' are used;
    // avoid spaces if possible, and the '|' character is forbidden (inverse masks are not allowed)),
    // if this is not an upgrade of "file viewer" (or the addition of the whole plugin) and 'force' is FALSE,
    // the call is ignored; if 'force' is TRUE, it always adds 'masks' (if they are not already on
    // the list) - it is necessary to prevent repeated 'force'==TRUE calls (continually re-adding 'masks')
    virtual void WINAPI AddViewer(const char* masks, BOOL force) = 0;

    // removes a mask from the "file viewer" list (only from items related to this plugin),
    // 'mask' is a viewer extension (a single one; wildcards '*' and '?' are used), protection is needed
    // against repeated calls (continuous removal of 'mask')
    virtual void WINAPI ForceRemoveViewer(const char* mask) = 0;

    // adds items to Salamander's Plugins/"plugin name" menu. 'iconIndex' is the index of the
    // item's icon (-1 = no icon; for specifying the bitmap with icons, see
    // CSalamanderConnectAbstract::SetBitmapWithIcons; ignored for separators). 'name' is the
    // item name (max. MAX_PATH - 1 characters) or NULL if it is a separator (the
    // 'state_or'+'state_and' parameters have no meaning in that case); 'hotKey' is the item's
    // hotkey obtained using the SALHOTKEY macro. 'name' may contain a hotkey hint,
    // separated by '\t'; in that case the 'hotKey' variable must be assigned the
    // SALHOTKEY_HINT constant, see the comment for SALHOTKEY_HINT for details. 'id' is a unique
    // identifier of the item within the plugin (for separators it matters only if 'callGetState' is TRUE).
    // If 'callGetState' is TRUE, the CPluginInterfaceForMenuExtAbstract::GetMenuItemState method is called
    // to determine the item state (for separators, only MENU_ITEM_STATE_HIDDEN is meaningful; the others are ignored);
    // otherwise 'state_or'+'state_and' are used to compute the item state (enabled/disabled) -
    // when computing the item state, an 'eventMask' is first built by ORing together all events that occurred (see
    // MENU_EVENT_XXX). The item will be enabled if the following expression evaluates to TRUE:
    //   ('eventMask' & 'state_or') != 0 && ('eventMask' & 'state_and') == 'state_and',
    // the 'skillLevel' parameter determines for which user levels the item (or separator)
    // is shown; the value contains one or more MENU_SKILLLEVEL_XXX constants ORed together.
    // Menu items are updated on every plugin load (the items may change depending on the configuration).
    // NOTE: CSalamanderBuildMenuAbstract::AddMenuItem is used for a "dynamic menu extension"
    virtual void WINAPI AddMenuItem(int iconIndex, const char* name, DWORD hotKey, int id, BOOL callGetState,
                                    DWORD state_or, DWORD state_and, DWORD skillLevel) = 0;

    // adds a submenu to Salamander's Plugins/"plugin name" menu, 'iconIndex'
    // is the submenu icon index (-1 = no icon; for specifying the bitmap with icons,
    // see CSalamanderConnectAbstract::SetBitmapWithIcons), 'name' is the submenu name
    // (max. MAX_PATH - 1 characters), 'id' is a unique identifier of the menu item
    // within the plugin (for a submenu it only matters if 'callGetState' is TRUE),
    // if 'callGetState' is TRUE, the
    // CPluginInterfaceForMenuExtAbstract::GetMenuItemState method is called to determine the
    // submenu state (only the MENU_ITEM_STATE_ENABLED and MENU_ITEM_STATE_HIDDEN states
    // matter, the others are ignored), otherwise 'state_or'+'state_and' are used
    // to calculate the item state (enabled/disabled) - for the item state calculation,
    // see CSalamanderConnectAbstract::AddMenuItem(), the 'skillLevel' parameter
    // determines for which user levels the submenu is shown, the value contains one or
    // more MENU_SKILLLEVEL_XXX constants ORed together, the submenu is ended by calling
    // CSalamanderConnectAbstract::AddSubmenuEnd();
    // menu items are updated on every plugin load (items may change according to configuration)
    // WARNING: CSalamanderBuildMenuAbstract::AddSubmenuStart is used for "dynamic menu extension"
    virtual void WINAPI AddSubmenuStart(int iconIndex, const char* name, int id, BOOL callGetState,
                                        DWORD state_or, DWORD state_and, DWORD skillLevel) = 0;

    // ends the submenu in Salamander's Plugins/"plugin name" menu; subsequent items will
    // be added to the higher (parent) menu level.
    // Menu items are refreshed on every plugin load (the items may change according to configuration).
    // NOTE: CSalamanderBuildMenuAbstract::AddSubmenuEnd is used for a "dynamic menu extension"
    virtual void WINAPI AddSubmenuEnd() = 0;

    // sets the item for the FS in the Change Drive menu and on the Drive bars. 'title' is its text,
    // 'iconIndex' is the index of its icon (-1 = no icon; providing the bitmap with icons is described in
    // CSalamanderConnectAbstract::SetBitmapWithIcons). 'title' may contain up to three columns
    // separated by '\t' (see the Alt+F1/F2 menu). The item visibility can be configured
    // from Plugins Manager or directly from the plugin via
    // CSalamanderGeneralAbstract::SetChangeDriveMenuItemVisibility
    virtual void WINAPI SetChangeDriveMenuItem(const char* title, int iconIndex) = 0;

    // informs Salamander that the plugin can load thumbnails for files matching the group mask in
    // 'masks' (the separator is ';' (the escape sequence for ';' is ";;") and the wildcards '*' and '?' are used);
    // to load thumbnails,
    // CPluginInterfaceForThumbLoaderAbstract::LoadThumbnail is called
    virtual void WINAPI SetThumbnailLoader(const char* masks) = 0;

    // sets the bitmap with the plugin icons. Salamander copies the bitmap contents into its internal
    // structures; the plugin is responsible for destroying the bitmap (Salamander
    // uses the bitmap only during this function). The number of icons is derived from the bitmap
    // width, icons are always 16x16 pixels. The transparent parts of the icons use the purple
    // color (RGB(255,0,255)), the bitmap color depth can be 4 or 8 bits (16 or 256
    // colors). Ideally prepare both color variants and pick one according to the result of
    // CSalamanderGeneralAbstract::CanUse256ColorsBitmap().
    // NOTE: this method is deprecated and does not support alpha transparency; use
    //        SetIconListForGUI() instead
    virtual void WINAPI SetBitmapWithIcons(HBITMAP bitmap) = 0;

    // sets the index of the plugin icon used in the Plugins/Plugins Manager window,
    // in the Help/About Plugin menu, and possibly for the plugin submenu in the Plugins menu (see
    // CSalamanderConnectAbstract::SetPluginMenuAndToolbarIcon() for details). If the plugin does not call this
    // method, the default Salamander icon for plugins is used. 'iconIndex'
    // is the icon index being set (providing the bitmap with icons is described in
    // CSalamanderConnectAbstract::SetBitmapWithIcons)
    virtual void WINAPI SetPluginIcon(int iconIndex) = 0;

    // sets the icon index for the plugin submenu that is used for the submenu
    // in the Plugins menu and possibly also in the top toolbar for the drop-down button
    // showing the plugin submenu. If the plugin does not call this method,
    // the plugin icon is used for the submenu in the Plugins menu (configured via
    // CSalamanderConnectAbstract::SetPluginIcon) and the plugin button does not appear in the top toolbar.
    // 'iconIndex' is the icon index being set (-1 = use the plugin icon,
    // see CSalamanderConnectAbstract::SetPluginIcon(); providing the bitmap
    // with icons is described in CSalamanderConnectAbstract::SetBitmapWithIcons);
    virtual void WINAPI SetPluginMenuAndToolbarIcon(int iconIndex) = 0;

    // sets the icon list with the plugin icons; the icon list must be allocated by calling
    // CSalamanderGUIAbstract::CreateIconList() and then created and filled using
    // methods of the CGUIIconListAbstract interface; icon dimensions must be 16x16 pixels;
    // Salamander takes ownership of the icon list object, and the plugin must not destroy it after calling
    // this function; the icon list is stored in Salamander's configuration
    // so that the icons can be used on the next run without loading the plugin, therefore
    // include only the necessary icons
    virtual void WINAPI SetIconListForGUI(CGUIIconListAbstract* iconList) = 0;
};

//
// ****************************************************************************
// CDynamicString
//
// dynamic string: reallocated as needed

class CDynamicString
{
public:
    // returns TRUE if the string 'str' of length 'len' was added successfully; if 'len' is -1,
    // 'len' is determined as "strlen(str)" (added without the null terminator); if 'len' is -2,
    // 'len' is determined as "strlen(str)+1" (added including the null terminator)
    virtual BOOL WINAPI Add(const char* str, int len = -1) = 0;
};

//
// ****************************************************************************
// CPluginInterfaceAbstract
//
// a set of plugin methods that Salamander needs in order to work with the plugin
//
// For better clarity the sections are split for:
// archivers - see CPluginInterfaceForArchiverAbstract,
// viewers - see CPluginInterfaceForViewerAbstract,
// menu extensions - see CPluginInterfaceForMenuExtAbstract,
// file systems - see CPluginInterfaceForFSAbstract,
// thumbnail loaders - see CPluginInterfaceForThumbLoaderAbstract.
// The parts are linked to CPluginInterfaceAbstract via CPluginInterfaceAbstract::GetInterfaceForXXX

// flags indicating which functions the plugin provides (which methods of the
// CPluginInterfaceAbstract descendant are actually implemented in the plugin):
#define FUNCTION_PANELARCHIVERVIEW 0x0001     // methods for "panel archiver view"
#define FUNCTION_PANELARCHIVEREDIT 0x0002     // methods for "panel archiver edit"
#define FUNCTION_CUSTOMARCHIVERPACK 0x0004    // methods for "custom archiver pack"
#define FUNCTION_CUSTOMARCHIVERUNPACK 0x0008  // methods for "custom archiver unpack"
#define FUNCTION_CONFIGURATION 0x0010         // Configuration method
#define FUNCTION_LOADSAVECONFIGURATION 0x0020 // methods for "load/save configuration"
#define FUNCTION_VIEWER 0x0040                // methods for "file viewer"
#define FUNCTION_FILESYSTEM 0x0080            // methods for "file system"
#define FUNCTION_DYNAMICMENUEXT 0x0100        // methods for "dynamic menu extension"

// event codes (and the meaning of the 'param' parameter) received by CPluginInterfaceAbstract::Event():
// colors changed (due to a system color change / WM_SYSCOLORCHANGE or due to a configuration change); the plugin can
// obtain the new Salamander colors via CSalamanderGeneralAbstract::GetCurrentColor;
// if the plugin has a file system with icons of type pitFromPlugin it should repaint the background of the image list
// with simple icons to the SALCOL_ITEM_BK_NORMAL color; 'param' is ignored here
#define PLUGINEVENT_COLORSCHANGED 0

// Salamander configuration changed; the plugin can obtain new versions of the Salamander
// configuration parameters via CSalamanderGeneralAbstract::GetConfigParameter;
// 'param' is ignored here
#define PLUGINEVENT_CONFIGURATIONCHANGED 1

// the left and right panels were swapped (Swap Panels - Ctrl+U)
// 'param' is ignored here
#define PLUGINEVENT_PANELSSWAPPED 2

// the active panel changed (switching between panels)
// 'param' is PANEL_LEFT or PANEL_RIGHT - indicates the panel that became active
#define PLUGINEVENT_PANELACTIVATED 3

// Salamander received WM_SETTINGCHANGE and regenerated toolbar fonts.
// It then broadcasts this event to all plugins so that they can call SetFont() on their toolbars.
// 'param' is ignored here
#define PLUGINEVENT_SETTINGCHANGE 4

// event codes in the Password Manager, received by CPluginInterfaceAbstract::PasswordManagerEvent():
#define PME_MASTERPASSWORDCREATED 1 // the user created a master password (passwords must be encrypted)
#define PME_MASTERPASSWORDCHANGED 2 // the user changed the master password (passwords must be decrypted and then re-encrypted)
#define PME_MASTERPASSWORDREMOVED 3 // the user removed the master password (passwords must be decrypted)

class CPluginInterfaceAbstract
{
#ifdef INSIDE_SALAMANDER
private: // protection against invalid direct method calls (see CPluginInterfaceEncapsulation)
    friend class CPluginInterfaceEncapsulation;
#else  // INSIDE_SALAMANDER
public:
#endif // INSIDE_SALAMANDER

    // called in response to the About button in the Plugins window or the Help/About Plugins menu command
    virtual void WINAPI About(HWND parent) = 0;

    // called before unloading the plugin (naturally only if SalamanderPluginEntry
    // returned this object and not NULL). Returns TRUE if the unload may proceed.
    // 'parent' is the parent for message boxes, 'force' is TRUE when the return value is ignored.
    // If it returns TRUE, this object and all other interfaces obtained from it will no longer be used
    // and the plugin will be unloaded. If a critical shutdown is in progress (see
    // CSalamanderGeneralAbstract::IsCriticalShutdown), there is no point in asking the user anything
    // (do not open any windows).
    // WARNING!!! All plugin threads must be terminated (if Release returns TRUE, FreeLibrary is called
    // on the plugin .SPL => the plugin code is unmapped from memory => threads have nothing to run =>
    // usually no bug report or Windows exception info is produced)
    virtual BOOL WINAPI Release(HWND parent, BOOL force) = 0;

    // function to load the default configuration and for "load/save configuration" (load from the plugin's
    // private registry key). 'parent' is the parent for message boxes. If 'regKey' == NULL it loads
    // the default configuration. 'registry' is the registry helper object. This method is always called
    // after SalamanderPluginEntry and before other calls (it loads from the private key if the
    // plugin provides this function and the key exists in the registry, otherwise it loads only the default
    // configuration)
    virtual void WINAPI LoadConfiguration(HWND parent, HKEY regKey, CSalamanderRegistryAbstract* registry) = 0;

    // function for "load/save configuration"; called to store the plugin configuration in its private
    // registry key. 'parent' is the parent for message boxes, 'registry' is the registry helper object.
    // When Salamander saves its configuration it calls this method as well (if the plugin implements it).
    // Salamander also offers to save the plugin configuration while unloading it (for example manually from
    // Plugins Manager); in that case the configuration is stored only if Salamander's registry key exists
    virtual void WINAPI SaveConfiguration(HWND parent, HKEY regKey, CSalamanderRegistryAbstract* registry) = 0;

    // called in response to the Configure button in the Plugins window
    virtual void WINAPI Configuration(HWND parent) = 0;

    // called to connect the plugin to Salamander, invoked only after LoadConfiguration.
    // 'parent' is the parent for message boxes, 'salamander' is the set of methods for hooking a plugin in

/*  RULES FOR IMPLEMENTING THE CONNECT METHOD
        (plugins must store their configuration version - see DEMOPLUGin,
         the ConfigVersion variable, and the CURRENT_CONFIG_VERSION constant.
         Below is an illustrative EXAMPLE that adds the "dmp2" extension to DEMOPLUGin):

      -increase CURRENT_CONFIG_VERSION with every change to the configuration
       (CURRENT_CONFIG_VERSION = 1 in the first version of the Connect method)
      -the base section (before any "if (ConfigVersion < YYY)" conditions) should:
        -contain the code that handles plugin installation (the very first load);
         see the methods of CSalamanderConnectAbstract
        -update the extension lists used for installing "custom archiver unpack"
         (AddCustomUnpacker), "panel archiver view/edit" (AddPanelArchiver),
         "file viewer" (AddViewer), menu items (AddMenuItem), etc. during upgrades
        -keep 'updateExts' and 'force' FALSE when calling AddPanelArchiver and AddViewer
         (otherwise we would force the user to re-add old extensions they may have
         removed manually)
        -pass "ConfigVersion < XXX" in the 'update' parameter of AddCustomPacker/
         AddCustomUnpacker, where XXX is the last version in which the custom packer/
         unpacker extensions changed (evaluate both calls separately; for simplicity
         we force all extensions here. If the user removed or added some, they will
         unfortunately have to adjust them manually again)
        -remember that AddMenuItem, SetChangeDriveMenuItem, and SetThumbnailLoader
         behave the same on every plugin load (installation and upgrade both start
         from a clean slate)
      -the upgrade-only section (after the base section) should:
        -add an "if (ConfigVersion < XXX)" condition where XXX is the new value of
         CURRENT_CONFIG_VERSION, and include a comment describing that upgrade.
         Inside the condition call:
          -AddPanelArchiver(PPP, EEE, TRUE) if new "panel archiver" extensions were
           added, where PPP contains only the new extensions separated by a semicolon
           and EEE is TRUE/FALSE ("panel view+edit"/"panel view only")
          -AddViewer(PPP, TRUE) if new "viewer" extensions were added, where PPP
           contains only the new extensions separated by a semicolon
          -ForceRemoveViewer(PPP) for every removed "viewer" extension PPP
          -ForceRemovePanelArchiver(PPP) for every removed "panel archiver" extension PPP

      CHECK: after making these changes I recommend verifying that everything works.
             Compile the plugin and load it into Salamander; it should perform an
             automatic upgrade from the previous version (no need to remove and add
             the plugin again):
             -Options/Configuration menu:
               -Viewers page: locate the added extensions and confirm that removed
                extensions are gone
               -Archives Associations in Panels page: locate the added extensions
               -Unpackers in Unpack Dialog Box page: locate your plugin and confirm
                that the mask list is correct
             -check the updated plugin submenu (in the Plugins menu)
             -check the updated Change Drive menu (Alt+F1/F2)
             -check the thumbnail masks in Plugins Manager (in the Plugins menu):
              focus your plugin and inspect the "Thumbnails" edit box
           +lastly, try removing and adding the plugin to confirm that the plugin
            "installation" works; use the checks above

      NOTE: when you add "panel archiver" extensions, also update the
            extension list in the 'extensions' parameter of SetBasicPluginData

      EXAMPLE OF ADDING THE "dmp2" EXTENSION TO THE VIEWER AND ARCHIVER:
        (lines starting with "-" were removed, lines starting with "+" added;
         a line beginning with "=====" indicates a break in the continuous code block)
        Summary of changes:
          -the configuration version increases from 2 to 3:
            -add a comment describing version 3
            -raise CURRENT_CONFIG_VERSION to 3
          -add the "dmp2" extension to the 'extensions' parameter of SetBasicPluginData
           (because we add the "dmp2" extension for the "panel archiver")
          -add the "*.dmp2" mask to AddCustomUnpacker and raise the condition
           from 1 to 3 (because we add the "dmp2" extension for the "custom unpacker")
          -add the "dmp2" extension to AddPanelArchiver (because we add the "dmp2" extension
           for the "panel archiver")
          -add the "*.dmp2" mask to AddViewer (because we add the "dmp2" extension
           for the "viewer")
          -add the upgrade condition for version 3 plus a comment describing that upgrade;
           inside the condition:
            -call AddPanelArchiver for the "dmp2" extension with 'updateExts' TRUE
             (because we add the "dmp2" extension for the "panel archiver")
            -call AddViewer for the "*.dmp2" mask with 'force' TRUE (because
             we add the "dmp2" extension for the "viewer")
=====
  // ConfigVersion: 0 - no configuration was loaded from the Registry (plugin installation),
  //                1 - first configuration version
  //                2 - second configuration version (some values added to the configuration)
+ //                3 - third configuration version (adding the "dmp2" extension)

  int ConfigVersion = 0;
- #define CURRENT_CONFIG_VERSION 2
+ #define CURRENT_CONFIG_VERSION 3
  const char *CONFIG_VERSION = "Version";
=====
  // set up the basic plugin information
  salamander->SetBasicPluginData("Salamander Demo Plugin",
                                 FUNCTION_PANELARCHIVERVIEW | FUNCTION_PANELARCHIVEREDIT |
                                 FUNCTION_CUSTOMARCHIVERPACK | FUNCTION_CUSTOMARCHIVERUNPACK |
                                 FUNCTION_CONFIGURATION | FUNCTION_LOADSAVECONFIGURATION |
                                 FUNCTION_VIEWER | FUNCTION_FILESYSTEM,
                                 "2.0",
                                 "Copyright © 1999-2023 Open Salamander Authors",
                                 "This plugin should help you to make your own plugins.",
-                                "DEMOPLUG", "dmp", "dfs");
+                                "DEMOPLUG", "dmp;dmp2", "dfs");
=====
  void WINAPI
  CPluginInterface::Connect(HWND parent, CSalamanderConnectAbstract *salamander)
  {
    CALL_STACK_MESSAGE1("CPluginInterface::Connect(,)");

    // base section:
    salamander->AddCustomPacker("DEMOPLUG (Plugin)", "dmp", FALSE);
-   salamander->AddCustomUnpacker("DEMOPLUG (Plugin)", "*.dmp", ConfigVersion < 1);
+   salamander->AddCustomUnpacker("DEMOPLUG (Plugin)", "*.dmp;*.dmp2", ConfigVersion < 3);
-   salamander->AddPanelArchiver("dmp", TRUE, FALSE);
+   salamander->AddPanelArchiver("dmp;dmp2", TRUE, FALSE);
-   salamander->AddViewer("*.dmp", FALSE);
+   salamander->AddViewer("*.dmp;*.dmp2", FALSE);
===== (I omitted adding menu items, setting icons and thumbnail masks)
    // upgrade section:
+   if (ConfigVersion < 3)   // version 3: added the "dmp2" extension
+   {
+     salamander->AddPanelArchiver("dmp2", TRUE, TRUE);
+     salamander->AddViewer("*.dmp2", TRUE);
+   }
  }
=====
    */
    virtual void WINAPI Connect(HWND parent, CSalamanderConnectAbstract* salamander) = 0;

    // releases the 'pluginData' interface that Salamander obtained from the plugin via
    // CPluginInterfaceForArchiverAbstract::ListArchive or
    // CPluginFSInterfaceAbstract::ListCurrentPath. Before this call Salamander
    // releases the file and directory data (CFileData::PluginData) using the
    // CPluginDataInterfaceAbstract methods
    virtual void WINAPI ReleasePluginDataInterface(CPluginDataInterfaceAbstract* pluginData) = 0;

    // returns the archiver interface. The plugin must return this interface if it provides
    // at least one of the following functions (see SetBasicPluginData): FUNCTION_PANELARCHIVERVIEW,
    // FUNCTION_PANELARCHIVEREDIT, FUNCTION_CUSTOMARCHIVERPACK and/or FUNCTION_CUSTOMARCHIVERUNPACK;
    // if the plugin does not include an archiver it returns NULL
    virtual CPluginInterfaceForArchiverAbstract* WINAPI GetInterfaceForArchiver() = 0;

    // returns the viewer interface. The plugin must return this interface if it provides
    // (see SetBasicPluginData) FUNCTION_VIEWER; if the plugin has no viewer it returns NULL
    virtual CPluginInterfaceForViewerAbstract* WINAPI GetInterfaceForViewer() = 0;

    // returns the menu extension interface; the plugin must return this interface if it adds
    // items to the menu (see CSalamanderConnectAbstract::AddMenuItem) or if it has the
    // FUNCTION_DYNAMICMENUEXT function (see SetBasicPluginData); otherwise it returns NULL
    virtual CPluginInterfaceForMenuExtAbstract* WINAPI GetInterfaceForMenuExt() = 0;

    // returns the file-system interface. The plugin must return this interface if it provides
    // (see SetBasicPluginData) FUNCTION_FILESYSTEM; if the plugin has no file system it returns NULL
    virtual CPluginInterfaceForFSAbstract* WINAPI GetInterfaceForFS() = 0;

    // returns the thumbnail loader interface. The plugin must return this interface if it informed
    // Salamander that it can load thumbnails (see CSalamanderConnectAbstract::SetThumbnailLoader);
    // if the plugin cannot load thumbnails it returns NULL
    virtual CPluginInterfaceForThumbLoaderAbstract* WINAPI GetInterfaceForThumbLoader() = 0;

    // handles various events, see the PLUGINEVENT_XXX codes; called only while the plugin
    // is loaded. 'param' is the event parameter.
    // NOTE: may be called at any time after the plugin entry point (SalamanderPluginEntry) finishes
    virtual void WINAPI Event(int event, DWORD param) = 0;

    // the user wants to clear all histories (they invoked Clear History from the History
    // page in the configuration). History here means everything that is created automatically from values
    // entered by the user (for example, the list of texts executed on the command line, the list of current paths on
    // individual drives, etc.). This does not include lists created by the user - e.g. hot-paths, user-menu,
    // etc. 'parent' is the parent of any message boxes. After saving the configuration, no history
    // may remain in the registry. If the plugin has open windows that contain histories (combo boxes), those
    // histories must be cleared there as well
    virtual void WINAPI ClearHistory(HWND parent) = 0;

    // receives information about a change on path 'path' (if 'includingSubdirs' is TRUE, it also
    // includes changes in subdirectories of 'path'); this method can be used, for example,
    // to invalidate/clear file or directory caches; NOTE: plugin file systems (FS)
    // have the method CPluginFSInterfaceAbstract::AcceptChangeOnPathNotification()
    virtual void WINAPI AcceptChangeOnPathNotification(const char* path, BOOL includingSubdirs) = 0;

    // this method is called only for plugins that use the Password Manager (see
    // CSalamanderGeneralAbstract::SetPluginUsesPasswordManager()):
    // informs the plugin about changes in the Password Manager; 'parent' is the parent of possible
    // message boxes/dialogs; 'event' contains the event, see PME_XXX
    virtual void WINAPI PasswordManagerEvent(HWND parent, int event) = 0;
};

//
// ****************************************************************************
// CSalamanderPluginEntryAbstract
//
// a set of Salamander methods used in SalamanderPluginEntry

// flags indicating why the plugin was loaded (see CSalamanderPluginEntryAbstract::GetLoadInformation)
#define LOADINFO_INSTALL 0x0001          // first load of the plugin (installation into Salamander)
#define LOADINFO_NEWSALAMANDERVER 0x0002 // new version of Salamander (installs all plugins from the \
                                         // plugins subdirectory), loads all plugins (possible \
                                         // upgrade of all of them)
#define LOADINFO_NEWPLUGINSVER 0x0004    // change in plugins.ver (installing/upgrading plugins), \
                                         // loads all plugins for simplicity (possible upgrade of \
                                         // all of them)
#define LOADINFO_LOADONSTART 0x0008      // loaded because the "load on start" flag was found

class CSalamanderPluginEntryAbstract
{
public:
    // returns the Salamander version, see spl_vers.h, constants LAST_VERSION_OF_SALAMANDER and REQUIRE_LAST_VERSION_OF_SALAMANDER
    virtual int WINAPI GetVersion() = 0;

    // returns the Salamander "parent" window (parent for message boxes)
    virtual HWND WINAPI GetParentWindow() = 0;

    // returns a pointer to the interface for Salamander debugging functions;
    // the interface is valid for the entire lifetime of the plugin (not just
    // during the "SalamanderPluginEntry" function) and is just a reference, so it is not released
    virtual CSalamanderDebugAbstract* WINAPI GetSalamanderDebug() = 0;

    // sets the basic data about the plugin (data Salamander remembers about the plugin together with the DLL name);
    // must be called, otherwise the plugin cannot be attached.
    // 'pluginName' is the plugin name; 'functions' contains all provided functions ORed together (see FUNCTION_XXX constants);
    // 'version'+'copyright'+'description' are user-facing data shown in the Plugins window;
    // 'regKeyName' is the suggested name of the private registry key used to store configuration (ignored without FUNCTION_LOADSAVECONFIGURATION);
    // 'extensions' are the basic extensions (e.g. just "ARJ"; not "A01", etc.) of the processed
    // archives separated by ';' (there is no escape sequence for ';' here) - Salamander uses these extensions
    // only when looking for a replacement for removed panel archivers (when a plugin is removed;
    // it solves "who now handles extension XXX when the original associated archiver
    // was removed as part of plugin PPP?") (ignored without FUNCTION_PANELARCHIVERVIEW and FUNCTION_PANELARCHIVEREDIT);
    // 'fsName' is the suggested name (acquiring the assigned name is done through
    // CSalamanderGeneralAbstract::GetPluginFSName) of the file system (ignored without FUNCTION_FILESYSTEM;
    // allowed characters are 'a-zA-Z0-9_+-', minimum length 2). If the plugin needs
    // more file system names it can use CSalamanderPluginEntryAbstract::AddFSName.
    // Returns TRUE if the data were accepted successfully
    virtual BOOL WINAPI SetBasicPluginData(const char* pluginName, DWORD functions,
                                           const char* version, const char* copyright,
                                           const char* description, const char* regKeyName = NULL,
                                           const char* extensions = NULL, const char* fsName = NULL) = 0;

    // returns a pointer to the interface for Salamander's general-purpose functions.
    // The interface is valid for the entire lifetime of the plugin (not just within the
    // "SalamanderPluginEntry" function) and is only a reference, so it is not released
    virtual CSalamanderGeneralAbstract* WINAPI GetSalamanderGeneral() = 0;

    // returns information related to plugin loading; the information is returned in a DWORD
    // as a bitwise combination of LOADINFO_XXX flags (to test whether a flag is present, use
    // the condition: (GetLoadInformation() & LOADINFO_XXX) != 0)
    virtual DWORD WINAPI GetLoadInformation() = 0;

    // loads the module with language-dependent resources (SLG); it always tries to load the module
    // in the same language currently used by Salamander. If it cannot find such a module (or
    // the version does not match), it lets the user choose an alternative module (if more than one
    // alternative exists and if the user selection from the previous plugin load is not stored).
    // If it finds no module, it returns NULL -> the plugin should terminate.
    // 'parent' is the parent window for error message boxes and the alternative language module selection dialog;
    // 'pluginName' is the plugin name (so the user knows which plugin is involved when an error is shown or when choosing an alternative module).
    // NOTE: this method can be called only once; the obtained language module handle
    //        is released automatically when the plugin is unloaded
    virtual HINSTANCE WINAPI LoadLanguageModule(HWND parent, const char* pluginName) = 0;

    // returns the ID of the language currently selected for Salamander (e.g. english.slg =
    // English (US) = 0x409, czech.slg = Czech = 0x405)
    virtual WORD WINAPI GetCurrentSalamanderLanguageID() = 0;

    // returns a pointer to the interface providing the customized Windows controls used
    // in Salamander; the interface remains valid for the entire lifetime of the plugin (not only
    // within the "SalamanderPluginEntry" function) and is only a reference, so it must not be released
    virtual CSalamanderGUIAbstract* WINAPI GetSalamanderGUI() = 0;

    // returns a pointer to the interface for convenient file operations.
    // The interface is valid for the entire lifetime of the plugin (not just during the
    // "SalamanderPluginEntry" function) and is just a reference, so it is not released
    virtual CSalamanderSafeFileAbstract* WINAPI GetSalamanderSafeFile() = 0;

    // sets the URL that should be shown as the plugin home page in the Plugins Manager window.
    // Salamander remembers the value until the next plugin load (the URL is shown even for
    // unloaded plugins). The URL must be set again on every plugin load, otherwise
    // no URL will be displayed (prevents keeping an invalid home page URL)
    virtual void WINAPI SetPluginHomePageURL(const char* url) = 0;

    // adds another file system name; without FUNCTION_FILESYSTEM in the 'functions'
    // parameter when calling SetBasicPluginData, this method always fails.
    // 'fsName' is the proposed file system name (the assigned name is obtained using
    // CSalamanderGeneralAbstract::GetPluginFSName; allowed characters are
    // 'a-zA-Z0-9_+-', minimum length 2 characters); 'newFSNameIndex' (must not be NULL)
    // receives the index of the newly added file system name; returns TRUE on success;
    // returns FALSE on a fatal error - in that case 'newFSNameIndex' is ignored.
    // limitation: must not be called before SetBasicPluginData
    virtual BOOL WINAPI AddFSName(const char* fsName, int* newFSNameIndex) = 0;
};

//
// ****************************************************************************
// FSalamanderPluginEntry
//
// Open Salamander 1.6 or Later Plugin Entry Point Function Type,
// the plugin exports this function as "SalamanderPluginEntry" and Salamander calls it
// to attach the plugin when the plugin is loaded.
// Returns the plugin interface on successful attachment, otherwise NULL.
// The plugin interface is released by calling its Release method before unloading the plugin

typedef CPluginInterfaceAbstract*(WINAPI* FSalamanderPluginEntry)(CSalamanderPluginEntryAbstract* salamander);

//
// ****************************************************************************
// FSalamanderPluginGetReqVer
//
// Open Salamander 2.5 Beta 2 or Later Plugin Get Required Version of Salamander Function Type,
// the plugin exports this function as "SalamanderPluginGetReqVer" and Salamander calls it
// as the first plugin function (before "SalamanderPluginGetSDKVer" and "SalamanderPluginEntry")
// when the plugin is loaded.
// Returns the Salamander version the plugin was built for (the oldest version into which the plugin can be loaded)

typedef int(WINAPI* FSalamanderPluginGetReqVer)();

//
// ****************************************************************************
// FSalamanderPluginGetSDKVer
//
// Open Salamander 2.52 beta 2 (PB 22) or Later Plugin Get SDK Version Function Type,
// the plugin can optionally export this function as "SalamanderPluginGetSDKVer" and Salamander
// tries to call it as the second plugin function (before "SalamanderPluginEntry")
// when the plugin is loaded.
// Returns the SDK version used to build the plugin (tells Salamander which methods
// the plugin provides). Exporting "SalamanderPluginGetSDKVer" makes sense only when
// "SalamanderPluginGetReqVer" returns a number smaller than LAST_VERSION_OF_SALAMANDER; it is recommended
// to return LAST_VERSION_OF_SALAMANDER directly

typedef int(WINAPI* FSalamanderPluginGetSDKVer)();

// ****************************************************************************
// SalIsWindowsVersionOrGreater
//
// Based on SDK 8.1 VersionHelpers.h
// Indicates if the current OS version matches, or is greater than, the provided
// version information. This function is useful in confirming a version of Windows
// Server that doesn't share a version number with a client release.
// http://msdn.microsoft.com/en-us/library/windows/desktop/dn424964%28v=vs.85%29.aspx
//

#ifdef __BORLANDC__
inline void* SecureZeroMemory(void* ptr, int cnt)
{
    char* vptr = (char*)ptr;
    while (cnt)
    {
        *vptr++ = 0;
        cnt--;
    }
    return ptr;
}
#endif // __BORLANDC__

inline BOOL SalIsWindowsVersionOrGreater(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor)
{
    OSVERSIONINFOEXW osvi;
    DWORDLONG const dwlConditionMask = VerSetConditionMask(VerSetConditionMask(VerSetConditionMask(0,
                                                                                                   VER_MAJORVERSION, VER_GREATER_EQUAL),
                                                                               VER_MINORVERSION, VER_GREATER_EQUAL),
                                                           VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);

    SecureZeroMemory(&osvi, sizeof(osvi)); // replacement for memset (does not require the runtime library)
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    osvi.dwMajorVersion = wMajorVersion;
    osvi.dwMinorVersion = wMinorVersion;
    osvi.wServicePackMajor = wServicePackMajor;
    return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask) != FALSE;
}

// Find Windows version using bisection method and VerifyVersionInfo.
// Author:   M1xA, www.m1xa.com
// Licence:  MIT
// Version:  1.0
// https://bitbucket.org/AnyCPU/findversion/src/ebdec778fdbcdee67ac9a4d520239e134e047d8d/include/findversion.h?at=default
// Tested on: Windows 2000 .. Windows 8.1.
//
// WARNING: This function is ***SLOW_HACK***, use SalIsWindowsVersionOrGreater() instead (if you can).

#define M1xA_FV_EQUAL 0
#define M1xA_FV_LESS -1
#define M1xA_FV_GREAT 1
#define M1xA_FV_MIN_VALUE 0
#define M1xA_FV_MINOR_VERSION_MAX_VALUE 16
inline int M1xA_testValue(OSVERSIONINFOEX* value, DWORD verPart, DWORDLONG eq, DWORDLONG gt)
{
    if (VerifyVersionInfo(value, verPart, eq) == FALSE)
    {
        if (VerifyVersionInfo(value, verPart, gt) == TRUE)
            return M1xA_FV_GREAT;
        return M1xA_FV_LESS;
    }
    else
        return M1xA_FV_EQUAL;
}

#define M1xA_findPartTemplate(T) \
    inline BOOL M1xA_findPart##T(T* part, DWORD partType, OSVERSIONINFOEX* ret, T a, T b) \
    { \
        int funx = M1xA_FV_EQUAL; \
\
        DWORDLONG const eq = VerSetConditionMask(0, partType, VER_EQUAL); \
        DWORDLONG const gt = VerSetConditionMask(0, partType, VER_GREATER); \
\
        T* p = part; \
\
        *p = (T)((a + b) / 2); \
\
        while ((funx = M1xA_testValue(ret, partType, eq, gt)) != M1xA_FV_EQUAL) \
        { \
            switch (funx) \
            { \
            case M1xA_FV_GREAT: \
                a = *p; \
                break; \
            case M1xA_FV_LESS: \
                b = *p; \
                break; \
            } \
\
            *p = (T)((a + b) / 2); \
\
            if (*p == a) \
            { \
                if (M1xA_testValue(ret, partType, eq, gt) == M1xA_FV_EQUAL) \
                    return TRUE; \
\
                *p = b; \
\
                if (M1xA_testValue(ret, partType, eq, gt) == M1xA_FV_EQUAL) \
                    return TRUE; \
\
                a = 0; \
                b = 0; \
                *p = 0; \
            } \
\
            if (a == b) \
            { \
                *p = 0; \
                return FALSE; \
            } \
        } \
\
        return TRUE; \
    }
M1xA_findPartTemplate(DWORD)
    M1xA_findPartTemplate(WORD)
        M1xA_findPartTemplate(BYTE)

            inline BOOL SalGetVersionEx(OSVERSIONINFOEX* osVer, BOOL versionOnly)
{
    BOOL ret = TRUE;
    ZeroMemory(osVer, sizeof(OSVERSIONINFOEX));
    osVer->dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if (!versionOnly)
    {
        ret &= M1xA_findPartDWORD(&osVer->dwPlatformId, VER_PLATFORMID, osVer, M1xA_FV_MIN_VALUE, MAXDWORD);
    }
    ret &= M1xA_findPartDWORD(&osVer->dwMajorVersion, VER_MAJORVERSION, osVer, M1xA_FV_MIN_VALUE, MAXDWORD);
    ret &= M1xA_findPartDWORD(&osVer->dwMinorVersion, VER_MINORVERSION, osVer, M1xA_FV_MIN_VALUE, M1xA_FV_MINOR_VERSION_MAX_VALUE);
    if (!versionOnly)
    {
        ret &= M1xA_findPartDWORD(&osVer->dwBuildNumber, VER_BUILDNUMBER, osVer, M1xA_FV_MIN_VALUE, MAXDWORD);
        ret &= M1xA_findPartWORD(&osVer->wServicePackMajor, VER_SERVICEPACKMAJOR, osVer, M1xA_FV_MIN_VALUE, MAXWORD);
        ret &= M1xA_findPartWORD(&osVer->wServicePackMinor, VER_SERVICEPACKMINOR, osVer, M1xA_FV_MIN_VALUE, MAXWORD);
        ret &= M1xA_findPartWORD(&osVer->wSuiteMask, VER_SUITENAME, osVer, M1xA_FV_MIN_VALUE, MAXWORD);
        ret &= M1xA_findPartBYTE(&osVer->wProductType, VER_PRODUCT_TYPE, osVer, M1xA_FV_MIN_VALUE, MAXBYTE);
    }
    return ret;
}

#ifdef _MSC_VER
#pragma pack(pop, enter_include_spl_base)
#endif // _MSC_VER
#ifdef __BORLANDC__
#pragma option -a
#endif // __BORLANDC__
