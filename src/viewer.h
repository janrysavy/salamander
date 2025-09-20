// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#define VIEW_BUFFER_SIZE 60000 // 0.5 * VIEW_BUFFER_SIZE must be greater than the maximum \
                               // displayable line length
#define BORDER_WIDTH 3         // separates the text from the window edge
#define APROX_LINE_LEN 1000

#define FIND_TEXT_LEN 201                    // +1; NOTE: should match GREP_TEXT_LEN
#define FIND_LINE_LEN 10000                  // must be > FIND_TEXT_LEN and the maximum line length for REGEXP (GREP uses a different macro)
#define TEXT_MAX_LINE_LEN 10000              // when the line is longer we prompt for hex mode; must be <= FIND_LINE_LEN
#define RECOGNIZE_FILE_TYPE_BUFFER_LEN 10000 // how many characters from the file start should be used when detecting file type (RecognizeFileType())

#define VIEWER_HISTORY_SIZE 30 // number of remembered strings

// menu positions - update when the menu changes!
#define VIEWER_FILE_MENU_INDEX 0         // in the viewer's main menu
#define VIEWER_FILE_MENU_OTHFILESINDEX 3 // in the File submenu of the viewer main menu
#define VIEWER_EDIT_MENU_INDEX 1         // in the viewer's main menu
#define VIEW_MENU_INDEX 3                // in the viewer's main menu
#define CODING_MENU_INDEX 4              // in the viewer's main menu
#define OPTIONS_MENU_INDEX 5             // in the viewer's main menu

#define WM_USER_VIEWERREFRESH WM_APP + 201 // [0, 0] - a refresh should be performed

#ifndef INSIDE_SALAMANDER
char* LoadStr(int resID);
char* GetErrorText(DWORD error);
#endif // INSIDE_SALAMANDER

extern char* ViewerHistory[VIEWER_HISTORY_SIZE];

void HistoryComboBox(HWND hWindow, CTransferInfo& ti, int ctrlID, char* Text,
                     int textLen, BOOL hexMode, int historySize, char* history[],
                     BOOL changeOnlyHistory = FALSE);
void DoHexValidation(HWND edit, const int textLen);
void ConvertHexToString(char* text, char* hex, int& len);

int GetHexOffsetMode(unsigned __int64 fileSize, int& hexOffsetLength);
void PrintHexOffset(char* s, unsigned __int64 offset, int mode);

void GetDefaultViewerLogFont(LOGFONT* lf);

// ****************************************************************************

class CFindSetDialog : public CCommonDialog
{
public:
    int Forward, // forward/backward (1/0)
        WholeWords,
        CaseSensitive,
        HexMode,
        Regular;

    char Text[FIND_TEXT_LEN];

    CFindSetDialog(HINSTANCE modul, int resID, UINT helpID)
        : CCommonDialog(modul, resID, helpID, NULL, ooStatic)
    {
        Forward = TRUE;
        WholeWords = FALSE;
        CaseSensitive = FALSE;
        HexMode = FALSE;
        Regular = FALSE;
        Text[0] = 0;
    }

    CFindSetDialog& operator=(CFindSetDialog& d)
    {
        Forward = d.Forward;
        WholeWords = d.WholeWords;
        CaseSensitive = d.CaseSensitive;
        HexMode = d.HexMode;
        Regular = d.Regular;
        memmove(Text, d.Text, FIND_TEXT_LEN);
        return *this;
    }

    virtual void Transfer(CTransferInfo& ti);

protected:
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    int CancelHexMode, // for correct Cancel button functionality only
        CancelRegular;
};

// ****************************************************************************

class CViewerGoToOffsetDialog : public CCommonDialog
{
public:
    CViewerGoToOffsetDialog(HWND parent, __int64* offset)
        : CCommonDialog(HLanguage, IDD_VIEWERGOTOOFFSET, IDD_VIEWERGOTOOFFSET, parent) { Offset = offset; }

    virtual void Validate(CTransferInfo& ti);
    virtual void Transfer(CTransferInfo& ti);

protected:
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
    __int64* Offset;
};

// ****************************************************************************

enum CViewType
{
    vtText,
    vtHex
};

class CViewerWindow : public CWindow
{
public:
    CViewerWindow(const char* fileName, CViewType type, const char* caption,
                  BOOL wholeCaption, CObjectOrigin origin, int enumFileNamesSourceUID,
                  int enumFileNamesLastFileIndex);
    ~CViewerWindow();

