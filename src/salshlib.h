// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// mutex for accessing shared memory
extern HANDLE SalShExtSharedMemMutex;
// shared memory - see the CSalShExtSharedMem structure
extern HANDLE SalShExtSharedMem;
// event used to request Paste in the source Salamander (only used on Vista+)
extern HANDLE SalShExtDoPasteEvent;
// mapped shared memory - see the CSalShExtSharedMem structure
extern CSalShExtSharedMem* SalShExtSharedMemView;

// TRUE if registering SalShExt/SalExten/SalamExt/SalExtX86/SalExtX64.DLL succeeded or it was already registered
extern BOOL SalShExtRegistered;

// Ugly hack: we need to know which window receives the Drop; GetData checks the cursor position and stores
// the last result in this variable
extern HWND LastWndFromGetData;

// Another hack: we need to know which window receives Paste; GetData checks the foreground window and stores
// the last result in this variable
extern HWND LastWndFromPasteGetData;

extern BOOL OurDataOnClipboard; // TRUE = the clipboard holds our data object (copy & paste from an archive)

//*****************************************************************************

// call before using the library
void InitSalShLib();

// call to release the library
void ReleaseSalShLib();

// returns TRUE if the data object contains only a "fake" directory; if 'fakeType' is not NULL,
// it returns 1 when the source is an archive and 2 when it is a plugin FS. If the source
// is an FS and 'srcFSPathBuf' is not NULL, the source FS path is copied there ('srcFSPathBufSize' is its size)
BOOL IsFakeDataObject(IDataObject* pDataObject, int* fakeType, char* srcFSPathBuf, int srcFSPathBufSize);

//
//*****************************************************************************
// CFakeDragDropDataObject
//
// data object used to determine the drop target during archive extraction or when
// copying from a plugin file system. It wraps the Windows data object obtained for
// a "fake" directory and adds the SALCF_FAKE_REALPATH format (path that should
// appear in the directory line and command line, also blocks drops on the user menu
// toolbar), SALCF_FAKE_SRCTYPE (source type: 1=archive, 2=FS) and, for FS sources,
// SALCF_FAKE_SRCFSPATH (source FS path) to GetData()

class CFakeDragDropDataObject : public IDataObject
{
private:
    long RefCount;
    IDataObject* WinDataObject;   // encapsulated data object
    char RealPath[2 * MAX_PATH];  // path used for the directory and command lines during a drop
    int SrcType;                  // source type (1=archive, 2=FS)
    char SrcFSPath[2 * MAX_PATH]; // only for FS sources: source FS path
    UINT CFSalFakeRealPath;       // clipboard format for sal-fake-real-path
    UINT CFSalFakeSrcType;        // clipboard format for sal-fake-src-type
    UINT CFSalFakeSrcFSPath;      // clipboard format for sal-fake-src-fs-path

public:
    CFakeDragDropDataObject(IDataObject* winDataObject, const char* realPath, int srcType,
                            const char* srcFSPath)
    {
        RefCount = 1;
        WinDataObject = winDataObject;
        WinDataObject->AddRef();
        lstrcpyn(RealPath, realPath, 2 * MAX_PATH);
        if (srcFSPath != NULL && srcType == 2 /* FS */)
            lstrcpyn(SrcFSPath, srcFSPath, 2 * MAX_PATH);
        else
            SrcFSPath[0] = 0;
        SrcType = srcType;
        CFSalFakeRealPath = RegisterClipboardFormat(SALCF_FAKE_REALPATH);
        CFSalFakeSrcType = RegisterClipboardFormat(SALCF_FAKE_SRCTYPE);
        CFSalFakeSrcFSPath = RegisterClipboardFormat(SALCF_FAKE_SRCFSPATH);
    }

