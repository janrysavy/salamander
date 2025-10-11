// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// Initializes the library.
BOOL InitializeShellib();

// Releases the library.
void ReleaseShellib();

// Safe wrapper around IContextMenu2::GetCommandString(), which occasionally crashes inside Windows.
HRESULT AuxGetCommandString(IContextMenu2* menu, UINT_PTR idCmd, UINT uType, UINT* pReserved, LPSTR pszName, UINT cchMax);

// Callback that returns the names of selected files for constructing subsequent interfaces.
typedef const char* (*CEnumFileNamesFunction)(int index, void* param);

// Creates a data object for drag-and-drop operations on the selected files and directories from rootDir.
IDataObject* CreateIDataObject(HWND hOwnerWindow, const char* rootDir, int files,
                               CEnumFileNamesFunction nextFile, void* param);

// Creates the context-menu interface for selected files and directories from rootDir.
IContextMenu2* CreateIContextMenu2(HWND hOwnerWindow, const char* rootDir, int files,
                                   CEnumFileNamesFunction nextFile, void* param);

// Creates the context-menu interface for the given directory.
IContextMenu2* CreateIContextMenu2(HWND hOwnerWindow, const char* dir);

// Does the given directory or file expose a drop target?
BOOL HasDropTarget(const char* dir);

// Creates a drop target for drag-and-drop operations into the given directory or file.
IDropTarget* CreateIDropTarget(HWND hOwnerWindow, const char* dir);

// Opens the shell window for a special folder.
void OpenSpecFolder(HWND hOwnerWindow, int specFolder);

// Opens the folder window for 'dir' and focuses 'item'.
void OpenFolderAndFocusItem(HWND hOwnerWindow, const char* dir, const char* item);

// Opens a browse dialog and selects a path (optionally restricted to network paths).
// hCenterWindow - window to which the dialog will be centered.
BOOL GetTargetDirectory(HWND parent, HWND hCenterWindow, const char* title, const char* comment,
                        char* path, BOOL onlyNet = FALSE, const char* initDir = NULL);

// Detects whether the path points into NetHood (a directory containing target.lnk).
// Optionally resolves target.lnk and writes the result into 'path'; 'path' is an in/out parameter
// (buffer of at least MAX_PATH characters).
void ResolveNetHoodPath(char* path);

class CMenuNew;

// Returns the New menu: the popup-menu handle and the IContextMenu used to invoke commands.
void GetNewOrBackgroundMenu(HWND hOwnerWindow, const char* dir, CMenuNew* menu,
                            int minCmd, int maxCmd, BOOL backgoundMenu);

struct CDragDropOperData
{
    char SrcPath[MAX_PATH];     // shared source path for all files/directories from Names ("" == failed to convert the path from Unicode)
    TIndirectArray<char> Names; // sorted, allocated names of files/directories (CF_HDROP does not distinguish files from directories) ("" == failed to convert the path from Unicode)

    CDragDropOperData() : Names(200, 200) { SrcPath[0] = 0; }
};

// Determines whether 'pDataObject' contains disk files and directories that all live under a single path.
// Optionally stores their names into 'namesList' (if not NULL).
BOOL IsSimpleSelection(IDataObject* pDataObject, CDragDropOperData* namesList);

// Retrieves a name for 'pidl' via GetDisplayNameOf(flags): shortens the ID list by one element, obtains
// the folder for the shortened list from the desktop, and calls that folder's GetDisplayNameOf for the last
// ID with the supplied 'flags'. On success returns TRUE and stores the name in 'name' (buffer of size 'nameSize').
// Does not deallocate 'pidl'; 'alloc' is the allocator obtained via CoGetMalloc.
BOOL GetSHObjectName(ITEMIDLIST* pidl, DWORD flags, char* name, int nameSize, IMalloc* alloc);

// TRUE when the drag-and-drop effect was computed by the plugin FS, so CImpIDropSource::GiveFeedback must not force Copy.
extern BOOL DragFromPluginFSEffectIsFromPlugin;

//*****************************************************************************
//
// CImpIDropSource
//
// Basic implementation; behaves normally (default cursors, etc.).
//
// Exception: when dragging from a plugin FS (with possible Copy and Move effects) into Explorer
// onto a disk containing a TEMP directory, Windows offers Move instead of Copy (which makes no sense—users
// expect Copy). In that case we force the behavior by showing a cursor that differs from dwEffect in GiveFeedback
// and taking the resulting effect from the last cursor shape instead of the DoDragDrop return value.

