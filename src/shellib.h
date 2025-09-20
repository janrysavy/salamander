// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// library initialization
BOOL InitializeShellib();

// library cleanup
void ReleaseShellib();

// safe call to IContextMenu2::GetCommandString() which sometimes crashes under Windows
HRESULT AuxGetCommandString(IContextMenu2* menu, UINT_PTR idCmd, UINT uType, UINT* pReserved, LPSTR pszName, UINT cchMax);

// callback returning names of selected files for interface creation
typedef const char* (*CEnumFileNamesFunction)(int index, void* param);

// creates a data object for dragging the selected files and directories from rootDir
IDataObject* CreateIDataObject(HWND hOwnerWindow, const char* rootDir, int files,
                               CEnumFileNamesFunction nextFile, void* param);

// creates the context-menu interface for the selected files and directories from rootDir
IContextMenu2* CreateIContextMenu2(HWND hOwnerWindow, const char* rootDir, int files,
                                   CEnumFileNamesFunction nextFile, void* param);

// creates the context-menu interface for the specified directory
IContextMenu2* CreateIContextMenu2(HWND hOwnerWindow, const char* dir);

// does the directory or file have a drop target?
BOOL HasDropTarget(const char* dir);

// creates a drop target for dragging to the specified directory or file
IDropTarget* CreateIDropTarget(HWND hOwnerWindow, const char* dir);

// opens a special-folder window
void OpenSpecFolder(HWND hOwnerWindow, int specFolder);

// opens folder 'dir' and focuses on 'item'
void OpenFolderAndFocusItem(HWND hOwnerWindow, const char* dir, const char* item);

// opens a browse dialog and selects a path (can be limited to network paths only)
// hCenterWindow - window used for centering the dialog
BOOL GetTargetDirectory(HWND parent, HWND hCenterWindow, const char* title, const char* comment,
                        char* path, BOOL onlyNet = FALSE, const char* initDir = NULL);

// detects whether the path is a NetHood link (a directory containing target.lnk)
// and resolves where target.lnk points; 'path' is an in/out buffer (at least MAX_PATH long)
void ResolveNetHoodPath(char* path);

class CMenuNew;

// returns the New menu - a popup handle and IContextMenu used to run commands
void GetNewOrBackgroundMenu(HWND hOwnerWindow, const char* dir, CMenuNew* menu,
                            int minCmd, int maxCmd, BOOL backgoundMenu);

struct CDragDropOperData
{
    char SrcPath[MAX_PATH];     // common source path of all files/directories in Names ("" if the Unicode conversion failed)
    TIndirectArray<char> Names; // sorted allocated names of files/directories (CF_HDROP does not distinguish between them) ("" if the Unicode conversion failed)

    CDragDropOperData() : Names(200, 200) { SrcPath[0] = 0; }
};

// checks whether 'pDataObject' contains files and directories from a single path
// and optionally stores their names into 'namesList' if not NULL
BOOL IsSimpleSelection(IDataObject* pDataObject, CDragDropOperData* namesList);

// retrieves the name for 'pidl' via GetDisplayNameOf(flags). It shortens the ID list,
// obtains the folder from the desktop for that shortened list and calls GetDisplayNameOf on
// the last ID with the given flags. On success returns TRUE and the name in 'name' (buffer size 'nameSize').
// 'pidl' is not deallocated; 'alloc' is the interface from CoGetMalloc
BOOL GetSHObjectName(ITEMIDLIST* pidl, DWORD flags, char* name, int nameSize, IMalloc* alloc);

// TRUE if the drag&drop effect was computed in the plugin FS, so CImpIDropSource::GiveFeedback
// does not need to force Copy
extern BOOL DragFromPluginFSEffectIsFromPlugin;

