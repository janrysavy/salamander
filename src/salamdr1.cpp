// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include <time.h>
//#ifdef MSVC_RUNTIME_CHECKS
#include <rtcapi.h>
//#endif // MSVC_RUNTIME_CHECKS

#include "allochan.h"
#include "menu.h"
#include "cfgdlg.h"
#include "plugins.h"
#include "fileswnd.h"
#include "mainwnd.h"
#include "shellib.h"
#include "worker.h"
#include "snooper.h"
#include "viewer.h"
#include "editwnd.h"
#include "find.h"
#include "zip.h"
#include "pack.h"
#include "cache.h"
#include "dialogs.h"
#include "gui.h"
#include "tasklist.h"
#include <uxtheme.h>
#include "olespy.h"
#include "geticon.h"
#include "logo.h"
#include "color.h"
#include "toolbar.h"

#include "svg.h"

extern "C"
{
#include "shexreg.h"
}
#include "salshlib.h"
#include "shiconov.h"
#include "salmoncl.h"
#include "jumplist.h"
#include "usermenu.h"
#include "execute.h"
#include "drivelst.h"

#pragma comment(linker, "/ENTRY:MyEntryPoint") // we want our own entry point for the application

#pragma comment(lib, "UxTheme.lib")

// make the original application entry point accessible
extern "C" int WinMainCRTStartup();

#ifdef X64_STRESS_TEST

#define X64_STRESS_TEST_ALLOC_COUNT 1000

LPVOID X64StressTestPointers[X64_STRESS_TEST_ALLOC_COUNT];

void X64StressTestAlloc()
{
    // at this moment the loader has already loaded the EXE and RTL and during RTL initialization
    // created and allocated the heap located below 4 GB; to push further allocations above
    // 4 GB we need to occupy the bottom of the virtual memory and then force the RTL to grow its heap
    //
    // occupy space in the virtual memory
    UINT64 vaAllocated = 0;
    _int64 allocSize[] = {10000000, 1000000, 100000, 10000, 1000, 100, 10, 1, 0};
    for (int i = 0; allocSize[i] != 0; i++)
        while (VirtualAlloc(0, allocSize[i], MEM_RESERVE, PAGE_NOACCESS) <= (LPVOID)(UINT_PTR)0x00000000ffffffff) // on access we want an exception and no MEM_COMMIT so we do not waste memory
            vaAllocated += allocSize[i];

    // now inflate the RTL heap
    UINT64 rtlAllocated = 0;
    _int64 rtlAllocSize[] = {10000000, 1000000, 100000, 10000, 1000, 100, 10, 1, 0};
    for (int i = 0; rtlAllocSize[i] != 0; i++)
        while (_malloc_dbg(rtlAllocSize[i], _CRT_BLOCK, __FILE__, __LINE__) <= (LPVOID)(UINT_PTR)0x00000000ffffffff)
            rtlAllocated += rtlAllocSize[i];

    // verify success
    void* testNew = new char; // new uses malloc so let's verify as well
    if (testNew <= (LPVOID)(UINT_PTR)0x00000000ffffffff)
        MessageBox(NULL, "new address <= 0x00000000ffffffff!\nPlease contact jan.rysavy@altap.cz with this information.", "X64_STRESS_TEST", MB_OK | MB_ICONEXCLAMATION);
    delete testNew;
}

#endif //X64_STRESS_TEST
// our custom entry point requested from the linker via pragma
int MyEntryPoint()
{
#ifdef X64_STRESS_TEST
    // allocate away the lower 4 GB so further allocations have pointers larger than DWORD
    X64StressTestAlloc();
#endif //X64_STRESS_TEST

    int ret = 1; // error

    // start Salmon so it can catch as many crashes as possible
    if (SalmonInit())
    {
        // call the original application entry point to run the program
        ret = WinMainCRTStartup();
    }
    else
        MessageBox(NULL, "Open Salamander Bug Reporter (salmon.exe) initialization has failed. Please reinstall Open Salamander.",
                   SALAMANDER_TEXT_VERSION, MB_OK | MB_ICONSTOP);

    // the debugger never reaches this point, RTL terminates us (tested with our RTL under VC 2008)

    // finished
    return ret;
}

BOOL SalamanderBusy = TRUE;       // is Salamander busy?
DWORD LastSalamanderIdleTime = 0; // GetTickCount() from the moment SalamanderBusy was last set to TRUE

int PasteLinkIsRunning = 0; // greater than zero while a Paste Shortcuts command is running in one of the panels

BOOL CannotCloseSalMainWnd = FALSE; // TRUE = closing the main window is not allowed

DWORD MainThreadID = -1;

int MenuNewExceptionHasOccured = 0;
int FGIExceptionHasOccured = 0;
int ICExceptionHasOccured = 0;
int QCMExceptionHasOccured = 0;
int OCUExceptionHasOccured = 0;
int GTDExceptionHasOccured = 0;
int SHLExceptionHasOccured = 0;
int RelExceptionHasOccured = 0;

char DecimalSeparator[5] = "."; // "characters" (max. 4 chars) retrieved from the system
int DecimalSeparatorLen = 1;    // length in characters without the terminating zero
char ThousandsSeparator[5] = " ";
int ThousandsSeparatorLen = 1;

BOOL WindowsXP64AndLater = FALSE;  // JRYFIXME - remove
BOOL WindowsVistaAndLater = FALSE; // JRYFIXME - remove
BOOL Windows7AndLater = FALSE;     // JRYFIXME - remove
BOOL Windows8AndLater = FALSE;
BOOL Windows8_1AndLater = FALSE;
BOOL Windows10AndLater = FALSE;

BOOL Windows64Bit = FALSE;

BOOL RunningAsAdmin = FALSE;

DWORD CCVerMajor = 0;
DWORD CCVerMinor = 0;

char ConfigurationName[MAX_PATH];
BOOL ConfigurationNameIgnoreIfNotExists = TRUE;

int StopRefresh = 0;

BOOL ExecCmdsOrUnloadMarkedPlugins = FALSE;
BOOL OpenPackOrUnpackDlgForMarkedPlugins = FALSE;

int StopIconRepaint = 0;
BOOL PostAllIconsRepaint = FALSE;

int StopStatusbarRepaint = 0;
BOOL PostStatusbarRepaint = FALSE;

int ChangeDirectoryAllowed = 0;
BOOL ChangeDirectoryRequest = FALSE;

BOOL SkipOneActivateRefresh = FALSE;

const char* DirColumnStr = NULL;
int DirColumnStrLen = 0;
const char* ColExtStr = NULL;
int ColExtStrLen = 0;
int TextEllipsisWidth = 0;
int TextEllipsisWidthEnv = 0;
const char* ProgDlgHoursStr = NULL;
const char* ProgDlgMinutesStr = NULL;
const char* ProgDlgSecsStr = NULL;

char FolderTypeName[80] = "";
int FolderTypeNameLen = 0;
const char* UpDirTypeName = NULL;
int UpDirTypeNameLen = 0;
const char* CommonFileTypeName = NULL;
int CommonFileTypeNameLen = 0;
const char* CommonFileTypeName2 = NULL;

char WindowsDirectory[MAX_PATH] = "";

// to ensure escape from removed drives to a fixed drive (after ejecting devices such as USB sticks)
BOOL ChangeLeftPanelToFixedWhenIdleInProgress = FALSE; // TRUE = path is currently changing, setting ChangeLeftPanelToFixedWhenIdle to TRUE is useless
BOOL ChangeLeftPanelToFixedWhenIdle = FALSE;
BOOL ChangeRightPanelToFixedWhenIdleInProgress = FALSE; // TRUE = path is currently changing, setting ChangeRightPanelToFixedWhenIdle to TRUE is useless
BOOL ChangeRightPanelToFixedWhenIdle = FALSE;
BOOL OpenCfgToChangeIfPathIsInaccessibleGoTo = FALSE; // TRUE = during idle open configuration on Drives and focus "If path in panel is inaccessible, go to:"

char IsSLGIncomplete[ISSLGINCOMPLETE_SIZE]; // if the string is empty, the SLG is fully translated; otherwise it contains a forum URL for the given language section

UINT TaskbarBtnCreatedMsg = 0;

// ****************************************************************************

C__MainWindowCS MainWindowCS;
BOOL CanDestroyMainWindow = FALSE;
CMainWindow* MainWindow = NULL;
CFilesWindow* DropSourcePanel = NULL;
BOOL OurClipDataObject = FALSE;
const char* SALCF_IDATAOBJECT = "SalIDataObject";
const char* SALCF_FAKE_REALPATH = "SalFakeRealPath";
const char* SALCF_FAKE_SRCTYPE = "SalFakeSrcType";
const char* SALCF_FAKE_SRCFSPATH = "SalFakeSrcFSPath";

const char* MAINWINDOW_NAME = "Open Salamander";
const char* CMAINWINDOW_CLASSNAME = "SalamanderMainWindowVer25";
const char* SAVEBITS_CLASSNAME = "SalamanderSaveBits";
const char* SHELLEXECUTE_CLASSNAME = "SalamanderShellExecute";

CAssociations Associations; // associations loaded from the registry
CShares Shares;

char DefaultDir['Z' - 'A' + 1][MAX_PATH];

HACCEL AccelTable1 = NULL;
HACCEL AccelTable2 = NULL;

HINSTANCE NtDLL = NULL;             // handle to ntdll.dll
HINSTANCE Shell32DLL = NULL;        // handle to shell32.dll (icons)
HINSTANCE ImageResDLL = NULL;       // handle to imageres.dll (icons - Vista)
HINSTANCE User32DLL = NULL;         // handle to user32.dll (DisableProcessWindowsGhosting)
HINSTANCE HLanguage = NULL;         // handle to language dependent resources (.SPL file)
char CurrentHelpDir[MAX_PATH] = ""; // after first use of help this holds path to the help directory (location of all .chm files)
WORD LanguageID = 0;                // language ID of the .SPL file

char OpenReadmeInNotepad[MAX_PATH]; // used only when started from installer: file to open in notepad during idle

BOOL UseCustomPanelFont = FALSE;
HFONT Font = NULL;
HFONT FontUL = NULL;
LOGFONT LogFont;
int FontCharHeight = 0;

HFONT EnvFont = NULL;
HFONT EnvFontUL = NULL;
//LOGFONT EnvLogFont;
int EnvFontCharHeight = 0;
HFONT TooltipFont = NULL;

HBRUSH HNormalBkBrush = NULL;
HBRUSH HFocusedBkBrush = NULL;
HBRUSH HSelectedBkBrush = NULL;
HBRUSH HFocSelBkBrush = NULL;
HBRUSH HDialogBrush = NULL;
HBRUSH HButtonTextBrush = NULL;
HBRUSH HDitherBrush = NULL;
HBRUSH HActiveCaptionBrush = NULL;
HBRUSH HInactiveCaptionBrush = NULL;

HBRUSH HMenuSelectedBkBrush = NULL;
HBRUSH HMenuSelectedTextBrush = NULL;
HBRUSH HMenuHilightBrush = NULL;
HBRUSH HMenuGrayTextBrush = NULL;

HPEN HActiveNormalPen = NULL; // pens for the frame around an item
HPEN HActiveSelectedPen = NULL;
HPEN HInactiveNormalPen = NULL;
HPEN HInactiveSelectedPen = NULL;

HPEN HThumbnailNormalPen = NULL; // pens for the frame around thumbnails
HPEN HThumbnailFucsedPen = NULL;
HPEN HThumbnailSelectedPen = NULL;
HPEN HThumbnailFocSelPen = NULL;

HPEN BtnShadowPen = NULL;
HPEN BtnHilightPen = NULL;
HPEN Btn3DLightPen = NULL;
HPEN BtnFacePen = NULL;
HPEN WndFramePen = NULL;
HPEN WndPen = NULL;
HBITMAP HFilter = NULL;
HBITMAP HHeaderSort = NULL;

HIMAGELIST HFindSymbolsImageList = NULL;
HIMAGELIST HMenuMarkImageList = NULL;
HIMAGELIST HGrayToolBarImageList = NULL;
HIMAGELIST HHotToolBarImageList = NULL;
HIMAGELIST HBottomTBImageList = NULL;
HIMAGELIST HHotBottomTBImageList = NULL;

CBitmap ItemBitmap;

HBITMAP HUpDownBitmap = NULL;
HBITMAP HZoomBitmap = NULL;

//HBITMAP HWorkerBitmap = NULL;

HCURSOR HHelpCursor = NULL;

int SystemDPI = 0; // global DPI across all monitors. Salamander does not support per-monitor DPI, see https://msdn.microsoft.com/library/windows/desktop/dn469266.aspx
int IconSizes[] = {16, 32, 48};
int IconLRFlags = 0;
HICON HSharedOverlays[] = {0};
HICON HShortcutOverlays[] = {0};
HICON HSlowFileOverlays[] = {0};
CIconList* SimpleIconLists[] = {0};
CIconList* ThrobberFrames = NULL;
CIconList* LockFrames = NULL; // for simplicity declared and loaded as the throbber

HICON HGroupIcon = NULL;
HICON HFavoritIcon = NULL;
HICON HSlowFileIcon = NULL;

RGBQUAD ColorTable[256] = {0};

DWORD MouseHoverTime = 0;

SYSTEMTIME SalamanderStartSystemTime = {0}; // Salamander start time (GetSystemTime)

BOOL WaitForESCReleaseBeforeTestingESC = FALSE; // wait for ESC release before beginning path traversal in panel?

int SPACE_WIDTH = 10;

const char* LOW_MEMORY = "Low memory.";

BOOL DragFullWindows = TRUE;

CWindowQueue ViewerWindowQueue("Internal Viewers");

CFindSetDialog GlobalFindDialog(NULL /* ignored */, 0 /* ignored */, 0 /* ignored */);

CNames GlobalSelection;
CDirectorySizesHolder DirectorySizesHolder;

HWND PluginProgressDialog = NULL;
HWND PluginMsgBoxParent = NULL;

BOOL CriticalShutdown = FALSE;

HANDLE SalOpenFileMapping = NULL;
void* SalOpenSharedMem = NULL;

// mutex for synchronizing load/save to the Registry (two processes at once cause trouble)
CLoadSaveToRegistryMutex LoadSaveToRegistryMutex;

BOOL IsNotAlphaNorNum[256]; // TRUE/FALSE array for characters (TRUE = not a letter nor a digit)
BOOL IsAlpha[256];          // TRUE/FALSE array for characters (TRUE = letter)

// default user charset for fonts; under W2K+ DEFAULT_CHARSET would be enough
//
// Under WinXP the regional settings can select Czech as default but the Advanced tab may not
// install Czech fonts. Then when constructing a font with UserCharset the system returns a font
// with a completely different face name just to match the requested charset. Therefore it is
// IMPORTANT to set the lfPitchAndFamily field correctly, choosing between FF_SWISS and
// FF_ROMAN fonts (sans serif / serif).
int UserCharset = DEFAULT_CHARSET;

DWORD AllocationGranularity = 1; // allocation granularity (needed for memory mapped files)

#ifdef USE_BETA_EXPIRATION_DATE

// determines the first day this beta/PB version will no longer run
// beta/PB version 4.0 beta 1 will run only until February 1, 2020
//                                 YEAR  MONTH DAY
SYSTEMTIME BETA_EXPIRATION_DATE = {2020, 2, 0, 1, 0, 0, 0, 0};
#endif // USE_BETA_EXPIRATION_DATE

//******************************************************************************
//
// Idle processing control (CMainWindow::OnEnterIdle)
//

BOOL IdleRefreshStates = TRUE;  // let the variables be set initially
BOOL IdleForceRefresh = FALSE;  // disables the Enabler* cache
BOOL IdleCheckClipboard = TRUE; // also check the clipboard

DWORD EnablerUpDir = FALSE;
DWORD EnablerRootDir = FALSE;
DWORD EnablerForward = FALSE;
DWORD EnablerBackward = FALSE;
DWORD EnablerFileOnDisk = FALSE;
DWORD EnablerLeftFileOnDisk = FALSE;
DWORD EnablerRightFileOnDisk = FALSE;
DWORD EnablerFileOnDiskOrArchive = FALSE;
DWORD EnablerFileOrDirLinkOnDisk = FALSE;
DWORD EnablerFiles = FALSE;
DWORD EnablerFilesOnDisk = FALSE;
DWORD EnablerFilesOnDiskCompress = FALSE;
DWORD EnablerFilesOnDiskEncrypt = FALSE;
DWORD EnablerFilesOnDiskOrArchive = FALSE;
DWORD EnablerOccupiedSpace = FALSE;
DWORD EnablerFilesCopy = FALSE;
DWORD EnablerFilesMove = FALSE;
DWORD EnablerFilesDelete = FALSE;
DWORD EnablerFileDir = FALSE;
DWORD EnablerFileDirANDSelected = FALSE;
DWORD EnablerQuickRename = FALSE;
DWORD EnablerOnDisk = FALSE;
DWORD EnablerCalcDirSizes = FALSE;
DWORD EnablerPasteFiles = FALSE;
DWORD EnablerPastePath = FALSE;
DWORD EnablerPasteLinks = FALSE;
DWORD EnablerPasteSimpleFiles = FALSE;
DWORD EnablerPasteDefEffect = FALSE;
DWORD EnablerPasteFilesToArcOrFS = FALSE;
DWORD EnablerPaste = FALSE;
DWORD EnablerPasteLinksOnDisk = FALSE;
DWORD EnablerSelected = FALSE;
DWORD EnablerUnselected = FALSE;
DWORD EnablerHiddenNames = FALSE;
DWORD EnablerSelectionStored = FALSE;
DWORD EnablerGlobalSelStored = FALSE;
DWORD EnablerSelGotoPrev = FALSE;
DWORD EnablerSelGotoNext = FALSE;
DWORD EnablerLeftUpDir = FALSE;
DWORD EnablerRightUpDir = FALSE;
DWORD EnablerLeftRootDir = FALSE;
DWORD EnablerRightRootDir = FALSE;
DWORD EnablerLeftForward = FALSE;
DWORD EnablerRightForward = FALSE;
DWORD EnablerLeftBackward = FALSE;
DWORD EnablerRightBackward = FALSE;
DWORD EnablerFileHistory = FALSE;
DWORD EnablerDirHistory = FALSE;
DWORD EnablerCustomizeLeftView = FALSE;
DWORD EnablerCustomizeRightView = FALSE;
DWORD EnablerDriveInfo = FALSE;
DWORD EnablerCreateDir = FALSE;
DWORD EnablerViewFile = FALSE;
DWORD EnablerChangeAttrs = FALSE;
DWORD EnablerShowProperties = FALSE;
DWORD EnablerItemsContextMenu = FALSE;
DWORD EnablerOpenActiveFolder = FALSE;
DWORD EnablerPermissions = FALSE;

COLORREF* CurrentColors = SalamanderColors;

COLORREF UserColors[NUMBER_OF_COLORS];

SALCOLOR ViewerColors[NUMBER_OF_VIEWERCOLORS] =
    {
        RGBF(0, 0, 0, SCF_DEFAULT),       // VIEWER_FG_NORMAL
        RGBF(255, 255, 255, SCF_DEFAULT), // VIEWER_BK_NORMAL
        RGBF(255, 255, 255, SCF_DEFAULT), // VIEWER_FG_SELECTED
        RGBF(0, 0, 0, SCF_DEFAULT),       // VIEWER_BK_SELECTED
};

COLORREF SalamanderColors[NUMBER_OF_COLORS] =
    {
        // pen colors for the frame around items
        RGBF(0, 0, 0, SCF_DEFAULT),       // FOCUS_ACTIVE_NORMAL
        RGBF(0, 0, 0, SCF_DEFAULT),       // FOCUS_ACTIVE_SELECTED
        RGBF(128, 128, 128, 0),           // FOCUS_FG_INACTIVE_NORMAL
        RGBF(128, 128, 128, 0),           // FOCUS_FG_INACTIVE_SELECTED
        RGBF(255, 255, 255, SCF_DEFAULT), // FOCUS_BK_INACTIVE_NORMAL
        RGBF(255, 255, 255, SCF_DEFAULT), // FOCUS_BK_INACTIVE_SELECTED

        // item text colors in panel
        RGBF(0, 0, 0, SCF_DEFAULT), // ITEM_FG_NORMAL
        RGBF(255, 0, 0, 0),         // ITEM_FG_SELECTED
        RGBF(0, 0, 0, SCF_DEFAULT), // ITEM_FG_FOCUSED
        RGBF(255, 0, 0, 0),         // ITEM_FG_FOCSEL
        RGBF(0, 0, 0, SCF_DEFAULT), // ITEM_FG_HIGHLIGHT

        // item background colors in panel
        RGBF(255, 255, 255, SCF_DEFAULT), // ITEM_BK_NORMAL
        RGBF(255, 255, 255, SCF_DEFAULT), // ITEM_BK_SELECTED
        RGBF(232, 232, 232, 0),           // ITEM_BK_FOCUSED
        RGBF(232, 232, 232, 0),           // ITEM_BK_FOCSEL
        RGBF(0, 0, 0, SCF_DEFAULT),       // ITEM_BK_HIGHLIGHT

        // colors used for icon blending
        RGBF(255, 128, 128, SCF_DEFAULT), // ICON_BLEND_SELECTED
        RGBF(128, 128, 128, 0),           // ICON_BLEND_FOCUSED
        RGBF(255, 0, 0, 0),               // ICON_BLEND_FOCSEL

        // progress bar colors
        RGBF(0, 0, 192, SCF_DEFAULT),     // PROGRESS_FG_NORMAL
        RGBF(255, 255, 255, SCF_DEFAULT), // PROGRESS_FG_SELECTED
        RGBF(255, 255, 255, SCF_DEFAULT), // PROGRESS_BK_NORMAL
        RGBF(0, 0, 192, SCF_DEFAULT),     // PROGRESS_BK_SELECTED

        // hot item colors
        RGBF(0, 0, 255, SCF_DEFAULT),     // HOT_PANEL
        RGBF(128, 128, 128, SCF_DEFAULT), // HOT_ACTIVE
        RGBF(128, 128, 128, SCF_DEFAULT), // HOT_INACTIVE

        // panel caption colors
        RGBF(255, 255, 255, SCF_DEFAULT), // ACTIVE_CAPTION_FG
        RGBF(0, 0, 128, SCF_DEFAULT),     // ACTIVE_CAPTION_BK
        RGBF(255, 255, 255, SCF_DEFAULT), // INACTIVE_CAPTION_FG
        RGBF(128, 128, 128, SCF_DEFAULT), // INACTIVE_CAPTION_BK

        // pen colors for the frame around thumbnails
        RGBF(192, 192, 192, 0), // THUMBNAIL_FRAME_NORMAL
        RGBF(0, 0, 0, 0),       // THUMBNAIL_FRAME_FOCUSED
        RGBF(255, 0, 0, 0),     // THUMBNAIL_FRAME_SELECTED
        RGBF(128, 0, 0, 0),     // THUMBNAIL_FRAME_FOCSEL
};

COLORREF ExplorerColors[NUMBER_OF_COLORS] =
    {
        // pen colors for the frame around an item
        RGBF(0, 0, 0, SCF_DEFAULT),       // FOCUS_ACTIVE_NORMAL
        RGBF(255, 255, 0, 0),             // FOCUS_ACTIVE_SELECTED
        RGBF(128, 128, 128, 0),           // FOCUS_FG_INACTIVE_NORMAL
        RGBF(0, 0, 128, 0),               // FOCUS_FG_INACTIVE_SELECTED
        RGBF(255, 255, 255, SCF_DEFAULT), // FOCUS_BK_INACTIVE_NORMAL
        RGBF(255, 255, 0, 0),             // FOCUS_BK_INACTIVE_SELECTED

        // item text colors in panel
        RGBF(0, 0, 0, SCF_DEFAULT), // ITEM_FG_NORMAL
        RGBF(255, 255, 255, 0),     // ITEM_FG_SELECTED
        RGBF(0, 0, 0, SCF_DEFAULT), // ITEM_FG_FOCUSED
        RGBF(255, 255, 255, 0),     // ITEM_FG_FOCSEL
        RGBF(0, 0, 0, SCF_DEFAULT), // ITEM_FG_HIGHLIGHT

        // item background colors in panel
        RGBF(255, 255, 255, SCF_DEFAULT), // ITEM_BK_NORMAL
        RGBF(0, 0, 128, 0),               // ITEM_BK_SELECTED
        RGBF(232, 232, 232, 0),           // ITEM_BK_FOCUSED
        RGBF(0, 0, 128, 0),               // ITEM_BK_FOCSEL
        RGBF(0, 0, 0, SCF_DEFAULT),       // ITEM_BK_HIGHLIGHT

        // colors used for icon blending
        RGBF(0, 0, 128, SCF_DEFAULT), // ICON_BLEND_SELECTED
        RGBF(128, 128, 128, 0),       // ICON_BLEND_FOCUSED
        RGBF(0, 0, 128, 0),           // ICON_BLEND_FOCSEL

        // progress bar colors
        RGBF(0, 0, 192, SCF_DEFAULT),     // PROGRESS_FG_NORMAL
        RGBF(255, 255, 255, SCF_DEFAULT), // PROGRESS_FG_SELECTED
        RGBF(255, 255, 255, SCF_DEFAULT), // PROGRESS_BK_NORMAL
        RGBF(0, 0, 192, SCF_DEFAULT),     // PROGRESS_BK_SELECTED

        // hot item colors
        RGBF(0, 0, 255, SCF_DEFAULT),     // HOT_PANEL
        RGBF(128, 128, 128, SCF_DEFAULT), // HOT_ACTIVE
        RGBF(128, 128, 128, SCF_DEFAULT), // HOT_INACTIVE

        // panel caption colors
        RGBF(255, 255, 255, SCF_DEFAULT), // ACTIVE_CAPTION_FG
        RGBF(0, 0, 128, SCF_DEFAULT),     // ACTIVE_CAPTION_BK
        RGBF(255, 255, 255, SCF_DEFAULT), // INACTIVE_CAPTION_FG
        RGBF(128, 128, 128, SCF_DEFAULT), // INACTIVE_CAPTION_BK

        // pen colors for the frame around thumbnails
        RGBF(192, 192, 192, 0), // THUMBNAIL_FRAME_NORMAL
        RGBF(0, 0, 128, 0),     // THUMBNAIL_FRAME_FOCUSED
        RGBF(0, 0, 128, 0),     // THUMBNAIL_FRAME_SELECTED
        RGBF(0, 0, 128, 0),     // THUMBNAIL_FRAME_FOCSEL
};