class CImpIDropSource : public IDropSource
{
private:
    long RefCount;
    DWORD MouseButton; // -1 = uninitialized value, otherwise MK_LBUTTON or MK_RBUTTON

public:
    // Last effect returned by GiveFeedback—added because DoDragDrop never returns dwEffect == DROPEFFECT_MOVE.
    // For MOVE it returns dwEffect == 0; see the "Handling Shell Data Transfer Scenarios" topic,
    // section "Handling Optimized Move Operations":
    // http://msdn.microsoft.com/en-us/library/windows/desktop/bb776904%28v=vs.85%29.aspx
    // (in short: an optimized Move is performed, so it does not copy to the target and subsequently delete the original.
    //            To avoid deleting the original prematurely (the move may still be pending), the result is
    //            DROPEFFECT_NONE or DROPEFFECT_COPY.)
    DWORD LastEffect;

    BOOL DragFromPluginFSWithCopyAndMove; // dragging from a plugin FS with possible Copy and Move; details above

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
            return 0; // must not touch the object; it no longer exists.
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
            { // Copy should be performed but Move is being offered -> force this situation, show the Copy cursor and store Copy in LastEffect
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
// Calls the specified callbacks to obtain the drop target (directory),
// to signal a drop or ESC,
// and leaves the remaining operations to the system IDropTarget object from IShellFolder.

// Record stored inside the copy/move callback payload.
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

// Payload for the copy/move callback.
class CCopyMoveData : public TIndirectArray<CCopyMoveRecord>
{
public:
    BOOL MakeCopyOfName; // TRUE if it should attempt "Copy of..." when the target already exists.

public:
    CCopyMoveData(int base, int delta) : TIndirectArray<CCopyMoveRecord>(base, delta)
    {
        MakeCopyOfName = FALSE;
    }
};

// Callback for copy and move operations; takes care of destroying 'data'.
typedef BOOL (*CDoCopyMove)(BOOL copy, char* targetDir, CCopyMoveData* data,
                            void* param);

// Callback for drag-and-drop operations. 'copy' selects Copy (TRUE) or Move (FALSE);
// 'toArchive' distinguishes archive/FS targets; 'archiveOrFSName' (may be NULL if the caller should obtain
// the information from the panel) names the archive file or plugin FS, 'archivePathOrUserPart' carries the
// path inside the archive or the user part of the FS path, 'data' describes the source files/directories and
// will be destroyed by the callback, and 'param' is the parameter passed to the CImpDropTarget constructor.
typedef void (*CDoDragDropOper)(BOOL copy, BOOL toArchive, const char* archiveOrFSName,
                                const char* archivePathOrUserPart, CDragDropOperData* data,
                                void* param);

// Callback that returns the target directory for the given point 'pt'.
typedef const char* (*CGetCurDir)(POINTL& pt, void* param, DWORD* pdwEffect, BOOL rButton,
                                  BOOL& isTgtFile, DWORD keyState, int& tgtType, int srcType);

// Callback notifying the end of the drop operation; drop == FALSE on ESC.
typedef void (*CDropEnd)(BOOL drop, BOOL shortcuts, void* param, BOOL ownRutine,
                         BOOL isFakeDataObject, int tgtType);

// Callback queried before completing the operation (drop).
typedef BOOL (*CConfirmDrop)(DWORD& effect, DWORD& defEffect, DWORD& grfKeyState);

// Callback announcing the mouse entering and leaving the target.
typedef void (*CEnterLeaveDrop)(BOOL enter, void* param);

// Callback that allows using our routines for copy/move.
typedef BOOL (*CUseOwnRutine)(IDataObject* pDataObject);

// Callback for determining the default drop effect when dragging from FS to FS.
typedef void (*CGetFSToFSDropEffect)(const char* srcFSPath, const char* tgtFSPath,
                                     DWORD allowedEffects, DWORD keyState,
                                     DWORD* dropEffect, void* param);

enum CIDTTgtType
{
    idtttWindows,          // files/directories from a Windows path to a Windows path
    idtttArchive,          // files/directories from a Windows path into an archive
    idtttPluginFS,         // files/directories from a Windows path into a plugin FS
    idtttArchiveOnWinPath, // archive located on a Windows path (drop = pack into the archive)
    idtttFullPluginFSPath, // plugin FS to plugin FS
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
    char OldDataObjectSrcFSPath[2 * MAX_PATH]; // only for the FS type: source FS path

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

    int TgtType; // see CIDTTgtType; idtttWindows also covers archives and FS without the option to drop the current data object.
    IDropTarget* CurDirDropTarget;
    char CurDir[2 * MAX_PATH];

    CEnterLeaveDrop EnterLeaveDrop;
    void* EnterLeaveDropParam;

    BOOL RButton; // action via the right mouse button?

    CUseOwnRutine UseOwnRutine;

    DWORD LastEffect; // last effect determined in DragEnter or DragOver (-1 => invalid)

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
            return 0; // must not touch the object; it no longer exists.
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
    IContextMenu2* Menu2; // menu interface 2
    HMENU Menu;           // New submenu

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
            return 0; // must not touch the object; it no longer exists.
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

// Releases CopyMoveData.
void DestroyCopyMoveData(CCopyMoveData* data);