//*****************************************************************************
//
// CImpIDropSource
//
// Basic version of the object behaving normally (default cursors etc.)
//
// Exception: when dragging from a plugin FS (allowing Copy and Move) to Explorer
// on a drive with a TEMP directory, Move is offered instead of Copy by default
// which makes no sense to users. We therefore override the cursor shown in
// GiveFeedback and take the final effect from the last cursor shape instead of
// DoDragDrop's result.

class CImpIDropSource : public IDropSource
{
private:
    long RefCount;
    DWORD MouseButton; // -1 = uninitialized, otherwise MK_LBUTTON or MK_RBUTTON

public:
    // last effect returned by GiveFeedback. We use this because DoDragDrop
    // does not return DROPEFFECT_MOVE; for MOVE it returns 0. See "Handling Shell
    // Data Transfer Scenarios" section "Handling Optimized Move Operations":
    // http://msdn.microsoft.com/en-us/library/windows/desktop/bb776904(v=vs.85).aspx
    // In short: an optimized Move means no copy to the destination followed by
    // deletion of the source. The source might be deleted inadvertently, so the
    // operation result is DROPEFFECT_NONE or DROPEFFECT_COPY.
    DWORD LastEffect;

    BOOL DragFromPluginFSWithCopyAndMove; // dragging from plugin FS with Copy and Move allowed; see above

public:
    CImpIDropSource(BOOL dragFromPluginFSWithCopyAndMove)
    {
        RefCount = 1;
        MouseButton = -1;
        LastEffect = -1;
        DragFromPluginFSWithCopyAndMove = dragFromPluginFSWithCopyAndMove;
    }

    virtual ~CImpIDropSource()
    {
        if (RefCount != 0)
            TRACE_E("Preliminary destruction of this object.");
    }

    STDMETHOD(QueryInterface)
    (REFIID, void FAR* FAR*);
    STDMETHOD_(ULONG, AddRef)
    (void) { return ++RefCount; }
    STDMETHOD_(ULONG, Release)
    (void)
    {
        if (--RefCount == 0)
        {
            delete this;
            return 0; // must not touch the object; it no longer exists
        }
        return RefCount;
    }

    STDMETHOD(GiveFeedback)
    (DWORD dwEffect)
    {
        if (DragFromPluginFSWithCopyAndMove && !DragFromPluginFSEffectIsFromPlugin)
        {
            BOOL shiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
            BOOL controlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            if ((!shiftPressed || controlPressed) && (dwEffect & DROPEFFECT_MOVE))
            { // Copy should occur but Move is offered -> override by showing the Copy cursor and storing Copy in LastEffect
                LastEffect = DROPEFFECT_COPY;
                SetCursor(LoadCursor(HInstance, MAKEINTRESOURCE(IDC_DRAGCOPYEFFECT)));
                return S_OK;
            }
        }
        DragFromPluginFSEffectIsFromPlugin = FALSE;
        LastEffect = dwEffect;
        return DRAGDROP_S_USEDEFAULTCURSORS;
    }

    STDMETHOD(QueryContinueDrag)
    (BOOL fEscapePressed, DWORD grfKeyState)
    {
        DWORD mb = grfKeyState & (MK_LBUTTON | MK_RBUTTON);
        if (mb == 0)
            return DRAGDROP_S_DROP;
        if (MouseButton == -1)
            MouseButton = mb;
        if (fEscapePressed || MouseButton != mb)
            return DRAGDROP_S_CANCEL;
        return S_OK;
    }
};

//*****************************************************************************
//
// CImpDropTarget
//
// calls defined callbacks to obtain the drop target (directory),
// notifies about dropping or ESC,
// and lets the system IDropTarget from IShellFolder handle the rest

// record used in data for the copy and move callback
struct CCopyMoveRecord
{
    char* FileName;
    char* MapName;

    CCopyMoveRecord(const char* fileName, const char* mapName);
    CCopyMoveRecord(const wchar_t* fileName, const char* mapName);
    CCopyMoveRecord(const char* fileName, const wchar_t* mapName);
    CCopyMoveRecord(const wchar_t* fileName, const wchar_t* mapName);