COLORREF NortonColors[NUMBER_OF_COLORS] =
    {
        // pen colors for the frame around items
        RGBF(0, 128, 128, 0), // FOCUS_ACTIVE_NORMAL
        RGBF(0, 128, 128, 0), // FOCUS_ACTIVE_SELECTED
        RGBF(0, 128, 128, 0), // FOCUS_FG_INACTIVE_NORMAL
        RGBF(0, 128, 128, 0), // FOCUS_FG_INACTIVE_SELECTED
        RGBF(0, 0, 128, 0),   // FOCUS_BK_INACTIVE_NORMAL
        RGBF(0, 0, 128, 0),   // FOCUS_BK_INACTIVE_SELECTED

        // item text colors in panel
        RGBF(0, 255, 255, 0),       // ITEM_FG_NORMAL
        RGBF(255, 255, 0, 0),       // ITEM_FG_SELECTED
        RGBF(0, 0, 0, SCF_DEFAULT), // ITEM_FG_FOCUSED
        RGBF(255, 255, 0, 0),       // ITEM_FG_FOCSEL
        RGBF(0, 0, 0, SCF_DEFAULT), // ITEM_FG_HIGHLIGHT

        // item background colors in panel
        RGBF(0, 0, 128, 0),         // ITEM_BK_NORMAL
        RGBF(0, 0, 128, 0),         // ITEM_BK_SELECTED
        RGBF(0, 128, 128, 0),       // ITEM_BK_FOCUSED
        RGBF(0, 128, 128, 0),       // ITEM_BK_FOCSEL
        RGBF(0, 0, 0, SCF_DEFAULT), // ITEM_BK_HIGHLIGHT

        // colors used for icon blending
        RGBF(255, 255, 0, SCF_DEFAULT), // ICON_BLEND_SELECTED
        RGBF(128, 128, 128, 0),         // ICON_BLEND_FOCUSED
        RGBF(255, 255, 0, 0),           // ICON_BLEND_FOCSEL

        // progress bar colors
        RGBF(0, 0, 192, SCF_DEFAULT),     // PROGRESS_FG_NORMAL
        RGBF(255, 255, 255, SCF_DEFAULT), // PROGRESS_FG_SELECTED
        RGBF(255, 255, 255, SCF_DEFAULT), // PROGRESS_BK_NORMAL
        RGBF(0, 0, 192, SCF_DEFAULT),     // PROGRESS_BK_SELECTED

        // hot item colors
        RGBF(0, 0, 255, SCF_DEFAULT),     // HOT_PANEL
        RGBF(128, 128, 128, SCF_DEFAULT), // HOT_ACTIVE
        RGBF(128, 128, 128, SCF_DEFAULT), // HOT_INACTIVE

        // panel caption colors
        RGBF(255, 255, 255, SCF_DEFAULT), // ACTIVE_CAPTION_FG
        RGBF(0, 0, 128, SCF_DEFAULT),     // ACTIVE_CAPTION_BK
        RGBF(255, 255, 255, SCF_DEFAULT), // INACTIVE_CAPTION_FG
        RGBF(128, 128, 128, SCF_DEFAULT), // INACTIVE_CAPTION_BK

        // pen colors for the frame around thumbnails
        RGBF(192, 192, 192, 0), // THUMBNAIL_FRAME_NORMAL
        RGBF(0, 128, 128, 0),   // THUMBNAIL_FRAME_FOCUSED
        RGBF(255, 255, 0, 0),   // THUMBNAIL_FRAME_SELECTED
        RGBF(255, 255, 0, 0),   // THUMBNAIL_FRAME_FOCSEL
};

COLORREF NavigatorColors[NUMBER_OF_COLORS] =
    {
        // pen colors for the frame around items
        RGBF(0, 128, 128, 0), // FOCUS_ACTIVE_NORMAL
        RGBF(0, 128, 128, 0), // FOCUS_ACTIVE_SELECTED
        RGBF(0, 128, 128, 0), // FOCUS_FG_INACTIVE_NORMAL
        RGBF(0, 128, 128, 0), // FOCUS_FG_INACTIVE_SELECTED
        RGBF(0, 0, 128, 0),   // FOCUS_BK_INACTIVE_NORMAL
        RGBF(0, 0, 128, 0),   // FOCUS_BK_INACTIVE_SELECTED

        // item text colors in panel
        RGBF(255, 255, 255, 0),     // ITEM_FG_NORMAL
        RGBF(255, 255, 0, 0),       // ITEM_FG_SELECTED
        RGBF(0, 0, 0, SCF_DEFAULT), // ITEM_FG_FOCUSED
        RGBF(255, 255, 0, 0),       // ITEM_FG_FOCSEL
        RGBF(0, 0, 0, SCF_DEFAULT), // ITEM_FG_HIGHLIGHT

        // item background colors in panel
        RGBF(80, 80, 80, 0),        // ITEM_BK_NORMAL
        RGBF(80, 80, 80, 0),        // ITEM_BK_SELECTED
        RGBF(0, 128, 128, 0),       // ITEM_BK_FOCUSED
        RGBF(0, 128, 128, 0),       // ITEM_BK_FOCSEL
        RGBF(0, 0, 0, SCF_DEFAULT), // ITEM_BK_HIGHLIGHT

        // colors used for icon blending
        RGBF(255, 255, 0, SCF_DEFAULT), // ICON_BLEND_SELECTED
        RGBF(128, 128, 128, 0),         // ICON_BLEND_FOCUSED
        RGBF(255, 255, 0, 0),           // ICON_BLEND_FOCSEL

        // progress bar colors
        RGBF(0, 0, 192, SCF_DEFAULT),     // PROGRESS_FG_NORMAL
        RGBF(255, 255, 255, SCF_DEFAULT), // PROGRESS_FG_SELECTED
        RGBF(255, 255, 255, SCF_DEFAULT), // PROGRESS_BK_NORMAL
        RGBF(0, 0, 192, SCF_DEFAULT),     // PROGRESS_BK_SELECTED

        // hot item colors
        RGBF(0, 0, 255, SCF_DEFAULT),     // HOT_PANEL
        RGBF(173, 182, 205, SCF_DEFAULT), // HOT_ACTIVE
        RGBF(212, 212, 212, SCF_DEFAULT), // HOT_INACTIVE

        // panel caption colors
        RGBF(255, 255, 255, SCF_DEFAULT), // ACTIVE_CAPTION_FG
        RGBF(0, 0, 128, SCF_DEFAULT),     // ACTIVE_CAPTION_BK
        RGBF(255, 255, 255, SCF_DEFAULT), // INACTIVE_CAPTION_FG
        RGBF(128, 128, 128, SCF_DEFAULT), // INACTIVE_CAPTION_BK

        // pen colors for the frame around thumbnails
        RGBF(192, 192, 192, 0), // THUMBNAIL_FRAME_NORMAL
        RGBF(0, 128, 128, 0),   // THUMBNAIL_FRAME_FOCUSED
        RGBF(255, 255, 0, 0),   // THUMBNAIL_FRAME_SELECTED
        RGBF(255, 255, 0, 0),   // THUMBNAIL_FRAME_FOCSEL
};

COLORREF CustomColors[NUMBER_OF_CUSTOMCOLORS] =
    {
        RGB(255, 255, 255),
        RGB(255, 255, 255),
        RGB(255, 255, 255),
        RGB(255, 255, 255),
        RGB(255, 255, 255),
        RGB(255, 255, 255),
        RGB(255, 255, 255),
        RGB(255, 255, 255),
        RGB(255, 255, 255),
        RGB(255, 255, 255),
        RGB(255, 255, 255),
        RGB(255, 255, 255),
        RGB(255, 255, 255),
        RGB(255, 255, 255),
        RGB(255, 255, 255),
        RGB(255, 255, 255),
};

//*****************************************************************************
//
// CRC32
//

static DWORD Crc32Tab[256];
static BOOL Crc32TabInitialized = FALSE;

void MakeCrc32Table(DWORD* crcTab)
{
    DWORD c;
    DWORD poly = 0xedb88320L; //polynomial exclusive-or pattern

    /*
  // generate crc polonomial, using precomputed poly should be faster
  // terms of polynomial defining this crc (except x^32):
  static const Byte p[] = {0,1,2,4,5,7,8,10,11,12,16,22,23,26};

  // make exclusive-or pattern from polynomial (0xedb88320L)
  poly = 0L;
  for (n = 0; n < sizeof(p)/sizeof(Byte); n++)
    poly |= 1L << (31 - p[n]);
*/
    int n;
    for (n = 0; n < 256; n++)
    {
        c = (UINT32)n;

        int k;
        for (k = 0; k < 8; k++)
            c = c & 1 ? poly ^ (c >> 1) : c >> 1;

        crcTab[n] = c;
    }
}

DWORD UpdateCrc32(const void* buffer, DWORD count, DWORD crcVal)
{
    CALL_STACK_MESSAGE_NONE

    if (buffer == NULL)
        return 0;

    if (!Crc32TabInitialized)
    {
        MakeCrc32Table(Crc32Tab);
        Crc32TabInitialized = TRUE;
    }

    BYTE* p = (BYTE*)buffer;
    DWORD c = crcVal ^ 0xFFFFFFFF;

    if (count)
        do
        {
            c = Crc32Tab[((int)c ^ (*p++)) & 0xff] ^ (c >> 8);
        } while (--count);

    // Honza: I measured the following optimizations and they are pointless;
    // the only chance would be rewriting into ASM and reading memory in DWORDs,
    // from which individual bytes could be extracted;
    // with the current release settings the compiler cannot perform this
    // optimization for us.
    /*
  int remain = count % 8;
  count -= remain;
  while (remain)
  {
    c = Crc32Tab[((int)c ^ (*p++)) & 0xff] ^ (c >> 8);
    remain--;
  }
  while (count)
  {
    c = Crc32Tab[((int)c ^ (*p++)) & 0xff] ^ (c >> 8);
    c = Crc32Tab[((int)c ^ (*p++)) & 0xff] ^ (c >> 8);
    c = Crc32Tab[((int)c ^ (*p++)) & 0xff] ^ (c >> 8);
    c = Crc32Tab[((int)c ^ (*p++)) & 0xff] ^ (c >> 8);
    c = Crc32Tab[((int)c ^ (*p++)) & 0xff] ^ (c >> 8);
    c = Crc32Tab[((int)c ^ (*p++)) & 0xff] ^ (c >> 8);
    c = Crc32Tab[((int)c ^ (*p++)) & 0xff] ^ (c >> 8);
    c = Crc32Tab[((int)c ^ (*p++)) & 0xff] ^ (c >> 8);
    count -= 8;
  }
*/
    /*
  int remain = count % 4;
  count -= remain;
  while (remain > 0)
  {
    c = Crc32Tab[((int)c ^ (*p++)) & 0xff] ^ (c >> 8);
    remain--;
  }


  DWORD *pdw = (DWORD*)p;
  DWORD dw;
  while (count > 0)
  {
    dw = *pdw++;
    c = Crc32Tab[((int)c ^ ((BYTE)(dw))) & 0xFF] ^ (c >> 8);
    c = Crc32Tab[((int)c ^ ((BYTE)(dw >> 8))) & 0xFF] ^ (c >> 8);
    c = Crc32Tab[((int)c ^ ((BYTE)(dw >> 16))) & 0xFF] ^ (c >> 8);
    c = Crc32Tab[((int)c ^ ((BYTE)(dw >> 24))) & 0xFF] ^ (c >> 8);
    count -= 4;
  }
*/
    return c ^ 0xFFFFFFFF; /* (instead of ~c for 64-bit machines) */
}

BOOL IsRemoteSession(void)
{
    return GetSystemMetrics(SM_REMOTESESSION);
}

// ****************************************************************************

BOOL SalamanderIsNotBusy(DWORD* lastIdleTime)
{
    // SalamanderBusy and LastSalamanderIdleTime are accessed without critical
    // sections; this is fine because they are DWORDs and cannot be "half-written"
    // during a context switch. There is always either the old value or the new one
    if (lastIdleTime != NULL)
        *lastIdleTime = LastSalamanderIdleTime;
    if (!SalamanderBusy)
        return TRUE;
    DWORD oldLastIdleTime = LastSalamanderIdleTime;
    if (GetTickCount() - oldLastIdleTime <= 100)
        // if SalamanderBusy hasn't been FALSE for long (e.g. a modal dialog is open)
        Sleep(100);                                                                // wait to see whether SalamanderBusy changes
    return !SalamanderBusy || (int)(LastSalamanderIdleTime - oldLastIdleTime) > 0; // not "busy" or at least oscillating
}

BOOL InitPreloadedStrings()
{
    DirColumnStr = DupStr(LoadStr(IDS_DIRCOLUMN));
    DirColumnStrLen = lstrlen(DirColumnStr);

    ColExtStr = DupStr(LoadStr(IDS_COLUMN_NAME_EXT));
    ColExtStrLen = lstrlen(ColExtStr);

    UpDirTypeName = DupStr(LoadStr(IDS_UPDIRTYPENAME));
    UpDirTypeNameLen = lstrlen(UpDirTypeName);

    CommonFileTypeName = DupStr(LoadStr(IDS_COMMONFILETYPE));
    CommonFileTypeNameLen = lstrlen(CommonFileTypeName);
    CommonFileTypeName2 = DupStr(LoadStr(IDS_COMMONFILETYPE2));

    ProgDlgHoursStr = DupStr(LoadStr(IDS_PROGDLGHOURS));
    ProgDlgMinutesStr = DupStr(LoadStr(IDS_PROGDLGMINUTES));
    ProgDlgSecsStr = DupStr(LoadStr(IDS_PROGDLGSECS));

    return TRUE;
}

void ReleasePreloadedStrings()
{
    if (DirColumnStr != NULL)
        free((void*)DirColumnStr);
    if (ColExtStr != NULL)
        free((void*)ColExtStr);

    if (UpDirTypeName != NULL)
        free((void*)UpDirTypeName);

    if (CommonFileTypeName != NULL)
        free((void*)CommonFileTypeName);
    if (CommonFileTypeName2 != NULL)
        free((void*)CommonFileTypeName2);

    if (ProgDlgHoursStr != NULL)
        free((void*)ProgDlgHoursStr);
    if (ProgDlgMinutesStr != NULL)
        free((void*)ProgDlgMinutesStr);
    if (ProgDlgSecsStr != NULL)
        free((void*)ProgDlgSecsStr);

    DirColumnStr = NULL;
    ColExtStr = NULL;

    UpDirTypeName = NULL;

    CommonFileTypeName = NULL;
    CommonFileTypeName2 = NULL;

    ProgDlgHoursStr = NULL;
    ProgDlgMinutesStr = NULL;
    ProgDlgSecsStr = NULL;
}

// ****************************************************************************

void InitLocales()
{
    int i;
    for (i = 0; i < 256; i++)
    {
        IsNotAlphaNorNum[i] = !IsCharAlphaNumeric((char)i);
        IsAlpha[i] = IsCharAlpha((char)i);
    }

    if ((DecimalSeparatorLen = GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, DecimalSeparator, 5)) == 0 ||
        DecimalSeparatorLen > 5)
    {
        strcpy(DecimalSeparator, ".");
        DecimalSeparatorLen = 1;
    }
    else
    {
        DecimalSeparatorLen--;
        DecimalSeparator[DecimalSeparatorLen] = 0; // ensure a null terminator
    }

    if ((ThousandsSeparatorLen = GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, ThousandsSeparator, 5)) == 0 ||
        ThousandsSeparatorLen > 5)
    {
        strcpy(ThousandsSeparator, " ");
        ThousandsSeparatorLen = 1;
    }
    else
    {
        ThousandsSeparatorLen--;
        ThousandsSeparator[ThousandsSeparatorLen] = 0; // ensure a null terminator
    }
}

// ****************************************************************************

HICON GetFileOrPathIconAux(const char* path, BOOL large, BOOL isDir)
{
    __try
    {
        SHFILEINFO shi;
        if (!GetFileIcon(path, FALSE, &shi.hIcon, large ? ICONSIZE_32 : ICONSIZE_16, TRUE, isDir))
            shi.hIcon = NULL;
        // we switched to our own implementation (less memory hungry, working XOR icons)
        //shi.hIcon = NULL;
        //SHGetFileInfo(path, 0, &shi, sizeof(shi),
        //              SHGFI_ICON | SHGFI_SHELLICONSIZE | (large ? 0 : SHGFI_SMALLICON));
        // add a handle for 'shi.hIcon' to HANDLES
        if (shi.hIcon != NULL)
            HANDLES_ADD(__htIcon, __hoLoadImage, shi.hIcon);
        return shi.hIcon;
    }
    __except (CCallStack::HandleException(GetExceptionInformation(), 13))
    {
        FGIExceptionHasOccured++;
    }
    return NULL;
}

HICON GetDriveIcon(const char* root, UINT type, BOOL accessible, BOOL large)
{
    CALL_STACK_MESSAGE5("GetDriveIcon(%s, %u, %d, %d)", root, type, accessible, large);
    int id;
    switch (type)
    {
    case DRIVE_REMOVABLE: // 3.5" and 5.25" floppy drive icons
    {
        HICON i = GetFileOrPathIconAux(root, large, TRUE);
        if (i != NULL)
            return i;
        id = 28; // 3 1/2" drive
        break;
    }

    case DRIVE_REMOTE:
        id = (accessible ? 33 : 31);
        break;
    case DRIVE_CDROM:
        id = 30;
        break;
    case DRIVE_RAMDISK:
        id = 34;
        break;

    default:
    {
        id = 32;
        if (type == DRIVE_FIXED && root[1] == ':')
        {
            char win[MAX_PATH];
            if (GetWindowsDirectory(win, MAX_PATH) && win[1] == ':' && win[0] == root[0])
                id = 36;
        }
        break;
    }
    }
    int iconSize = IconSizes[large ? ICONSIZE_32 : ICONSIZE_16];
    return SalLoadIcon(ImageResDLL, id, iconSize);

    // JRYFIXME - investigate whether IconLRFlags can be removed (W7+)

    /* JRYFIXME - grep the sources for LoadImage / IMAGE_ICON
  return (HICON)HANDLES(LoadImage(ImageResDLL, MAKEINTRESOURCE(id), IMAGE_ICON,
                                  large ? ICON32_CX : ICON16_CX,
                                  large ? ICON32_CX : ICON16_CX,
                                  IconLRFlags));
  */
}

HICON SalLoadIcon(HINSTANCE hDLL, int id, int iconSize)
{
    //return (HICON)HANDLES(LoadImage(hDLL, MAKEINTRESOURCE(id), IMAGE_ICON, iconSize, iconSize, IconLRFlags));
    HICON hIcon;
    LoadIconWithScaleDown(hDLL, MAKEINTRESOURCEW(id), iconSize, iconSize, &hIcon);
    HANDLES_ADD(__htIcon, __hoLoadImage, hIcon);
    return hIcon;
}

// ****************************************************************************

char* BuildName(char* path, char* name, char* dosName, BOOL* skip, BOOL* skipAll, const char* sourcePath)
{
    if (skip != NULL)
        *skip = FALSE;
    int l1 = (int)strlen(path); // always located on the stack
    int l2, len = l1;
    if (name != NULL)
    {
        l2 = (int)strlen(name);
        len += l2;
        if (path[l1 - 1] != '\\')
            len++;
        if (len >= MAX_PATH && dosName != NULL)
        {
            int l3 = (int)strlen(dosName);
            if (len - l2 + l3 < MAX_PATH)
            {
                len = len - l2 + l3;
                name = dosName;
                l2 = l3;
            }
        }
    }
    if (len >= MAX_PATH)
    {
        char text[2 * MAX_PATH + 100];
        _snprintf_s(text, _TRUNCATE, LoadStr(IDS_NAMEISTOOLONG), name, path);

        if (skip != NULL)
        {
            if (skipAll == NULL || !*skipAll)
            {
                MSGBOXEX_PARAMS params;
                memset(&params, 0, sizeof(params));
                params.HParent = MainWindow->HWindow;
                params.Flags = MSGBOXEX_YESNOOKCANCEL | MB_ICONEXCLAMATION | MSGBOXEX_DEFBUTTON3 | MSGBOXEX_SILENT;
                params.Caption = LoadStr(IDS_ERRORTITLE);
                params.Text = text;
                char aliasBtnNames[200];
                /* used by the export_mnu.py script which generates salmenu.mnu for Translator
    we let the message box buttons resolve hotkey collisions by pretending it is a menu
MENU_TEMPLATE_ITEM MsgBoxButtons[] = 
{
  {MNTT_PB, 0
  {MNTT_IT, IDS_MSGBOXBTN_SKIP
  {MNTT_IT, IDS_MSGBOXBTN_SKIPALL
  {MNTT_IT, IDS_MSGBOXBTN_FOCUS
  {MNTT_PE, 0
};
*/
                sprintf(aliasBtnNames, "%d\t%s\t%d\t%s\t%d\t%s",
                        DIALOG_YES, LoadStr(IDS_MSGBOXBTN_SKIP),
                        DIALOG_NO, LoadStr(IDS_MSGBOXBTN_SKIPALL),
                        DIALOG_OK, LoadStr(IDS_MSGBOXBTN_FOCUS));
                params.AliasBtnNames = aliasBtnNames;
                int msgRes = SalMessageBoxEx(&params);
                if (msgRes == DIALOG_YES /* Skip */ || msgRes == DIALOG_NO /* Skip All */)
                    *skip = TRUE;
                if (msgRes == DIALOG_NO /* Skip All */ && skipAll != NULL)
                    *skipAll = TRUE;
                if (msgRes == DIALOG_OK /* Focus */)
                    MainWindow->PostFocusNameInPanel(PANEL_SOURCE, sourcePath, name);
            }
            else
                *skip = TRUE;
        }
        else
        {
            SalMessageBox(MainWindow->HWindow, text, LoadStr(IDS_ERRORTITLE),
                          MB_OK | MB_ICONEXCLAMATION);
        }
        return NULL;
    }
    char* txt = (char*)malloc(len + 1);
    if (txt == NULL)
    {
        TRACE_E(LOW_MEMORY);
        return txt;
    }
    if (name != NULL)
    {
        memmove(txt, path, l1);
        if (path[l1 - 1] != '\\')
            txt[l1++] = '\\';
        memmove(txt + l1, name, l2 + 1);
    }
    else
        memmove(txt, path, l1 + 1);
    return txt;
}

// ****************************************************************************

BOOL HasTheSameRootPath(const char* path1, const char* path2)
{
    if (LowerCase[path1[0]] == LowerCase[path2[0]] && path1[1] == path2[1])
    {
        if (path1[1] == ':')
            return TRUE; // same root for standard ("c:\path") paths
        else
        {
            if (path1[0] == '\\' && path1[1] == '\\') // both are UNC
            {
                const char* s1 = path1 + 2;
                const char* s2 = path2 + 2;
                while (*s1 != 0 && *s1 != '\\')
                {
                    if (LowerCase[*s1] == LowerCase[*s2])
                    {
                        s1++;
                        s2++;
                    }
                    else
                        break; // different machines
                }
                if (*s1 != 0 && *s1++ == *s2++) // skip '\\'
                {
                    while (*s1 != 0 && *s1 != '\\')
                    {
                        if (LowerCase[*s1] == LowerCase[*s2])
                        {
                            s1++;
                            s2++;
                        }
                        else
                            break; // different drives
                    }
                    return (*s1 == 0 && (*s2 == 0 || *s2 == '\\')) || *s1 == *s2 ||
                           (*s2 == 0 && (*s1 == 0 || *s1 == '\\'));
                }
            }
        }
    }
    return FALSE;
}

// ****************************************************************************