    void OpenFile(const char* file, const char* caption, BOOL wholeCaption); // does not handle Lock

    virtual BOOL Is(int type) { return type == otViewerWindow || CWindow::Is(type); }
    BOOL IsGood() { return Buffer != NULL && ViewerFont != NULL; }
    void InitFindDialog(CFindSetDialog& dlg)
    {
        FindDialog = dlg;
        if (FindDialog.Text[0] == 0)
            return;
        else
        {
            if (FindDialog.Regular)
            {
                RegExp.Set(FindDialog.Text, 0);
            }
            else
            {
                if (FindDialog.HexMode)
                {
                    char hex[FIND_TEXT_LEN];
                    int len;
                    ConvertHexToString(FindDialog.Text, hex, len);
                    SearchData.Set(hex, len, 0);
                }
                else
                    SearchData.Set(FindDialog.Text, 0);
            }
        }
    }

    HANDLE GetLockObject(); // object for disk cache when viewing from a ZIP archive
    void CloseLockObject();

    void ConfigHasChanged(); // called after pressing OK in the configuration dialog

    // returns text for Find - the marked block, null-terminated, 'buf' is at least
    // FIND_TEXT_LEN bytes; returns TRUE if buf is filled (a block exists, etc.) and
    // writes the number of characters without the null terminator into 'len'
    BOOL GetFindText(char* buf, int& len);

protected:
    void FatalFileErrorOccured(DWORD repeatCmd = -1); // called when a file error occurs (viewer must refresh or clear)

    void OnVScroll();

    void CodeCharacters(unsigned char* start, unsigned char* end);
    // if 'hFile' is NULL, Prepare/LoadBefore/LoadBehind open and close the file themselves
    // if 'hFile' points to a variable (initialize it to NULL initially), the methods open the file
    // and store the handle there on success. On the next call they reuse that handle without reopening.
    // The handle is not closed on exit; the caller must do that. This optimizes network disks
    // where repeated open/close operations severely slowed down searching.
    BOOL LoadBefore(HANDLE* hFile);
    BOOL LoadBehind(HANDLE* hFile);

    // if a read error occurs and fatalErr == TRUE, ExitTextMode is not set here (it never becomes TRUE)
    __int64 Prepare(HANDLE* hFile, __int64 offset, __int64 bytes, BOOL& fatalErr);

    void GoToEnd() { SeekY = MaxSeekY; }
    // if a read error occurs and fatalErr == TRUE, ExitTextMode is TRUE when switching to Hex mode
    void FileChanged(HANDLE file, BOOL testOnlyFileSize, BOOL& fatalErr, BOOL detectFileType,
                     BOOL* calledHeightChanged = NULL);
    // if a read error occurs and fatalErr == TRUE, ExitTextMode is TRUE when switching to Hex mode
    void HeightChanged(BOOL& fatalErr);
    // if a read error occurs and fatalErr == TRUE, ExitTextMode is TRUE when switching to Hex mode
    __int64 ZeroLineSize(BOOL& fatalErr, __int64* firstLineEndOff = NULL, __int64* firstLineCharLen = NULL);

    // text mode only; if a read error occurs and fatalErr == TRUE, ExitTextMode is TRUE when switching to Hex mode
    __int64 FindSeekBefore(__int64 seek, int lines, BOOL& fatalErr, __int64* firstLineEndOff = NULL,
                           __int64* firstLineCharLen = NULL, BOOL addLineIfSeekIsWrap = FALSE);
    // see the comment for Prepare() about the 'hFile' parameter; if a read error occurs and fatalErr == TRUE,
    // ExitTextMode is not set here (it never becomes TRUE)
    BOOL FindNextEOL(HANDLE* hFile, __int64 seek, __int64 maxSeek, __int64& lineEnd, __int64& nextLineBegin, BOOL& fatalErr);
    // if a read error occurs and fatalErr == TRUE, ExitTextMode is TRUE when switching to Hex mode
    BOOL FindPreviousEOL(HANDLE* hFile, __int64 seek, __int64 minSeek, __int64& lineBegin,
                         __int64& previousLineEnd, BOOL allowWrap,
                         BOOL takeLineBegin, BOOL& fatalErr, int* lines, __int64* firstLineEndOff = NULL,
                         __int64* firstLineCharLen = NULL, BOOL addLineIfSeekIsWrap = FALSE);

    // if a read error occurs and fatalErr == TRUE, ExitTextMode is TRUE when switching to Hex mode
    __int64 FindBegin(__int64 seek, BOOL& fatalErr);
    void ChangeType(CViewType type);

