// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

//*****************************************************************************
//
// CToolBarItem
//

class CToolBar;
class CTBCustomizeDialog;

class CToolBarItem
{
protected:
    DWORD Style;    // TLBI_STYLE_xxx
    DWORD State;    // TLBI_STATE_xxx
    DWORD ID;       // command id
    char* Text;     // allocated string
    int TextLen;    // length of string
    int ImageIndex; // Image index of the item. Set this member to -1 to
                    // indicate that the button does not have an image.
                    // The button layout will not include any space for
                    // a bitmap, only text.
    HICON HIcon;
    HICON HOverlay;
    DWORD CustomData; // FIXME_X64 - small for a pointer, is it ever needed?
    int Width;        // width of item (computed if TLBI_STYLE_AUTOSIZE is set)

    char* Name; // name in customize dialog (valid during custimize session)

    // these values allow optimized access to item states
    DWORD* Enabler; // Points to the variable controlling the item state.
                    // Nonzero value means TLBI_STATE_GRAYED bit is cleared.
                    // Zero value means TLBI_STATE_GRAYED bit is set.

    // internal data
    int Height; // height of item
    int Offset; // position of item in whole toolbar

    WORD IconX; // layout positions of the individual parts
    WORD TextX;
    WORD InnerX;
    WORD OutterX;

public:
    CToolBarItem();
    ~CToolBarItem();

    BOOL SetText(const char* text, int len = -1);

    friend class CToolBar;
    friend class CTBCustomizeDialog;
};

//*****************************************************************************
//
// CToolBar
//

class CBitmap;

class CToolBar : public CWindow, public CGUIToolBarAbstract
{
protected:
    TIndirectArray<CToolBarItem> Items;

    int Width; // size of the entire window
    int Height;
    HFONT HFont;
    int FontHeight;
    HWND HNotifyWindow; // target window for notifications
    HIMAGELIST HImageList;
    HIMAGELIST HHotImageList;
    int ImageWidth; // dimensions of a single image in the image list
    int ImageHeight;
    DWORD Style;          // TLB_STYLE_xxx
    BOOL DirtyItems;      // an operation affecting item layout occurred
                          // so a recalculation is required
    CBitmap* CacheBitmap; // off-screen bitmap used for drawing
    CBitmap* MonoBitmap;  // monochrome version for grayed icons
    int CacheWidth;       // bitmap dimensions
    int CacheHeight;
    int HotIndex; // -1 = none
    int DownIndex;
    BOOL DropPressed;
    BOOL MonitorCapture;
    BOOL RelayToolTip;
    TOOLBAR_PADDING Padding;
    BOOL HasIcon;       // if an icon is present, GetNeededSpace() accounts for its height
    BOOL HasIconDirty;  // should GetNeededSpace() re-check icon presence?
    BOOL Customizing;   // is the toolbar currently in customize mode?
    int InserMarkIndex; // -1 = none
    BOOL InserMarkAfter;
    BOOL MouseIsTracked;  // is the mouse tracked by TrackMouseEvent?
    DWORD DropDownUpTime; // time [ms] when the drop down was clicked to prevent re-activation
    BOOL HelpMode;        // Salamander is in Shift+F1 (context help) mode so disabled items should highlight too

public:
    //
    // Custom methods
    //
    CToolBar(HWND hNotifyWindow, CObjectOrigin origin = ooAllocated);
    ~CToolBar();

    //
    // Implementation of CGUIToolBarAbstract methods
    //

    virtual BOOL WINAPI CreateWnd(HWND hParent);
    virtual HWND WINAPI GetHWND() { return HWindow; }

    virtual int WINAPI GetNeededWidth(); // returns the width needed for the window
    virtual int WINAPI GetNeededHeight();

    virtual void WINAPI SetFont();
    virtual BOOL WINAPI GetItemRect(int index, RECT& r); // returns item location in screen coordinates

