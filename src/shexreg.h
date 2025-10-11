// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#pragma warning(3 : 4706) // warning C4706: assignment within conditional expression

// #define ENABLE_SH_MENU_EXT     // define this macro to add the context menu as well (not just the "copy hook")

//
// Shell Extensions Configuration
//

//
// ============================================= shared part
//

//
// part of Open Salamander - used for configuration and mutual communication
// and at the same time part of SHELLEXT.DLL - used for presentation and mutual communication
//

//
// Class ID of this shell extension.
//
// salshext.dll (Servant Salamander 2.5 beta 1) class id:        c78b6130-f3ea-11d2-94a1-00e0292a01e3
// salexten.dll (Servant Salamander 2.5 beta 2 through RC1) class id: c78b6131-f3ea-11d2-94a1-00e0292a01e3 (copied into the TEMP directory, shared across multiple Salamander versions)
// salamext.dll (Servant Salamander 2.5 RC2) class id:           c78b6132-f3ea-11d2-94a1-00e0292a01e3 (first version that stays in the Salamander installation + every Salamander version has its own shell extension)
// salamext.dll (Servant Salamander 2.5 RC3) class id:           c78b6133-f3ea-11d2-94a1-00e0292a01e3
// salamext.dll (Altap Salamander 2.5 RC3) class id:             c78b6134-f3ea-11d2-94a1-00e0292a01e3
// salamext.dll (Altap Salamander 2.5) class id:                 c78b6135-f3ea-11d2-94a1-00e0292a01e3
// salamext.dll (Altap Salamander 2.51) class id:                c78b6136-f3ea-11d2-94a1-00e0292a01e3
// salamext.dll (Altap Salamander 2.52 beta 1) class id:         c78b6137-f3ea-11d2-94a1-00e0292a01e3
// salamext.dll (Altap Salamander 2.52 beta 1) class id:         c78b6138-f3ea-11d2-94a1-00e0292a01e3 (changing how the mutex is created/opened led to incompatibility with older versions)
// salamext.dll (Altap Salamander 2.52 beta 1) class id:         c78b6139-f3ea-11d2-94a1-00e0292a01e3 (creating the mutex with restricted rights caused older Salamander versions (e.g. 2.51) not to open the mutex at all, so I changed the mutex, memory, etc. names)
// salamext.dll (Altap Salamander 2.52 beta 2) class id:         c78b613a-f3ea-11d2-94a1-00e0292a01e3
// salamext.dll (Altap Salamander 2.52) class id:                c78b613b-f3ea-11d2-94a1-00e0292a01e3
// salamext.dll (Altap Salamander 2.53 beta 1) class id:         c78b613c-f3ea-11d2-94a1-00e0292a01e3
// salamext.dll (Altap Salamander 2.53) class id:                c78b613d-f3ea-11d2-94a1-00e0292a01e3 (unused, we eventually released 2.53 beta 2)
// salamext.dll (Altap Salamander 2.53 beta 2) class id:         c78b613e-f3ea-11d2-94a1-00e0292a01e3
// salamext.dll (Altap Salamander 2.53) class id:                c78b613f-f3ea-11d2-94a1-00e0292a01e3
// salamext.dll (Altap Salamander 2.54) class id:                c78b6140-f3ea-11d2-94a1-00e0292a01e3
// salamext.dll (Altap Salamander 2.55 beta 1) class id:         c78b6141-f3ea-11d2-94a1-00e0292a01e3
// salextx86.dll+salextx64.dll (Salamander 3.0 beta 1) class id: c78b6142-f3ea-11d2-94a1-00e0292a01e3 (first version that uses the x86+x64 builds simultaneously)
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
// string appended to guarantee a unique shell extension name in the registry (SHEXREG_OPENSALAMANDER)
//
// Servant Salamander 2.5 RC2                - 25RC2
// Servant Salamander 2.5 RC3                - 25RC3
// Altap Salamander 2.5 RC3                  - 25RC3
// Altap Salamander 2.5                      - 25
// Altap Salamander 2.51                     - 251
// Altap Salamander 2.52 beta 1              - 252B1
// Altap Salamander 2.52 beta 1              - 252B1a  (changing how the mutex was created/opened made it incompatible with older versions)
// Altap Salamander 2.52 beta 1              - 252B1b  (creating the mutex with restricted rights caused older Salamander versions (e.g. 2.51) not to open the mutex at all, so I changed the mutex, memory, etc. names)
// Altap Salamander 2.52 beta 2              - 252B2
// Altap Salamander 2.52                     - 252
// Altap Salamander 2.53 beta 1              - 253B1
// Altap Salamander 2.53                     - 253     (unused, we eventually released 2.53 beta 2)
// Altap Salamander 2.53 beta 2              - 253B2
// Altap Salamander 2.53                     - 253R
// Altap Salamander 2.54                     - 254
// Altap Salamander 2.55 beta 1              - 255B1   (unused, we eventually released 3.0 beta 1)
// Altap Salamander 3.0 beta 1               - 300B1
// Altap Salamander 3.0 beta 2               - 300B2
// Altap Salamander 3.0 beta 3               - 300B3
// Altap Salamander 3.0 beta 4               - 300B4
// Altap Salamander 3.0                      - 300B5   (accidentally kept as "3.0 beta 5"; we released 3.0 instead)
// Altap Salamander 3.1 beta 1 (unreleased)  - 310B1
// Altap Salamander 3.01                     - 301
// Altap Salamander 3.1 beta 1 (unreleased)  - 310B1_2 (second attempt to ship "3.1 beta 1")
// Altap Salamander 3.02                     - 302
// Altap Salamander 3.1 beta 1 (unreleased)  - 310B1_3 (third attempt to ship "3.1 beta 1")
// Altap Salamander 3.03                     - 303
// Altap Salamander 3.1 beta 1 (unreleased)  - 310B1_4 (fourth attempt to ship "3.1 beta 1")
// Altap Salamander 3.04                     - 304
// Altap Salamander 3.1 beta 1               - 310B1_5 (fifth attempt to ship "3.1 beta 1")
// Altap Salamander 3.05                     - 305
// Altap Salamander 3.1 beta 1               - 310B1_6 (sixth attempt to ship "3.1 beta 1")
// Altap Salamander 3.06                     - 306
// Altap Salamander 3.1 beta 1               - 310B1_7 (seventh attempt to ship "3.1 beta 1")
// Altap Salamander 3.07                     - 307
// Altap Salamander 4.0 beta 1               - 400B1
// Altap Salamander 3.08                     - 308
// Altap Salamander 4.0 beta 1               - 400B1_2 (second attempt to ship "4.0 beta 1")
// Altap Salamander 4.0                      - 400
// Open Salamander 5.0                       - 500