    void Paint(HDC dc);
    void SetScrollBar();
    virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    // posts WM_MOUSEMOVE (used to move the block end to new mouse coordinates or
    // to recompute the block end when the view moves)
    void PostMouseMove();

    // scroll the view one line up
    BOOL ScrollViewLineUp(DWORD repeatCmd = -1, BOOL* scrolled = NULL, BOOL repaint = TRUE,
                          __int64* firstLineEndOff = NULL, __int64* firstLineCharLen = NULL);

    // scroll the view one line down
    BOOL ScrollViewLineDown(BOOL fullRedraw = FALSE);

    // invalidate and optionally update the selected rows of the view
    void InvalidateRows(int minRow, int maxRow, BOOL update = TRUE);

    // adjusts OriginX if necessary so that the x-coordinate 'x' is visible in the view
    void EnsureXVisibleInView(__int64 x, BOOL showPrevChar, BOOL& fullRedraw,
                              __int64 newFirstLineLen = -1, BOOL ignoreFirstLine = FALSE,
                              __int64 maxLineLen = -1);

    // returns the maximum length of a visible line in the view (text mode: the view must be repainted,
    // otherwise LineOffset is out of date)
    __int64 GetMaxVisibleLineLen(__int64 newFirstLineLen = -1, BOOL ignoreFirstLine = FALSE);

    // returns the maximum OriginX for the current view (text mode: the view must be repainted,
    // otherwise LineOffset is out of date)
    __int64 GetMaxOriginX(__int64 newFirstLineLen = -1, BOOL ignoreFirstLine = FALSE, __int64 maxLineLen = -1);

    // determine the x-coordinate 'x' of file offset 'offset' on a line;
    // if 'lineInView' is not -1, the line is taken from LineOffset and
    // 'lineBegOff', 'lineCharLen' and 'lineEndOff' are ignored
    BOOL GetXFromOffsetInText(__int64* x, __int64 offset, int lineInView, __int64 lineBegOff = -1,
                              __int64 lineCharLen = -1, __int64 lineEndOff = -1);

    // finds the nearest file position 'offset' for the proposed x-coordinate 'suggestedX' on a line;
    // 'x' receives the x-coordinate of 'offset' on the line; if 'lineInView' is not -1,
    // the line 'lineInView' from LineOffset is used and 'lineBegOff', 'lineCharLen' and 'lineEndOff'
    // are ignored
    BOOL GetOffsetFromXInText(__int64* x, __int64* offset, __int64 suggestedX, int lineInView,
                              __int64 lineBegOff = -1, __int64 lineCharLen = -1,
                              __int64 lineEndOff = -1);

    // returns the file offset corresponding to screen coordinates [x, y]; if 'leftMost'
    // is FALSE the offset differs for the left and right half of a character,
    // TRUE means any part of the character yields the same offset (used for block detection);
    // if 'onHexNum' != NULL and the viewer is in hex mode and [x, y] lands on a hex digit or character,
    // '*onHexNum' will be TRUE
    // if a read error occurs, fatalErr == TRUE and ExitTextMode is not triggered
    BOOL GetOffset(__int64 x, __int64 y, __int64& offset, BOOL& fatalErr, BOOL leftMost = FALSE,
                   BOOL* onHexNum = NULL);

    // returns the offset 'offset' in the file for x-coordinate 'x' in the line starting at
    // 'lineBegOff' of length 'lineCharLen' (expanded tabs, not used in hex mode -> set to 0)
    // and ending at 'lineEndOff' (not used in hex mode -> set to 0);
    // 'offsetX' receives the x-coordinate of 'offset' (text mode only and may differ from 'x');
    // if 'onHexNum' != NULL and hex mode is active and the x-coordinate 'x' hits a hex digit or character,
    // '*onHexNum' will be TRUE; if a read error occurs, fatalErr == TRUE and ExitTextMode is not triggered;
    // when 'getXFromOffset' is TRUE (text mode only, pass NULL for 'offset', 'offsetX' and 'onHexNum'),
    // the function returns the X coordinate (in 'foundX') of the character at 'findOffset'
    BOOL GetOffsetOrXAbs(__int64 x, __int64* offset, __int64* offsetX, __int64 lineBegOff, __int64 lineCharLen,
                         __int64 lineEndOff, BOOL& fatalErr, BOOL* onHexNum, BOOL getXFromOffset = FALSE,
                         __int64 findOffset = -1, __int64* foundX = NULL);