    virtual BOOL WINAPI CheckItem(DWORD position, BOOL byPosition, BOOL checked);
    virtual BOOL WINAPI EnableItem(DWORD position, BOOL byPosition, BOOL enabled);

    // if an image list is assigned, insert the icon at the given position
    // the 'normal' and 'hot' flags select which image lists are affected
    virtual BOOL WINAPI ReplaceImage(DWORD position, BOOL byPosition, HICON hIcon, BOOL normal = TRUE, BOOL hot = FALSE);

    virtual int WINAPI FindItemPosition(DWORD id);

    virtual void WINAPI SetImageList(HIMAGELIST hImageList);
    virtual HIMAGELIST WINAPI GetImageList();

    virtual void WINAPI SetHotImageList(HIMAGELIST hImageList);
    virtual HIMAGELIST WINAPI GetHotImageList();

    // toolbar style
    virtual void WINAPI SetStyle(DWORD style);
    virtual DWORD WINAPI GetStyle();

    virtual BOOL WINAPI RemoveItem(DWORD position, BOOL byPosition);
    virtual void WINAPI RemoveAllItems();

    virtual int WINAPI GetItemCount() { return Items.Count; }

    // opens the customization dialog
    virtual void WINAPI Customize();

    virtual void WINAPI SetPadding(const TOOLBAR_PADDING* padding);
    virtual void WINAPI GetPadding(TOOLBAR_PADDING* padding);

    // walks all items and if they have an 'EnablerData' pointer assigned,
    // compares the pointed value with the actual state. If it differs, updates the item
    virtual void WINAPI UpdateItemsState();

    // if the point is over an item (not a separator), returns its index
    // otherwise returns a negative number
    virtual int WINAPI HitTest(int xPos, int yPos);

    // returns TRUE if the position is on the item border and sets 'index'
    // to that item and 'after' to indicate left or right side
    // Returns FALSE if the point is over an item, TRUE and index = -1 if not over any item.
    virtual BOOL WINAPI InsertMarkHitTest(int xPos, int yPos, int& index, BOOL& after);

    // sets the InsertMark at position 'index' (before or after)
    // if index == -1 the InsertMark is removed
    virtual void WINAPI SetInsertMark(int index, BOOL after);

    // Sets the hot item in a toolbar. Returns the index of the previous hot item, or -1 if there was no hot item.
    virtual int WINAPI SetHotItem(int index);

    // color depth may have changed; rebuild CacheBitmap
    virtual void WINAPI OnColorsChanged();

    virtual BOOL WINAPI InsertItem2(DWORD position, BOOL byPosition, const TLBI_ITEM_INFO2* tii);
    virtual BOOL WINAPI SetItemInfo2(DWORD position, BOOL byPosition, const TLBI_ITEM_INFO2* tii);
    virtual BOOL WINAPI GetItemInfo2(DWORD position, BOOL byPosition, TLBI_ITEM_INFO2* tii);

protected:
    virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    void DrawDropDown(HDC hDC, int x, int y, BOOL grayed);
    void DrawItem(int index);
    void DrawItem(HDC hDC, int index);
    void DrawAllItems(HDC hDC);

    void DrawInsertMark(HDC hDC);

    // returns TRUE when an item exists at the position and sets 'index'
    // otherwise returns FALSE; if the user clicked the drop-down arrow,
    // 'dropDown' is set to TRUE
    BOOL HitTest(int xPos, int yPos, int& index, BOOL& dropDown);

    // walks all items and computes their 'MinWidth' and 'XOffset'
    // controlled by (and updating) DirtyItems
    // returns TRUE if all items were redrawn
    BOOL Refresh();

    friend class CTBCustomizeDialog;
};

//*****************************************************************************
//
// CTBCustomizeDialog
//

