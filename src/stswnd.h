// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

//
// ****************************************************************************

class CMainToolBar;

enum CBorderLines
{
    blNone = 0x00,
    blTop = 0x01,
    blBottom = 0x02
};

enum CSecurityIconState
{
    sisNone = 0x00,      // icon is not displayed
    sisUnsecured = 0x01, // unlocked lock icon is displayed
    sisSecured = 0x02    // locked lock icon is displayed
};

/*
enum
{
  otStatusWindow = otLastWinLibObject
};
*/

//
// CHotTrackItem
//
// Item holds the index of the first character, number of characters,
// pixel offset and length of the first character in pixels. A list of these
// items is built for the displayed path and stored in an array.
//
// For the path "\\john\\c\\winnt"
//
// the following items are created:
//
// (0, 9,  0, length of the first nine characters)   = \\john\\c\\
// (0, 14, 0, length of 14 characters)               = \\john\\c\\winnt
//
// For "DIR: 12"
//
// (0, 3, 0, length of the three characters DIR)
// (5, 2, pixel offset of "12", length of the two characters "12")

struct CHotTrackItem
{
    WORD Offset;       // offset of the first character in characters
    WORD Chars;        // number of characters
    WORD PixelsOffset; // pixel offset of the first character
    WORD Pixels;       // length in pixels
};

class CStatusWindow : public CWindow
{
public:
    CMainToolBar* ToolBar;
    CFilesWindow* FilesWindow;

protected:
    TDirectArray<CHotTrackItem> HotTrackItems;
    BOOL HotTrackItemsMeasured;

    int Border; // separator line at top/bottom
    char* Text;
    int TextLen; // number of characters pointed to by 'Text' without terminator
    char* Size;
    int PathLen;          // -1 means the entire Text is the path, otherwise it specifies path length in Text (the remainder is filter)
    BOOL History;         // display arrow between text and size?
    BOOL Hidden;          // display filter icon?
    int HiddenFilesCount; // number of filtered out files
    int HiddenDirsCount;  // number of filtered out directories
    BOOL WholeTextVisible;

    BOOL ShowThrobber;             // TRUE if a 'progress' throbber should be shown behind the text/hidden filter (independent of window existence)
    BOOL DelayedThrobber;          // TRUE if the timer for displaying the throbber is already running
    DWORD DelayedThrobberShowTime; // GetTickCount() value when the delayed throbber should appear (0 = no delayed display)
    BOOL Throbber;                 // show the 'progress' throbber behind the text/hidden filter? (TRUE only if the window exists)
    int ThrobberFrame;             // index of the current animation frame
    char* ThrobberTooltip;         // if NULL, the tooltip is not shown
    int ThrobberID;                // throbber identification number (-1 = invalid)

    CSecurityIconState Security;
    char* SecurityTooltip; // if NULL, the tooltip is not shown

    int Allocated;
    int* AlpDX; // array of lengths (from the first to the X-th character in the string)
    BOOL Left;

    int ToolBarWidth; // current toolbar width

    int EllipsedChars; // number of omitted characters after the root; otherwise -1
    int EllipsedWidth; // length of the omitted string after the root; otherwise -1

    CHotTrackItem* HotItem;     // highlighted item
    CHotTrackItem* LastHotItem; // last highlighted item
    BOOL HotSize;               // the size field is highlighted
    BOOL HotHistory;            // the history field is highlighted
    BOOL HotZoom;               // the zoom field is highlighted
    BOOL HotHidden;             // the filter symbol is highlighted
    BOOL HotSecurity;           // the lock icon is highlighted

    RECT TextRect;     // rectangle where the text was drawn
    RECT HiddenRect;   // rectangle where the filter symbol was drawn
    RECT SizeRect;     // rectangle where the size text was drawn
    RECT HistoryRect;  // rectangle where the history drop-down was drawn
    RECT ZoomRect;     // rectangle where the zoom drop-down was drawn
    RECT ThrobberRect; // rectangle where the throbber was drawn
    RECT SecurityRect; // rectangle where the lock icon was drawn
    int MaxTextRight;
    BOOL MouseCaptured;
    BOOL RButtonDown;
    BOOL LButtonDown;
    POINT LButtonDownPoint; // location where the user pressed the left button