#define SALSHEXT_SHAREDNAMESAPPENDIX "500"

#ifdef ENABLE_SH_MENU_EXT

#define SEC_NAMEMAX 400

#define SEC_SUBMENUNAME_MAX 100

typedef struct CShellExtConfigItem CShellExtConfigItem;

// basic item representing a row in the context menu
// these items are stored in a singly linked list
struct CShellExtConfigItem
{
    char Name[SEC_NAMEMAX]; // text shown for the item in the context menu
    // conditions under which this item appears in the context menu
    BOOL OneFile;
    BOOL OneDirectory;
    BOOL MoreFiles;
    BOOL MoreDirectories;
    BOOL LogicalAnd;

    // non-persisted data
    CShellExtConfigItem* Next; // next item; if NULL, this is the last one
    UINT Cmd;                  // used to look up the item again during InvokeCommand
};

// zeroes the item
void SECClearItem(CShellExtConfigItem* item);

// head of the list
extern CShellExtConfigItem* ShellExtConfigFirst;

// configuration items
extern BOOL ShellExtConfigSubmenu;
extern char ShellExtConfigSubmenuName[SEC_SUBMENUNAME_MAX];

// current configuration version number
// when storing the configuration this number must be incremented by one
// when loading the configuration the version must be checked and, if it differs from the one currently
// used, the registry needs to be reloaded
extern DWORD ShellExtConfigVersion;

// return the item at the given index
CShellExtConfigItem* SECGetItem(int index);

// if it finds an item with the same Cmd, fill the index and return TRUE; otherwise return FALSE
BOOL SECGetItemIndex(UINT cmd, int* index);

BOOL SECLoadRegistry();

// read the item name
const char* SECGetName(int index);

// remove all items from the list
void SECDeleteAllItems();

// return the index of the created item or -1 on failure
// refItem receives a pointer to the added item; it may remain NULL
int SECAddItem(CShellExtConfigItem** refItem);

#endif // ENABLE_SH_MENU_EXT

//
// Data and constants used for communication between Salamander and SalamExt
//

// name of the mutex used to access the shared memory (opened via OpenMutex,
// originally created via CreateMutex)
extern const char* SALSHEXT_SHAREDMEMMUTEXNAME;
// name of the shared memory (opened via OpenFileMapping, originally
// created via CreateFileMapping)
extern const char* SALSHEXT_SHAREDMEMNAME;
// name of the event used to request execution of Paste in the source Salamander, used only on
// Vista+ (for older OS it is enough to post the WM_USER_SALSHEXT_PASTE message directly from the copy hook; on Vista+
// this post does not work for Salamander launched "as admin")
extern const char* SALSHEXT_DOPASTEEVENTNAME;