    char* AllocChars(const char* name);
    char* AllocChars(const wchar_t* name);
};

// data for the copy and move callback
class CCopyMoveData : public TIndirectArray<CCopyMoveRecord>
{
public:
    BOOL MakeCopyOfName; // TRUE if it should attempt "Copy of..." when the target already exists

public:
    CCopyMoveData(int base, int delta) : TIndirectArray<CCopyMoveRecord>(base, delta)
    {
        MakeCopyOfName = FALSE;
    }
};

// callback for copy and move operations, responsible for destroying 'data'
typedef BOOL (*CDoCopyMove)(BOOL copy, char* targetDir, CCopyMoveData* data,
                            void* param);

// callback for drag&drop operations; 'copy' is TRUE/FALSE (copy/move), 'toArchive' is TRUE/FALSE
// (to archive/FS); 'archiveOrFSName' may be NULL if the information should be read from the panel
// it is either the archive file name or FS name. 'archivePathOrUserPart' is the path inside the archive or
// the user part of the FS path. 'data' describes the source files/directories and the function destroys
// the 'data' object. 'param' is the parameter passed to CImpDropTarget's constructor
typedef void (*CDoDragDropOper)(BOOL copy, BOOL toArchive, const char* archiveOrFSName,
                                const char* archivePathOrUserPart, CDragDropOperData* data,
                                void* param);

// callback returning the target directory for the point 'pt'
typedef const char* (*CGetCurDir)(POINTL& pt, void* param, DWORD* pdwEffect, BOOL rButton,
                                  BOOL& isTgtFile, DWORD keyState, int& tgtType, int srcType);

// callback signaling the end of the drop operation, drop == FALSE when ESC is pressed
typedef void (*CDropEnd)(BOOL drop, BOOL shortcuts, void* param, BOOL ownRutine,
                         BOOL isFakeDataObject, int tgtType);

// callback asking before finishing the operation (drop)
typedef BOOL (*CConfirmDrop)(DWORD& effect, DWORD& defEffect, DWORD& grfKeyState);

// callback notifying mouse entry and exit of the target
typedef void (*CEnterLeaveDrop)(BOOL enter, void* param);

// callback that permits using our routines for copy/move
typedef BOOL (*CUseOwnRutine)(IDataObject* pDataObject);

// callback used to determine the default drop effect when dragging from FS to FS
typedef void (*CGetFSToFSDropEffect)(const char* srcFSPath, const char* tgtFSPath,
                                     DWORD allowedEffects, DWORD keyState,
                                     DWORD* dropEffect, void* param);

enum CIDTTgtType
{
    idtttWindows,          // files/directories from a Windows path to a Windows path
    idtttArchive,          // files/directories from a Windows path to an archive
    idtttPluginFS,         // files/directories from a Windows path to a FS
    idtttArchiveOnWinPath, // archive on a Windows path (drop=pack to archive)
    idtttFullPluginFSPath, // FS to FS
};

class CImpDropTarget : public IDropTarget
{
private:
    long RefCount;
    HWND OwnerWindow;
    IDataObject* OldDataObject;
    BOOL OldDataObjectIsFake;
    int OldDataObjectIsSimple;                 // -1 (unknown value), TRUE/FALSE = is/isn't simple (all names on one path)
    int OldDataObjectSrcType;                  // 0 (unknown type), 1/2 = archive/FS
    char OldDataObjectSrcFSPath[2 * MAX_PATH]; // only for FS type: source FS path

    CDoCopyMove DoCopyMove;
    void* DoCopyMoveParam;

    CDoDragDropOper DoDragDropOper;
    void* DoDragDropOperParam;

    CGetCurDir GetCurDir;
    void* GetCurDirParam;

    CDropEnd DropEnd;
    void* DropEndParam;

    CConfirmDrop ConfirmDrop;
    BOOL* ConfirmDropEnable;