class CTBCustomizeDialog : public CCommonDialog
{
    enum TBCDDragMode
    {
        tbcdDragNone,
        tbcdDragAvailable,
        tbcdDragCurrent,
    };

protected:
    TDirectArray<TLBI_ITEM_INFO2> AllItems; // all available items
    CToolBar* ToolBar;
    HWND HAvailableLB;
    HWND HCurrentLB;
    DWORD DragNotify;
    TBCDDragMode DragMode;
    int DragIndex;

public:
    CTBCustomizeDialog(CToolBar* toolBar);
    ~CTBCustomizeDialog();
    BOOL Execute();

protected:
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    void DestroyItems();
    BOOL EnumButtons(); // fills the Items array with all buttons the toolbar can hold using the WM_USER_TBENUMBUTTON2 notification

    BOOL PresentInToolBar(DWORD id);      // is this command present in the toolbar?
    BOOL FindIndex(DWORD id, int* index); // find the command in AllItems
    void FillLists();                     // fill both list boxes

    void EnableControls();
    void MoveItem(int srcIndex, int tgtIndex);
    void OnAdd();
    void OnRemove();
    void OnUp();
    void OnDown();
    void OnReset();
};

//*****************************************************************************
//
// CMainToolBar
//
// Configurable toolbar carrying command buttons. It sits at the top of Salamander and above each panel.
//

enum CMainToolBarType
{
    mtbtTop,
    mtbtMiddle,
    mtbtLeft,
    mtbtRight,
};

class CMainToolBar : public CToolBar
{
protected:
    CMainToolBarType Type;

public:
    CMainToolBar(HWND hNotifyWindow, CMainToolBarType type, CObjectOrigin origin = ooStatic);

    BOOL Load(const char* data);
    BOOL Save(char* data);

    // a tooltip needs to be returned
    void OnGetToolTip(LPARAM lParam);
    // during customization fills the dialog with toolbar items
    BOOL OnEnumButton(LPARAM lParam);
    // user pressed reset in the customization dialog - load the default layout
    void OnReset();

    void SetType(CMainToolBarType type);

protected:
    // fills 'tii' with data for item 'tbbeIndex' and returns TRUE
    // returns FALSE if the item is incomplete (command removed)
    BOOL FillTII(int tbbeIndex, TLBI_ITEM_INFO2* tii, BOOL fillName); // 'buttonIndex' comes from the TBBE_xxxx family; -1 = separator
};

//*****************************************************************************
//
// CBottomToolBar
//
// toolbar at the bottom of Salamander - shows help for F1-F12 combined
// with Ctrl, Alt and Shift
//

enum CBottomTBStateEnum
{
    btbsNormal,
    btbsAlt,
    btbsCtrl,
    btbsShift,
    btbsCtrlShift,
    //  btbsCtrlAlt,
    btbsAltShift,
    //  btbsCtrlAltShift,
    btbsMenu,
    btbsCount
};

class CBottomToolBar : public CToolBar
{
public:
    CBottomToolBar(HWND hNotifyWindow, CObjectOrigin origin = ooStatic);

    virtual BOOL WINAPI CreateWnd(HWND hParent);

    // called whenever modifiers (Ctrl, Alt, Shift) change - iterates the
    // toolbar items and sets their texts and IDs
    BOOL SetState(CBottomTBStateEnum state);

    // initializes the static array later used to fill the toolbar
    static BOOL InitDataFromResources();

    void OnGetToolTip(LPARAM lParam);

    virtual void WINAPI SetFont();

protected:
    CBottomTBStateEnum State;

    // internal function called from InitDataFromResources
    static BOOL InitDataResRow(CBottomTBStateEnum state, int textResID);

    // for each button finds the longest text and sets its width accordingly
    BOOL SetMaxItemWidths();
};

//*****************************************************************************
//
// CUserMenuBar
//

class CUserMenuBar : public CToolBar
{
public:
    CUserMenuBar(HWND hNotifyWindow, CObjectOrigin origin = ooStatic);

    // loads items from UserMenu and fills the toolbar with buttons
    BOOL CreateButtons();

    void ToggleLabels();
    virtual int WINAPI GetNeededHeight();

    virtual void WINAPI Customize();