BOOL HasTheSameRootPathAndVolume(const char* p1, const char* p2)
{
    CALL_STACK_MESSAGE3("HasTheSameRootPathAndVolume(%s, %s)", p1, p2);

    BOOL ret = FALSE;
    if (HasTheSameRootPath(p1, p2))
    {
        ret = TRUE;
        char root[MAX_PATH];
        char ourPath[MAX_PATH];
        char p1Volume[100] = "1";
        char p2Volume[100] = "2";
        char resPath[MAX_PATH];
        lstrcpyn(resPath, p1, MAX_PATH);
        ResolveSubsts(resPath);
        GetRootPath(root, resPath);
        if (!IsUNCPath(root) && GetDriveType(root) == DRIVE_FIXED) // searching for reparse points makes sense only on fixed disks
        {
            // if it is not a root path we also try walking through reparse points
            BOOL cutPathIsPossible = TRUE;
            char p1NetPath[MAX_PATH];
            p1NetPath[0] = 0;
            ResolveLocalPathWithReparsePoints(ourPath, p1, &cutPathIsPossible, NULL, NULL, NULL, NULL, p1NetPath);

            if (p1NetPath[0] == 0) // volume name cannot be obtained from a network path, so we do not even try
            {
                while (!GetVolumeNameForVolumeMountPoint(ourPath, p1Volume, 100))
                {
                    if (!cutPathIsPossible || !CutDirectory(ourPath))
                    {
                        strcpy(p1Volume, "fail"); // even the root call failed, unexpected (unfortunately this happens on subst drives under W2K - tuned at Bachaalany - when failing on both paths we return SAME because it is more likely)
                        break;
                    }
                    SalPathAddBackslash(ourPath, MAX_PATH);
                }
            }

            // under W2K and when not a root path, also walk through reparse points
            cutPathIsPossible = TRUE;
            char p2NetPath[MAX_PATH];
            p2NetPath[0] = 0;
            ResolveLocalPathWithReparsePoints(ourPath, p2, &cutPathIsPossible, NULL, NULL, NULL, NULL, p2NetPath);

            if ((p1NetPath[0] == 0) != (p2NetPath[0] == 0) || // if only one of the paths is network
                p1NetPath[0] != 0 && !HasTheSameRootPath(p1NetPath, p2NetPath))
                ret = FALSE; // roots differ, report different volumes (cannot verify volumes on network paths)

            if (p2NetPath[0] == 0 && ret) // volume name cannot be obtained from a network path, so we do not even try; if already decided we will not try again
            {
                while (!GetVolumeNameForVolumeMountPoint(ourPath, p2Volume, 100))
                {
                    if (!cutPathIsPossible || !CutDirectory(ourPath))
                    {
                        strcpy(p2Volume, "fail"); // even the root call failed, unexpected (unfortunately this happens on subst drives under W2K - tuned at Bachaalany - when failing on both paths we return SAME because it is more likely)
                        break;
                    }
                    SalPathAddBackslash(ourPath, MAX_PATH);
                }
                if (strcmp(p1Volume, p2Volume) != 0)
                    ret = FALSE;
            }
        }
    }
    return ret;
}

// ****************************************************************************

BOOL PathsAreOnTheSameVolume(const char* path1, const char* path2, BOOL* resIsOnlyEstimation)
{
    char root1[MAX_PATH];
    char root2[MAX_PATH];
    char ourPath[MAX_PATH];
    char path1NetPath[MAX_PATH];
    char path2NetPath[MAX_PATH];
    lstrcpyn(ourPath, path1, MAX_PATH);
    ResolveSubsts(ourPath);
    GetRootPath(root1, ourPath);
    lstrcpyn(ourPath, path2, MAX_PATH);
    ResolveSubsts(ourPath);
    GetRootPath(root2, ourPath);
    BOOL ret = TRUE;
    BOOL trySimpleTest = TRUE;
    if (resIsOnlyEstimation != NULL)
        *resIsOnlyEstimation = TRUE;
    if (!IsUNCPath(path1) && !IsUNCPath(path2)) // volume comparison on UNC paths makes no sense
    {
        char p1Volume[100] = "1";
        char p2Volume[100] = "2";
        UINT drvType1 = GetDriveType(root1);
        UINT drvType2 = GetDriveType(root2);
        if (drvType1 != DRIVE_REMOTE && drvType2 != DRIVE_REMOTE) // except for network drives we have a chance to obtain the "volume name"
        {
            BOOL cutPathIsPossible = TRUE;
            path1NetPath[0] = 0;         // network path to which the current (last) local symlink points
            if (drvType1 == DRIVE_FIXED) // searching for reparse points makes sense only on fixed disks
            {
                // under W2K and when not a root path, also walk through reparse points
                ResolveLocalPathWithReparsePoints(ourPath, path1, &cutPathIsPossible, NULL, NULL, NULL, NULL, path1NetPath);
            }
            else
                lstrcpyn(ourPath, root1, MAX_PATH);
            int numOfGetVolNamesFailed = 0;
            if (path1NetPath[0] == 0) // the volume name cannot be retrieved from a network path, so we will not attempt it
            {
                while (!GetVolumeNameForVolumeMountPoint(ourPath, p1Volume, 100))
                {
                    if (!cutPathIsPossible || !CutDirectory(ourPath))
                    { // even the root failed, unexpected (unfortunately happens on SUBSTed drives under W2K - debugged at Bachaalany - if both paths fail with the same roots we return EQUAL because it is more probable)
                        numOfGetVolNamesFailed++;
                        break;
                    }
                    SalPathAddBackslash(ourPath, MAX_PATH);
                }
            }

            cutPathIsPossible = TRUE;
            path2NetPath[0] = 0;         // network path to which the current (last) local symlink points
            if (drvType2 == DRIVE_FIXED) // searching for reparse points makes sense only on fixed disks
            {
                // under W2K and when not a root path, also walk through reparse points
                ResolveLocalPathWithReparsePoints(ourPath, path2, &cutPathIsPossible, NULL, NULL, NULL, NULL, path2NetPath);
            }
            else
                lstrcpyn(ourPath, root2, MAX_PATH);
            if (path2NetPath[0] == 0) // the volume name cannot be retrieved from a network path, so we will not attempt it
            {
                if (path1NetPath[0] == 0)
                {
                    while (!GetVolumeNameForVolumeMountPoint(ourPath, p2Volume, 100))
                    {
                        if (!cutPathIsPossible || !CutDirectory(ourPath))
                        { // even the root failed, unexpected (unfortunately happens on SUBSTed drives under W2K - debugged at Bachaalany - if both paths fail with the same roots we return EQUAL because it is more probable)
                            numOfGetVolNamesFailed++;
                            break;
                        }
                        SalPathAddBackslash(ourPath, MAX_PATH);
                    }
                    if (numOfGetVolNamesFailed != 2)
                    {
                        if (numOfGetVolNamesFailed == 0 && resIsOnlyEstimation != NULL)
                            *resIsOnlyEstimation = FALSE; // we are certain of the result only when a volume name was obtained from both paths (so they could not be network paths)
                        if (numOfGetVolNamesFailed == 1 || strcmp(p1Volume, p2Volume) != 0)
                            ret = FALSE; // only one volume name was obtained, so these are not the same volumes (and if they were we could not find out - perhaps resolving the SUBST target would help)
                        trySimpleTest = FALSE;
                    }
                }
                else // only one path is network, so these are not the same volumes (and if they are we cannot determine it)
                {
                    ret = FALSE;
                    trySimpleTest = FALSE;
                }
            }
            else
            {
                if (path1NetPath[0] != 0) // compare roots of network paths
                {
                    GetRootPath(root1, path1NetPath);
                    GetRootPath(root2, path2NetPath);
                }
                else // only one path is network, so these are not the same volumes (and if they are we cannot determine it)
                {
                    ret = FALSE;
                    trySimpleTest = FALSE;
                }
            }
        }
    }

    if (trySimpleTest) // try a simple test whether root paths match (network paths + everything on NT)
    {
        ret = _stricmp(root1, root2) == 0;

        if (resIsOnlyEstimation != NULL)
        {
            lstrcpyn(path1NetPath, path1, MAX_PATH);
            lstrcpyn(path2NetPath, path2, MAX_PATH);
            if (ResolveSubsts(path1NetPath) && ResolveSubsts(path2NetPath))
            {
                if (IsTheSamePath(path1NetPath, path2NetPath))
                    *resIsOnlyEstimation = FALSE; // identical paths definitely mean identical volumes
            }
        }
    }
    return ret;
}

// ****************************************************************************

BOOL IsTheSamePath(const char* path1, const char* path2)
{
    if (*path1 == '\\')
        path1++;
    if (*path2 == '\\')
        path2++;
    while (*path1 != 0 && LowerCase[*path1] == LowerCase[*path2])
    {
        path1++;
        path2++;
    }
    if (*path1 == '\\')
        path1++;
    if (*path2 == '\\')
        path2++;
    return *path1 == 0 && *path2 == 0;
}

// ****************************************************************************

int CommonPrefixLength(const char* path1, const char* path2)
{
    const char* lastBackslash = path1;
    int backslashCount = 0;
    int sameCount = 0;
    const char* s1 = path1;
    const char* s2 = path2;
    while (*s1 != 0 && *s2 != 0 && LowerCase[*s1] == LowerCase[*s2])
    {
        if (*s1 == '\\')
        {
            lastBackslash = s1;
            backslashCount++;
        }
        s1++;
        s2++;
    }

    if (s1 - path1 < 3)
        return 0;

    if (*s1 == 0 && *s2 == '\\' || *s1 == '\\' && *s2 == 0 ||
        *s1 == 0 && *s2 == 0 && *(s1 - 1) != '\\')
    {
        lastBackslash = s1; // this terminator will not be in lastBackslash
        backslashCount++;
    }

    if (path1[1] == ':')
    {
        // standard path
        if (path1[2] != '\\')
            return 0;

        // special case: for a root path we must return the length including the trailing backslash
        if (lastBackslash - path1 < 3)
            return 3;

        return (int)(lastBackslash - path1);
    }
    else
    {
        // UNC path
        if (path1[0] != '\\' || path1[1] != '\\')
            return 0;
        if (backslashCount < 4) // the path must have the form "\\machine\share"
            return 0;

        return (int)(lastBackslash - path1);
    }
}

// ****************************************************************************

BOOL SalPathIsPrefix(const char* prefix, const char* path)
{
    int commonLen = CommonPrefixLength(prefix, path);
    if (commonLen == 0)
        return FALSE;

    int prefixLen = (int)strlen(prefix);
    if (prefixLen < 3)
        return FALSE;

    // CommonPrefixLength returned the length without the final backslash (unless it was a root path)
    // if our prefix ends with a backslash, drop it
    if (prefixLen > 3 && prefix[prefixLen - 1] == '\\')
        prefixLen--;

    return (commonLen == prefixLen);
}

// ****************************************************************************

BOOL IsDirError(DWORD err)
{
    return err == ERROR_NETWORK_ACCESS_DENIED ||
           err == ERROR_ACCESS_DENIED ||
           err == ERROR_SECTOR_NOT_FOUND ||
           err == ERROR_SHARING_VIOLATION ||
           err == ERROR_BAD_PATHNAME ||
           err == ERROR_FILE_NOT_FOUND ||
           err == ERROR_PATH_NOT_FOUND ||
           err == ERROR_INVALID_NAME ||   // when a path contains an accented character on English Windows this error is reported instead of ERROR_PATH_NOT_FOUND
           err == ERROR_INVALID_FUNCTION; // reported by one user on WinXP with a network drive Y: when Salamander accessed a path that no longer existed (no truncation occurred; Shift+F7 on Y:\ solved it)
}

// ****************************************************************************

BOOL CutDirectory(char* path, char** cutDir)
{
    CALL_STACK_MESSAGE2("CutDirectory(%s,)", path);
    int l = (int)strlen(path);
    char* lastBackslash = path + l - 1;
    while (--lastBackslash >= path && *lastBackslash != '\\')
        ;
    char* nextBackslash = lastBackslash;
    while (--nextBackslash >= path && *nextBackslash != '\\')
        ;
    if (lastBackslash < path)
    {
        if (cutDir != NULL)
            *cutDir = path + l;
        return FALSE; // "somedir" or "c:\"
    }
    if (nextBackslash < path) // "c:\somedir" or "c:\somedir\"
    {
        if (cutDir != NULL)
        {
            if (*(path + l - 1) == '\\')
                *(path + --l) = 0; // remove trailing \ at the end
            memmove(lastBackslash + 2, lastBackslash + 1, l - (lastBackslash - path));
            *cutDir = lastBackslash + 2; // "somedir" or "seconddir"
        }
        *(lastBackslash + 1) = 0; // "c:\"
    }
    else // "c:\firstdir\seconddir" or "c:\firstdir\seconddir\"
    {    // UNC: "\\server\share\path"
        if (path[0] == '\\' && path[1] == '\\' && nextBackslash <= path + 2)
        { // "\\server\share" - cannot be shortened
            if (cutDir != NULL)
                *cutDir = path + l;
            return FALSE;
        }
        *lastBackslash = 0;
        if (cutDir != NULL) // cut off '\' at the end
        {
            if (*(path + l - 1) == '\\')
                *(path + l - 1) = 0;
            *cutDir = lastBackslash + 1;
        }
    }
    return TRUE;
}

// ****************************************************************************

int GetRootPath(char* root, const char* path)
{                                           // WARNING: unusual use from GetShellFolder(): "\\\\" returns "\\\\\\", "\\\\server" returns "\\\\server\\"
    if (path[0] == '\\' && path[1] == '\\') // UNC
    {
        const char* s = path + 2;
        while (*s != 0 && *s != '\\')
            s++;
        if (*s != 0)
            s++; // '\\'
        while (*s != 0 && *s != '\\')
            s++;
        int len = (int)(s - path);
        if (len > MAX_PATH - 2)
            len = MAX_PATH - 2; // fits with trailing '\\' into a MAX_PATH buffer (expected size); truncation should never happen but is harmless
        memcpy(root, path, len);
        root[len] = '\\';
        root[len + 1] = 0;
        return len + 1;
    }
    else
    {
        root[0] = path[0];
        root[1] = ':';
        root[2] = '\\';
        root[3] = 0;
        return 3;
    }
}

// ****************************************************************************

// iterate all colors from configuration and if they have the default value set,
// set them to the corresponding color values

COLORREF GetHilightColor(COLORREF clr1, COLORREF clr2)
{
    WORD h1, l1, s1;
    ColorRGBToHLS(clr1, &h1, &l1, &s1);
    BYTE gray1 = GetGrayscaleFromRGB(GetRValue(clr1), GetGValue(clr1), GetBValue(clr1));
    BYTE gray2 = GetGrayscaleFromRGB(GetRValue(clr2), GetGValue(clr2), GetBValue(clr2));
    COLORREF res;
    if (gray2 < 170 && gray1 <= 220)
    {
        unsigned wantedGray = (unsigned)gray1 + 20 + (220 - (unsigned)gray1) / 2;
        if (wantedGray < (unsigned)gray2 + 100)
            wantedGray = (unsigned)gray2 + 100;
        if (wantedGray > 255)
            wantedGray = 255;
        BOOL first = TRUE;
        while (first || l1 != 240)
        {
            first = FALSE;
            l1 += 5;
            if (l1 > 240)
                l1 = 240;
            res = ColorHLSToRGB(h1, l1, s1);
            if ((unsigned)GetGrayscaleFromRGB(GetRValue(res), GetGValue(res), GetBValue(res)) >= wantedGray)
                break;
        }
    }
    else
    {
        if ((gray1 >= gray2 ? gray1 - gray2 : gray2 - gray1) > 85 ||
            gray2 < 85 ||
            gray1 < 75)
        {
            if (gray1 > gray2)
            {
                res = RGB((4 * (unsigned)GetRValue(clr1) + 3 * (unsigned)GetRValue(clr2)) / 7,
                          (4 * (unsigned)GetGValue(clr1) + 3 * (unsigned)GetGValue(clr2)) / 7,
                          (4 * (unsigned)GetBValue(clr1) + 3 * (unsigned)GetBValue(clr2)) / 7);
            }
            else
            {
                res = RGB(((unsigned)GetRValue(clr1) + (unsigned)GetRValue(clr2)) / 2,
                          ((unsigned)GetGValue(clr1) + (unsigned)GetGValue(clr2)) / 2,
                          ((unsigned)GetBValue(clr1) + (unsigned)GetBValue(clr2)) / 2);
            }
        }
        else
        {
            res = RGB(0, 0, 0);
        }
    }
    return res;
}

COLORREF GetFullRowHighlight(COLORREF bkHighlightColor) // returns a "heuristic" highlight for full row mode
{
    // a bit of heuristics: slightly darken light backgrounds and slightly lighten dark backgrounds
    WORD h, l, s;
    ColorRGBToHLS(bkHighlightColor, &h, &l, &s);

    if (l < 121) // [DARK]  0-120 -> brighten the luminance gradually 0..120 -> +40..+20
        l += 20 + 20 * (120 - l) / 120;
    else // [LIGHT] 121-240 -> darken the luminance by a constant 20
        l -= 20;

    return ColorHLSToRGB(h, l, s);
}

void UpdateDefaultColors(SALCOLOR* colors, CHighlightMasks* highlightMasks, BOOL processColors, BOOL processMasks)
{
    if (processColors)
    {
        int bitsPerPixel = GetCurrentBPP();

        // take pen colors for item frames from the system window text color
        if (GetFValue(colors[FOCUS_ACTIVE_NORMAL]) & SCF_DEFAULT)
            SetRGBPart(&colors[FOCUS_ACTIVE_NORMAL], GetSysColor(COLOR_WINDOWTEXT));
        if (GetFValue(colors[FOCUS_ACTIVE_SELECTED]) & SCF_DEFAULT)
            SetRGBPart(&colors[FOCUS_ACTIVE_SELECTED], GetSysColor(COLOR_WINDOWTEXT));
        if (GetFValue(colors[FOCUS_BK_INACTIVE_NORMAL]) & SCF_DEFAULT)
            SetRGBPart(&colors[FOCUS_BK_INACTIVE_NORMAL], GetSysColor(COLOR_WINDOW));
        if (GetFValue(colors[FOCUS_BK_INACTIVE_SELECTED]) & SCF_DEFAULT)
            SetRGBPart(&colors[FOCUS_BK_INACTIVE_SELECTED], GetSysColor(COLOR_WINDOW));

        // item text colors in the panel use the system window text color
        if (GetFValue(colors[ITEM_FG_NORMAL]) & SCF_DEFAULT)
            SetRGBPart(&colors[ITEM_FG_NORMAL], GetSysColor(COLOR_WINDOWTEXT));
        if (GetFValue(colors[ITEM_FG_FOCUSED]) & SCF_DEFAULT)
            SetRGBPart(&colors[ITEM_FG_FOCUSED], GetSysColor(COLOR_WINDOWTEXT));
        if (GetFValue(colors[ITEM_FG_HIGHLIGHT]) & SCF_DEFAULT) // FULL ROW HIGHLIGHT is based on _NORMAL
            SetRGBPart(&colors[ITEM_FG_HIGHLIGHT], GetCOLORREF(colors[ITEM_FG_NORMAL]));

        // item background colors in the panel use the system window background color
        if (GetFValue(colors[ITEM_BK_NORMAL]) & SCF_DEFAULT)
            SetRGBPart(&colors[ITEM_BK_NORMAL], GetSysColor(COLOR_WINDOW));
        if (GetFValue(colors[ITEM_BK_SELECTED]) & SCF_DEFAULT)
            SetRGBPart(&colors[ITEM_BK_SELECTED], GetSysColor(COLOR_WINDOW));
        if (GetFValue(colors[ITEM_BK_HIGHLIGHT]) & SCF_DEFAULT) // HIGHLIGHT copies NORMAL so custom/Norton modes work
            SetRGBPart(&colors[ITEM_BK_HIGHLIGHT], GetFullRowHighlight(GetCOLORREF(colors[ITEM_BK_NORMAL])));

        // progress bar colors
        if (GetFValue(colors[PROGRESS_FG_NORMAL]) & SCF_DEFAULT)
            SetRGBPart(&colors[PROGRESS_FG_NORMAL], GetSysColor(COLOR_WINDOWTEXT));
        if (GetFValue(colors[PROGRESS_FG_SELECTED]) & SCF_DEFAULT)
            SetRGBPart(&colors[PROGRESS_FG_SELECTED], GetSysColor(COLOR_HIGHLIGHTTEXT));
        if (GetFValue(colors[PROGRESS_BK_NORMAL]) & SCF_DEFAULT)
            SetRGBPart(&colors[PROGRESS_BK_NORMAL], GetSysColor(COLOR_WINDOW));
        if (GetFValue(colors[PROGRESS_BK_SELECTED]) & SCF_DEFAULT)
            SetRGBPart(&colors[PROGRESS_BK_SELECTED], GetSysColor(COLOR_HIGHLIGHT));

        // color for the selected icon shade
        if (GetFValue(colors[ICON_BLEND_SELECTED]) & SCF_DEFAULT)
        {
            // normally copy the color for selected from focused+selected
            SetRGBPart(&colors[ICON_BLEND_SELECTED], GetCOLORREF(colors[ICON_BLEND_FOCSEL]));
            // when it is red (Salamander profile and we can afford it thanks to color depth
            // allowed) we use a lighter shade for selected
            if (bitsPerPixel > 8 && GetCOLORREF(colors[ICON_BLEND_FOCSEL]) == RGB(255, 0, 0))
                SetRGBPart(&colors[ICON_BLEND_SELECTED], RGB(255, 128, 128));
        }

#define COLOR_HOTLIGHT 26 // winuser.h

        // panel captions (active/inactive)

        // active panel caption: background
        if (GetFValue(colors[ACTIVE_CAPTION_BK]) & SCF_DEFAULT)
            SetRGBPart(&colors[ACTIVE_CAPTION_BK], GetSysColor(COLOR_ACTIVECAPTION));
        // active panel caption: text
        if (GetFValue(colors[ACTIVE_CAPTION_FG]) & SCF_DEFAULT)
            SetRGBPart(&colors[ACTIVE_CAPTION_FG], GetSysColor(COLOR_CAPTIONTEXT));
        // inactive panel caption: background
        if (GetFValue(colors[INACTIVE_CAPTION_BK]) & SCF_DEFAULT)
            SetRGBPart(&colors[INACTIVE_CAPTION_BK], GetSysColor(COLOR_INACTIVECAPTION));
        // inactive panel caption: text
        if (GetFValue(colors[INACTIVE_CAPTION_FG]) & SCF_DEFAULT)
        {
            // we prefer the same text color as for the active caption, but sometimes this color is too
            // close to the background color, then we use the inactive caption text color
            COLORREF clrBk = GetCOLORREF(colors[INACTIVE_CAPTION_BK]);
            COLORREF clrFgAc = GetSysColor(COLOR_CAPTIONTEXT);
            COLORREF clrFgIn = GetSysColor(COLOR_INACTIVECAPTIONTEXT);
            BYTE grayBk = GetGrayscaleFromRGB(GetRValue(clrBk), GetGValue(clrBk), GetBValue(clrBk));
            BYTE grayFgAc = GetGrayscaleFromRGB(GetRValue(clrFgAc), GetGValue(clrFgAc), GetBValue(clrFgAc));
            BYTE grayFgIn = GetGrayscaleFromRGB(GetRValue(clrFgIn), GetGValue(clrFgIn), GetBValue(clrFgIn));
            SetRGBPart(&colors[INACTIVE_CAPTION_FG], (abs(grayFgAc - grayBk) >= abs(grayFgIn - grayBk)) ? clrFgAc : clrFgIn);
        }

        // hot item colors
        COLORREF hotColor = GetSysColor(COLOR_HOTLIGHT);
        if (GetFValue(colors[HOT_PANEL]) & SCF_DEFAULT)
            SetRGBPart(&colors[HOT_PANEL], hotColor);

        // highlight for the active panel caption
        if (GetFValue(colors[HOT_ACTIVE]) & SCF_DEFAULT)
        {
            COLORREF clr = GetCOLORREF(colors[ACTIVE_CAPTION_FG]);
            if (bitsPerPixel > 4)
                clr = GetHilightColor(clr, GetCOLORREF(colors[ACTIVE_CAPTION_BK]));
            SetRGBPart(&colors[HOT_ACTIVE], clr);
        }
        // highlight for the inactive panel caption
        if (GetFValue(colors[HOT_INACTIVE]) & SCF_DEFAULT)
        {
            COLORREF clr = GetCOLORREF(colors[INACTIVE_CAPTION_FG]);
            if (bitsPerPixel > 4)
                clr = GetHilightColor(clr, GetCOLORREF(colors[INACTIVE_CAPTION_BK]));
            SetRGBPart(&colors[HOT_INACTIVE], clr);
        }
    }

    if (processMasks)
    {
        // colors depending on file name and attributes
        int i;
        for (i = 0; i < highlightMasks->Count; i++)
        {
            CHighlightMasksItem* item = highlightMasks->At(i);
            if (GetFValue(item->NormalFg) & SCF_DEFAULT)
                SetRGBPart(&item->NormalFg, GetCOLORREF(colors[ITEM_FG_NORMAL]));
            if (GetFValue(item->NormalBk) & SCF_DEFAULT)
                SetRGBPart(&item->NormalBk, GetCOLORREF(colors[ITEM_BK_NORMAL]));
            if (GetFValue(item->FocusedFg) & SCF_DEFAULT)
                SetRGBPart(&item->FocusedFg, GetCOLORREF(colors[ITEM_FG_FOCUSED]));
            if (GetFValue(item->FocusedBk) & SCF_DEFAULT)
                SetRGBPart(&item->FocusedBk, GetCOLORREF(colors[ITEM_BK_FOCUSED]));
            if (GetFValue(item->SelectedFg) & SCF_DEFAULT)
                SetRGBPart(&item->SelectedFg, GetCOLORREF(colors[ITEM_FG_SELECTED]));
            if (GetFValue(item->SelectedBk) & SCF_DEFAULT)
                SetRGBPart(&item->SelectedBk, GetCOLORREF(colors[ITEM_BK_SELECTED]));
            if (GetFValue(item->FocSelFg) & SCF_DEFAULT)
                SetRGBPart(&item->FocSelFg, GetCOLORREF(colors[ITEM_FG_FOCSEL]));
            if (GetFValue(item->FocSelBk) & SCF_DEFAULT)
                SetRGBPart(&item->FocSelBk, GetCOLORREF(colors[ITEM_BK_FOCSEL]));
            if (GetFValue(item->HighlightFg) & SCF_DEFAULT)
                SetRGBPart(&item->HighlightFg, GetCOLORREF(item->NormalFg));
            if (GetFValue(item->HighlightBk) & SCF_DEFAULT) // FULL ROW HIGHLIGHT is based on _NORMAL
                SetRGBPart(&item->HighlightBk, GetFullRowHighlight(GetCOLORREF(item->NormalBk)));
        }
    }
}