    // for large selections (over 100 MB) ask the user if they really want to allocate the selection
    // for drag & drop or clipboard use
    BOOL CheckSelectionIsNotTooBig(HWND parent, BOOL* msgBoxDisplayed = NULL);

    // if a read error occurs and fatalErr == TRUE, ExitTextMode is not set here (it never becomes TRUE)
    HGLOBAL GetSelectedText(BOOL& fatalErr); // text for clipboard and drag&drop operations

    void SetToolTipOffset(__int64 offset);

    void SetViewerCaption();

    // sets CodeType, UseCodeTable, CodeTable and the window caption
    // NOTE: CodeTables.Valid(c) must return TRUE
    void SetCodeType(int c);

    BOOL CreateViewerBrushs();
    void ReleaseViewerBrushs();
    void SetViewerFont();

    void ResetMouseWheelAccumulator()
    {
        MouseWheelAccumulator = 0;
        MouseHWheelAccumulator = 0;
    }

    void ReleaseMouseDrag();

    void FindNewSeekY(__int64 newSeekY, BOOL& fatalErr);

    // internally calls SalMessageBox; it blocks Paint only for it (only clears the viewer background, does not touch the file)
    int SalMessageBoxViewerPaintBlocked(HWND hParent, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType);

    unsigned char* Buffer; // buffer of size VIEW_BUFFER_SIZE
    char* FileName;        // currently viewed file
    __int64 Seek,          // offset of the first byte in Buffer within the file
        Loaded,            // number of valid bytes in Buffer
        OriginX,           // first displayed column (in characters)
        SeekY,             // seek of the first displayed line
        MaxSeekY,          // seek of the end of the viewed file
        FileSize,          // file size
        ViewSize,          // size of the viewed part of the file (in bytes)
        FirstLineSize,     // size of the first displayed line (in bytes)
        LastLineSize;      // size of the last completely displayed line (in bytes)

    __int64 StartSelection,           // seek position of the selection start (including that character) (-1 = no selection yet)
        EndSelection;                 // seek position of the selection end (up to but not including that character) (-1 = no selection yet)
    int TooBigSelAction;              // drag-and-drop or automatic copy to clipboard of a block over 100 MB: 0 = ask, 1 = YES, 2 = NO
    int EndSelectionRow;              // y-offset of the row containing EndSelection (relative to the client area)
                                      // valid only while dragging a block; used to optimize painting
                                      // if -1, no optimization is performed
    __int64 EndSelectionPrefX;        // preferred x-coordinate when dragging the block end via Shift+Up/Down (-1 = none)
    TDirectArray<__int64> LineOffset; // array with line start and end offsets (without EOL) and lengths in displayed characters (a triplet per line)
    BOOL WrapIsBeforeFirstLine;       // text view with wrap: a wrap precedes the first line of the view (not an EOL)
    BOOL MouseDrag;                   // dragging a block with the mouse?
    BOOL ChangingSelWithShiftKey;     // changing the selection via Shift+key (arrows, End, Home)

    CFindSetDialog FindDialog;
    CSearchData SearchData;
    CRegularExpression RegExp;
    __int64 FindOffset,              // offset from which to search
        LastFindSeekY,               // seek of the first screen line after a search, used to detect back and forth movement
        LastFindOffset;              // offset from which to search (set after searching), used to detect back and forth movement
    BOOL ResetFindOffsetOnNextPaint; // TRUE = set FindOffset during the next WM_PAINT (page size is known after painting and FindOffset can be set for searching backward)
    BOOL SelectionIsFindResult;      // TRUE = the selection is the result of a find

    int DefViewMode; // 0 = Auto-Select, 1 = Text, 2 = Hex
    CViewType Type;  // view mode
    BOOL EraseBkgnd; // erase the background once at startup

    int Width,  // window width (in points)
        Height; // window height (in points)

    BOOL EnablePaint; // because of recursive Paint calls on errors: FALSE = only clears the viewer background (does not touch the file)

    BOOL ScrollToSelection; // on the next redraw scroll to the selection (OriginX)

    double ScrollScaleX,  // horizontal scrollbar coefficient
        ScrollScaleY;     // vertical scrollbar coefficient
    BOOL EnableSetScroll; // do not refresh scrollbar data during a drag

    __int64 ToolTipOffset; // hex mode: file offset shown in the tooltip
    HWND HToolTip;         // tooltip window