    virtual void WINAPI SetInsertMark(int index, BOOL after);
    virtual int WINAPI SetHotItem(int index);

    void OnGetToolTip(LPARAM lParam);

protected:
    virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

//*****************************************************************************
//
// CHotPathsBar
//

class CHotPathsBar : public CToolBar
{
public:
    CHotPathsBar(HWND hNotifyWindow, CObjectOrigin origin = ooStatic);

    // loads items from HotPaths and fills the toolbar with buttons
    BOOL CreateButtons();

    void ToggleLabels();
    virtual int WINAPI GetNeededHeight();

    virtual void WINAPI Customize();

    //    void SetInsertMark(int index, BOOL after);
    //    int SetHotItem(int index);

    void OnGetToolTip(LPARAM lParam);

protected:
    virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

//*****************************************************************************
//
// CDriveBar
//

class CDrivesList;

class CDriveBar : public CToolBar
{
protected:
    // return values for List
    DWORD DriveType;
    DWORD_PTR DriveTypeParam;
    int PostCmd;
    void* PostCmdParam;
    BOOL FromContextMenu;
    CDrivesList* List;

    // cache: contains ?: or \\ for UNC or an empty string
    char CheckedDrive[3];

public:
    // plugin icons should be displayed in grayscale, so we keep them in image lists
    HIMAGELIST HDrivesIcons;
    HIMAGELIST HDrivesIconsGray;

public:
    CDriveBar(HWND hNotifyWindow, CObjectOrigin origin = ooStatic);
    ~CDriveBar();

    void DestroyImageLists();

    // removes existing buttons and adds new ones;
    // if 'copyDrivesListFrom' is not NULL, drive data are copied instead of re-read
    // 'copyDrivesListFrom' may even refer to this object
    BOOL CreateDriveButtons(CDriveBar* copyDrivesListFrom);

    virtual int WINAPI GetNeededHeight();

    void OnGetToolTip(LPARAM lParam);

    // user clicked the button with command id
    void Execute(DWORD id);

    // presses the icon matching the path; if none is found, none will be pressed; the 'force' variable bypasses the cache
    void SetCheckedDrive(CFilesWindow* panel, BOOL force = FALSE);

    // when a disk is added or removed we must rebuild the list;
    // if 'copyDrivesListFrom' is not NULL, drive data are copied instead of re-read
    // 'copyDrivesListFrom' may refer to this object
    void RebuildDrives(CDriveBar* copyDrivesListFrom = NULL);

    // a context menu should be displayed; the item is determined from GetMessagePos
    // returns TRUE when a button was hit and the menu was shown; otherwise FALSE
    BOOL OnContextMenu();

    // returns the bitmask of drives obtained during the last List->BuildData()
    // if BuildData() hasn't run yet, returns 0
    // can be used to quickly detect whether any drive changed
    DWORD GetCachedDrivesMask();

    // returns the bitmask of available cloud storages obtained during the last List->BuildData()
    // if BuildData() hasn't run yet, returns 0
    // helps detect changes in availability of cloud storages
    DWORD GetCachedCloudStoragesMask();

protected:
    virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

//*****************************************************************************
//
// CPluginsBar
//

class CPluginsBar : public CToolBar
{
protected:
    // icons representing plugins created via CPlugins::CreateIconsList
    HIMAGELIST HPluginsIcons;
    HIMAGELIST HPluginsIconsGray;

public:
    CPluginsBar(HWND hNotifyWindow, CObjectOrigin origin = ooStatic);
    ~CPluginsBar();

    void DestroyImageLists();

    // removes existing buttons and creates new ones
    BOOL CreatePluginButtons();

    virtual int WINAPI GetNeededHeight();

    virtual void WINAPI Customize();

    void OnGetToolTip(LPARAM lParam);

    //  protected:
    //    virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

extern void PrepareToolTipText(char* buff, BOOL stripHotKey);

extern void GetSVGIconsMainToolbar(CSVGIcon** svgIcons, int* svgIconsCount);