//****************************************************************************
//
// Depending on the display color depth determines whether to use 256-color
// or 16-color bitmaps.
//

BOOL Use256ColorsBitmap()
{
    int bitsPerPixel = GetCurrentBPP();
    return (bitsPerPixel > 8); // more than 256 colors
}

DWORD GetImageListColorFlags()
{
    // if the image list uses 16-bit color depth the alpha channel of new icons misbehaves on WinXP 32-bit color display (32-bit depth works)
    // if the image list uses 32-bit color depth the blend of a selected item misbehaves on Win2K 32-bit color display (16-bit depth works)
    return ILC_COLOR32;
}

// ****************************************************************************

int LoadColorTable(int id, RGBQUAD* rgb, int rgbCount)
{
    int count = 0;
    HRSRC hRsrc = FindResource(HInstance, MAKEINTRESOURCE(id), RT_RCDATA);
    if (hRsrc)
    {
        void* data = LoadResource(HInstance, hRsrc);
        if (data)
        {
            DWORD size = SizeofResource(HInstance, hRsrc);
            if (size > 0)
            {
                int max = min(rgbCount, (WORD)size / 3);
                BYTE* ptr = (BYTE*)data;
                int i;
                for (i = 0; i < max; i++)
                {
                    rgb[i].rgbBlue = *ptr++;
                    rgb[i].rgbGreen = *ptr++;
                    rgb[i].rgbRed = *ptr++;
                    rgb[i].rgbReserved = 0;
                    count++;
                }
            }
        }
    }
    return count;
}

BOOL InitializeConstGraphics()
{
    // ensure smooth graphical output
    // 20 GDI API calls should be more than enough
    // this is the default value from NT 4.0 WS
    if (GdiGetBatchLimit() < 20)
    {
        TRACE_I("Increasing GdiBatchLimit");
        GdiSetBatchLimit(20);
    }

    if (LoadColorTable(IDC_COLORTABLE, ColorTable, 256) != 256)
    {
        TRACE_E("Loading ColorTable failed");
        return FALSE;
    }

    if (SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0, &DragFullWindows, FALSE) == 0)
        DragFullWindows = TRUE;

    // initialize the LogFont structure
    NONCLIENTMETRICS ncm;
    ncm.cbSize = sizeof(ncm);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);
    LogFont = ncm.lfStatusFont;
    /*
  LogFont.lfHeight = -10;
  LogFont.lfWidth = 0;
  LogFont.lfEscapement = 0;
  LogFont.lfOrientation = 0;
  LogFont.lfWeight = FW_NORMAL;
  LogFont.lfItalic = 0;
  LogFont.lfUnderline = 0;
  LogFont.lfStrikeOut = 0;
  LogFont.lfCharSet = UserCharset;
  LogFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
  LogFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
  LogFont.lfQuality = DEFAULT_QUALITY;
  LogFont.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
  strcpy(LogFont.lfFaceName, "MS Shell Dlg 2");
  */

    // these brushes are allocated by the system and automatically change when colors change
    HDialogBrush = GetSysColorBrush(COLOR_BTNFACE);
    HButtonTextBrush = GetSysColorBrush(COLOR_BTNTEXT);
    HMenuSelectedBkBrush = GetSysColorBrush(COLOR_HIGHLIGHT);
    HMenuSelectedTextBrush = GetSysColorBrush(COLOR_HIGHLIGHTTEXT);
    HMenuHilightBrush = GetSysColorBrush(COLOR_3DHILIGHT);
    HMenuGrayTextBrush = GetSysColorBrush(COLOR_3DSHADOW);
    if (HDialogBrush == NULL || HButtonTextBrush == NULL ||
        HMenuSelectedTextBrush == NULL || HMenuHilightBrush == NULL ||
        HMenuGrayTextBrush == NULL)
    {
        TRACE_E("Unable to create brush.");
        return FALSE;
    }
    ItemBitmap.CreateBmp(NULL, 1, 1); // ensure the bitmap exists

    // the bitmap is loaded only once and not refreshed when the resolution changes
    // if the user switched colors above 256, a LoadBitmap compatible with the
    // display DC would leave the bitmap degraded; therefore we load it as a DIB
    // HWorkerBitmap = HANDLES(LoadBitmap(HInstance, MAKEINTRESOURCE(IDB_WORKER)));
    //HWorkerBitmap = (HBITMAP)HANDLES(LoadImage(HInstance, MAKEINTRESOURCE(IDB_WORKER), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION));
    //if (HWorkerBitmap == NULL)
    //  return FALSE;

    // when the font changes CreatePanelFont and CreateEnvFont are called explicitly; perform the initial setup here
    CreatePanelFont();
    CreateEnvFonts();

    if (Font == NULL || FontUL == NULL || EnvFont == NULL || EnvFontUL == NULL || TooltipFont == NULL)
    {
        TRACE_E("Unable to create fonts.");
        return FALSE;
    }

    return TRUE;
}

void ReleaseConstGraphics()
{
    ItemBitmap.Destroy();
    //if (HWorkerBitmap != NULL)
    //{
    //  HANDLES(DeleteObject(HWorkerBitmap));
    //  HWorkerBitmap = NULL;
    //}

    if (Font != NULL)
    {
        HANDLES(DeleteObject(Font));
        Font = NULL;
    }

    if (FontUL != NULL)
    {
        HANDLES(DeleteObject(FontUL));
        FontUL = NULL;
    }

    if (TooltipFont != NULL)
    {
        HANDLES(DeleteObject(TooltipFont));
        TooltipFont = NULL;
    }

    if (EnvFont != NULL)
    {
        HANDLES(DeleteObject(EnvFont));
        EnvFont = NULL;
    }

    if (EnvFontUL != NULL)
    {
        HANDLES(DeleteObject(EnvFontUL));
        EnvFontUL = NULL;
    }
}

BOOL AuxAllocateImageLists()
{
    int i;
    for (i = 0; i < ICONSIZE_COUNT; i++)
    {
        SimpleIconLists[i] = new CIconList();
        if (SimpleIconLists[i] == NULL)
        {
            TRACE_E(LOW_MEMORY);
            return FALSE;
        }
    }

    ThrobberFrames = new CIconList();
    if (ThrobberFrames == NULL)
    {
        TRACE_E(LOW_MEMORY);
        return FALSE;
    }

    LockFrames = new CIconList();
    if (LockFrames == NULL)
    {
        TRACE_E(LOW_MEMORY);
        return FALSE;
    }

    return TRUE;
}

// using TweakUI users can change the shortcut icon (default, custom, none)
// we will try to honor it
BOOL GetShortcutOverlay()
{
    int i;
    for (i = 0; i < ICONSIZE_COUNT; i++)
    {
        if (HShortcutOverlays[i] != NULL)
        {
            HANDLES(DestroyIcon(HShortcutOverlays[i]));
            HShortcutOverlays[i] = NULL;
        }
    }

    /*  
  //#include <CommonControls.h>

    // reading overlay icons from the system image list is unnecessarily slow; we load them directly from imageres.dll
    // this code is kept here just in case we need to locate the icons again
  typedef DECLSPEC_IMPORT HRESULT (WINAPI *F__SHGetImageList)(int iImageList, REFIID riid, void **ppvObj);

  F__SHGetImageList MySHGetImageList = (F__SHGetImageList)GetProcAddress(Shell32DLL, "SHGetImageList"); // Min: XP
  if (MySHGetImageList != NULL)
  {
    int shareIndex = SHGetIconOverlayIndex(NULL, IDO_SHGIOI_SHARE);
    int linkIndex = SHGetIconOverlayIndex(NULL, IDO_SHGIOI_LINK);
    int offlineIndex = SHGetIconOverlayIndex(NULL, IDO_SHGIOI_SLOWFILE);
    shareIndex = SHGetIconOverlayIndex(NULL, IDO_SHGIOI_SHARE);

    IImageList *imageList;
    if (MySHGetImageList(1 /* SHIL_SMALL * /, IID_IImageList, (void **)&imageList) == S_OK &&
        imageList != NULL)
    {
      int i;
      imageList->GetOverlayImage(linkIndex, &i);
      HICON icon;
      if (imageList->GetIcon(i, 0, &icon) != S_OK)
        icon = NULL;
      HShortcutOverlays[ICONSIZE_16] = icon;
      imageList->Release();
    }
    if (MySHGetImageList(0 /* SHIL_LARGE * /, IID_IImageList, (void **)&imageList) == S_OK &&
        imageList != NULL)
    {
      int i;
      imageList->GetOverlayImage(linkIndex, &i);
      HICON icon;
      if (imageList->GetIcon(i, 0, &icon) != S_OK)
        icon = NULL;
      HShortcutOverlays[ICONSIZE_32] = icon;
      imageList->Release();
    }
    if (MySHGetImageList(2 /* SHIL_EXTRALARGE * /, IID_IImageList, (void **)&imageList) == S_OK &&
        imageList != NULL)
    {
      int i;
      imageList->GetOverlayImage(linkIndex, &i);
      HICON icon;
      if (imageList->GetIcon(i, 0, &icon) != S_OK)
        icon = NULL;
      HShortcutOverlays[ICONSIZE_48] = icon;
      imageList->Release();
    }
  }
*/

    HKEY hKey;
    if (NOHANDLES(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                               "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Icons",
                               0, KEY_QUERY_VALUE, &hKey)) == ERROR_SUCCESS)
    {
        char buff[MAX_PATH + 10];
        DWORD buffLen = sizeof(buff);
        buff[0] = 0;
        SalRegQueryValueEx(hKey, "29", NULL, NULL, (LPBYTE)buff, &buffLen);
        if (buff[0] != 0)
        {
            char* num = strrchr(buff, ','); // the icon number is after the last comma
            if (num != NULL)
            {
                int index = atoi(num + 1);
                *num = 0;

                HICON hIcons[2] = {0, 0};

                ExtractIcons(buff, index,
                             MAKELONG(32, 16),
                             MAKELONG(32, 16),
                             hIcons, NULL, 2, IconLRFlags);

                HShortcutOverlays[ICONSIZE_32] = hIcons[0];
                HShortcutOverlays[ICONSIZE_16] = hIcons[1];

                ExtractIcons(buff, index,
                             48,
                             48,
                             hIcons, NULL, 1, IconLRFlags);
                HShortcutOverlays[ICONSIZE_48] = hIcons[0];

                for (i = 0; i < ICONSIZE_COUNT; i++)
                    if (HShortcutOverlays[i] != NULL)
                        HANDLES_ADD(__htIcon, __hoLoadImage, HShortcutOverlays[i]);
            }
        }
        NOHANDLES(RegCloseKey(hKey));
    }

    for (i = 0; i < ICONSIZE_COUNT; i++)
    {
        if (HShortcutOverlays[i] == NULL)
        {
            HShortcutOverlays[i] = (HICON)HANDLES(LoadImage(ImageResDLL, MAKEINTRESOURCE(163),
                                                            IMAGE_ICON, IconSizes[i], IconSizes[i], IconLRFlags));
        }
    }
    return (HShortcutOverlays[ICONSIZE_16] != NULL &&
            HShortcutOverlays[ICONSIZE_32] != NULL &&
            HShortcutOverlays[ICONSIZE_48] != NULL);
}

int GetCurrentBPP(HDC hDC)
{
    HDC hdc;
    if (hDC == NULL)
        hdc = GetDC(NULL);
    else
        hdc = hDC;
    int bpp = GetDeviceCaps(hdc, PLANES) * GetDeviceCaps(hdc, BITSPIXEL);
    if (hDC == NULL)
        ReleaseDC(NULL, hdc);

    return bpp;
}

int GetSystemDPI()
{
    if (SystemDPI == 0)
    {
        TRACE_E("GetSystemDPI() SystemDPI == 0!");
        return 96;
    }
    else
    {
        return SystemDPI;
    }
}

int GetScaleForSystemDPI()
{
    int dpi = GetSystemDPI();
    int scale;
    if (dpi <= 96)
        scale = 100;
    else if (dpi <= 120)
        scale = 125;
    else if (dpi <= 144)
        scale = 150;
    else if (dpi <= 192)
        scale = 200;
    else if (dpi <= 240)
        scale = 250;
    else if (dpi <= 288)
        scale = 300;
    else if (dpi <= 384)
        scale = 400;
    else
        scale = 500;

    return scale;
}

int GetIconSizeForSystemDPI(CIconSizeEnum iconSize)
{
    if (SystemDPI == 0)
    {
        TRACE_E("GetIconSizeForSystemDPI() SystemDPI == 0!");
        return 16;
    }

    if (iconSize < ICONSIZE_16 || iconSize >= ICONSIZE_COUNT)
    {
        TRACE_E("GetIconSizeForSystemDPI() unknown iconSize!");
        return 16;
    }

    // DPI Name      DPI   Scale factor
    // --------------------------------
    // Smaller        96   1.00 (100%)
    // Medium        120   1.25 (125%)
    // Larger        144   1.50 (150%)
    // Extra Large   192   2.00 (200%)
    // Custom        240   2.50 (250%)
    // Custom        288   3.00 (300%)
    // Custom        384   4.00 (400%)
    // Custom        480   5.00 (500%)

    int scale = GetScaleForSystemDPI();

    int baseIconSize[ICONSIZE_COUNT] = {16, 32, 48}; // must correspond to CIconSizeEnum

    return (baseIconSize[iconSize] * scale) / 100;
}

void GetSystemDPI(HDC hDC)
{
    HDC hTmpDC;
    if (hDC == NULL)
        hTmpDC = GetDC(NULL);
    else
        hTmpDC = hDC;
    SystemDPI = GetDeviceCaps(hTmpDC, LOGPIXELSX);
#ifdef _DEBUG
    if (SystemDPI != GetDeviceCaps(hTmpDC, LOGPIXELSY))
        TRACE_E("Unexpected situation: LOGPIXELSX != LOGPIXELSY.");
#endif
    if (hDC == NULL)
        ReleaseDC(NULL, hTmpDC);
}