    int TgtType; // values see CIDTTgtType; idtttWindows also for archives and FS with no option to drop the current data object
    IDropTarget* CurDirDropTarget;
    char CurDir[2 * MAX_PATH];

    CEnterLeaveDrop EnterLeaveDrop;
    void* EnterLeaveDropParam;

    BOOL RButton; // action with the right mouse button?

    CUseOwnRutine UseOwnRutine;

    DWORD LastEffect; // last effect detected in DragEnter or DragOver (-1 => invalid)

    CGetFSToFSDropEffect GetFSToFSDropEffect;
    void* GetFSToFSDropEffectParam;

public:
    CImpDropTarget(HWND ownerWindow, CDoCopyMove doCopyMove, void* doCopyMoveParam,
                   CGetCurDir getCurDir, void* getCurDirParam, CDropEnd dropEnd,
                   void* dropEndParam, CConfirmDrop confirmDrop, BOOL* confirmDropEnable,
                   CEnterLeaveDrop enterLeaveDrop, void* enterLeaveDropParam,
                   CUseOwnRutine useOwnRutine, CDoDragDropOper doDragDropOper,
                   void* doDragDropOperParam, CGetFSToFSDropEffect getFSToFSDropEffect,
                   void* getFSToFSDropEffectParam)
    {
        RefCount = 1;
        OwnerWindow = ownerWindow;
        DoCopyMove = doCopyMove;
        DoCopyMoveParam = doCopyMoveParam;
        DoDragDropOper = doDragDropOper;
        DoDragDropOperParam = doDragDropOperParam;
        GetCurDir = getCurDir;
        GetCurDirParam = getCurDirParam;
        TgtType = idtttWindows;
        CurDirDropTarget = NULL;
        CurDir[0] = 0;
        DropEnd = dropEnd;
        DropEndParam = dropEndParam;
        OldDataObject = NULL;
        OldDataObjectIsFake = FALSE;
        OldDataObjectIsSimple = -1; // unknown value
        OldDataObjectSrcType = 0;   // unknown type
        OldDataObjectSrcFSPath[0] = 0;
        ConfirmDrop = confirmDrop;
        ConfirmDropEnable = confirmDropEnable;
        RButton = FALSE;
        EnterLeaveDrop = enterLeaveDrop;
        EnterLeaveDropParam = enterLeaveDropParam;
        UseOwnRutine = useOwnRutine;
        LastEffect = -1;
        GetFSToFSDropEffect = getFSToFSDropEffect;
        GetFSToFSDropEffectParam = getFSToFSDropEffectParam;
    }
    virtual ~CImpDropTarget()
    {
        if (RefCount != 0)
            TRACE_E("Preliminary destruction of this object.");
        if (CurDirDropTarget != NULL)
            CurDirDropTarget->Release();
    }

    void SetDirectory(const char* path, DWORD grfKeyState, POINTL pt,
                      DWORD* effect, IDataObject* dataObject, BOOL tgtIsFile, int tgtType);
    BOOL TryCopyOrMove(BOOL copy, IDataObject* pDataObject, UINT CF_FileMapA,
                       UINT CF_FileMapW, BOOL cfFileMapA, BOOL cfFileMapW);
    BOOL ProcessClipboardData(BOOL copy, const DROPFILES* data, const char* mapA,
                              const wchar_t* mapW);

    STDMETHOD(QueryInterface)
    (REFIID, void FAR* FAR*);
    STDMETHOD_(ULONG, AddRef)
    (void) { return ++RefCount; }
    STDMETHOD_(ULONG, Release)
    (void)
    {
        if (--RefCount == 0)
        {
            delete this;
            return 0; // object no longer exists, do not access it
        }
        return RefCount;
    }

