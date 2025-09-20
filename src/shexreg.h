// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#pragma warning(3 : 4706) // warning C4706: assignment within conditional expression

// #define ENABLE_SH_MENU_EXT     // define to also register the context menu (not just the "copy hook")

//
// Shell Extensions Configuration
//

//
// ============================================= common section
//

//
// part of Open Salamander for configuration and mutual communication
// and also part of SHELLEXT.DLL for presentation and mutual communication
//

//
// The class ID of this Shell extension class.
//
// salshext.dll (Servant Salamander 2.5 beta 1) class id:        c78b6130-f3ea-11d2-94a1-00e0292a01e3
// salexten.dll (Servant Salamander 2.5 beta 2 through RC1) class id: c78b6131-f3ea-11d2-94a1-00e0292a01e3 (copied to the TEMP directory, shared among multiple Salamander versions)
// salamext.dll (Servant Salamander 2.5 RC2) class id:           c78b6132-f3ea-11d2-94a1-00e0292a01e3 (first version that remains in the Salamander installation; each version has its own shell extension)
// salamext.dll (Servant Salamander 2.5 RC3) class id:           c78b6133-f3ea-11d2-94a1-00e0292a01e3
// salamext.dll (Altap Salamander 2.5 RC3) class id:             c78b6134-f3ea-11d2-94a1-00e0292a01e3
// salamext.dll (Altap Salamander 2.5) class id:                 c78b6135-f3ea-11d2-94a1-00e0292a01e3
// salamext.dll (Altap Salamander 2.51) class id:                c78b6136-f3ea-11d2-94a1-00e0292a01e3
// salamext.dll (Altap Salamander 2.52 beta 1) class id:         c78b6137-f3ea-11d2-94a1-00e0292a01e3
// salamext.dll (Altap Salamander 2.52 beta 1) class id:         c78b6138-f3ea-11d2-94a1-00e0292a01e3 (changing mutex creation/opening caused incompatibility with older versions)
// salamext.dll (Altap Salamander 2.52 beta 1) class id:         c78b6139-f3ea-11d2-94a1-00e0292a01e3 (creating the mutex with limited rights caused older Salamander versions (e.g. 2.51) not to open it at all, so mutex names, memory, etc., were changed)
// salamext.dll (Altap Salamander 2.52 beta 1) class id:         c78b6139-f3ea-11d2-94a1-00e0292a01e3 (creating the mutex with limited rights caused older Salamander versions (e.g. 2.51) not to open it at all, so mutex names, memory, etc., were changed)
// salamext.dll (Altap Salamander 2.52 beta 2) class id:         c78b613a-f3ea-11d2-94a1-00e0292a01e3
// salamext.dll (Altap Salamander 2.52) class id:                c78b613b-f3ea-11d2-94a1-00e0292a01e3
// salamext.dll (Altap Salamander 2.53 beta 1) class id:         c78b613c-f3ea-11d2-94a1-00e0292a01e3
// salamext.dll (Altap Salamander 2.53) class id:                c78b613d-f3ea-11d2-94a1-00e0292a01e3 (unused; we eventually released 2.53 beta 2)
// salamext.dll (Altap Salamander 2.53 beta 2) class id:         c78b613e-f3ea-11d2-94a1-00e0292a01e3
// salamext.dll (Altap Salamander 2.53) class id:                c78b613f-f3ea-11d2-94a1-00e0292a01e3
// salamext.dll (Altap Salamander 2.54) class id:                c78b6140-f3ea-11d2-94a1-00e0292a01e3
// salamext.dll (Altap Salamander 2.55 beta 1) class id:         c78b6141-f3ea-11d2-94a1-00e0292a01e3
// salextx86.dll+salextx64.dll (Salamander 3.0 beta 1) class id: c78b6142-f3ea-11d2-94a1-00e0292a01e3 (first version where x86 and x64 versions are used together)
// salextx86.dll+salextx64.dll (Salamander 3.0 beta 2) class id: c78b6143-f3ea-11d2-94a1-00e0292a01e3
// salextx86.dll+salextx64.dll (Salamander 3.0 beta 3) class id: c78b6144-f3ea-11d2-94a1-00e0292a01e3
// salextx86.dll+salextx64.dll (Salamander 3.0 beta 4) class id: c78b6145-f3ea-11d2-94a1-00e0292a01e3
// salextx86.dll+salextx64.dll (Salamander 3.0) class id:        c78b6146-f3ea-11d2-94a1-00e0292a01e3
// salextx86.dll+salextx64.dll (Salamander 3.01) class id:       c78b6147-f3ea-11d2-94a1-00e0292a01e3
// salextx86.dll+salextx64.dll (Salamander 3.02) class id:       c78b6148-f3ea-11d2-94a1-00e0292a01e3
// salextx86.dll+salextx64.dll (Salamander 3.03) class id:       c78b6149-f3ea-11d2-94a1-00e0292a01e3
// salextx86.dll+salextx64.dll (Salamander 3.04) class id:       c78b614a-f3ea-11d2-94a1-00e0292a01e3
// salextx86.dll+salextx64.dll (Salamander 3.05) class id:       c78b614b-f3ea-11d2-94a1-00e0292a01e3
// salextx86.dll+salextx64.dll (Salamander 3.06) class id:       c78b614c-f3ea-11d2-94a1-00e0292a01e3
// salextx86.dll+salextx64.dll (Salamander 3.07) class id:       c78b614d-f3ea-11d2-94a1-00e0292a01e3
// salextx86.dll+salextx64.dll (Salamander 3.08) class id:       c78b614e-f3ea-11d2-94a1-00e0292a01e3
// salextx86.dll+salextx64.dll (Salamander 4.0) class id:        c78b614f-f3ea-11d2-94a1-00e0292a01e3
//

