// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#define USRMNUARGS_MAXLEN 32772    // buffer size (+1 beyond max string length) (=32776 (Vista/Win7 via .bat) - 5 ("C:\\a ") + 1)
#define USRMNUCMDLINE_MAXLEN 32777 // buffer size (+1 beyond maximum string length)

//****************************************************************************
//
// CUserMenuIconBkgndReader
//

struct CUserMenuIconData
{
    char FileName[MAX_PATH];  // file whose icon we read from IconIndex using ExtractIconEx()
    DWORD IconIndex;          // see comment for FileName
    char UMCommand[MAX_PATH]; // file whose icon we read via GetFileOrPathIconAux()

    HICON LoadedIcon; // NULL = icon not loaded, otherwise handle of the loaded icon

    CUserMenuIconData(const char* fileName, DWORD iconIndex, const char* umCommand);
    ~CUserMenuIconData();

    void Clear();
};

class CUserMenuIconDataArr : public TIndirectArray<CUserMenuIconData>
{
protected:
    DWORD IRThreadID; // unique thread ID used when loading these icons

public:
    CUserMenuIconDataArr() : TIndirectArray<CUserMenuIconData>(50, 50) { IRThreadID = 0; }

    void SetIRThreadID(DWORD id) { IRThreadID = id; }
    DWORD GetIRThreadID() { return IRThreadID; }

    HICON GiveIconForUMI(const char* fileName, DWORD iconIndex, const char* umCommand);
};

class CUserMenuIconBkgndReader
{
protected:
    BOOL SysColorsChanged; // helper variable to detect system-color changes since the configuration dialog opened

    CRITICAL_SECTION CS;       // critical section protecting access to data members
    DWORD IconReaderThreadUID; // unique thread ID generator for icon reading
    BOOL CurIRThreadIDIsValid; // TRUE if the thread is running and CurIRThreadID is valid
    DWORD CurIRThreadID;       // unique thread ID (see IconReaderThreadUID) used for reading icons of the current user-menu version
    BOOL AlreadyStopped;       // TRUE once icon reading is finished because the main window closed or is closing

    int UserMenuIconsInUse;                            // > 0: icons from the user menu are currently shown in an open menu, so we cannot update them immediately; at most 2 (Salamander config + Find dialog)
    CUserMenuIconDataArr* UserMenuIIU_BkgndReaderData; // storage for newly read icons while UserMenuIconsInUse > 0
    DWORD UserMenuIIU_ThreadID;                        // stored thread ID (to check data validity) when UserMenuIconsInUse > 0

public:
    CUserMenuIconBkgndReader();
    ~CUserMenuIconBkgndReader();

    // the main window is closing - stop accepting any icon data for the user menu
    void EndProcessing();

    // WARNING: 'bkgndReaderData' is deallocated inside
    void StartBkgndReadingIcons(CUserMenuIconDataArr* bkgndReaderData);

    BOOL IsCurrentIRThreadID(DWORD threadID);

    BOOL IsReadingIcons();

    // WARNING: after calling this function this object becomes responsible for releasing 'bkgndReaderData'
    void ReadingFinished(DWORD threadID, CUserMenuIconDataArr* bkgndReaderData);

    // enter/leave the section during which icons from the user menu are in use
    // and therefore cannot be updated (mainly when opening the user menu)
    void BeginUserMenuIconsInUse();
    void EndUserMenuIconsInUse();

    // if icons were loaded for an outdated user menu it returns FALSE;
    // if the icons are currently displayed in an open menu (see UserMenuIconsInUse) it returns FALSE;
    // if they are not displayed it returns TRUE and WARNING: the critical section is not exited,
    // so other threads (mostly the Find dialog) remain blocked. After updating icons use LeaveCSAfterUMIconsUpdate() to exit.
    BOOL EnterCSIfCanUpdateUMIcons(CUserMenuIconDataArr** bkgndReaderData, DWORD threadID);
    void LeaveCSAfterUMIconsUpdate();

    void ResetSysColorsChanged() { SysColorsChanged = FALSE; }
    void SetSysColorsChanged() { SysColorsChanged = TRUE; }
    BOOL HasSysColorsChanged() { return SysColorsChanged; }
};

extern CUserMenuIconBkgndReader UserMenuIconBkgndReader;

//****************************************************************************
//
// CUserMenuItem
//

enum CUserMenuItemType
{
    umitItem,         // regular item
    umitSubmenuBegin, // marks the start of a popup
    umitSubmenuEnd,   // marks the end of a popup
    umitSeparator     // marks the end of a popup
};

struct CUserMenuItem
{
    char *ItemName,
        *UMCommand,
        *Arguments,
        *InitDir,
        *Icon;

    int ThroughShell,
        CloseShell,
        UseWindow,
        ShowInToolbar;

    CUserMenuItemType Type;

    HICON UMIcon;

    CUserMenuItem(char* name, char* umCommand, char* arguments, char* initDir, char* icon,
                  int throughShell, int closeShell, int useWindow, int showInToolbar,
                  CUserMenuItemType type, CUserMenuIconDataArr* bkgndReaderData);

    CUserMenuItem();

    CUserMenuItem(CUserMenuItem& item, CUserMenuIconDataArr* bkgndReaderData);

    ~CUserMenuItem();

    // tries to obtain the icon handle in this order
    // a) the Icon variable
    // b) SHGetFileInfo
    // c) use the system default icon
    // background icon loading: if bkgndReaderData is NULL we load immediately; otherwise icons are loaded in the background. If getIconsFromReader is FALSE we gather what to load into bkgndReaderData; if TRUE the icons are already loaded and we just reuse handles from bkgndReaderData
    BOOL GetIconHandle(CUserMenuIconDataArr* bkgndReaderData, BOOL getIconsFromReader);

    // search ItemName for an & and return the hot key; TRUE if found
    BOOL GetHotKey(char* key);

    BOOL Set(char* name, char* umCommand, char* arguments, char* initDir, char* icon);
    void SetType(CUserMenuItemType type);
    BOOL IsGood() { return ItemName != NULL && UMCommand != NULL &&
                           Arguments != NULL && InitDir != NULL && Icon != NULL; }
};

//****************************************************************************
//
// CUserMenuItems
//

class CUserMenuItems : public TIndirectArray<CUserMenuItem>
{
public:
    CUserMenuItems(DWORD base, DWORD delta, CDeleteType dt = dtDelete)
        : TIndirectArray<CUserMenuItem>(base, delta, dt) {}

    // copies the item list from 'source'
    BOOL LoadUMI(CUserMenuItems& source, BOOL readNewIconsOnBkgnd);

    // searches for the closing item of the submenu identified by 'index'
    // returns -1 if the terminator is not found
    int GetSubmenuEndIndex(int index);
};