    STDMETHOD(DragEnter)
    (IDataObject* pDataObject, DWORD grfKeyState,
     POINTL pt, DWORD* pdwEffect);
    STDMETHOD(DragOver)
    (DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
    STDMETHOD(DragLeave)
    ();
    STDMETHOD(Drop)
    (IDataObject* pDataObject, DWORD grfKeyState, POINTL pt,
     DWORD* pdwEffect);
};

struct IShellFolder;
struct IContextMenu;
struct IContextMenu2;

class CMenuNew
{
protected:
    IContextMenu2* Menu2; // menu-interface 2
    HMENU Menu;           // submenu New

public:
    CMenuNew() { Init(); }
    ~CMenuNew() { Release(); }

    void Init()
    {
        Menu2 = NULL;
        Menu = NULL;
    }

    void Set(IContextMenu2* menu2, HMENU menu)
    {
        if (menu == NULL)
            return; // is-not-set
        Menu2 = menu2;
        Menu = menu;
    }

    BOOL MenuIsAssigned() { return Menu != NULL; }

    HMENU GetMenu() { return Menu; }
    IContextMenu2* GetMenu2() { return Menu2; }

    void Release();
    void ReleaseBody();
};

//
//*****************************************************************************
// CTextDataObject
//

class CTextDataObject : public IDataObject
{
private:
    long RefCount;
    HGLOBAL Data;

public:
    CTextDataObject(HGLOBAL data)
    {
        RefCount = 1;
        Data = data;
    }
    virtual ~CTextDataObject()
    {
        if (RefCount != 0)
            TRACE_E("Preliminary destruction of this object.");
        NOHANDLES(GlobalFree(Data));
    }

    STDMETHOD(QueryInterface)
    (REFIID, void FAR* FAR*);
    STDMETHOD_(ULONG, AddRef)
    (void) { return ++RefCount; }
    STDMETHOD_(ULONG, Release)
    (void)
    {
        if (--RefCount == 0)
        {
            delete this;
            return 0; // object no longer exists, do not access it
        }
        return RefCount;
    }

    STDMETHOD(GetData)
    (FORMATETC* formatEtc, STGMEDIUM* medium);

    STDMETHOD(GetDataHere)
    (FORMATETC* pFormatetc, STGMEDIUM* pmedium)
    {
        return E_NOTIMPL;
    }

    STDMETHOD(QueryGetData)
    (FORMATETC* formatEtc)
    {
        if (formatEtc == NULL)
            return E_INVALIDARG;
        if ((formatEtc->cfFormat == CF_TEXT || formatEtc->cfFormat == CF_UNICODETEXT) &&
            (formatEtc->tymed & TYMED_HGLOBAL))
        {
            return S_OK;
        }
        return (formatEtc->tymed & TYMED_HGLOBAL) ? DV_E_FORMATETC : DV_E_TYMED;
    }

    STDMETHOD(GetCanonicalFormatEtc)
    (FORMATETC* pFormatetcIn, FORMATETC* pFormatetcOut)
    {
        return E_NOTIMPL;
    }

    STDMETHOD(SetData)
    (FORMATETC* pFormatetc, STGMEDIUM* pmedium, BOOL fRelease)
    {
        return E_NOTIMPL;
    }

    STDMETHOD(EnumFormatEtc)
    (DWORD dwDirection, IEnumFORMATETC** ppenumFormatetc)
    {
        return E_NOTIMPL;
    }

    STDMETHOD(DAdvise)
    (FORMATETC* pFormatetc, DWORD advf, IAdviseSink* pAdvSink,
     DWORD* pdwConnection)
    {
        return E_NOTIMPL;
    }

    STDMETHOD(DUnadvise)
    (DWORD dwConnection)
    {
        return OLE_E_ADVISENOTSUPPORTED;
    }

    STDMETHOD(EnumDAdvise)
    (IEnumSTATDATA** ppenumAdvise)
    {
        return OLE_E_ADVISENOTSUPPORTED;
    }
};

// releases CopyMoveData
void DestroyCopyMoveData(CCopyMoveData* data);