    virtual ~CFakeDragDropDataObject()
    {
        if (RefCount != 0)
            TRACE_E("Preliminary destruction of this object.");
        WinDataObject->Release();
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
            return 0; // we must not touch the object after deletion
        }
        return RefCount;
    }

    STDMETHOD(GetData)
    (FORMATETC* formatEtc, STGMEDIUM* medium);

    STDMETHOD(GetDataHere)
    (FORMATETC* pFormatetc, STGMEDIUM* pmedium)
    {
        return WinDataObject->GetDataHere(pFormatetc, pmedium);
    }

    STDMETHOD(QueryGetData)
    (FORMATETC* formatEtc)
    {
        if (formatEtc->cfFormat == CF_HDROP)
            return DV_E_FORMATETC; // this forces a "NO" drop in simpler programs (BOSS, WinCmd, SpeedCommander, MSIE, Word, etc.)
        return WinDataObject->QueryGetData(formatEtc);
    }

    STDMETHOD(GetCanonicalFormatEtc)
    (FORMATETC* pFormatetcIn, FORMATETC* pFormatetcOut)
    {
        return WinDataObject->GetCanonicalFormatEtc(pFormatetcIn, pFormatetcOut);
    }

    STDMETHOD(SetData)
    (FORMATETC* pFormatetc, STGMEDIUM* pmedium, BOOL fRelease)
    {
        return WinDataObject->SetData(pFormatetc, pmedium, fRelease);
    }

    STDMETHOD(EnumFormatEtc)
    (DWORD dwDirection, IEnumFORMATETC** ppenumFormatetc)
    {
        return WinDataObject->EnumFormatEtc(dwDirection, ppenumFormatetc);
    }

    STDMETHOD(DAdvise)
    (FORMATETC* pFormatetc, DWORD advf, IAdviseSink* pAdvSink,
     DWORD* pdwConnection)
    {
        return WinDataObject->DAdvise(pFormatetc, advf, pAdvSink, pdwConnection);
    }

    STDMETHOD(DUnadvise)
    (DWORD dwConnection)
    {
        return WinDataObject->DUnadvise(dwConnection);
    }

    STDMETHOD(EnumDAdvise)
    (IEnumSTATDATA** ppenumAdvise)
    {
        return WinDataObject->EnumDAdvise(ppenumAdvise);
    }
};

//
//*****************************************************************************
// CSalShExtPastedData
//
// data for clipboard Paste stored inside the "source" Salamander

class CSalamanderDirectory;

class CSalShExtPastedData
{
protected:
    DWORD DataID; // version of the data saved for clipboard Paste

    BOOL Lock; // TRUE = locked against deletion, FALSE = not locked

    char ArchiveFileName[MAX_PATH]; // full path to the archive
    char PathInArchive[MAX_PATH];   // path inside the archive where the Copy to clipboard happened
    CNames SelFilesAndDirs;         // file and directory names from PathInArchive that will be extracted

    CSalamanderDirectory* StoredArchiveDir;             // stored archive directory structure (used when the archive isn't open in a panel)
    CPluginDataInterfaceEncapsulation StoredPluginData; // stored plugin-data interface (used when the archive isn't open in a panel)
    FILETIME StoredArchiveDate;                         // archive file date (for validating the archive listing)
    CQuadWord StoredArchiveSize;                        // archive file size (for validating the archive listing)

public:
    CSalShExtPastedData();
    ~CSalShExtPastedData();

    DWORD GetDataID() { return DataID; }
    void SetDataID(DWORD dataID) { DataID = dataID; }

    BOOL IsLocked() { return Lock; }
    void SetLock(BOOL lock) { Lock = lock; }

    // sets the object's data and returns TRUE on success; on failure the object remains empty
    // and the function returns FALSE
    BOOL SetData(const char* archiveFileName, const char* pathInArchive, CFilesArray* files,
                 CFilesArray* dirs, BOOL namesAreCaseSensitive, int* selIndexes,
                 int selIndexesCount);

    // releases data stored in StoredArchiveDir and StoredPluginData
    void ReleaseStoredArchiveData();

    // clears the object (removes all data but keeps it ready for reuse)
    void Clear();

    // performs the paste operation with the current data; 'copy' is TRUE to copy the data and
    // FALSE to move it. 'tgtPath' is the target disk path
    void DoPasteOperation(BOOL copy, const char* tgtPath);

    // if the provided data suits the object it is kept and the method returns TRUE, otherwise
    // FALSE is returned (and the data is released)
    BOOL WantData(const char* archiveFileName, CSalamanderDirectory* archiveDir,
                  CPluginDataInterfaceEncapsulation pluginData,
                  FILETIME archiveDate, CQuadWord archiveSize);

    // returns TRUE if the plugin 'plugin' can be unloaded; if the object contains data from
    // that plugin, it attempts to release it so TRUE can be returned
    BOOL CanUnloadPlugin(HWND parent, CPluginInterfaceAbstract* plugin);
};

// data for Paste from the clipboard stored inside the "source" Salamander
extern CSalShExtPastedData SalShExtPastedData;