// NOTE: these constants must not change (older versions must remain compatible)
#define WM_USER_SALSHEXT_PASTE WM_APP + 139      // [postMsgIndex, 0] - SalamExt requests execution of the Paste command
#define WM_USER_SALSHEXT_TRYRELDATA WM_APP + 143 // [0, 0] - SalamExt reports unblocking of the paste data (see CSalShExtSharedMem::BlockPasteDataRelease); if the data are no longer protected, allow them to be destroyed

#define SALSHEXT_NONE 0
#define SALSHEXT_COPY 1
#define SALSHEXT_MOVE 2

// shared memory structure
#pragma pack(push)
#pragma pack(4)
struct CSalShExtSharedMem
{
    int Size; // size of the structure (for version detection + protection against memory overwrites)

    // drag & drop section
    BOOL DoDragDropFromSalamander;      // TRUE = DoDragDrop launched from Salamander (only with the "fake" directory)
    char DragDropFakeDirName[MAX_PATH]; // full name of the "fake" directory used for drag & drop
    BOOL DropDone;                      // TRUE = a drop occurred, the operation started, TargetPath + Operation are valid

    // copy/cut + paste section
    BOOL DoPasteFromSalamander;       // TRUE = data object pasted from Salamander (only with the "fake" directory)
    DWORD ClipDataObjLastGetDataTime; // time of the last GetData call on the data object on the clipboard
    char PasteFakeDirName[MAX_PATH];  // full name of the "fake" directory used for paste
    DWORD SalamanderMainWndPID;       // process ID of Salamander's main window that put the pasted data object on the clipboard (SalamExt must send it the command to perform the paste)
    DWORD SalamanderMainWndTID;       // thread ID of Salamander's main window that put the pasted data object on the clipboard (SalamExt must send it the command to perform the paste)
    UINT64 SalamanderMainWnd;         // main window of Salamander that put the pasted data object on the clipboard (SalamExt must send it the command to perform the paste) - HWND is 64-bit on x64 (even though reportedly only the lower 32 bits are used); the x86 build zeroes the upper 32 bits
    int PostMsgIndex;                 // index of the posted WM_USER_SALSHEXT_PASTE message that SalamExt is waiting for (after the wait timeout expires the index increments -> Salamander then skips the message when it finally arrives)
    BOOL BlockPasteDataRelease;       // probably redundant since W2K+: if TRUE, fakedataobj->Release() does not discard the paste data in Salamander
    int SalBusyState;                 // 0 = checking whether Salamander is "busy"; 1 = Salamander is not "busy" and is already waiting for the paste request; 2 = Salamander is "busy", postpone the paste
    DWORD PastedDataID;               // identifier of the pasted data (used to identify the data for paste inside Salamander - only Salamander knows what should actually be pasted; this stores just the ID)
    BOOL PasteDone;                   // TRUE = paste operation started, TargetPath + Operation are valid
    char ArcUnableToPaste1[300];      // prepared message for the copy hook for Paste errors (see IDS_ARCUNABLETOPASTE1)
    char ArcUnableToPaste2[300];      // prepared message for the copy hook for Paste errors (see IDS_ARCUNABLETOPASTE2)

    // resulting operation
    char TargetPath[2 * MAX_PATH]; // target path (where to unpack files/directories from the archive or copy files/directories from the file system)
    int Operation;                 // SALSHEXT_COPY or SALSHEXT_MOVE (or SALSHEXT_NONE after the structure is initialized)
};
typedef struct CSalShExtSharedMem CSalShExtSharedMem;
#pragma pack(pop)

//
// ============================================= Altap Salamander only
//

#ifdef INSIDE_SALAMANDER

// write the registry entries required for the library to operate
// parameters: library path; FALSE/TRUE to skip/test the DLL version by loading it; 0 or the registry view (32-bit or 64-bit) to write into
BOOL SECRegisterToRegistry(const char* shellExtensionPath, BOOL doNotLoadDLL, REGSAM regView);

#ifdef ENABLE_SH_MENU_EXT

// store data into the registry
BOOL SECSaveRegistry();

// return the number of items in the list
int SECGetCount();

// remove an item from the list
BOOL SECDeleteItem(int index);

// swap two items in the list
BOOL SECSwapItems(int index1, int index2);

// set the item name
BOOL SECSetName(int index, const char* name);

#endif // ENABLE_SH_MENU_EXT

#endif //INSIDE_SALAMANDER