BOOL InitializeGraphics(BOOL colorsOnly)
{
    // 48x48 available only starting with XP
    // in reality large icons have been supported for a long time and can be enabled
    // Desktop/Properties/???/Large Icons; note that the system image list will then not exist
    // for 32x32 icons; additionally we should obtain the real icon sizes from the system
    // for now we ignore this and allow 48x48 only from XP, where they are commonly available

    //
    // Retrieve the requested icon color depth from the Registry
    //
    int iconColorsCount = 0;
    HDC hDesktopDC = GetDC(NULL);
    int bpp = GetCurrentBPP(hDesktopDC);
    GetSystemDPI(hDesktopDC);
    ReleaseDC(NULL, hDesktopDC);

    IconSizes[ICONSIZE_16] = GetIconSizeForSystemDPI(ICONSIZE_16);
    IconSizes[ICONSIZE_32] = GetIconSizeForSystemDPI(ICONSIZE_32);
    IconSizes[ICONSIZE_48] = GetIconSizeForSystemDPI(ICONSIZE_48);

    HKEY hKey;
    if (OpenKeyAux(NULL, HKEY_CURRENT_USER, "Control Panel\\Desktop\\WindowMetrics", hKey))
    {
        // other interesting values: "Shell Icon Size", "Shell Small Icon Size"
        char buff[100];
        if (GetValueAux(NULL, hKey, "Shell Icon Bpp", REG_SZ, buff, 100))
        {
            iconColorsCount = atoi(buff);
        }
        else
        {
            if (WindowsVistaAndLater)
            {
                // this key simply does not exist on Vista and I do not know its replacement,
                // so we pretend icons use full colors (otherwise we displayed 16-color ugliness)
                iconColorsCount = 32;
            }
        }

        if (iconColorsCount > bpp)
            iconColorsCount = bpp;
        if (bpp <= 8)
            iconColorsCount = 0;

        HANDLES(RegCloseKey(hKey));
    }

    TRACE_I("InitializeGraphics() bpp=" << bpp << " iconColorsCount=" << iconColorsCount);
    if (bpp >= 4 && iconColorsCount <= 4)
        IconLRFlags = LR_VGACOLOR;
    else
        IconLRFlags = 0;

    HDC dc = HANDLES(GetDC(NULL));
    CHighlightMasks* masks = MainWindow != NULL ? MainWindow->HighlightMasks : NULL;
    UpdateDefaultColors(CurrentColors, masks, TRUE, masks != NULL);
    if (!colorsOnly)
    {
        ImageResDLL = HANDLES(LoadLibraryEx("imageres.dll", NULL, LOAD_LIBRARY_AS_DATAFILE));
        if (ImageResDLL == NULL)
        {
            TRACE_E("Unable to load library imageres.dll.");
            return FALSE;
        }

        Shell32DLL = HANDLES(LoadLibraryEx("shell32.dll", NULL, LOAD_LIBRARY_AS_DATAFILE));
        if (Shell32DLL == NULL) // this should never happen (core Win 4.0)
        {
            TRACE_E("Unable to load library shell32.dll.");
            return FALSE;
        }

        HINSTANCE iconDLL = ImageResDLL;
        int iconIndex = 164;
        int i;
        for (i = 0; i < ICONSIZE_COUNT; i++)
        {
            HSharedOverlays[i] = (HICON)HANDLES(LoadImage(iconDLL, MAKEINTRESOURCE(iconIndex),
                                                          IMAGE_ICON, IconSizes[i], IconSizes[i], IconLRFlags));
        }
        GetShortcutOverlay(); // HShortcutOverlayXX

        iconIndex = 97;
        for (i = 0; i < ICONSIZE_COUNT; i++)
        {
            HSlowFileOverlays[i] = (HICON)HANDLES(LoadImage(iconDLL, MAKEINTRESOURCE(iconIndex),
                                                            IMAGE_ICON, IconSizes[i], IconSizes[i], IconLRFlags));
        }

        HGroupIcon = SalLoadImage(4, 20, IconSizes[ICONSIZE_16], IconSizes[ICONSIZE_16], IconLRFlags);
        HFavoritIcon = (HICON)HANDLES(LoadImage(Shell32DLL, MAKEINTRESOURCE(319), IMAGE_ICON, IconSizes[ICONSIZE_16], IconSizes[ICONSIZE_16], IconLRFlags));
        if (HSharedOverlays[ICONSIZE_16] == NULL ||
            HSharedOverlays[ICONSIZE_32] == NULL ||
            HSharedOverlays[ICONSIZE_48] == NULL ||
            HShortcutOverlays[ICONSIZE_16] == NULL ||
            HShortcutOverlays[ICONSIZE_32] == NULL ||
            HShortcutOverlays[ICONSIZE_48] == NULL ||
            HSlowFileOverlays[ICONSIZE_16] == NULL ||
            HSlowFileOverlays[ICONSIZE_32] == NULL ||
            HSlowFileOverlays[ICONSIZE_48] == NULL ||
            HGroupIcon == NULL || HFavoritIcon == NULL)
        {
            TRACE_E("Unable to read icon overlays for shared directories, shortcuts or slow files, or icon for groups or favorites.");
            return FALSE;
        }

        // the compiler reported error C2712: Cannot use __try in functions that require object unwinding
        // workaround: allocate inside a function
        //    SymbolsIconList = new CIconList();
        //    LargeSymbolsIconList = new CIconList();
        if (!AuxAllocateImageLists())
            return FALSE;

        for (i = 0; i < ICONSIZE_COUNT; i++)
        {
            if (!SimpleIconLists[i]->Create(IconSizes[i], IconSizes[i], symbolsCount))
            {
                TRACE_E("Unable to create image lists.");
                return FALSE;
            }
            SimpleIconLists[i]->SetBkColor(GetCOLORREF(CurrentColors[ITEM_BK_NORMAL]));
        }

        if (!ThrobberFrames->CreateFromPNG(HInstance, MAKEINTRESOURCE(IDB_THROBBER), THROBBER_WIDTH))
        {
            TRACE_E("Unable to create throbber.");
            return FALSE;
        }

        if (!LockFrames->CreateFromPNG(HInstance, MAKEINTRESOURCE(IDB_LOCK), LOCK_WIDTH))
        {
            TRACE_E("Unable to create lock.");
            return FALSE;
        }

        HFindSymbolsImageList = ImageList_Create(IconSizes[ICONSIZE_16], IconSizes[ICONSIZE_16], ILC_MASK | GetImageListColorFlags(), 2, 0);
        if (HFindSymbolsImageList == NULL)
        {
            TRACE_E("Unable to create image list.");
            return FALSE;
        }
        ImageList_SetImageCount(HFindSymbolsImageList, 2); // initialization
                                                           //    ImageList_SetBkColor(HFindSymbolsImageList, GetSysColor(COLOR_WINDOW)); // needed on XP for transparent icons

        int iconSize = IconSizes[ICONSIZE_16];
        HBITMAP hTmpMaskBitmap;
        HBITMAP hTmpGrayBitmap;
        HBITMAP hTmpColorBitmap;
        if (!CreateToolbarBitmaps(HInstance,
                                  IDB_MENU,
                                  RGB(255, 0, 255), GetSysColor(COLOR_BTNFACE),
                                  hTmpMaskBitmap, hTmpGrayBitmap, hTmpColorBitmap,
                                  FALSE, NULL, 0))
            return FALSE;
        HMenuMarkImageList = ImageList_Create(iconSize, iconSize, ILC_MASK | ILC_COLORDDB, 2, 1);
        ImageList_Add(HMenuMarkImageList, hTmpColorBitmap, hTmpMaskBitmap);
        HANDLES(DeleteObject(hTmpMaskBitmap));
        HANDLES(DeleteObject(hTmpGrayBitmap));
        HANDLES(DeleteObject(hTmpColorBitmap));

        CSVGIcon* svgIcons;
        int svgIconsCount;
        GetSVGIconsMainToolbar(&svgIcons, &svgIconsCount);
        if (!CreateToolbarBitmaps(HInstance,
                                  Use256ColorsBitmap() ? IDB_TOOLBAR_256 : IDB_TOOLBAR_16,
                                  RGB(255, 0, 255), GetSysColor(COLOR_BTNFACE),
                                  hTmpMaskBitmap, hTmpGrayBitmap, hTmpColorBitmap,
                                  TRUE, svgIcons, svgIconsCount))
            return FALSE;
        HHotToolBarImageList = ImageList_Create(iconSize, iconSize, ILC_MASK | ILC_COLORDDB, IDX_TB_COUNT, 1);
        HGrayToolBarImageList = ImageList_Create(iconSize, iconSize, ILC_MASK | ILC_COLORDDB, IDX_TB_COUNT, 1);
        ImageList_Add(HHotToolBarImageList, hTmpColorBitmap, hTmpMaskBitmap);
        ImageList_Add(HGrayToolBarImageList, hTmpGrayBitmap, hTmpMaskBitmap);
        HANDLES(DeleteObject(hTmpMaskBitmap));
        HANDLES(DeleteObject(hTmpGrayBitmap));
        HANDLES(DeleteObject(hTmpColorBitmap));

        if (HHotToolBarImageList == NULL || HGrayToolBarImageList == NULL)
        {
            TRACE_E("Unable to create image list.");
            return FALSE;
        }

        HBottomTBImageList = ImageList_Create(BOTTOMBAR_CX, BOTTOMBAR_CY, ILC_MASK | ILC_COLORDDB, 12, 0);
        HHotBottomTBImageList = ImageList_Create(BOTTOMBAR_CX, BOTTOMBAR_CY, ILC_MASK | ILC_COLORDDB, 12, 0);
        if (HBottomTBImageList == NULL || HHotBottomTBImageList == NULL)
        {
            TRACE_E("Unable to create image list.");
            return FALSE;
        }

        // extract icons from shell32:
        int indexes[] = {symbolsExecutable, symbolsDirectory, symbolsNonAssociated, symbolsAssociated, -1};
        int resID[] = {3, 4, 1, 2, -1};
        int vistaResID[] = {15, 4, 2, 90, -1};
        HICON hIcon;
        for (i = 0; indexes[i] != -1; i++)
        {
            int sizeIndex;
            for (sizeIndex = 0; sizeIndex < ICONSIZE_COUNT; sizeIndex++)
            {
                hIcon = SalLoadImage(vistaResID[i], resID[i], IconSizes[sizeIndex], IconSizes[sizeIndex], IconLRFlags);
                if (hIcon != NULL)
                {
                    SimpleIconLists[sizeIndex]->ReplaceIcon(indexes[i], hIcon);
                    if (sizeIndex == ICONSIZE_16)
                    {
                        if (indexes[i] == symbolsDirectory)
                            ImageList_ReplaceIcon(HFindSymbolsImageList, 0, hIcon);
                        if (indexes[i] == symbolsNonAssociated)
                            ImageList_ReplaceIcon(HFindSymbolsImageList, 1, hIcon);
                    }
                    HANDLES(DestroyIcon(hIcon));
                }
                else
                    TRACE_E("Cannot retrieve icon from IMAGERES.DLL or SHELL32.DLL resID=" << resID[i]);
            }
        }
        char systemDir[MAX_PATH];
        GetSystemDirectory(systemDir, MAX_PATH);
        // 16x16, 32x32, 48x48
        int sizeIndex;
        for (sizeIndex = ICONSIZE_16; sizeIndex < ICONSIZE_COUNT; sizeIndex++)
        {
            // directory icon
            hIcon = NULL;
            __try
            {
                if (!GetFileIcon(systemDir, FALSE, &hIcon, (CIconSizeEnum)sizeIndex, FALSE, FALSE))
                    hIcon = NULL;
            }
            __except (CCallStack::HandleException(GetExceptionInformation(), 15))
            {
                FGIExceptionHasOccured++;
                hIcon = NULL;
            }
            if (hIcon != NULL) // if the icon cannot be retrieved there is still a fallback ID 4 from shell32.dll
            {
                SimpleIconLists[sizeIndex]->ReplaceIcon(symbolsDirectory, hIcon);
                NOHANDLES(DestroyIcon(hIcon));
            }

            // the ".." icon
            hIcon = (HICON)HANDLES(LoadImage(HInstance, MAKEINTRESOURCE(IDI_UPPERDIR),
                                             IMAGE_ICON, IconSizes[sizeIndex], IconSizes[sizeIndex],
                                             IconLRFlags));
            SimpleIconLists[sizeIndex]->ReplaceIcon(symbolsUpDir, hIcon);
            HANDLES(DestroyIcon(hIcon));

            // archive icon
            hIcon = LoadArchiveIcon(IconSizes[sizeIndex], IconSizes[sizeIndex], IconLRFlags);
            SimpleIconLists[sizeIndex]->ReplaceIcon(symbolsArchive, hIcon);
            HANDLES(DestroyIcon(hIcon));
        }

        WORD bits[8] = {0x0055, 0x00aa, 0x0055, 0x00aa,
                        0x0055, 0x00aa, 0x0055, 0x00aa};
        HBITMAP hBrushBitmap = HANDLES(CreateBitmap(8, 8, 1, 1, &bits));
        HDitherBrush = HANDLES(CreatePatternBrush(hBrushBitmap));
        HANDLES(DeleteObject(hBrushBitmap));
        if (HDitherBrush == NULL)
            return FALSE;

        HUpDownBitmap = HANDLES(LoadBitmap(HInstance, MAKEINTRESOURCE(IDB_UPDOWN)));
        HZoomBitmap = HANDLES(LoadBitmap(HInstance, MAKEINTRESOURCE(IDB_ZOOM)));
        HFilter = HANDLES(LoadBitmap(HInstance, MAKEINTRESOURCE(IDB_FILTER)));

        if (HUpDownBitmap == NULL ||
            HZoomBitmap == NULL || HFilter == NULL)
        {
            TRACE_E("HUpDownBitmap == NULL || HZoomBitmap == NULL || HFilter == NULL");
            return FALSE;
        }

        SVGArrowRight.Load(IDV_ARROW_RIGHT, -1, -1, SVGSTATE_ENABLED | SVGSTATE_DISABLED);
        SVGArrowRightSmall.Load(IDV_ARROW_RIGHT, -1, (int)((double)iconSize / 2.5), SVGSTATE_ENABLED | SVGSTATE_DISABLED);
        SVGArrowMore.Load(IDV_ARROW_MORE, -1, -1, SVGSTATE_ENABLED | SVGSTATE_DISABLED);
        SVGArrowLess.Load(IDV_ARROW_LESS, -1, -1, SVGSTATE_ENABLED | SVGSTATE_DISABLED);
        SVGArrowDropDown.Load(IDV_ARROW_DOWN, -1, -1, SVGSTATE_ENABLED | SVGSTATE_DISABLED);
    }

    ImageList_SetBkColor(HHotToolBarImageList, GetSysColor(COLOR_BTNFACE));
    ImageList_SetBkColor(HGrayToolBarImageList, GetSysColor(COLOR_BTNFACE));

    if (SystemParametersInfo(SPI_GETMOUSEHOVERTIME, 0, &MouseHoverTime, FALSE) == 0)
    {
        if (SystemParametersInfo(SPI_GETMENUSHOWDELAY, 0, &MouseHoverTime, FALSE) == 0)
            MouseHoverTime = 400;
    }
    //  TRACE_I("MouseHoverTime="<<MouseHoverTime);

    COLORREF normalBkgnd = GetNearestColor(dc, GetCOLORREF(CurrentColors[ITEM_BK_NORMAL]));
    COLORREF selectedBkgnd = GetNearestColor(dc, GetCOLORREF(CurrentColors[ITEM_BK_SELECTED]));
    COLORREF focusedBkgnd = GetNearestColor(dc, GetCOLORREF(CurrentColors[ITEM_BK_FOCUSED]));
    COLORREF focselBkgnd = GetNearestColor(dc, GetCOLORREF(CurrentColors[ITEM_BK_FOCSEL]));
    COLORREF activeCaption = GetNearestColor(dc, GetCOLORREF(CurrentColors[ACTIVE_CAPTION_BK]));
    COLORREF inactiveCaption = GetNearestColor(dc, GetCOLORREF(CurrentColors[INACTIVE_CAPTION_BK]));
    HANDLES(ReleaseDC(NULL, dc));

    HNormalBkBrush = HANDLES(CreateSolidBrush(normalBkgnd));
    HFocusedBkBrush = HANDLES(CreateSolidBrush(focusedBkgnd));
    HSelectedBkBrush = HANDLES(CreateSolidBrush(selectedBkgnd));
    HFocSelBkBrush = HANDLES(CreateSolidBrush(focselBkgnd));
    HActiveCaptionBrush = HANDLES(CreateSolidBrush(activeCaption));
    HInactiveCaptionBrush = HANDLES(CreateSolidBrush(inactiveCaption));

    if (HNormalBkBrush == NULL || HFocusedBkBrush == NULL ||
        HSelectedBkBrush == NULL || HFocSelBkBrush == NULL ||
        HActiveCaptionBrush == NULL || HInactiveCaptionBrush == NULL ||
        HMenuSelectedBkBrush == NULL)
    {
        TRACE_E("Unable to create brush.");
        return FALSE;
    }

    HActiveNormalPen = HANDLES(CreatePen(PS_SOLID, 0, GetCOLORREF(CurrentColors[FOCUS_ACTIVE_NORMAL])));
    HActiveSelectedPen = HANDLES(CreatePen(PS_SOLID, 0, GetCOLORREF(CurrentColors[FOCUS_ACTIVE_SELECTED])));
    HInactiveNormalPen = HANDLES(CreatePen(PS_DOT, 0, GetCOLORREF(CurrentColors[FOCUS_FG_INACTIVE_NORMAL])));
    HInactiveSelectedPen = HANDLES(CreatePen(PS_DOT, 0, GetCOLORREF(CurrentColors[FOCUS_FG_INACTIVE_SELECTED])));

    HThumbnailNormalPen = HANDLES(CreatePen(PS_SOLID, 0, GetCOLORREF(CurrentColors[THUMBNAIL_FRAME_NORMAL])));
    HThumbnailFucsedPen = HANDLES(CreatePen(PS_SOLID, 0, GetCOLORREF(CurrentColors[THUMBNAIL_FRAME_FOCUSED])));
    HThumbnailSelectedPen = HANDLES(CreatePen(PS_SOLID, 0, GetCOLORREF(CurrentColors[THUMBNAIL_FRAME_SELECTED])));
    HThumbnailFocSelPen = HANDLES(CreatePen(PS_SOLID, 0, GetCOLORREF(CurrentColors[THUMBNAIL_FRAME_FOCSEL])));

    BtnShadowPen = HANDLES(CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNSHADOW)));
    BtnHilightPen = HANDLES(CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNHILIGHT)));
    Btn3DLightPen = HANDLES(CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DLIGHT)));
    BtnFacePen = HANDLES(CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNFACE)));
    WndFramePen = HANDLES(CreatePen(PS_SOLID, 0, GetSysColor(COLOR_WINDOWFRAME)));
    WndPen = HANDLES(CreatePen(PS_SOLID, 0, GetSysColor(COLOR_WINDOW)));
    if (HActiveNormalPen == NULL || HActiveSelectedPen == NULL ||
        HInactiveNormalPen == NULL || HInactiveSelectedPen == NULL ||
        HThumbnailNormalPen == NULL || HThumbnailFucsedPen == NULL ||
        HThumbnailSelectedPen == NULL || HThumbnailFocSelPen == NULL ||
        BtnShadowPen == NULL || BtnHilightPen == NULL || BtnFacePen == NULL ||
        Btn3DLightPen == NULL || WndFramePen == NULL || WndPen == NULL)
    {
        TRACE_E("Unable to create a pen.");
        return FALSE;
    }

    COLORMAP clrMap[3];
    clrMap[0].from = RGB(255, 0, 255);
    clrMap[0].to = GetSysColor(COLOR_BTNFACE);
    clrMap[1].from = RGB(255, 255, 255);
    clrMap[1].to = GetSysColor(COLOR_BTNHILIGHT);
    clrMap[2].from = RGB(128, 128, 128);
    clrMap[2].to = GetSysColor(COLOR_BTNSHADOW);
    HHeaderSort = HANDLES(CreateMappedBitmap(HInstance, IDB_HEADER, 0, clrMap, 3));
    if (HHeaderSort == NULL)
    {
        TRACE_E("Unable to load bitmap HHeaderSort.");
        return FALSE;
    }

    clrMap[0].from = RGB(128, 128, 128); // gray -> COLOR_BTNSHADOW
    clrMap[0].to = GetSysColor(COLOR_BTNSHADOW);
    clrMap[1].from = RGB(0, 0, 0); // black -> COLOR_BTNTEXT
    clrMap[1].to = GetSysColor(COLOR_BTNTEXT);
    clrMap[2].from = RGB(255, 255, 255); // white -> transparent
    clrMap[2].to = RGB(255, 0, 255);
    HBITMAP hBottomTB = HANDLES(CreateMappedBitmap(HInstance, IDB_BOTTOMTOOLBAR, 0, clrMap, 3));
    BOOL remapWhite = FALSE;
    if (GetCurrentBPP() > 8)
    {
        clrMap[2].from = RGB(255, 255, 255); // white -> light gray (so it isn't glaring)
        clrMap[2].to = RGB(235, 235, 235);
        remapWhite = TRUE;
    }
    HBITMAP hHotBottomTB = HANDLES(CreateMappedBitmap(HInstance, IDB_BOTTOMTOOLBAR, 0, clrMap, remapWhite ? 3 : 2));
    ImageList_RemoveAll(HBottomTBImageList);
    ImageList_AddMasked(HBottomTBImageList, hBottomTB, RGB(255, 0, 255));
    ImageList_RemoveAll(HHotBottomTBImageList);
    ImageList_AddMasked(HHotBottomTBImageList, hHotBottomTB, RGB(255, 0, 255));
    HANDLES(DeleteObject(hBottomTB));
    HANDLES(DeleteObject(hHotBottomTB));
    ImageList_SetBkColor(HBottomTBImageList, GetSysColor(COLOR_BTNFACE));
    ImageList_SetBkColor(HHotBottomTBImageList, GetSysColor(COLOR_BTNFACE));
    return TRUE;
}

// ****************************************************************************

void ReleaseGraphics(BOOL colorsOnly)
{
    if (!colorsOnly)
    {
        int i;
        for (i = 0; i < ICONSIZE_COUNT; i++)
        {
            if (HSharedOverlays[i] != NULL)
            {
                HANDLES(DestroyIcon(HSharedOverlays[i]));
                HSharedOverlays[i] = NULL;
            }
            if (HShortcutOverlays[i] != NULL)
            {
                HANDLES(DestroyIcon(HShortcutOverlays[i]));
                HShortcutOverlays[i] = NULL;
            }
            if (HSlowFileOverlays[i] != NULL)
            {
                HANDLES(DestroyIcon(HSlowFileOverlays[i]));
                HSlowFileOverlays[i] = NULL;
            }
        }

        if (HGroupIcon != NULL)
        {
            HANDLES(DestroyIcon(HGroupIcon));
            HGroupIcon = NULL;
        }

        if (HFavoritIcon != NULL)
        {
            HANDLES(DestroyIcon(HFavoritIcon));
            HFavoritIcon = NULL;
        }

        if (HZoomBitmap != NULL)
        {
            HANDLES(DeleteObject(HZoomBitmap));
            HZoomBitmap = NULL;
        }

        if (HFilter != NULL)
        {
            HANDLES(DeleteObject(HFilter));
            HFilter = NULL;
        }

        if (HUpDownBitmap != NULL)
        {
            HANDLES(DeleteObject(HUpDownBitmap));
            HUpDownBitmap = NULL;
        }
    }

    if (HNormalBkBrush != NULL)
    {
        HANDLES(DeleteObject(HNormalBkBrush));
        HNormalBkBrush = NULL;
    }
    if (HFocusedBkBrush != NULL)
    {
        HANDLES(DeleteObject(HFocusedBkBrush));
        HFocusedBkBrush = NULL;
    }
    if (HSelectedBkBrush != NULL)
    {
        HANDLES(DeleteObject(HSelectedBkBrush));
        HSelectedBkBrush = NULL;
    }
    if (HFocSelBkBrush != NULL)
    {
        HANDLES(DeleteObject(HFocSelBkBrush));
        HFocSelBkBrush = NULL;
    }
    if (HActiveCaptionBrush != NULL)
    {
        HANDLES(DeleteObject(HActiveCaptionBrush));
        HActiveCaptionBrush = NULL;
    }
    if (HInactiveCaptionBrush != NULL)
    {
        HANDLES(DeleteObject(HInactiveCaptionBrush));
        HInactiveCaptionBrush = NULL;
    }
    if (HActiveNormalPen != NULL)
    {
        HANDLES(DeleteObject(HActiveNormalPen));
        HActiveNormalPen = NULL;
    }
    if (HActiveSelectedPen != NULL)
    {
        HANDLES(DeleteObject(HActiveSelectedPen));
        HActiveSelectedPen = NULL;
    }
    if (HInactiveNormalPen != NULL)
    {
        HANDLES(DeleteObject(HInactiveNormalPen));
        HInactiveNormalPen = NULL;
    }
    if (HInactiveSelectedPen != NULL)
    {
        HANDLES(DeleteObject(HInactiveSelectedPen));
        HInactiveSelectedPen = NULL;
    }
    if (HThumbnailNormalPen != NULL)
    {
        HANDLES(DeleteObject(HThumbnailNormalPen));
        HThumbnailNormalPen = NULL;
    }
    if (HThumbnailFucsedPen != NULL)
    {
        HANDLES(DeleteObject(HThumbnailFucsedPen));
        HThumbnailFucsedPen = NULL;
    }
    if (HThumbnailSelectedPen != NULL)
    {
        HANDLES(DeleteObject(HThumbnailSelectedPen));
        HThumbnailSelectedPen = NULL;
    }
    if (HThumbnailFocSelPen != NULL)
    {
        HANDLES(DeleteObject(HThumbnailFocSelPen));
        HThumbnailFocSelPen = NULL;
    }
    if (BtnShadowPen != NULL)
    {
        HANDLES(DeleteObject(BtnShadowPen));
        BtnShadowPen = NULL;
    }
    if (BtnHilightPen != NULL)
    {
        HANDLES(DeleteObject(BtnHilightPen));
        BtnHilightPen = NULL;
    }
    if (Btn3DLightPen != NULL)
    {
        HANDLES(DeleteObject(Btn3DLightPen));
        Btn3DLightPen = NULL;
    }
    if (BtnFacePen != NULL)
    {
        HANDLES(DeleteObject(BtnFacePen));
        BtnFacePen = NULL;
    }
    if (WndFramePen != NULL)
    {
        HANDLES(DeleteObject(WndFramePen));
        WndFramePen = NULL;
    }
    if (WndPen != NULL)
    {
        HANDLES(DeleteObject(WndPen));
        WndPen = NULL;
    }
    if (HHeaderSort != NULL)
    {
        HANDLES(DeleteObject(HHeaderSort));
        HHeaderSort = NULL;
    }

    if (!colorsOnly)
    {
        if (HDitherBrush != NULL)
        {
            HANDLES(DeleteObject(HDitherBrush));
            HDitherBrush = NULL;
        }
        if (HHotToolBarImageList != NULL)
        {
            ImageList_Destroy(HHotToolBarImageList);
            HHotToolBarImageList = NULL;
        }
        if (HGrayToolBarImageList != NULL)
        {
            ImageList_Destroy(HGrayToolBarImageList);
            HGrayToolBarImageList = NULL;
        }
        if (HBottomTBImageList != NULL)
        {
            ImageList_Destroy(HBottomTBImageList);
            HBottomTBImageList = NULL;
        }
        if (HHotBottomTBImageList != NULL)
        {
            ImageList_Destroy(HHotBottomTBImageList);
            HHotBottomTBImageList = NULL;
        }
        if (HMenuMarkImageList != NULL)
        {
            ImageList_Destroy(HMenuMarkImageList);
            HMenuMarkImageList = NULL;
        }
        int i;
        for (i = 0; i < ICONSIZE_COUNT; i++)
        {
            if (SimpleIconLists[i] != NULL)
            {
                delete SimpleIconLists[i];
                SimpleIconLists[i] = NULL;
            }
        }
        if (ThrobberFrames != NULL)
        {
            delete ThrobberFrames;
            ThrobberFrames = NULL;
        }
        if (LockFrames != NULL)
        {
            delete LockFrames;
            LockFrames = NULL;
        }
        if (HFindSymbolsImageList != NULL)
        {
            ImageList_Destroy(HFindSymbolsImageList);
            HFindSymbolsImageList = NULL;
        }
        if (Shell32DLL != NULL)
        {
            HANDLES(FreeLibrary(Shell32DLL));
            Shell32DLL = NULL;
        }
        if (ImageResDLL != NULL)
        {
            HANDLES(FreeLibrary(ImageResDLL));
            ImageResDLL = NULL;
        }
    }
}

// ****************************************************************************

char* NumberToStr(char* buffer, const CQuadWord& number)
{
    _ui64toa(number.Value, buffer, 10);
    int l = (int)strlen(buffer);
    char* s = buffer + l;
    int c = 0;
    while (--s > buffer)
    {
        if ((++c % 3) == 0)
        {
            memmove(s + ThousandsSeparatorLen, s, (c / 3) * 3 + (c / 3 - 1) * ThousandsSeparatorLen + 1);
            memcpy(s, ThousandsSeparator, ThousandsSeparatorLen);
        }
    }
    return buffer;
}

int NumberToStr2(char* buffer, const CQuadWord& number)
{
    _ui64toa(number.Value, buffer, 10);
    int l = (int)strlen(buffer);
    char* s = buffer + l;
    int c = 0;
    while (--s > buffer)
    {
        if ((++c % 3) == 0)
        {
            memmove(s + ThousandsSeparatorLen, s, (c / 3) * 3 + (c / 3 - 1) * ThousandsSeparatorLen + 1);
            memcpy(s, ThousandsSeparator, ThousandsSeparatorLen);
            l += ThousandsSeparatorLen;
        }
    }
    return l;
}

// ****************************************************************************

BOOL PointToLocalDecimalSeparator(char* buffer, int bufferSize)
{
    char* s = strrchr(buffer, '.');
    if (s != NULL)
    {
        int len = (int)strlen(buffer);
        if (len - 1 + DecimalSeparatorLen > bufferSize - 1)
        {
            TRACE_E("PointToLocalDecimalSeparator() small buffer!");
            return FALSE;
        }
        memmove(s + DecimalSeparatorLen, s + 1, len - (s - buffer));
        memcpy(s, DecimalSeparator, DecimalSeparatorLen);
    }
    return TRUE;
}

// ****************************************************************************
//
// GetCmdLine - obtain parameters from the command line
//
// buf + size - buffer for parameters
// argv - array of pointers that gets filled with parameters
// argCount - on input it is the size of argv, on output it holds the number of parameters
// cmdLine - command line parameters (without the .exe file name - from WinMain)

BOOL GetCmdLine(char* buf, int size, char* argv[], int& argCount, char* cmdLine)
{
    int space = argCount;
    argCount = 0;
    char* c = buf;
    char* end = buf + size;

    char* s = cmdLine;
    char term;
    while (*s != 0)
    {
        if (*s == '"') // opening quote
        {
            if (*++s == 0)
                break;
            term = '"';
        }
        else
            term = ' ';

        if (argCount < space && c < end)
            argv[argCount++] = c;
        else
            return c < end; // error only if the buffer is too small

        while (1)
        {
            if (*s == term || *s == 0)
            {
                if (*s == 0 || term != '"' || *++s != '"') // unless this is the "" -> "" escape
                {
                    if (*s != 0)
                        s++;
                    while (*s != 0 && *s == ' ')
                        s++;
                    if (c < end)
                    {
                        *c++ = 0;
                        break;
                    }
                    else
                        return FALSE;
                }
            }
            if (c < end)
                *c++ = *s++;
            else
                return FALSE;
        }
    }
    return TRUE;
}

// ****************************************************************************
//
// GetComCtlVersion
//

typedef struct _DllVersionInfo
{
    DWORD cbSize;
    DWORD dwMajorVersion; // Major version
    DWORD dwMinorVersion; // Minor version
    DWORD dwBuildNumber;  // Build number
    DWORD dwPlatformID;   // DLLVER_PLATFORM_*
} DLLVERSIONINFO;

typedef HRESULT(CALLBACK* DLLGETVERSIONPROC)(DLLVERSIONINFO*);

HRESULT GetComCtlVersion(LPDWORD pdwMajor, LPDWORD pdwMinor)
{
    HINSTANCE hComCtl;
    //load the DLL
    hComCtl = HANDLES(LoadLibrary(TEXT("comctl32.dll")));
    if (hComCtl)
    {
        HRESULT hr = S_OK;
        DLLGETVERSIONPROC pDllGetVersion;
        /*
     You must get this function explicitly because earlier versions of the DLL
     don't implement this function. That makes the lack of implementation of the
     function a version marker in itself.
    */
        pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hComCtl, TEXT("DllGetVersion")); // no header
        if (pDllGetVersion)
        {
            DLLVERSIONINFO dvi;
            ZeroMemory(&dvi, sizeof(dvi));
            dvi.cbSize = sizeof(dvi);
            hr = (*pDllGetVersion)(&dvi);
            if (SUCCEEDED(hr))
            {
                *pdwMajor = dvi.dwMajorVersion;
                *pdwMinor = dvi.dwMinorVersion;
            }
            else
            {
                hr = E_FAIL;
            }
        }
        else
        {
            /*
      If GetProcAddress failed, then the DLL is a version previous to the one
      shipped with IE 3.x.
      */
            *pdwMajor = 4;
            *pdwMinor = 0;
        }
        HANDLES(FreeLibrary(hComCtl));
        return hr;
    }
    TRACE_E("LoadLibrary on comctl32.dll failed");
    return E_FAIL;
}

// ****************************************************************************

void InitDefaultDir()
{
    char dir[4] = " :\\";
    char d;
    for (d = 'A'; d <= 'Z'; d++)
    {
        dir[0] = d;
        strcpy(DefaultDir[d - 'A'], dir);
    }
}

// ****************************************************************************

BOOL PackErrorHandler(HWND parent, const WORD err, ...)
{
    va_list argList;
    char buff[1000];
    BOOL ret = FALSE;

    parent = parent == NULL ? (MainWindow != NULL ? MainWindow->HWindow : NULL) : parent;

    va_start(argList, err);
    FormatMessage(FORMAT_MESSAGE_FROM_STRING, LoadStr(err), 0, 0, buff, 1000, &argList);
    if (err < IDS_PACKQRY_PREFIX)
        SalMessageBox(parent, buff, LoadStr(IDS_PACKERR_TITLE), MB_OK | MB_ICONEXCLAMATION);
    else
        ret = SalMessageBox(parent, buff, LoadStr(IDS_PACKERR_TITLE), MB_OKCANCEL | MB_ICONQUESTION) == IDOK;
    va_end(argList);
    return ret;
}

//
// ****************************************************************************
// ColorsChanged
//