DEFINE_GUID(CLSID_ShellExtension, 0xc78b614fL, 0xf3ea, 0x11d2, 0x94, 0xa1, 0x00, 0xe0, 0x29, 0x2a, 0x01, 0xe3);

//
// string appended to ensure a unique shell extension name in the registry (SHEXREG_OPENSALAMANDER)
//
// Servant Salamander 2.5 RC2                - 25RC2
// Servant Salamander 2.5 RC3                - 25RC3
// Altap Salamander 2.5 RC3                  - 25RC3
// Altap Salamander 2.5                      - 25
// Altap Salamander 2.51                     - 251
// Altap Salamander 2.52 beta 1              - 252B1
// Altap Salamander 2.52 beta 1              - 252B1a  (changing mutex creation/opening caused incompatibility with older versions)
// Altap Salamander 2.52 beta 1              - 252B1b  (creating the mutex with limited rights caused older Salamander versions (e.g. 2.51) not to open it at all, so mutex names, memory, etc., were changed)
// Altap Salamander 2.52 beta 2              - 252B2
// Altap Salamander 2.52                     - 252
// Altap Salamander 2.53 beta 1              - 253B1
// Altap Salamander 2.53                     - 253     (unused; we eventually released 2.53 beta 2)
// Altap Salamander 2.53 beta 2              - 253B2
// Altap Salamander 2.53                     - 253R
// Altap Salamander 2.54                     - 254
// Altap Salamander 2.55 beta 1              - 255B1   (unused; we eventually released 3.0 beta 1)
// Altap Salamander 3.0 beta 1               - 300B1
// Altap Salamander 3.0 beta 2               - 300B2
// Altap Salamander 3.0 beta 3               - 300B3
// Altap Salamander 3.0 beta 4               - 300B4
// Altap Salamander 3.0                      - 300B5   (by mistake the tag "3.0 beta 5" remained even though we released 3.0)
// Altap Salamander 3.1 beta 1 (unreleased)    - 310B1
// Altap Salamander 3.01                     - 301
// Altap Salamander 3.1 beta 1 (unreleased)    - 310B1_2 (second attempt to release "3.1 beta 1")
// Altap Salamander 3.02                     - 302
// Altap Salamander 3.1 beta 1 (unreleased)    - 310B1_3 (third attempt to release "3.1 beta 1")
// Altap Salamander 3.03                     - 303
// Altap Salamander 3.1 beta 1 (unreleased)    - 310B1_4 (fourth attempt to release "3.1 beta 1")
// Altap Salamander 3.04                     - 304
// Altap Salamander 3.1 beta 1               - 310B1_5 (5th attempt to release "3.1 beta 1")
// Altap Salamander 3.05                     - 305
// Altap Salamander 3.1 beta 1               - 310B1_6 (6th attempt to release "3.1 beta 1")
// Altap Salamander 3.06                     - 306
// Altap Salamander 3.1 beta 1               - 310B1_7 (7th attempt to release "3.1 beta 1")
// Altap Salamander 3.07                     - 307
// Altap Salamander 4.0 beta 1               - 400B1
// Altap Salamander 3.08                     - 308
// Altap Salamander 4.0 beta 1               - 400B1_2 (2nd attempt to release "4.0 beta 1")
// Altap Salamander 4.0                      - 400
// Open Salamander 5.0                       - 500

