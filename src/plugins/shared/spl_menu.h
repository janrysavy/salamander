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
#pragma pack(push, enter_include_spl_menu) // to keep the structures independent of the configured alignment
#pragma pack(4)
#endif // _MSC_VER
#ifdef __BORLANDC__
#pragma option -a4
#endif // __BORLANDC__

class CSalamanderForOperationsAbstract;

//
// ****************************************************************************
// CSalamanderBuildMenuAbstract
//
// set of Salamander methods for building plugin menus
//
// this is a subset of CSalamanderConnectAbstract methods; they behave the same,
// use the same constants, and are documented in CSalamanderConnectAbstract

class CSalamanderBuildMenuAbstract
{
public:
    // configure icons through CSalamanderBuildMenuAbstract::SetIconListForMenu; see
    // CSalamanderConnectAbstract::AddMenuItem for the remaining description
    virtual void WINAPI AddMenuItem(int iconIndex, const char* name, DWORD hotKey, int id, BOOL callGetState,
                                    DWORD state_or, DWORD state_and, DWORD skillLevel) = 0;

    // configure icons through CSalamanderBuildMenuAbstract::SetIconListForMenu; see
    // CSalamanderConnectAbstract::AddSubmenuStart for the remaining description
    virtual void WINAPI AddSubmenuStart(int iconIndex, const char* name, int id, BOOL callGetState,
                                        DWORD state_or, DWORD state_and, DWORD skillLevel) = 0;

    // description see CSalamanderConnectAbstract::AddSubmenuEnd
    virtual void WINAPI AddSubmenuEnd() = 0;

    // sets the bitmap with plugin icons for the menu; the bitmap must be allocated by
    // calling CSalamanderGUIAbstract::CreateIconList() and then created and filled with the
    // CGUIIconListAbstract interface methods; the icon dimensions must be 16x16 pixels;
    // Salamander assumes ownership of the bitmap object, so the plugin must not destroy it
    // after calling this function; Salamander keeps it in memory only and does not save it elsewhere
    virtual void WINAPI SetIconListForMenu(CGUIIconListAbstract* iconList) = 0;
};

//
// ****************************************************************************
// CPluginInterfaceForMenuExtAbstract
//

// state flags of menu items (for menu extension plugins)
#define MENU_ITEM_STATE_ENABLED 0x01 // enabled, without this flag the item is disabled
#define MENU_ITEM_STATE_CHECKED 0x02 // a "check" or "radio" mark is shown before the item
#define MENU_ITEM_STATE_RADIO 0x04   // ignored without MENU_ITEM_STATE_CHECKED, \
                                     // "radio" mark, without this flag a "check" mark
#define MENU_ITEM_STATE_HIDDEN 0x08  // the item should not appear in the menu at all

class CPluginInterfaceForMenuExtAbstract
{
#ifdef INSIDE_SALAMANDER
private: // protection against incorrect direct method calls (see CPluginInterfaceForMenuExtEncapsulation)
    friend class CPluginInterfaceForMenuExtEncapsulation;
#else  // INSIDE_SALAMANDER
public:
#endif // INSIDE_SALAMANDER

    // returns the state of the menu item with identifier 'id'; the return value is a combination
    // of flags (see MENU_ITEM_STATE_XXX); for 'eventMask' see CSalamanderConnectAbstract::AddMenuItem
    virtual DWORD WINAPI GetMenuItemState(int id, DWORD eventMask) = 0;

    // runs the menu command with identifier 'id'; for 'eventMask' see
    // CSalamanderConnectAbstract::AddMenuItem; 'salamander' provides a set of usable Salamander
    // methods for carrying out operations (WARNING: it can be NULL, see the description of
    // CSalamanderGeneralAbstract::PostMenuExtCommand); 'parent' is the parent message box;
    // returns TRUE if the panel selection should be cleared (Cancel was not used, Skip might have been),
    // otherwise returns FALSE (no deselection takes place);
    // WARNING: If the command changes anything on a path (disk/FS), it should call
    //          CSalamanderGeneralAbstract::PostChangeOnPathNotification to inform panels
    //          without automatic refresh and open FS (active and disconnected)
    // NOTE: if the command works with files/directories from the active panel path, or even with
    //       that path directly, it must call CSalamanderGeneralAbstract::SetUserWorkedOnPanelPath
    //       for the active panel; otherwise the path will not be added to the List of Working
    //       Directories (Alt+F12)
    virtual BOOL WINAPI ExecuteMenuItem(CSalamanderForOperationsAbstract* salamander, HWND parent,
                                        int id, DWORD eventMask) = 0;

    // shows help for the menu command with identifier 'id' (the user presses Shift+F1,
    // finds this plugin's menu in Plugins, and selects a command from it); 'parent' is the parent
    // message box; returns TRUE if specific help was displayed; otherwise Salamander shows the
    // "Using Plugins" chapter from its help
    virtual BOOL WINAPI HelpForMenuItem(HWND parent, int id) = 0;

    // function for the "dynamic menu extension", called only if you pass FUNCTION_DYNAMICMENUEXT to
    // SetBasicPluginData; builds the plugin menu when it is loaded, and then again right before the menu
    // is opened in the Plugins menu or on the Plugin bar (also before opening the Keyboard Shortcuts
    // window from the Plugins Manager); commands in the new menu should keep the same IDs as the previous
    // menu so that user-assigned hotkeys remain valid and they can still be treated as the last command used
    // (see Plugins / Last Command); 'parent' is the parent message box; 'salamander' is the helper used to build the menu
    virtual void WINAPI BuildMenu(HWND parent, CSalamanderBuildMenuAbstract* salamander) = 0;
};

#ifdef _MSC_VER
#pragma pack(pop, enter_include_spl_menu)
#endif // _MSC_VER
#ifdef __BORLANDC__
#pragma option -a
#endif // __BORLANDC__