void ColorsChanged(BOOL refresh, BOOL colorsOnly, BOOL reloadUMIcons)
{
    CALL_STACK_MESSAGE2("ColorsChanged(%d)", refresh);
    // NOTE! fonts must be FALSE to avoid changing font handles, which
    // the toolbars need to know about
    ReleaseGraphics(colorsOnly);
    InitializeGraphics(colorsOnly);
    ItemBitmap.ReCreateForScreenDC();
    UpdateViewerColors(ViewerColors);
    if (!colorsOnly)
        ShellIconOverlays.ColorsChanged();

    if (MainWindow != NULL && MainWindow->EditWindow != NULL)
        MainWindow->EditWindow->SetFont();

    Associations.ColorsChanged();

    if (MainWindow != NULL)
    {
        MainWindow->OnColorsChanged(reloadUMIcons);
    }

    // notify Find dialogs about color changes
    FindDialogQueue.BroadcastMessage(WM_USER_COLORCHANGEFIND, 0, 0);

    // distribute this news to plugins as well
    Plugins.Event(PLUGINEVENT_COLORSCHANGED, 0);

    if (MainWindow != NULL && MainWindow->HTopRebar != NULL)
        SendMessage(MainWindow->HTopRebar, RB_SETBKCOLOR, 0, (LPARAM)GetSysColor(COLOR_BTNFACE));

    if (refresh && MainWindow != NULL)
    {
        InvalidateRect(MainWindow->HWindow, NULL, TRUE);
    }
    // Internal Viewer and Find: refresh all windows
    BroadcastConfigChanged();
}

#ifdef USE_BETA_EXPIRATION_DATE

int ShowBetaExpDlg()
{
    CBetaExpiredDialog dlg(NULL);
    return (int)dlg.Execute();
}

#endif // USE_BETA_EXPIRATION_DATE

//
// ****************************************************************************

struct VS_VERSIONINFO_HEADER
{
    WORD wLength;
    WORD wValueLength;
    WORD wType;
};

BOOL GetModuleVersion(HINSTANCE hModule, WORD* major, WORD* minor)
{
    HRSRC hRes = FindResource(hModule, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION);
    if (hRes == NULL)
        return FALSE;

    HGLOBAL hVer = LoadResource(hModule, hRes);
    if (hVer == NULL)
        return FALSE;

    DWORD resSize = SizeofResource(hModule, hRes);
    const BYTE* first = (BYTE*)LockResource(hVer);
    if (resSize == 0 || first == 0)
        return FALSE;

    const BYTE* iterator = first + sizeof(VS_VERSIONINFO_HEADER);

    DWORD signature = 0xFEEF04BD;

    while (memcmp(iterator, &signature, 4) != 0)
    {
        iterator++;
        if (iterator + 4 >= first + resSize)
            return FALSE;
    }

    VS_FIXEDFILEINFO* ffi = (VS_FIXEDFILEINFO*)iterator;

    *major = HIWORD(ffi->dwFileVersionMS);
    *minor = LOWORD(ffi->dwFileVersionMS);

    return TRUE;
}

//****************************************************************************
//
// CMessagesKeeper
//

CMessagesKeeper::CMessagesKeeper()
{
    Index = 0;
    Count = 0;
}

void CMessagesKeeper::Add(const MSG* msg)
{
    Messages[Index] = *msg;
    Index = (Index + 1) % MESSAGES_KEEPER_COUNT;
    if (Count < MESSAGES_KEEPER_COUNT)
        Count++;
}

void CMessagesKeeper::Print(char* buffer, int buffMax, int index)
{
    if (buffMax <= 0)
        return;
    if (index >= Count)
    {
        _snprintf_s(buffer, buffMax, _TRUNCATE, "(error)");
    }
    else
    {
        int i;
        if (Count == MESSAGES_KEEPER_COUNT)
            i = (Index + index) % MESSAGES_KEEPER_COUNT;
        else
            i = index;
        const MSG* msg = &Messages[i];
        _snprintf_s(buffer, buffMax, _TRUNCATE, "w=0x%p m=0x%X w=0x%IX l=0x%IX t=%u p=%d,%d",
                    msg->hwnd, msg->message, msg->wParam, msg->lParam, msg->time - SalamanderExceptionTime, msg->pt.x, msg->pt.y);
    }
}

CMessagesKeeper MessagesKeeper;

typedef VOID(WINAPI* FDisableProcessWindowsGhosting)(VOID);

void TurnOFFWindowGhosting() // if "ghosting" is not disabled, safe-wait windows hide after five seconds of "not responding" when the app does not process messages
{
    if (User32DLL != NULL)
    {
        FDisableProcessWindowsGhosting disableProcessWindowsGhosting = (FDisableProcessWindowsGhosting)GetProcAddress(User32DLL, "DisableProcessWindowsGhosting"); // Min: XP
        if (disableProcessWindowsGhosting != NULL)
            disableProcessWindowsGhosting();
    }
}

//
// ****************************************************************************

void UIDToString(GUID* uid, char* buff, int buffSize)
{
    wchar_t buffw[64] = {0};
    StringFromGUID2(*uid, buffw, 64);
    WideCharToMultiByte(CP_ACP, 0, buffw, -1, buff, buffSize, NULL, NULL);
    buff[buffSize - 1] = 0;
}

void StringToUID(char* buff, GUID* uid)
{
    wchar_t buffw[64] = {0};
    MultiByteToWideChar(CP_ACP, 0, buff, -1, buffw, 64);
    buffw[63] = 0;
    CLSIDFromString(buffw, uid);
}

void CleanUID(char* uid)
{
    char* s = uid;
    char* d = uid;
    while (*s != 0)
    {
        while (*s == '{' || *s == '}' || *s == '-')
            s++;
        *d++ = *s++;
    }
}

//
// ****************************************************************************

//#ifdef MSVC_RUNTIME_CHECKS
char RTCErrorDescription[RTC_ERROR_DESCRIPTION_SIZE] = {0};
// custom reporting function copied from MSDN - http://msdn.microsoft.com/en-us/library/cb00sk7k(v=VS.90).aspx
#pragma runtime_checks("", off)
int MyRTCErrorFunc(int errType, const wchar_t* file, int line,
                   const wchar_t* module, const wchar_t* format, ...)
{
    // Prevent re-entrance.
    static long running = 0;
    while (InterlockedExchange(&running, 1))
        Sleep(0);
    // Now, disable all RTC failures.
    int numErrors = _RTC_NumErrors();
    int* errors = (int*)_alloca(numErrors);
    for (int i = 0; i < numErrors; i++)
        errors[i] = _RTC_SetErrorType((_RTC_ErrorNumber)i, _RTC_ERRTYPE_IGNORE);

    // First, get the rtc error number from the var-arg list.
    va_list vl;
    va_start(vl, format);
    _RTC_ErrorNumber rtc_errnum = va_arg(vl, _RTC_ErrorNumber);
    va_end(vl);

    static wchar_t buf[RTC_ERROR_DESCRIPTION_SIZE];
    static char bufA[RTC_ERROR_DESCRIPTION_SIZE];
    const char* err = _RTC_GetErrDesc(rtc_errnum);
    _snwprintf_s(buf, _TRUNCATE, L"  Error Number: %d\r\n  Description: %S\r\n  Line: #%d\r\n  File: %s\r\n  Module: %s\r\n",
                 rtc_errnum,
                 err,
                 line,
                 file ? file : L"Unknown",
                 module ? module : L"Unknown");

    WideCharToMultiByte(CP_ACP, 0, buf, -1, bufA, RTC_ERROR_DESCRIPTION_SIZE, NULL, NULL);
    bufA[RTC_ERROR_DESCRIPTION_SIZE - 1] = 0;
    lstrcpyn(RTCErrorDescription, bufA, RTC_ERROR_DESCRIPTION_SIZE);

    // we wrap this with an exception so the call stack is clearer; if it is not, we can remove this exception
    // see the _CrtDbgReportW behavior description - http://msdn.microsoft.com/en-us/library/8hyw4sy7(v=VS.90).aspx
    RaiseException(OPENSAL_EXCEPTION_RTC, 0, 0, NULL); // our own "rtc" exception

    // execution never reaches here; the process was terminated, the following continues only for completeness

    // Now, restore the RTC errortypes.
    for (int i = 0; i < numErrors; i++)
        _RTC_SetErrorType((_RTC_ErrorNumber)i, errors[i]);
    running = 0;

    return -1;
}
#pragma runtime_checks("", restore)
//#endif // MSVC_RUNTIME_CHECKS

//
// ****************************************************************************

#ifdef _DEBUG

DWORD LastCrtCheckMemoryTime; // when we last checked memory while idle

#endif //_DEBUG

STDAPI _StrRetToBuf(STRRET* psr, LPCITEMIDLIST pidl, LPSTR pszBuf, UINT cchBuf);

BOOL FindPluginsWithoutImportedCfg(BOOL* doNotDeleteImportedCfg)
{
    char names[1000];
    int skipped;
    Plugins.RemoveNoLongerExistingPlugins(FALSE, TRUE, names, 1000, 10, &skipped, MainWindow->HWindow);
    if (names[0] != 0)
    {
        *doNotDeleteImportedCfg = TRUE;
        MSGBOXEX_PARAMS params;
        memset(&params, 0, sizeof(params));
        params.HParent = MainWindow->HWindow;
        params.Flags = MB_OKCANCEL | MB_ICONQUESTION;
        params.Caption = SALAMANDER_TEXT_VERSION;
        char skippedNames[200];
        skippedNames[0] = 0;
        if (skipped > 0)
            sprintf(skippedNames, LoadStr(IDS_NUMOFSKIPPEDPLUGINNAMES), skipped);
        char msg[2000];
        sprintf(msg, LoadStr(IDS_NOTALLPLUGINSCFGIMPORTED), names, skippedNames);
        params.Text = msg;
        char aliasBtnNames[200];
        /* used by the export_mnu.py script which generates salmenu.mnu for Translator
   we let the message box buttons resolve hotkey collisions by simulating a menu
MENU_TEMPLATE_ITEM MsgBoxButtons[] = 
{
  {MNTT_PB, 0
  {MNTT_IT, IDS_STARTWITHOUTMISSINGPLUGINS
  {MNTT_IT, IDS_SELLANGEXITBUTTON
  {MNTT_PE, 0
};
*/
        sprintf(aliasBtnNames, "%d\t%s\t%d\t%s",
                DIALOG_OK, LoadStr(IDS_STARTWITHOUTMISSINGPLUGINS),
                DIALOG_CANCEL, LoadStr(IDS_SELLANGEXITBUTTON));
        params.AliasBtnNames = aliasBtnNames;
        return SalMessageBoxEx(&params) == IDCANCEL;
    }
    return FALSE;
}

void StartNotepad(const char* file)
{
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi;
    char buf[MAX_PATH];
    char buf2[MAX_PATH + 50];

    if (lstrlen(file) >= MAX_PATH)
        return;

    GetSystemDirectory(buf, MAX_PATH); // give it the system directory so deleting the current working directory is not blocked
    wsprintf(buf2, "notepad.exe \"%s\"", file);
    si.cb = sizeof(STARTUPINFO);
    if (HANDLES(CreateProcess(NULL, buf2, NULL, NULL, TRUE, CREATE_DEFAULT_ERROR_MODE | NORMAL_PRIORITY_CLASS,
                              NULL, buf, &si, &pi)))
    {
        HANDLES(CloseHandle(pi.hProcess));
        HANDLES(CloseHandle(pi.hThread));
    }
}

BOOL RunningInCompatibilityMode()
{
    // When running on XP or later the user might have enabled Compatibility Mode.
    // If so we display a warning.
    // NOTE: Application Verifier reports a newer Windows version than the real one
    // when testing the application for the "Windows 7 Software Logo".
    WORD kernel32major, kernel32minor;
    if (GetModuleVersion(GetModuleHandle("kernel32.dll"), &kernel32major, &kernel32minor))
    {
        TRACE_I("kernel32.dll: " << kernel32major << ":" << kernel32minor);
        // we must call GetVersionEx because it reports values according to the selected Compatibility Mode
        // (SalIsWindowsVersionOrGreater ignores Compatibility Mode)
        OSVERSIONINFO os;
        os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

        // just avoiding the deprecated warning; GetVersionEx should always exist
        typedef BOOL(WINAPI * FDynGetVersionExA)(LPOSVERSIONINFOA lpVersionInformation);
        FDynGetVersionExA DynGetVersionExA = (FDynGetVersionExA)GetProcAddress(GetModuleHandle("kernel32.dll"),
                                                                               "GetVersionExA");
        if (DynGetVersionExA == NULL)
        {
            TRACE_E("RunningInCompatibilityMode(): unable to get address of GetVersionEx()");
            return FALSE;
        }

        DynGetVersionExA(&os);
        TRACE_I("GetVersionEx(): " << os.dwMajorVersion << ":" << os.dwMinorVersion);

        // the current version of Salamander is manifested for Windows 10
        const DWORD SAL_MANIFESTED_FOR_MAJOR = 10;
        const DWORD SAL_MANIFESTED_FOR_MINOR = 0;

        // GetVersionEx never returns more than os.dwMajorVersion == SAL_MANIFESTED_FOR_MAJOR
        // and os.dwMinorVersion == SAL_MANIFESTED_FOR_MINOR; if kernel32.dll is newer
        // we cannot fully detect Compatibility Mode. Salamander must be manifested
        // for newer Windows and the constants SAL_MANIFESTED_FOR_MAJOR and
        // SAL_MANIFESTED_FOR_MINOR updated - at least detect Compatibility Mode
        // for older Windows than Salamander is manifested for
        if (kernel32major > SAL_MANIFESTED_FOR_MAJOR ||
            kernel32major == SAL_MANIFESTED_FOR_MAJOR && kernel32minor > SAL_MANIFESTED_FOR_MINOR)
        {
            kernel32major = SAL_MANIFESTED_FOR_MAJOR;
            kernel32minor = SAL_MANIFESTED_FOR_MINOR;
            TRACE_I("kernel32.dll version was limited by Salamander's manifest to: " << kernel32major << ":" << kernel32minor);
        }
        if (kernel32major > os.dwMajorVersion || kernel32major == os.dwMajorVersion && kernel32minor > os.dwMinorVersion)
            return TRUE;
    }
    return FALSE;
}

void GetCommandLineParamExpandEnvVars(const char* argv, char* target, DWORD targetSize, BOOL hotpathForJumplist)
{
    char curDir[MAX_PATH];
    if (hotpathForJumplist)
    {
        BOOL ret = ExpandHotPath(NULL, argv, target, targetSize, FALSE); // if the path syntax is invalid, TRACE_E is triggered, which we do not care about
        if (!ret)
        {
            TRACE_E("ExpandHotPath failed.");
            // if expansion fails we use the string without expansion
            lstrcpyn(target, argv, targetSize);
        }
    }
    else
    {
        DWORD auxRes = ExpandEnvironmentStrings(argv, target, targetSize); // users wanted to pass environment variables as parameters
        if (auxRes == 0 || auxRes > targetSize)
        {
            TRACE_E("ExpandEnvironmentStrings failed.");
            // if expansion fails we use the string without expansion
            lstrcpyn(target, argv, targetSize);
        }
    }
    if (!IsPluginFSPath(target) && GetCurrentDirectory(MAX_PATH, curDir))
    {
        SalGetFullName(target, NULL, curDir, NULL, NULL, targetSize);
    }
}

// returns TRUE if parameters are valid, otherwise FALSE
BOOL ParseCommandLineParameters(LPSTR cmdLine, CCommandLineParams* cmdLineParams)
{
    // we do not want to change paths, icon, or prefix - everything needs to be cleared
    ZeroMemory(cmdLineParams, sizeof(CCommandLineParams));

    char buf[4096];
    char* argv[20];
    int p = 20; // number of elements in argv

    char curDir[MAX_PATH];
    GetModuleFileName(HInstance, ConfigurationName, MAX_PATH);
    *(strrchr(ConfigurationName, '\\') + 1) = 0;
    const char* configReg = "config.reg";
    strcat(ConfigurationName, configReg);
    if (!FileExists(ConfigurationName) && GetOurPathInRoamingAPPDATA(curDir) &&
        SalPathAppend(curDir, configReg, MAX_PATH) && FileExists(curDir))
    { // if config.reg is not next to the .exe, try APPDATA as well
        lstrcpyn(ConfigurationName, curDir, MAX_PATH);
        ConfigurationNameIgnoreIfNotExists = FALSE;
    }
    OpenReadmeInNotepad[0] = 0;
    if (GetCmdLine(buf, _countof(buf), argv, p, cmdLine))
    {
        int i;
        for (i = 0; i < p; i++)
        {
            if (StrICmp(argv[i], "-l") == 0) // left panel path
            {
                if (i + 1 < p)
                {
                    GetCommandLineParamExpandEnvVars(argv[i + 1], cmdLineParams->LeftPath, 2 * MAX_PATH, FALSE);
                    i++;
                    continue;
                }
            }

            if (StrICmp(argv[i], "-r") == 0) // right panel path
            {
                if (i + 1 < p)
                {
                    GetCommandLineParamExpandEnvVars(argv[i + 1], cmdLineParams->RightPath, 2 * MAX_PATH, FALSE);
                    i++;
                    continue;
                }
            }

            if (StrICmp(argv[i], "-a") == 0) // active panel path
            {
                if (i + 1 < p)
                {
                    GetCommandLineParamExpandEnvVars(argv[i + 1], cmdLineParams->ActivePath, 2 * MAX_PATH, FALSE);
                    i++;
                    continue;
                }
            }

            if (StrICmp(argv[i], "-aj") == 0) // active panel path (hot paths syntax for jumplist) - internal, undocumented
            {
                if (i + 1 < p)
                {
                    GetCommandLineParamExpandEnvVars(argv[i + 1], cmdLineParams->ActivePath, 2 * MAX_PATH, TRUE);
                    i++;
                    continue;
                }
            }

            if (StrICmp(argv[i], "-c") == 0) // default config file
            {
                if (i + 1 < p)
                {
                    char* s = argv[i + 1];
                    if (*s == '\\' && *(s + 1) == '\\' || // UNC full path
                        *s != 0 && *(s + 1) == ':')       // "c:\" full path
                    {                                     // absolute name
                        lstrcpyn(ConfigurationName, argv[i + 1], MAX_PATH);
                    }
                    else // relative name
                    {
                        GetModuleFileName(HInstance, ConfigurationName, MAX_PATH);
                        *(strrchr(ConfigurationName, '\\') + 1) = 0;
                        SalPathAppend(ConfigurationName, s, MAX_PATH);
                        if (!FileExists(ConfigurationName) && GetOurPathInRoamingAPPDATA(curDir) &&
                            SalPathAppend(curDir, s, MAX_PATH) && FileExists(curDir))
                        { // if the relative file for -C next to the .exe does not exist, also look in APPDATA
                            lstrcpyn(ConfigurationName, curDir, MAX_PATH);
                        }
                    }
                    ConfigurationNameIgnoreIfNotExists = FALSE;
                    i++;
                    continue;
                }
            }

            if (StrICmp(argv[i], "-i") == 0) // icon index
            {
                if (i + 1 < p)
                {
                    char* s = argv[i + 1];
                    if ((*s == '0' || *s == '1' || *s == '2' || *s == '3') && *(s + 1) == 0) // 0, 1, 2, 3
                    {
                        Configuration.MainWindowIconIndexForced = (*s - '0');

                        cmdLineParams->SetMainWindowIconIndex = TRUE;
                        cmdLineParams->MainWindowIconIndex = Configuration.MainWindowIconIndexForced;
                    }
                    i++;
                    continue;
                }
            }

            if (StrICmp(argv[i], "-t") == 0) // title prefix
            {
                if (i + 1 < p)
                {
                    Configuration.UseTitleBarPrefixForced = TRUE;
                    char* s = argv[i + 1];
                    if (*s != 0)
                    {
                        lstrcpyn(Configuration.TitleBarPrefixForced, s, TITLE_PREFIX_MAX);

                        cmdLineParams->SetTitlePrefix = TRUE;
                        lstrcpyn(cmdLineParams->TitlePrefix, s, MAX_PATH);
                    }
                    i++;
                    continue;
                }
            }

            if (StrICmp(argv[i], "-o") == 0) // behave as if OnlyOneInstance was enabled
            {
                Configuration.ForceOnlyOneInstance = TRUE;
                continue;
            }

            if (StrICmp(argv[i], "-p") == 0) // activate panel
            {
                if (i + 1 < p)
                {
                    char* s = argv[i + 1];
                    if ((*s == '0' || *s == '1' || *s == '2') && *(s + 1) == 0) // 0, 1, 2
                    {
                        cmdLineParams->ActivatePanel = (*s - '0');
                    }
                    i++;
                    continue;
                }
            }

            if (StrICmp(argv[i], "-run_notepad") == 0 && i + 1 < p)
            { // Vista+: after installation: installer (SFX7ZIP) executes Salamander and asks for execution of notepad with readme file
                lstrcpyn(OpenReadmeInNotepad, argv[i + 1], MAX_PATH);
                i++;
                continue;
            }

            return FALSE; // wrong parameters
        }
    }
    return TRUE;
}