#define SALSHEXT_SHAREDNAMESAPPENDIX "500"

#ifdef ENABLE_SH_MENU_EXT

#define SEC_NAMEMAX 400

#define SEC_SUBMENUNAME_MAX 100

typedef struct CShellExtConfigItem CShellExtConfigItem;

// basic item representing one line in the context menu
// these items are stored in a singly linked list
struct CShellExtConfigItem
{
    char Name[SEC_NAMEMAX]; // text displayed for the item in the context menu
    // conditions for displaying this item in the context menu
    BOOL OneFile;
    BOOL OneDirectory;
    BOOL MoreFiles;
    BOOL MoreDirectories;
    BOOL LogicalAnd;

    // transient data
    CShellExtConfigItem* Next; // next item; if NULL this is the last one
    UINT Cmd;                  // used to find the item during InvokeCommand
};

// clears the item
void SECClearItem(CShellExtConfigItem* item);

// start of the list
extern CShellExtConfigItem* ShellExtConfigFirst;

// items loaded from the configuration
extern BOOL ShellExtConfigSubmenu;
extern char ShellExtConfigSubmenuName[SEC_SUBMENUNAME_MAX];

// current configuration version number
// increase this number by one when saving the configuration
// when loading, verify the version and if it differs from the current
// configuration, reload the registry
extern DWORD ShellExtConfigVersion;

// returns the item with a given index
CShellExtConfigItem* SECGetItem(int index);

// if an item with the same Cmd is found it stores its index and returns TRUE; otherwise FALSE
BOOL SECGetItemIndex(UINT cmd, int* index);

BOOL SECLoadRegistry();

// retrieves the item name
const char* SECGetName(int index);

// removes all items from the list
void SECDeleteAllItems();

// returns the index of the created item or -1 on error
// 'refItem' returns a pointer to the added item and may be NULL
int SECAddItem(CShellExtConfigItem** refItem);

#endif // ENABLE_SH_MENU_EXT

//
// Data and constants for mutual communication between Salamander and SalamExt
//

// name of the mutex used for accessing shared memory (opened via OpenMutex,
// previously created by CreateMutex)
extern const char* SALSHEXT_SHAREDMEMMUTEXNAME;
// name of the shared memory (opened via OpenFileMapping, previously
// created by CreateFileMapping)
extern const char* SALSHEXT_SHAREDMEMNAME;
// name of the event used to request Paste in the source Salamander; used only on
// Vista+ (on older OS a WM_USER_SALSHEXT_PASTE message posted from the copy hook is enough;
// this post does not work for Salamander started "as admin")
extern const char* SALSHEXT_DOPASTEEVENTNAME;

// NOTE: constants must not change (older versions must remain compatible)
#define WM_USER_SALSHEXT_PASTE WM_APP + 139      // [postMsgIndex, 0] - SalamExt requests execution of the Paste command
#define WM_USER_SALSHEXT_TRYRELDATA WM_APP + 143 // [0, 0] - SalamExt reports unlocking paste data (see CSalShExtSharedMem::BlockPasteDataRelease); if the data are no longer protected, let them be discarded