    HANDLE Lock; // handle for disk cache

    BOOL WrapText; // local copy of Configuration.WrapText

    BOOL CodePageAutoSelect;  // local copy of Configuration.CodePageAutoSelect
    char DefaultConvert[200]; // local copy of Configuration.DefaultConvert

    BOOL ExitTextMode;  // TRUE = processing of the current message must quickly end; switching to hex
                        //        mode (the file is unsuitable for text view, it lacks EOLs)
    BOOL ForceTextMode; // TRUE = the user wants text mode at any cost (will wait)

    int CodeType;        // numeric identifier of the encoding, memory of the CodeTables object for this viewer window
    BOOL UseCodeTable;   // should it be recoded using CodeTable?
    char CodeTable[256]; // encoding table

    char CurrentDir[MAX_PATH]; // directory for the open dialog

    BOOL WaitForViewerRefresh;   // TRUE - waiting for WM_USER_VIEWERREFRESH; other commands are skipped
    __int64 LastSeekY;           // SeekY before the error
    __int64 LastOriginX;         // OriginX before the error
    DWORD RepeatCmdAfterRefresh; // command to repeat after refresh (-1 = no command)

    char* Caption;     // if not NULL, contains the proposed window caption for the viewer
    BOOL WholeCaption; // meaningful only when Caption != NULL. TRUE -> only Caption is shown in
                       // the viewer title; FALSE -> the standard " - Viewer" suffix is appended

    BOOL CanSwitchToHex,           // TRUE if switching to "hex" is possible when more than 10000 characters per line
        CanSwitchQuietlyToHex,     // TRUE if no confirmation is needed for switching (when loading a file)
        FindingSoDonotSwitchToHex; // TRUE to block switching to "hex" with lines over 10000 chars (undesired during text search)

    int HexOffsetLength; // hex mode: number of characters in the offset (in the first column from the left)

    // GDI objects (each thread has its own)
    HBRUSH BkgndBrush;    // solid brush of the window background color
    HBRUSH BkgndBrushSel; // solid brush of the window background color - selected text

    CBitmap Bitmap;
    HFONT ViewerFont;

    int EnumFileNamesSourceUID;     // UID of our source for enumerating names in the viewer
    int EnumFileNamesLastFileIndex; // index of the currently viewed file

    WPARAM VScrollWParam; // wParam from WM_VSCROLL/SB_THUMB*; -1 when no dragging is in progress
    WPARAM VScrollWParamOld;

    int MouseWheelAccumulator;  // vertical
    int MouseHWheelAccumulator; // horizontal
};

// ****************************************************************************

BOOL InitializeViewer();
void ReleaseViewer();
void ClearViewerHistory(BOOL dataOnly); // clears histories; when dataOnly==FALSE clears the Find dialog combobox (if any)
void UpdateViewerColors(SALCOLOR* colors);

extern const char* CVIEWERWINDOW_CLASSNAME; // viewer window class name

extern CWindowQueue ViewerWindowQueue; // list of all viewer windows

extern CFindSetDialog GlobalFindDialog; // for initializing a new viewer window

extern BOOL UseCustomViewerFont; // if TRUE, use the ViewerLogFont structure from the configuration; otherwise use defaults
extern LOGFONT ViewerLogFont;
extern HMENU ViewerMenu;
extern HACCEL ViewerTable;
extern int CharWidth, // character width (in points)
    CharHeight;       // character height (in points)

// Vista: the Fixedsys font contains characters that do not have the expected
// width even though it is fixed-width. We therefore measure each character
// and map those with wrong width to replacement characters of the proper width
extern CRITICAL_SECTION ViewerFontMeasureCS; // critical section for measuring the font
extern BOOL ViewerFontMeasured;              // TRUE = the font derived from ViewerLogFont has already been measured; FALSE = font measurement is needed
extern BOOL ViewerFontNeedsMapping;          // TRUE = ViewerFontMapping must be used; FALSE = the font is fine and no mapping is needed
extern char ViewerFontMapping[256];          // remapping to characters that have the expected fixed width

extern HANDLE ViewerContinue; // helper event used to wait for the message-loop thread to start

BOOL OpenViewer(const char* name, CViewType mode, int left, int top, int width, int height,
                UINT showCmd, BOOL returnLock, HANDLE* lock, BOOL* lockOwner,
                CSalamanderPluginViewerData* viewerData, int enumFileNamesSourceUID,
                int enumFileNamesLastFileIndex);