int WinMainBody(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR cmdLine, int cmdShow)
{
    int myExitCode = 1;

    //--- suppress critical errors such as "no disk in drive A:"
    SetErrorMode(SetErrorMode(0) | SEM_FAILCRITICALERRORS);

    // seed the random number generator
    srand((unsigned)time(NULL) ^ (unsigned)_getpid());

#ifdef _DEBUG
    // #define _CRTDBG_ALLOC_MEM_DF        0x01  /* Turn on debug allocation */ (DEFAULT ON)
    // #define _CRTDBG_DELAY_FREE_MEM_DF   0x02  /* Don't actually free memory */
    // #define _CRTDBG_CHECK_ALWAYS_DF     0x04  /* Check heap every alloc/dealloc */
    // #define _CRTDBG_RESERVED_DF         0x08  /* Reserved - do not use */
    // #define _CRTDBG_CHECK_CRT_DF        0x10  /* Leak check/diff CRT blocks */
    // #define _CRTDBG_LEAK_CHECK_DF       0x20  /* Leak check at program exit */

    // if memory corruption is suspected uncomment the following two lines
    // Salamander will slow down and every free/alloc will verify heap consistency
    // int crtDbg = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);   // Get the current bits
    // _CrtSetDbgFlag(crtDbg | _CRTDBG_CHECK_ALWAYS_DF);
    // _CrtSetDbgFlag(crtDbg | _CRTDBG_LEAK_CHECK_DF);

    // another useful debugging feature: when a memory leak occurs the number in braces
    // indicates the allocation order, e.g. _CRT_WARN: {104200};
    // the function _CrtSetBreakAlloc allows breaking on that block
    // _CrtSetBreakAlloc(33521);

    LastCrtCheckMemoryTime = GetTickCount();

    // in this case of memory overrun the protection triggers: a message box appears in IDLE
    // and debug messages are sent to the TraceServer
    //
//  char *p1 = (char*)malloc( 4 );
//  strcpy( p1 , "Oops" );
#endif //_DEBUG

    /*
    // test "Heap Block Corruptions: Full-page heap", see http://support.microsoft.com/kb/286470
    // allocates all blocks (at least 16 bytes) so an inaccessible page follows them,
    // thus any overrun triggers an exception
    // install Debugging Tools for Windows and in gflags.exe choose "Enable page heap" for "salamand.exe",
    // worked for me on W2K and XP (should work on Vista too)
    // OR use the prepared sal-pageheap-register.reg and sal-pageheap-unregister.reg (no need for Debugging Tools)
    //
    // December/2011: tested under VS2008 + page heap + Win7x64 and the following overwrite did not trigger an exception
    // I found a description of allocation in this mode: http://msdn.microsoft.com/en-us/library/ms220938(v=VS.90).aspx

  char *test = (char *)malloc(16);
//  char *test = (char *)HeapAlloc(GetProcessHeap(), 0, 16);
  char bufff[100];
  sprintf(bufff, "test=%p", test);
  MessageBox(NULL, bufff, "a", MB_OK);
  test[16] = 0;
*/

    char testCharValue = 129;
    int testChar = testCharValue;
    if (testChar != 129) // if testChar were negative, LowerCase[testCharValue] would be out of bounds...
    {
        MessageBox(NULL, "Default type 'char' is not 'unsigned char', but 'signed char'. See '/J' compiler switch in MSVC.",
                   "Compilation Error", MB_OK | MB_ICONSTOP);
    }

    MainThreadID = GetCurrentThreadId();
    HInstance = hInstance;
    CALL_STACK_MESSAGE4("WinMainBody(0x%p, , %s, %d)", hInstance, cmdLine, cmdShow);

    // Well, it's not our fault... since the competition does it we must too - inspired by Explorer.
    // And I wondered why their Paint works so nicely.
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

    SetTraceProcessName("Salamander");
    SetThreadNameInVCAndTrace("Main");
    SetMessagesTitle(MAINWINDOW_NAME);
    TRACE_I("Begin");

    // OLE initialization
    if (FAILED(OleInitialize(NULL)))
    {
        TRACE_E("Error in CoInitialize.");
        return 1;
    }

    //  HOldWPHookProc = SetWindowsHookEx(WH_CALLWNDPROC,     // HANDLES cannot handle this!
    //                                    WPMessageHookProc,
    //                                    NULL, GetCurrentThreadId());

    User32DLL = NOHANDLES(LoadLibrary("user32.dll"));
    if (User32DLL == NULL)
        TRACE_E("Unable to load library user32.dll."); // not a fatal error

    TurnOFFWindowGhosting();

    NtDLL = HANDLES(LoadLibrary("NTDLL.DLL"));
    if (NtDLL == NULL)
        TRACE_E("Unable to load library ntdll.dll."); // not a fatal error

    // detection of the default user charset for fonts
    CHARSETINFO ci;
    memset(&ci, 0, sizeof(ci));
    char bufANSI[10];
    if (GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDEFAULTANSICODEPAGE, bufANSI, 10))
    {
        if (TranslateCharsetInfo((DWORD*)(DWORD_PTR)MAKELONG(atoi(bufANSI), 0), &ci, TCI_SRCCODEPAGE))
        {
            UserCharset = ci.ciCharset;
        }
    }

    // due to memory-mapped files we need to obtain the allocation granularity
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    AllocationGranularity = si.dwAllocationGranularity;

    // Windows Versions supported by Open Salamander
    //
    // Name               wMajorVersion  wMinorVersion
    //------------------------------------------------
    // Windows XP         5              1
    // Windows XP x64     5              2
    // Windows Vista      6              0
    // Windows 7          6              1
    // Windows 8          6              2
    // Windows 8.1        6              3
    // Windows 10         10             0             (note: preview versions of W10 from 2014 returned version 6.4)

    if (!SalIsWindowsVersionOrGreater(6, 1, 0))
    {
        // this should not happen; on older systems missing exports of statically linked
        // libraries would show an obscure message from the Windows PE loader
        // do not call SalMessageBox
        MessageBox(NULL, "You need at least Windows 7 to run this program.",
                   SALAMANDER_TEXT_VERSION, MB_OK | MB_ICONEXCLAMATION);
    EXIT_1:
        if (User32DLL != NULL)
        {
            NOHANDLES(FreeLibrary(User32DLL));
            User32DLL = NULL;
        }
        if (NtDLL != NULL)
        {
            HANDLES(FreeLibrary(NtDLL));
            NtDLL = NULL;
        }
        return myExitCode;
    }

    WindowsVistaAndLater = SalIsWindowsVersionOrGreater(6, 0, 0);
    WindowsXP64AndLater = SalIsWindowsVersionOrGreater(5, 2, 0);
    Windows7AndLater = SalIsWindowsVersionOrGreater(6, 1, 0);
    Windows8AndLater = SalIsWindowsVersionOrGreater(6, 2, 0);
    Windows8_1AndLater = SalIsWindowsVersionOrGreater(6, 3, 0);
    Windows10AndLater = SalIsWindowsVersionOrGreater(10, 0, 0);

    DWORD integrityLevel;
    if (GetProcessIntegrityLevel(&integrityLevel) && integrityLevel >= SECURITY_MANDATORY_HIGH_RID)
        RunningAsAdmin = TRUE;

    // if possible use GetNativeSystemInfo, otherwise keep the result of GetSystemInfo
    typedef void(WINAPI * PGNSI)(LPSYSTEM_INFO);
    PGNSI pGNSI = (PGNSI)GetProcAddress(GetModuleHandle("kernel32.dll"), "GetNativeSystemInfo"); // Min: XP
    if (pGNSI != NULL)
        pGNSI(&si);
    Windows64Bit = si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64;

    if (!GetWindowsDirectory(WindowsDirectory, MAX_PATH))
        WindowsDirectory[0] = 0;

    // we need the ITaskbarList3 interface introduced by Microsoft in Windows 7, for example to show progress in taskbar buttons
    if (Windows7AndLater)
    {
        TaskbarBtnCreatedMsg = RegisterWindowMessage("TaskbarButtonCreated");
        if (TaskbarBtnCreatedMsg == 0)
        {
            DWORD err = GetLastError();
            TRACE_E("RegisterWindowMessage() failed for 'TaskbarButtonCreated'. Error:" << err);
        }
    }

    // with the global variables set we can initialize this mutex
    if (!TaskList.Init())
        TRACE_E("TaskList.Init() failed!");

    if (!InitializeWinLib())
        goto EXIT_1; // we must initialize WinLib before the first display
                     // wait dialog (the window classes must be registered)
                     // ImportConfiguration may already open this dialog

    LoadSaveToRegistryMutex.Init();

    // try to retrieve "AutoImportConfig" from the current configuration -> present when performing an UPGRADE
    BOOL autoImportConfig = FALSE;
    char autoImportConfigFromKey[200];
    autoImportConfigFromKey[0] = 0;
    if (!GetUpgradeInfo(&autoImportConfig, autoImportConfigFromKey, 200)) // user requests program exit
    {
        myExitCode = 0;
    EXIT_1a:
        ReleaseWinLib();
        goto EXIT_1;
    }
    const char* configKey = autoImportConfig ? autoImportConfigFromKey : SalamanderConfigurationRoots[0];

    // try to read from the current configuration the key specifying the language
    LoadSaveToRegistryMutex.Enter();
    HKEY hSalamander;
    DWORD langChanged = FALSE; // TRUE when starting Salamander for the first time with a different language (load all plugins to verify we have this language version and let the user choose fallback versions if needed)
    if (OpenKey(HKEY_CURRENT_USER, configKey, hSalamander))
    {
        HKEY actKey;
        DWORD configVersion = 1; // this configuration is from 1.52 and older
        if (OpenKey(hSalamander, SALAMANDER_VERSION_REG, actKey))
        {
            configVersion = 2; // this configuration is from 1.6b1
            GetValue(actKey, SALAMANDER_VERSIONREG_REG, REG_DWORD,
                     &configVersion, sizeof(DWORD));
            CloseKey(actKey);
        }
        if (configVersion >= 59 /* 2.53 beta 2 */ && // before 2.53 beta 2 there was only English so reading makes no sense; offer user's default language or manual selection
            OpenKey(hSalamander, SALAMANDER_CONFIG_REG, actKey))
        {
            GetValue(actKey, CONFIG_LANGUAGE_REG, REG_SZ,
                     Configuration.SLGName, MAX_PATH);
            GetValue(actKey, CONFIG_USEALTLANGFORPLUGINS_REG, REG_DWORD,
                     &Configuration.UseAsAltSLGInOtherPlugins, sizeof(DWORD));
            GetValue(actKey, CONFIG_ALTLANGFORPLUGINS_REG, REG_SZ,
                     Configuration.AltPluginSLGName, MAX_PATH);
            GetValue(actKey, CONFIG_LANGUAGECHANGED_REG, REG_DWORD, &langChanged, sizeof(DWORD));
            CloseKey(actKey);
        }
        CloseKey(hSalamander);
    }
    LoadSaveToRegistryMutex.Leave();

FIND_NEW_SLG_FILE:

    // if the key does not exist show the selection dialog
    BOOL newSLGFile = FALSE; // TRUE if an .SLG file was chosen during this start of Salamander
    if (Configuration.SLGName[0] == 0)
    {
        CLanguageSelectorDialog slgDialog(NULL, Configuration.SLGName, NULL);
        slgDialog.Initialize();
        if (slgDialog.GetLanguagesCount() == 0)
        {
            MessageBox(NULL, "Unable to find any language file (.SLG) in subdirectory LANG.\n"
                             "Please reinstall Open Salamander.",
                       SALAMANDER_TEXT_VERSION, MB_OK | MB_ICONERROR);
            goto EXIT_1a;
        }
        Configuration.UseAsAltSLGInOtherPlugins = FALSE;
        Configuration.AltPluginSLGName[0] = 0;

        char prevVerSLGName[MAX_PATH];
        if (!autoImportConfig &&                            // during UPGRADE this is pointless (the language was read a few lines above and this routine would just read it again)
            FindLanguageFromPrevVerOfSal(prevVerSLGName) && // import language from the previous version; likely the user wants to use it again (importing old Salamander configuration)
            slgDialog.SLGNameExists(prevVerSLGName))
        {
            lstrcpy(Configuration.SLGName, prevVerSLGName);
        }
        else
        {
            int langIndex = slgDialog.GetPreferredLanguageIndex(NULL, TRUE);
            if (langIndex == -1) // this installation lacks a language matching the current user locale in Windows
            {

// if this is commented out we will not send people to download language versions from the web (e.g. when none are available)
// JRY: for AS 2.53, which ships with Czech, German and English, we direct other languages to the
//      "Translations" forum https://forum.altap.cz/viewforum.php?f=23 in hopes someone will create a translation
#define OFFER_OTHERLANGUAGE_VERSIONS

#ifndef OFFER_OTHERLANGUAGE_VERSIONS
                if (slgDialog.GetLanguagesCount() == 1)
                    slgDialog.GetSLGName(Configuration.SLGName); // if there is only one language, use it
                else
                {
#endif // OFFER_OTHERLANGUAGE_VERSIONS

                    // open the language selection dialog so the user can download and install more languages
                    if (slgDialog.Execute() == IDCANCEL)
                        goto EXIT_1a;

#ifndef OFFER_OTHERLANGUAGE_VERSIONS
                }
#endif // OFFER_OTHERLANGUAGE_VERSIONS
            }
            else
            {
                slgDialog.GetSLGName(Configuration.SLGName, langIndex); // use it if a language matching the current user locale exists
            }
        }
        newSLGFile = TRUE;
        langChanged = TRUE;
    }

    char path[MAX_PATH];
    char errorText[MAX_PATH + 200];
    GetModuleFileName(NULL, path, MAX_PATH);
    sprintf(strrchr(path, '\\') + 1, "lang\\%s", Configuration.SLGName);
    HLanguage = HANDLES(LoadLibrary(path));
    LanguageID = 0;
    if (HLanguage == NULL || !IsSLGFileValid(HInstance, HLanguage, LanguageID, IsSLGIncomplete))
    {
        if (HLanguage != NULL)
            HANDLES(FreeLibrary(HLanguage));
        if (!newSLGFile) // the remembered .SLG file probably no longer exists, trying to find another one
        {
            sprintf(errorText, "File %s was not found or is not valid language file.\nOpen Salamander "
                               "will try to search for some other language file (.SLG).",
                    path);
            MessageBox(NULL, errorText, SALAMANDER_TEXT_VERSION, MB_OK | MB_ICONERROR);
            Configuration.SLGName[0] = 0;
            goto FIND_NEW_SLG_FILE;
        }
        else // should never happen - the .SLG file has already been tested
        {
            sprintf(errorText, "File %s was not found or is not valid language file.\n"
                               "Please run Open Salamander again and try to choose some other language file.",
                    path);
            MessageBox(NULL, errorText, "Open Salamander", MB_OK | MB_ICONERROR);
            goto EXIT_1a;
        }
    }

    strcpy(Configuration.LoadedSLGName, Configuration.SLGName);

    // let the already running Salmon load the chosen SLG (it used some provisional one so far)
    SalmonSetSLG(Configuration.SLGName);

    // set localized messages for the ALLOCHAN module (reports low memory to the user, offers Retry, and if all fails also Cancel to terminate the program)
    SetAllocHandlerMessage(LoadStr(IDS_ALLOCHANDLER_MSG), SALAMANDER_TEXT_VERSION,
                           LoadStr(IDS_ALLOCHANDLER_WRNIGNORE), LoadStr(IDS_ALLOCHANDLER_WRNABORT));

    CCommandLineParams cmdLineParams;
    if (!ParseCommandLineParameters(cmdLine, &cmdLineParams))
    {
        SalMessageBox(NULL, LoadStr(IDS_INVALIDCMDLINE), SALAMANDER_TEXT_VERSION, MB_OK | MB_ICONERROR);

    EXIT_2:
        if (HLanguage != NULL)
            HANDLES(FreeLibrary(HLanguage));
        goto EXIT_1a;
    }

    if (RunningInCompatibilityMode())
    {
        CCommonDialog dlg(HLanguage, IDD_COMPATIBILITY_MODE, NULL);
        if (dlg.Execute() == IDCANCEL)
            goto EXIT_2;
    }

#ifdef USE_BETA_EXPIRATION_DATE
    // this beta version is time-limited, see BETA_EXPIRATION_DATE
    // if today is on or after that date we show a dialog and quit
    SYSTEMTIME st;
    GetLocalTime(&st);
    SYSTEMTIME* expire = &BETA_EXPIRATION_DATE;
    if (st.wYear > expire->wYear ||
        (st.wYear == expire->wYear && st.wMonth > expire->wMonth) ||
        (st.wYear == expire->wYear && st.wMonth == expire->wMonth && st.wDay >= expire->wDay))
    {
        if (ShowBetaExpDlg() == IDCANCEL)
            goto EXIT_2;
    }