#define SALSHEXT_NONE 0
#define SALSHEXT_COPY 1
#define SALSHEXT_MOVE 2

// shared memory structure
#pragma pack(push)
#pragma pack(4)
struct CSalShExtSharedMem
{
    int Size; // structure size (used to determine version and guard against memory overwrites)

    // drag&drop section
    BOOL DoDragDropFromSalamander;      // TRUE = DoDragDrop launched from Salamander (uses a "fake" directory)
    char DragDropFakeDirName[MAX_PATH]; // full name of the "fake" directory used for drag&drop
    BOOL DropDone;                      // TRUE = drop occurred, operation started, TargetPath+Operation are valid

    // copy/cut + paste section
    BOOL DoPasteFromSalamander;       // TRUE = paste data object from Salamander (uses a "fake" directory)
    DWORD ClipDataObjLastGetDataTime; // time of the last GetData call on the clipboard data object
    char PasteFakeDirName[MAX_PATH];  // full name of the "fake" directory used for paste
    DWORD SalamanderMainWndPID;       // process ID of the Salamander main window that put pasted data on the clipboard (SalamExt must send it a command to perform paste)
    DWORD SalamanderMainWndTID;       // thread ID of the Salamander main window that put pasted data on the clipboard (SalamExt must send it a command to perform paste)
    UINT64 SalamanderMainWnd;         // main Salamander window that put pasted data on the clipboard (SalamExt must send it a paste command) - HWND is 64-bit on x64 (reportedly only the lower 32 bits are used), the x86 build zeros the upper 32 bits
    int PostMsgIndex;                 // index of the posted WM_USER_SALSHEXT_PASTE message SalamExt waits for (after a timeout the index increases and Salamander skips the message when it eventually arrives)
    BOOL BlockPasteDataRelease;       // probably not needed since W2K+: if TRUE, fakedataobj->Release() does not cancel paste data in Salamander
    int SalBusyState;                 // 0 = checking if Salamander is busy; 1 = Salamander is not busy and already waiting to paste; 2 = Salamander is busy, paste postponed
    DWORD PastedDataID;               // identifier of pasted data (used inside Salamander to know what to paste; only the ID is stored here)
    BOOL PasteDone;                   // TRUE = paste operation started, TargetPath+Operation are valid
    char ArcUnableToPaste1[300];      // prebuilt message for copy-hook about Paste error (see IDS_ARCUNABLETOPASTE1)
    char ArcUnableToPaste2[300];      // prebuilt message for copy-hook about Paste error (see IDS_ARCUNABLETOPASTE2)

    // final operation
    char TargetPath[2 * MAX_PATH]; // target path (where to extract files/directories from archive or copy from file-system)
    int Operation;                 // SALSHEXT_COPY or SALSHEXT_MOVE (or SALSHEXT_NONE after initialization)
};
typedef struct CSalShExtSharedMem CSalShExtSharedMem;
#pragma pack(pop)

//
// ============================================= Altap Salamander only
//

#ifdef INSIDE_SALAMANDER

// stores registry entries necessary for the library
// parameters are the library path, FALSE/TRUE whether to test the DLL version by loading it,
// and which registry view (32-bit or 64-bit) to write to
BOOL SECRegisterToRegistry(const char* shellExtensionPath, BOOL doNotLoadDLL, REGSAM regView);

#ifdef ENABLE_SH_MENU_EXT

// saves data to the registry
BOOL SECSaveRegistry();

// returns the number of items in the list
int SECGetCount();

// removes an item from the list
BOOL SECDeleteItem(int index);

// swaps two items in the list
BOOL SECSwapItems(int index1, int index2);

// sets the item name
BOOL SECSetName(int index, const char* name);

#endif // ENABLE_SH_MENU_EXT

#endif //INSIDE_SALAMANDER