    int Height;
    int Width; // dimensions

    BOOL NeedToInvalidate; // for SetAutomatic() - a change occurred, should we repaint?

    DWORD* SubTexts;     // array of DWORDs: LOWORD is position, HIWORD is length
    DWORD SubTextsCount; // number of items in the SubTexts array

    IDropTarget* IDropTargetPtr;

public:
    CStatusWindow(CFilesWindow* filesWindow, int border, CObjectOrigin origin = ooAllocated);
    ~CStatusWindow();

    BOOL SetSubTexts(DWORD* subTexts, DWORD subTextsCount);
    // sets the text into the status line; 'pathLen' specifies the path length (the remainder is filter)
    // if 'pathLen' is unused (the path is the entire 'text') it is -1
    BOOL SetText(const char* text, int pathLen = -1);

    // builds the HotTrackItems array: for disks and archives it uses backslash positions
    // and for file systems it queries the plugin
    void BuildHotTrackItems();

    void GetHotText(char* buffer, int bufSize);

    void DestroyWindow();

    int GetToolBarWidth() { return ToolBarWidth; }

    int GetNeededHeight();
    void SetSize(const CQuadWord& size);
    void SetHidden(int hiddenFiles, int hiddenDirs);
    void SetHistory(BOOL history);
    void SetThrobber(BOOL show, int delay = 0, BOOL calledFromDestroyWindow = FALSE); // call only from the main (GUI) thread like other object methods
    // sets the tooltip text shown when the mouse is over the throbber; the object makes a copy
    // if NULL, no tooltip is shown
    void SetThrobberTooltip(const char* throbberTooltip);
    int ChangeThrobberID(); // changes ThrobberID and returns its new value
    BOOL IsThrobberVisible(int throbberID) { return ShowThrobber && ThrobberID == throbberID; }
    void HideThrobberAndSecurityIcon();

    void SetSecurity(CSecurityIconState iconState);
    void SetSecurityTooltip(const char* tooltip);

    void InvalidateIfNeeded();

    void LayoutWindow();
    void Paint(HDC hdc, BOOL highlightText = FALSE, BOOL highlightHotTrackOnly = FALSE);
    void Repaint(BOOL flashText = FALSE, BOOL hotTrackOnly = FALSE);
    void InvalidateAndUpdate(BOOL update); // can also be called when HWindow == NULL
    void FlashText(BOOL hotTrackOnly = FALSE);

    BOOL FindHotTrackItem(int xPos, int& index);

    void SetLeftPanel(BOOL left);
    BOOL ToggleToolBar();

    BOOL IsLeft() { return Left; }

    BOOL SetDriveIcon(HICON hIcon);     // the icon is copied to the image list - the caller must destroy it
    void SetDrivePressed(BOOL pressed); // presses the drive icon

    BOOL GetTextFrameRect(RECT* r);   // returns rectangle around the text in screen coordinates
    BOOL GetFilterFrameRect(RECT* r); // returns rectangle around the filter symbol in screen coordinates

    // the display color depth might have changed; CacheBitmap must be rebuilt
    void OnColorsChanged();

    void SetFont();

protected:
    virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    void RegisterDragDrop();
    void RevokeDragDrop();

    // creates an image list with a single item used to display drag progress
    // after dragging ends this image list must be freed
    // the input point is used to compute dxHotspot and dyHotspot offsets
    HIMAGELIST CreateDragImage(const char* text, int& dxHotspot, int& dyHotspot, int& imgWidth, int& imgHeight);

    void PaintThrobber(HDC hDC);
    //    void RepaintThrobber();

    void PaintSecurity(HDC hDC);
};