#endif // USE_BETA_EXPIRATION_DATE

    // open the splash screen

    GetSystemDPI(NULL);

    // if the configuration does not exist or is changed later during import from a file,
    // the user is out of luck and the splash screen will follow the default or old value
    LoadSaveToRegistryMutex.Enter();
    if (OpenKey(HKEY_CURRENT_USER, configKey, hSalamander))
    {
        HKEY actKey;
        if (OpenKey(hSalamander, SALAMANDER_CONFIG_REG, actKey))
        {
            GetValue(actKey, CONFIG_SHOWSPLASHSCREEN_REG, REG_DWORD,
                     &Configuration.ShowSplashScreen, sizeof(DWORD));
            CloseKey(actKey);
        }
        CloseKey(hSalamander);
    }
    LoadSaveToRegistryMutex.Leave();

    if (Configuration.ShowSplashScreen)
        SplashScreenOpen();

    // the configuration import window uses a listview with check boxes; we must initialize COMMON CONTROLS
    INITCOMMONCONTROLSEX initCtrls;
    initCtrls.dwSize = sizeof(INITCOMMONCONTROLSEX);
    initCtrls.dwICC = ICC_BAR_CLASSES | ICC_LISTVIEW_CLASSES |
                      ICC_TAB_CLASSES | ICC_COOL_CLASSES |
                      ICC_DATE_CLASSES | ICC_USEREX_CLASSES;
    if (!InitCommonControlsEx(&initCtrls))
    {
        TRACE_E("InitCommonControlsEx failed");
        SplashScreenCloseIfExist();
        goto EXIT_2;
    }

    SetWinLibStrings(LoadStr(IDS_INVALIDNUMBER), MAINWINDOW_NAME); // j.r. - move to the proper place

    // packer initialization; previously done in constructors, now moved here
    // when the language DLL is already determined
    PackerFormatConfig.InitializeDefaultValues();
    ArchiverConfig.InitializeDefaultValues();
    PackerConfig.InitializeDefaultValues();
    UnpackerConfig.InitializeDefaultValues();

    // if the file exists it will be imported into the registry
    BOOL importCfgFromFileWasSkipped = FALSE;
    ImportConfiguration(NULL, ConfigurationName, ConfigurationNameIgnoreIfNotExists, autoImportConfig,
                        &importCfgFromFileWasSkipped);

    // handle the transition from the old config to the new one

    // Call a function that tries to find a configuration matching our program version.
    // If found it sets 'loadConfiguration' and returns TRUE. If the configuration does not yet exist,
    // the function searches the old configurations in 'SalamanderConfigurationRoots' from newest to oldest.
    // When one is found it shows a dialog offering to convert it to the current configuration and delete it from the registry.
    // After the last dialog it returns TRUE and sets 'deleteConfigurations' and 'loadConfiguration' according to user choices.
    // If the user chooses to exit the application the function returns FALSE.

    // array holding indexes of configurations in 'SalamanderConfigurationRoots'
    // that should be deleted (0 -> none)
    BOOL deleteConfigurations[SALCFG_ROOTS_COUNT];
    ZeroMemory(deleteConfigurations, sizeof(deleteConfigurations));

    CALL_STACK_MESSAGE1("WinMainBody::FindLatestConfiguration");

    // pointer into the 'SalamanderConfigurationRoots' array to the configuration that should be loaded
    // (NULL -> none; default values will be used)
    if (autoImportConfig)
        SALAMANDER_ROOT_REG = autoImportConfigFromKey; // when upgrading, searching for the configuration makes no sense
    else
    {
        if (!FindLatestConfiguration(deleteConfigurations, SALAMANDER_ROOT_REG))
        {
            SplashScreenCloseIfExist();
            goto EXIT_2;
        }
    }

    InitializeShellib(); // OLE must be initialized before opening HTML help - CSalamanderEvaluation

    // if a new configuration key does not exist yet, create it before possibly deleting
    // the old keys
    BOOL currentCfgDoesNotExist = autoImportConfig || SALAMANDER_ROOT_REG != SalamanderConfigurationRoots[0];
    BOOL saveNewConfig = currentCfgDoesNotExist;

    // if the user does not want multiple instances, just activate the previous one
    if (!currentCfgDoesNotExist &&
        CheckOnlyOneInstance(&cmdLineParams))
    {
        SplashScreenCloseIfExist();
        myExitCode = 0;
    EXIT_3:
        ReleaseShellib();
        goto EXIT_2;
    }

    // verify the Common Controls version
    if (GetComCtlVersion(&CCVerMajor, &CCVerMinor) != S_OK) // JRYFIXME - move common controls tests to W7+
    {
        CCVerMajor = 0; // this probably never happens - comctl32.dll missing
        CCVerMinor = 0;
    }

    CALL_STACK_MESSAGE1("WinMainBody::StartupDialog");

    //  StartupDialog.Open(HLanguage);

    int i;
    for (i = 0; i < NUMBER_OF_COLORS; i++)
        UserColors[i] = SalamanderColors[i];

    //--- initialization section
    CALL_STACK_MESSAGE1("WinMainBody::inicialization");
    IfExistSetSplashScreenText(LoadStr(IDS_STARTUP_DATA));

    InitDefaultDir();
    PackSetErrorHandler(PackErrorHandler);
    InitLocales();

    if (!InitPreloadedStrings())
    {
        SplashScreenCloseIfExist();
    EXIT_4:
        ReleasePreloadedStrings();
        goto EXIT_3;
    }
    if (!InitializeCheckThread() || !InitializeFind())
    {
        SplashScreenCloseIfExist();
    EXIT_5:
        ReleaseCheckThreads();
        goto EXIT_4;
    }
    InitializeMenuWheelHook();
    SetupWinLibHelp(&SalamanderHelp);
    if (!InitializeDiskCache())
    {
        SplashScreenCloseIfExist();
    EXIT_6:
        ReleaseFind();
        goto EXIT_5;
    }
    if (!InitializeConstGraphics())
    {
        SplashScreenCloseIfExist();
    EXIT_7:
        ReleaseConstGraphics();
        goto EXIT_6;
    }
    if (!InitializeGraphics(FALSE))
    {
        SplashScreenCloseIfExist();
    EXIT_8:
        ReleaseGraphics(FALSE);
        goto EXIT_7;
    }
    if (!InitializeMenu() || !BuildSalamanderMenus())
    {
        SplashScreenCloseIfExist();
        goto EXIT_8;
    }
    if (!InitializeThread())
    {
        SplashScreenCloseIfExist();
    EXIT_9:
        TerminateThread();
        goto EXIT_8;
    }
    if (!InitializeViewer())
    {
        SplashScreenCloseIfExist();
        ReleaseViewer();
        goto EXIT_9;
    }

    // attaching OLE SPY
    // moved below InitializeGraphics, which leaked under WinXP (probably some cache)
    // OleSpyRegister();    // disabled because after the Windows 2000 update from 02/2005 MSVC started
    // to hit a debug breakpoint on start/exit: Invalid Address specified to RtlFreeHeap( 130000, 14bc74 )
    // probably MS began calling RtlFreeHeap directly instead of OLE free and thanks to
    // the spy info block at the beginning of an allocated block it blew up (malloc returns pointer beyond it)
    //OleSpySetBreak(2754); // break on the n-th allocation from the dump

    // worker initialization (disk operations)
    InitWorker();

    // initialization of the library for communication with SalShExt/SalamExt/SalExtX86/SalExtX64.DLL (shell copy hook + shell context menu)
    InitSalShLib();

    // initialization of the library for shell icon overlays (Tortoise SVN + CVS)
    LoadIconOvrlsInfo(SALAMANDER_ROOT_REG);
    InitShellIconOverlays();

    // initialization of functions for browsing next/prev file in panel/Find from the viewer
    InitFileNamesEnumForViewers();

    // load the list of shared directories
    IfExistSetSplashScreenText(LoadStr(IDS_STARTUP_SHARES));
    Shares.Refresh();

    CMainWindow::RegisterUniversalClass(CS_DBLCLKS | CS_SAVEBITS,
                                        0,
                                        0,
                                        NULL,
                                        LoadCursor(NULL, IDC_ARROW),
                                        (HBRUSH)(COLOR_3DFACE + 1),
                                        NULL,
                                        SAVEBITS_CLASSNAME,
                                        NULL);
    CMainWindow::RegisterUniversalClass(CS_DBLCLKS,
                                        0,
                                        0,
                                        NULL,
                                        LoadCursor(NULL, IDC_ARROW),
                                        (HBRUSH)(COLOR_3DFACE + 1),
                                        NULL,
                                        SHELLEXECUTE_CLASSNAME,
                                        NULL);

    Associations.ReadAssociations(FALSE); // load associations from the Registry

    // register shell extensions
    // if we find the library in subdirectory "utils" verify its registration and register it if necessary
    char shellExtPath[MAX_PATH];
    GetModuleFileName(HInstance, shellExtPath, MAX_PATH);
    char* shellExtPathSlash = strrchr(shellExtPath, '\\');
    if (shellExtPathSlash != NULL)
    {
        strcpy(shellExtPathSlash + 1, "utils\\salextx86.dll");
#ifdef _WIN64
        if (FileExists(shellExtPath))
            SalShExtRegistered = SECRegisterToRegistry(shellExtPath, TRUE, KEY_WOW64_32KEY);
        strcpy(shellExtPathSlash + 1, "utils\\salextx64.dll");
        if (FileExists(shellExtPath))
            SalShExtRegistered &= SECRegisterToRegistry(shellExtPath, FALSE, 0);
        else
            SalShExtRegistered = FALSE;
#else  // _WIN64
        if (FileExists(shellExtPath))
            SalShExtRegistered = SECRegisterToRegistry(shellExtPath, FALSE, 0);
        if (Windows64Bit)
        {
            strcpy(shellExtPathSlash + 1, "utils\\salextx64.dll");
            if (FileExists(shellExtPath))
                SalShExtRegistered &= SECRegisterToRegistry(shellExtPath, TRUE, KEY_WOW64_64KEY);
            else
                SalShExtRegistered = FALSE;
        }
#endif // _WIN64
    }

    //--- create the main window
    if (CMainWindow::RegisterUniversalClass(CS_DBLCLKS | CS_OWNDC,
                                            0,
                                            0,
                                            NULL, // HIcon
                                            LoadCursor(NULL, IDC_ARROW),
                                            NULL /*(HBRUSH)(COLOR_WINDOW + 1)*/, // HBrush
                                            NULL,
                                            CFILESBOX_CLASSNAME,
                                            NULL) &&
        CMainWindow::RegisterUniversalClass(CS_DBLCLKS,
                                            0,
                                            0,
                                            HANDLES(LoadIcon(HInstance,
                                                             MAKEINTRESOURCE(IDI_SALAMANDER))),
                                            LoadCursor(NULL, IDC_ARROW),
                                            (HBRUSH)(COLOR_WINDOW + 1),
                                            NULL,
                                            CMAINWINDOW_CLASSNAME,
                                            NULL))
    {
        MainWindow = new CMainWindow;
        if (MainWindow != NULL)
        {
            MainWindow->CmdShow = cmdShow;
            if (MainWindow->Create(CMAINWINDOW_CLASSNAME,
                                   "",
                                   WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                                   CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
                                   NULL,
                                   NULL,
                                   HInstance,
                                   MainWindow))
            {
                SetMessagesParent(MainWindow->HWindow);
                PluginMsgBoxParent = MainWindow->HWindow;

                // pull Group Policy from the registry
                IfExistSetSplashScreenText(LoadStr(IDS_STARTUP_POLICY));
                SystemPolicies.LoadFromRegistry();

                CALL_STACK_MESSAGE1("WinMainBody::load_config");
                BOOL setActivePanelAndPanelPaths = FALSE; // active panel and panel paths are set in MainWindow->LoadConfig()
                if (!MainWindow->LoadConfig(currentCfgDoesNotExist, !importCfgFromFileWasSkipped ? &cmdLineParams : NULL))
                {
                    setActivePanelAndPanelPaths = TRUE;
                    UpdateDefaultColors(CurrentColors, MainWindow->HighlightMasks, FALSE, TRUE);
                    Plugins.CheckData();
                    MainWindow->InsertMenuBand();
                    if (Configuration.TopToolBarVisible)
                        MainWindow->ToggleTopToolBar();
                    if (Configuration.DriveBarVisible)
                        MainWindow->ToggleDriveBar(Configuration.DriveBar2Visible, FALSE);
                    if (Configuration.PluginsBarVisible)
                        MainWindow->TogglePluginsBar();
                    if (Configuration.MiddleToolBarVisible)
                        MainWindow->ToggleMiddleToolBar();
                    if (Configuration.BottomToolBarVisible)
                        MainWindow->ToggleBottomToolBar();
                    MainWindow->CreateAndInsertWorkerBand(); // add the worker last
                    MainWindow->LeftPanel->UpdateDriveIcon(TRUE);
                    MainWindow->RightPanel->UpdateDriveIcon(TRUE);
                    MainWindow->LeftPanel->UpdateFilterSymbol();
                    MainWindow->RightPanel->UpdateFilterSymbol();
                    if (!SystemPolicies.GetNoRun())
                        SendMessage(MainWindow->HWindow, WM_COMMAND, CM_TOGGLEEDITLINE, TRUE);
                    MainWindow->SetWindowIcon();
                    MainWindow->SetWindowTitle();
                    SplashScreenCloseIfExist();
                    ShowWindow(MainWindow->HWindow, cmdShow);
                    UpdateWindow(MainWindow->HWindow);
                    MainWindow->RefreshDirs();
                    MainWindow->FocusLeftPanel();
                }

                if (Configuration.ReloadEnvVariables)
                    InitEnvironmentVariablesDifferences();

                if (newSLGFile)
                {
                    Plugins.ClearLastSLGNames(); // so a new fallback language can be chosen for all plugins if needed
                    Configuration.ShowSLGIncomplete = TRUE;
                }

                MainMenu.SetSkillLevel(CfgSkillLevelToMenu(Configuration.SkillLevel));

                if (!MainWindow->IsGood())
                {
                    SetMessagesParent(NULL);
                    DestroyWindow(MainWindow->HWindow);
                    TRACE_E(LOW_MEMORY);
                }
                else
                {
                    if (!importCfgFromFileWasSkipped) // only if the application does not exit immediately (otherwise pointless)
                        MainWindow->ApplyCommandLineParams(&cmdLineParams, setActivePanelAndPanelPaths);

                    if (Windows7AndLater)
                        CreateJumpList();

                    IdleRefreshStates = TRUE;  // on next Idle force checking of state variables
                    IdleCheckClipboard = TRUE; // also check the clipboard

                    AccelTable1 = HANDLES(LoadAccelerators(HInstance, MAKEINTRESOURCE(IDA_MAINACCELS1)));
                    AccelTable2 = HANDLES(LoadAccelerators(HInstance, MAKEINTRESOURCE(IDA_MAINACCELS2)));

                    MainWindow->CanClose = TRUE; // only now allow closing the main window
                    // prevent files from appearing gradually (while their icons load)
                    UpdateWindow(MainWindow->HWindow);

                    BOOL doNotDeleteImportedCfg = FALSE;
                    if (autoImportConfig && // check whether the new version has fewer plugins than the old one so part of the old configuration would not transfer
                        FindPluginsWithoutImportedCfg(&doNotDeleteImportedCfg))
                    {                               // the program must exit without saving the configuration
                        SALAMANDER_ROOT_REG = NULL; // this should reliably prevent writing configuration to the registry
                        PostMessage(MainWindow->HWindow, WM_USER_FORCECLOSE_MAINWND, 0, 0);
                    }
                    else
                    {
                        if (Configuration.ConfigVersion < THIS_CONFIG_VERSION
#ifndef _WIN64 // FIXME_X64_WINSCP
                            || Configuration.AddX86OnlyPlugins
#endif // _WIN64
                        )
                        {                                            // auto-install plugins from the standard "plugins" subdirectory
#ifndef _WIN64                                                       // FIXME_X64_WINSCP
                            Configuration.AddX86OnlyPlugins = FALSE; // once is enough
#endif                                                               // _WIN64
                            Plugins.AutoInstallStdPluginsDir(MainWindow->HWindow);
                            Configuration.LastPluginVer = 0;   // when switching to a new version the plugins.ver file will be removed
                            Configuration.LastPluginVerOP = 0; // when switching to a new version the plugins.ver file for the other platform will be removed
                            saveNewConfig = TRUE;              // the new configuration must be saved (so this does not repeat next run)
                        }
                        // load the plugins.ver file ((re)install plugins); required even the first time (for plugins installed before first start)
                        if (Plugins.ReadPluginsVer(MainWindow->HWindow, Configuration.ConfigVersion < THIS_CONFIG_VERSION))
                            saveNewConfig = TRUE; // the new configuration must be saved (so this does not repeat next run)
                                                  // load plugins that have the load-on-start flag set
                        Plugins.HandleLoadOnStartFlag(MainWindow->HWindow);
                        // if starting for the first time with a different language, load all plugins to see
                        // whether they have this language version and to let the user choose fallback languages
                        if (langChanged)
                            Plugins.LoadAll(MainWindow->HWindow);

                        // FTP and WinSCP plugins now call SalamanderGeneral->SetPluginUsesPasswordManager() to subscribe to password manager events
                        // introduced with configuration version 45 - allow all plugins to register
                        if (Configuration.ConfigVersion < 45) // introduction of the password manager
                            Plugins.LoadAll(MainWindow->HWindow);

                        // saving will go to the newest key
                        SALAMANDER_ROOT_REG = SalamanderConfigurationRoots[0];
                        // save the configuration immediately while it is a clean conversion of the old version --
                        // the user might have "Save Cfg on Exit" disabled and could change something during runtime
                        if (saveNewConfig)
                        {
                            MainWindow->SaveConfig();
                        }
                        // search the array and if any root is marked for deletion remove it along with the old configuration
                        // after an UPGRADE also remove the "AutoImportConfig" value in the configuration key of this version
                        MainWindow->DeleteOldConfigurations(deleteConfigurations, autoImportConfig, autoImportConfigFromKey,
                                                            doNotDeleteImportedCfg);

                        // only the first instance of Salamander: check whether TEMP needs cleaning from leftover disk-cache files
                        // (after a crash or locking by another application files may remain in TEMP)
                        // we must test a global variable (across all sessions) so two instances started under FastUserSwitching see each other
                        // problem reported on the forum: https://forum.altap.cz/viewtopic.php?t=2643
                        if (FirstInstance_3_or_later)
                        {
                            DiskCache.ClearTEMPIfNeeded(MainWindow->HWindow, MainWindow->GetActivePanelHWND());
                        }

                        if (importCfgFromFileWasSkipped) // if we skipped importing config.reg or another .reg file (parameter -C)
                        {                                // inform the user that Salamander needs to be restarted and let them exit the program
                            MSGBOXEX_PARAMS params;
                            memset(&params, 0, sizeof(params));
                            params.HParent = MainWindow->HWindow;
                            params.Flags = MB_OK | MB_ICONINFORMATION;
                            params.Caption = SALAMANDER_TEXT_VERSION;
                            params.Text = LoadStr(IDS_IMPORTCFGFROMFILESKIPPED);
                            char aliasBtnNames[200];
                            /* used by the export_mnu.py script which generates salmenu.mnu for Translator
   we let the message box buttons resolve hotkey collisions by simulating a menu
MENU_TEMPLATE_ITEM MsgBoxButtons[] = 
{
  {MNTT_PB, 0
  {MNTT_IT, IDS_SELLANGEXITBUTTON
  {MNTT_PE, 0
};
*/
                            sprintf(aliasBtnNames, "%d\t%s", DIALOG_OK, LoadStr(IDS_SELLANGEXITBUTTON));
                            params.AliasBtnNames = aliasBtnNames;
                            SalMessageBoxEx(&params);
                            PostMessage(MainWindow->HWindow, WM_USER_FORCECLOSE_MAINWND, 0, 0);
                        }
                        /*
            // if needed, display the Tip of the Day dialog
            // 0xffffffff = open quietly; if it fails, do not bother the user
            if (Configuration.ShowTipOfTheDay)
              PostMessage(MainWindow->HWindow, WM_COMMAND, CM_HELP_TIP, 0xffffffff);
  */
                    }

                    // from now on closed paths will be remembered
                    MainWindow->CanAddToDirHistory = TRUE;

                    // users want the start-up path in history even if they did not dirty it
                    MainWindow->LeftPanel->UserWorkedOnThisPath = TRUE;
                    MainWindow->RightPanel->UserWorkedOnThisPath = TRUE;

                    // inform the process list that we are running and have the main window (allows activation when OnlyOneInstance is used)
                    TaskList.SetProcessState(PROCESS_STATE_RUNNING, MainWindow->HWindow);

                    // ask Salmon to check if there are old bug reports on disk that should be sent
                    SalmonCheckBugs();

                    if (IsSLGIncomplete[0] != 0 && Configuration.ShowSLGIncomplete)
                        PostMessage(MainWindow->HWindow, WM_USER_SLGINCOMPLETE, 0, 0);

                    //--- application loop
                    CALL_STACK_MESSAGE1("WinMainBody::message_loop");
                    DWORD activateParamsRequestUID = 0;
                    BOOL skipMenuBar;
                    MSG msg;
                    BOOL haveMSG = FALSE; // FALSE if GetMessage() should be called in the loop condition
                    while (haveMSG || GetMessage(&msg, NULL, 0, 0))
                    {
                        haveMSG = FALSE;
                        if (msg.message != WM_USER_SHOWWINDOW && msg.message != WM_USER_WAKEUP_FROM_IDLE && /*msg.message != WM_USER_SETPATHS &&*/
                            msg.message != WM_QUERYENDSESSION && msg.message != WM_USER_SALSHEXT_PASTE &&
                            msg.message != WM_USER_CLOSE_MAINWND && msg.message != WM_USER_FORCECLOSE_MAINWND)
                        { // except for "connect", "shutdown", "do-paste" and "close-main-wnd" messages everything starts the BUSY mode
                            SalamanderBusy = TRUE;
                            LastSalamanderIdleTime = GetTickCount();
                        }

                        if ((msg.message == WM_SYSKEYDOWN || msg.message == WM_KEYDOWN) &&
                            msg.wParam != VK_MENU && msg.wParam != VK_CONTROL && msg.wParam != VK_SHIFT)
                        {
                            SetCurrentToolTip(NULL, 0); // hide the tooltip
                        }

                        skipMenuBar = FALSE;
                        if (Configuration.QuickSearchEnterAlt && msg.message == WM_SYSCHAR)
                            skipMenuBar = TRUE;

                        // forward messages to our menu (avoids needing a keyboard hook)
                        if (MainWindow == NULL || MainWindow->MenuBar == NULL || !MainWindow->CaptionIsActive ||
                            MainWindow->QuickRenameWindowActive() ||
                            skipMenuBar || GetCapture() != NULL || // if the mouse is captured we could cause visual problems
                            !MainWindow->MenuBar->IsMenuBarMessage(&msg))
                        {
                            CWindowsObject* wnd = WindowsManager.GetWindowPtr(GetActiveWindow());

                            // Bottom Toolbar - change text based on VK_CTRL, VK_MENU and VK_SHIFT
                            if ((msg.message == WM_SYSKEYDOWN || msg.message == WM_KEYDOWN ||
                                 msg.message == WM_SYSKEYUP || msg.message == WM_KEYUP) &&
                                MainWindow != NULL)
                                MainWindow->UpdateBottomToolBar();

                            if ((wnd == NULL || !wnd->Is(otDialog) ||
                                 !IsDialogMessage(wnd->HWindow, &msg)) &&
                                (MainWindow == NULL || !MainWindow->CaptionIsActive || // added "!MainWindow->CaptionIsActive" so accelerators are not translated in non-modal plugin windows (F7 in "FTP Logs" would be bad)
                                 MainWindow->QuickRenameWindowActive() ||
                                 !TranslateAccelerator(MainWindow->HWindow, AccelTable1, &msg) &&
                                     (MainWindow->EditMode || !TranslateAccelerator(MainWindow->HWindow, AccelTable2, &msg))))
                            {
                                TranslateMessage(&msg);
                                DispatchMessage(&msg);
                            }
                        }

                        if (MainWindow != NULL && MainWindow->CanClose)
                        { // once Salamander has started we can declare it NOT BUSY
                            SalamanderBusy = FALSE;
                        }

                    TEST_IDLE:
                        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                        {
                            if (msg.message == WM_QUIT)
                                break;      // the equivalent of GetMessage() returning FALSE
                            haveMSG = TRUE; // we have a message, process it without calling GetMessage()
                        }
                        else // if there is no message in the queue, perform Idle processing
                        {
#ifdef _DEBUG
                            // every three seconds check heap consistency
                            if (_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) & _CRTDBG_ALLOC_MEM_DF)
                            {
                                if (GetTickCount() - LastCrtCheckMemoryTime > 3000) // every three seconds
                                {
                                    if (!_CrtCheckMemory())
                                    {
                                        HWND hParent = NULL;
                                        if (MainWindow != NULL)
                                            hParent = MainWindow->HWindow;
                                        MessageBox(hParent, "_CrtCheckMemory failed. Look to the Trace Server for details.", "Open Salamander", MB_OK | MB_ICONERROR);
                                    }
                                    LastCrtCheckMemoryTime = GetTickCount();
                                }
                            }
#endif //_DEBUG

                            if (MainWindow != NULL)
                            {
                                CannotCloseSalMainWnd = TRUE; // we must prevent closing the main window while the following routines run
                                MainWindow->OnEnterIdle();

                                // wait for ESC release only if a panel refresh directly follows
                                // which does not happen through IDLE
                                if (WaitForESCReleaseBeforeTestingESC)
                                    WaitForESCReleaseBeforeTestingESC = FALSE;

                                // check whether another "OnlyOneInstance" Salamander requests activation and setting panel paths
                                // FControlThread would set parameters into the global CommandLineParams and increase RequestUID
                                // if the main thread was idle it was awakened by a posted WM_USER_WAKEUP_FROM_IDLE
                                if (!SalamanderBusy && CommandLineParams.RequestUID > activateParamsRequestUID)
                                {
                                    CCommandLineParams paramsCopy;
                                    BOOL applyParams = FALSE;

                                    NOHANDLES(EnterCriticalSection(&CommandLineParamsCS));
                                    // right before entering the critical section the control thread could have timed out; verify it still wants the result
                                    // also verify the request has not expired (the caller waits only TASKLIST_TODO_TIMEOUT and then
                                    // gives up and starts a new Salamander instance; in that case we should not fulfill the request)
                                    DWORD tickCount = GetTickCount();
                                    if (CommandLineParams.RequestUID != 0 && tickCount - CommandLineParams.RequestTimestamp < TASKLIST_TODO_TIMEOUT)
                                    {
                                        memcpy(&paramsCopy, &CommandLineParams, sizeof(CCommandLineParams));
                                        applyParams = TRUE;

                                        // store the UID we already handled so we do not loop
                                        activateParamsRequestUID = CommandLineParams.RequestUID;
                                        // notify the control thread that we received the paths
                                        SetEvent(CommandLineParamsProcessed);
                                    }
                                    NOHANDLES(LeaveCriticalSection(&CommandLineParamsCS));

                                    // we released shared resources; now we can manipulate the paths
                                    if (applyParams && MainWindow != NULL)
                                    {
                                        SendMessage(MainWindow->HWindow, WM_USER_SHOWWINDOW, 0, 0);
                                        MainWindow->ApplyCommandLineParams(&paramsCopy);
                                    }
                                }

                                // ensure escape from removed drives to a fixed drive (after ejecting devices such as USB flash drives)
                                if (!SalamanderBusy && ChangeLeftPanelToFixedWhenIdle)
                                {
                                    ChangeLeftPanelToFixedWhenIdle = FALSE;
                                    ChangeLeftPanelToFixedWhenIdleInProgress = TRUE;
                                    if (MainWindow != NULL && MainWindow->LeftPanel != NULL)
                                        MainWindow->LeftPanel->ChangeToRescuePathOrFixedDrive(MainWindow->LeftPanel->HWindow);
                                    ChangeLeftPanelToFixedWhenIdleInProgress = FALSE;
                                }
                                // ensure escape from removed drives for the right panel as well
                                if (!SalamanderBusy && ChangeRightPanelToFixedWhenIdle)
                                {
                                    ChangeRightPanelToFixedWhenIdle = FALSE;
                                    ChangeRightPanelToFixedWhenIdleInProgress = TRUE;
                                    if (MainWindow != NULL && MainWindow->RightPanel != NULL)
                                        MainWindow->RightPanel->ChangeToRescuePathOrFixedDrive(MainWindow->RightPanel->HWindow);
                                    ChangeRightPanelToFixedWhenIdleInProgress = FALSE;
                                }
                                if (!SalamanderBusy && OpenCfgToChangeIfPathIsInaccessibleGoTo)
                                {
                                    OpenCfgToChangeIfPathIsInaccessibleGoTo = FALSE;
                                    if (MainWindow != NULL)
                                        PostMessage(MainWindow->HWindow, WM_USER_CONFIGURATION, 6, 0);
                                }

                                // if some plugin requested unload or menu rebuild, do it... (only when not "busy")
                                if (!SalamanderBusy && ExecCmdsOrUnloadMarkedPlugins)
                                {
                                    int cmd;
                                    CPluginData* data;
                                    Plugins.GetCmdAndUnloadMarkedPlugins(MainWindow->HWindow, &cmd, &data);
                                    ExecCmdsOrUnloadMarkedPlugins = (cmd != -1);
                                    if (cmd >= 0 && cmd < 500) // run a Salamander command requested by a plugin
                                    {
                                        int wmCmd = GetWMCommandFromSalCmd(cmd);
                                        if (wmCmd != -1)
                                        {
                                            // generate WM_COMMAND and process it immediately
                                            msg.hwnd = MainWindow->HWindow;
                                            msg.message = WM_COMMAND;
                                            msg.wParam = (DWORD)LOWORD(wmCmd); // cut off the upper WORD just in case (0 - menu command)
                                            msg.lParam = 0;
                                            msg.time = GetTickCount();
                                            GetCursorPos(&msg.pt);

                                            haveMSG = TRUE; // we have a message, process it without calling GetMessage()
                                        }
                                    }
                                    else
                                    {
                                        if (cmd >= 500 && cmd < 1000500) // run a menuExt command requested by a plugin
                                        {
                                            int id = cmd - 500;
                                            SalamanderBusy = TRUE; // executing a menu command - we are "busy" again
                                            LastSalamanderIdleTime = GetTickCount();
                                            if (data != NULL && data->GetLoaded())
                                            {
                                                if (data->GetPluginInterfaceForMenuExt()->NotEmpty())
                                                {
                                                    CALL_STACK_MESSAGE4("CPluginInterfaceForMenuExt::ExecuteMenuItem(, , %d,) (%s v. %s)",
                                                                        id, data->DLLName, data->Version);

                                                    // lower the thread priority to "normal" so operations do not overload the machine
                                                    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);

                                                    CSalamanderForOperations sm(MainWindow->GetActivePanel());
                                                    data->GetPluginInterfaceForMenuExt()->ExecuteMenuItem(&sm, MainWindow->HWindow, id, 0);

                                                    // raise the thread priority again after the operation ends
                                                    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
                                                }
                                                else
                                                {
                                                    TRACE_E("Plugin must have PluginInterfaceForMenuExt when "
                                                            "calling CSalamanderGeneral::PostMenuExtCommand()!");
                                                }
                                            }
                                            else
                                            {
                                                // the plugin may not be loaded; PostMenuExtCommand might be called from Release,
                                                // which was triggered by a posted unload
                                                // TRACE_E("Unexpected situation during call of menu command in \"sal-idle\".");
                                            }
                                            if (MainWindow != NULL && MainWindow->CanClose) // end of menu command execution
                                            {                                               // once Salamander has started we can declare it NOT BUSY
                                                SalamanderBusy = FALSE;
                                            }
                                            CannotCloseSalMainWnd = FALSE;
                                            goto TEST_IDLE; // try "idle" again (e.g. so another posted command/unload can be processed)
                                        }
                                    }
                                }
                                if (!SalamanderBusy && OpenPackOrUnpackDlgForMarkedPlugins)
                                {
                                    CPluginData* data;
                                    int pluginIndex;
                                    Plugins.OpenPackOrUnpackDlgForMarkedPlugins(&data, &pluginIndex);
                                    OpenPackOrUnpackDlgForMarkedPlugins = (data != NULL);
                                    if (data != NULL) // opening Pack/Unpack dialog at plugin request
                                    {
                                        SalamanderBusy = TRUE; // executing a menu command - we are "busy" again
                                        LastSalamanderIdleTime = GetTickCount();
                                        if (data->OpenPackDlg)
                                        {
                                            CFilesWindow* activePanel = MainWindow->GetActivePanel();
                                            if (activePanel != NULL && activePanel->Is(ptDisk))
                                            { // opening the Pack dialog
                                                MainWindow->CancelPanelsUI();
                                                activePanel->UserWorkedOnThisPath = TRUE;
                                                activePanel->StoreSelection(); // save the selection for Restore Selection
                                                activePanel->Pack(MainWindow->GetNonActivePanel(), pluginIndex,
                                                                  data->Name, data->PackDlgDelFilesAfterPacking);
                                            }
                                            else
                                                TRACE_E("Unexpected situation: type of active panel is not Disk!");
                                            data->OpenPackDlg = FALSE;
                                            data->PackDlgDelFilesAfterPacking = 0;
                                        }
                                        else
                                        {
                                            if (data->OpenUnpackDlg)
                                            {
                                                CFilesWindow* activePanel = MainWindow->GetActivePanel();
                                                if (activePanel != NULL && activePanel->Is(ptDisk))
                                                { // opening the Unpack dialog
                                                    MainWindow->CancelPanelsUI();
                                                    activePanel->UserWorkedOnThisPath = TRUE;
                                                    activePanel->StoreSelection(); // save the selection for the Restore Selection command
                                                    activePanel->Unpack(MainWindow->GetNonActivePanel(), pluginIndex,
                                                                        data->Name, data->UnpackDlgUnpackMask);
                                                }
                                                else
                                                    TRACE_E("Unexpected situation: type of active panel is not Disk!");
                                                data->OpenUnpackDlg = FALSE;
                                                if (data->UnpackDlgUnpackMask != NULL)
                                                    free(data->UnpackDlgUnpackMask);
                                                data->UnpackDlgUnpackMask = NULL;
                                            }
                                        }
                                        if (MainWindow != NULL && MainWindow->CanClose) // end of Pack/Unpack dialog opening
                                        {                                               // once Salamander has started we can declare it NOT BUSY
                                            SalamanderBusy = FALSE;
                                        }
                                        CannotCloseSalMainWnd = FALSE;
                                        goto TEST_IDLE; // try "idle" again (e.g. so another posted command/unload/Pack/Unpack can be processed)
                                    }
                                }
                                if (!SalamanderBusy && OpenReadmeInNotepad[0] != 0)
                                { // start notepad with the file 'OpenReadmeInNotepad' for the installer on Vista+
                                    StartNotepad(OpenReadmeInNotepad);
                                    OpenReadmeInNotepad[0] = 0;
                                }
                                CannotCloseSalMainWnd = FALSE;
                            }
                        }
                    }
                }
                PluginMsgBoxParent = NULL;
            }
            else
            {
                TRACE_E(LOW_MEMORY);
            }
        }
    }
    else
        TRACE_E("Unable to register main window class.");

    // in case of an error try closing the dialog
    SplashScreenCloseIfExist();

    // restore the priority to its original state
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);

    //--- give all windows one second to close, then disconnect them
    int timeOut = 10;
    int winsCount = WindowsManager.GetCount();
    while (timeOut-- && winsCount > 0)
    {
        Sleep(100);
        int c = WindowsManager.GetCount();
        if (winsCount > c) // windows are still closing, so we wait at least one more second
        {
            winsCount = c;
            timeOut = 10;
        }
    }

//--- information
#ifdef __DEBUG_WINLIB
    TRACE_I("WindowsManager: " << WindowsManager.maxWndCount << " windows, " << WindowsManager.search << " searches, " << WindowsManager.cache << " cached searches.");
#endif
    //---
    DestroySafeWaitWindow(TRUE); // send a "terminate" command to the safe-wait-message thread
    Sleep(1000);                 // give all viewer threads time to finish
    NBWNetAC3Thread.Close(TRUE); // kill the running thread (moved to AuxThreads) and block further actions
    TerminateAuxThreads();       // force termination of the rest
                                 //---
    TerminateThread();
    ReleaseFileNamesEnumForViewers();
    ReleaseShellIconOverlays();
    ReleaseSalShLib();
    ReleaseWorker();
    ReleaseViewer();
    ReleaseWinLib();
    ReleaseMenuWheelHook();
    ReleaseFind();
    ReleaseCheckThreads();
    ReleasePreloadedStrings();
    ReleaseShellib();
    ReleaseGraphics(FALSE);
    ReleaseConstGraphics();

    HANDLES(FreeLibrary(HLanguage));
    HLanguage = NULL;

    // just to be safe we close it last, although this is probably unnecessary
    ReleaseSalOpen();

    if (NtDLL != NULL)
    {
        HANDLES(FreeLibrary(NtDLL));
        NtDLL = NULL;
    }
    if (User32DLL != NULL)
    {
        NOHANDLES(FreeLibrary(User32DLL));
        User32DLL = NULL;
    }

    //OleSpyStressTest(); // multi-threaded stress test
    // OleSpyRevoke();     // disconnect OLESPY
    OleUninitialize(); // deinitialize OLE
    // OleSpyDump();       // dump leaks

    TRACE_I("End");
    return 0;
}

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmdLine, int cmdShow)
{
#ifndef CALLSTK_DISABLE
    __try
    {
#endif // CALLSTK_DISABLE

        //#ifdef MSVC_RUNTIME_CHECKS
        _RTC_SetErrorFuncW(&MyRTCErrorFunc);
        //#endif // MSVC_RUNTIME_CHECKS

        int result = WinMainBody(hInstance, hPrevInstance, cmdLine, cmdShow);

        return result;
#ifndef CALLSTK_DISABLE
    }
    __except (CCallStack::HandleException(GetExceptionInformation()))
    {
        TRACE_I("Thread Main: calling ExitProcess(1).");
        //    ExitProcess(1);
        TerminateProcess(GetCurrentProcess(), 1); // harder exit (this call still performs some operations)
        return 1;
    }
#endif // CALLSTK_DISABLE
}