//
//*****************************************************************************
// CFakeCopyPasteDataObject
//
// data object used to determine the target of a copy&paste operation (used when
// extracting from archives). It wraps the Windows data object obtained for a
// "fake" directory and deletes that directory from disk once the object is
// released from the clipboard

class CFakeCopyPasteDataObject : public IDataObject
{
private:
    long RefCount;
    IDataObject* WinDataObject; // encapsulated data object
    char FakeDir[MAX_PATH];     // "fake" directory
    UINT CFSalFakeRealPath;     // clipboard format for sal-fake-real-path
    UINT CFIdList;              // clipboard format for shell ID list (Explorer uses this instead of the simpler CF_HDROP)

    DWORD LastGetDataCallTime; // time of the last GetData() call
    BOOL CutOrCopyDone;        // FALSE = the object is still being put on the clipboard; Release does nothing until CutOrCopyDone is TRUE

public:
    CFakeCopyPasteDataObject(IDataObject* winDataObject, const char* fakeDir)
    {
        RefCount = 1;
        WinDataObject = winDataObject;
        WinDataObject->AddRef();
        lstrcpyn(FakeDir, fakeDir, MAX_PATH);
        CFSalFakeRealPath = RegisterClipboardFormat(SALCF_FAKE_REALPATH);
        CFIdList = RegisterClipboardFormat(CFSTR_SHELLIDLIST);
        LastGetDataCallTime = GetTickCount() - 60000; // initialize one minute before the object is created
        CutOrCopyDone = FALSE;
    }

    virtual ~CFakeCopyPasteDataObject()
    {
        if (RefCount != 0)
            TRACE_E("Preliminary destruction of this object.");
        WinDataObject->Release();
    }

    void SetCutOrCopyDone() { CutOrCopyDone = TRUE; }

    STDMETHOD(QueryInterface)
    (REFIID, void FAR* FAR*);
    STDMETHOD_(ULONG, AddRef)
    (void)
    {
        //      TRACE_I("AddRef");
        return ++RefCount;
    }
    STDMETHOD_(ULONG, Release)
    (void);

    STDMETHOD(GetData)
    (FORMATETC* formatEtc, STGMEDIUM* medium);

    STDMETHOD(GetDataHere)
    (FORMATETC* pFormatetc, STGMEDIUM* pmedium)
    {
        //      TRACE_I("GetDataHere");
        return WinDataObject->GetDataHere(pFormatetc, pmedium);
    }

    STDMETHOD(QueryGetData)
    (FORMATETC* formatEtc)
    {
        //      TRACE_I("QueryGetData");
        if (formatEtc->cfFormat == CF_HDROP)
            return DV_E_FORMATETC; // this forces a "NO" drop in simpler programs (BOSS, WinCmd, SpeedCommander, MSIE, Word, etc.)
        return WinDataObject->QueryGetData(formatEtc);
    }

    STDMETHOD(GetCanonicalFormatEtc)
    (FORMATETC* pFormatetcIn, FORMATETC* pFormatetcOut)
    {
        //      TRACE_I("GetCanonicalFormatEtc");
        return WinDataObject->GetCanonicalFormatEtc(pFormatetcIn, pFormatetcOut);
    }

    STDMETHOD(SetData)
    (FORMATETC* pFormatetc, STGMEDIUM* pmedium, BOOL fRelease)
    {
        //      TRACE_I("SetData");
        return WinDataObject->SetData(pFormatetc, pmedium, fRelease);
    }

    STDMETHOD(EnumFormatEtc)
    (DWORD dwDirection, IEnumFORMATETC** ppenumFormatetc)
    {
        //      TRACE_I("EnumFormatEtc");
        return WinDataObject->EnumFormatEtc(dwDirection, ppenumFormatetc);
    }

    STDMETHOD(DAdvise)
    (FORMATETC* pFormatetc, DWORD advf, IAdviseSink* pAdvSink,
     DWORD* pdwConnection)
    {
        //      TRACE_I("DAdvise");
        return WinDataObject->DAdvise(pFormatetc, advf, pAdvSink, pdwConnection);
    }

    STDMETHOD(DUnadvise)
    (DWORD dwConnection)
    {
        //      TRACE_I("DUnadvise");
        return WinDataObject->DUnadvise(dwConnection);
    }

    STDMETHOD(EnumDAdvise)
    (IEnumSTATDATA** ppenumAdvise)
    {
        //      TRACE_I("EnumDAdvise");
        return WinDataObject->EnumDAdvise(ppenumAdvise);
    }
};
