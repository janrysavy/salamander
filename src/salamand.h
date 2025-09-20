// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

//****************************************************************************
//
// InitializeGraphics
//
// Initializes shared GDI objects used by Salamander.
// Called before opening the main window with firstRun==TRUE, colorsOnly==FALSE
// and fonts==TRUE.
//
// If colors or system settings change while the application is running,
// the function is called with firstRun==FALSE.
//

BOOL InitializeGraphics(BOOL colorsOnly);
void ReleaseGraphics(BOOL colorsOnly);

// initializes objects that do not change with color or resolution changes
BOOL InitializeConstGraphics();
void ReleaseConstGraphics();

class CMenuPopup;

//
// ****************************************************************************

class CFilesArray : public TDirectArray<CFileData>
{
protected:
    BOOL DeleteData; // should destructors of removed items be called?

public:
    // j.r. increased the delta to 800 because entering large directories
    // (thousands of files) makes Enlarge() noticeably consume CPU according
    // to the profiler
    CFilesArray(int base = 200, int delta = 800) : TDirectArray<CFileData>(base, delta) { DeleteData = TRUE; }
    ~CFilesArray() { Destroy(); }

    void SetDeleteData(BOOL deleteData) { DeleteData = deleteData; }

    void DestroyMembers()
    {
        if (DeleteData)
            TDirectArray<CFileData>::DestroyMembers();
        else
            TDirectArray<CFileData>::DetachMembers();
    }

    void Destroy()
    {
        if (!DeleteData)
            DetachMembers();
        TDirectArray<CFileData>::Destroy();
    }

    void Delete(int index)
    {
        if (DeleteData)
            TDirectArray<CFileData>::Delete(index);
        else
            TDirectArray<CFileData>::Detach(index);
    }

    virtual void CallDestructor(CFileData& member)
    {
#ifdef _DEBUG
        if (!DeleteData)
            TRACE_E("Unexpected situation in CFilesArray::CallDestructor()");
#endif // _DEBUG
        free(member.Name);
        if (member.DosName != NULL)
            free(member.DosName);
    }
};

//****************************************************************************
//
// CNames
//
// array of allocated strings that can be sorted alphabetically and searched
// using binary search
//

class CNames
{
protected:
    TDirectArray<char*> Dirs;
    TDirectArray<char*> Files;
    BOOL CaseSensitive;
    BOOL NeedSort; // guard for correct class usage

public:
    CNames();
    ~CNames();

    // empties and deallocates both arrays
    void Clear();

    // sets the behavior of Sort and Contains; when caseSensitive == TRUE
    // names differing only in case are distinguished
    void SetCaseSensitive(BOOL caseSensitive);

    // copies the content of 'name' into its own buffer
    // adds it to the list (Dirs if 'nameIsDir' is TRUE, otherwise Files)
    // returns TRUE on success, otherwise FALSE
    BOOL Add(BOOL nameIsDir, const char* name);

    // sorts the Dirs and Files arrays so that Contains() can be called
    void Sort();

    // returns TRUE if the name specified by 'nameIsDir' and 'name' is present;
    // if 'foundOnIndex' is not NULL, it receives the index of the found item
    // in one of the arrays
    BOOL Contains(BOOL nameIsDir, const char* name, int* foundOnIndex = NULL);

    // returns the total number of stored names
    int GetCount() { return Dirs.Count + Files.Count; }
    // returns the number of stored directories
    int GetDirsCount() { return Dirs.Count; }
    // returns the number of stored files
    int GetFilesCount() { return Files.Count; }

    // loads the list of names from the clipboard; Dirs remain empty, everything goes into Files
    // hWindow is passed to OpenClipboard (not sure if required)
    BOOL LoadFromClipboard(HWND hWindow);
};

//
// ****************************************************************************

class CPathHistoryItem
{
protected:
    int Type;                             // type: 0 disk, 1 archive, 2 plugin FS
    char* PathOrArchiveOrFSName;          // disk path, archive name, or FS name
    char* ArchivePathOrFSUserPart;        // path inside archive or FS user part
    HICON HIcon;                          // icon for the path (may be NULL; destroyed in destructor)
    CPluginFSInterfaceAbstract* PluginFS; // only for Type==2: last used interface for the FS path

    int TopIndex;      // top index at the time the panel state was saved
    char* FocusedName; // focus at the time the panel state was saved

public:
    CPathHistoryItem(int type, const char* pathOrArchiveOrFSName,
                     const char* archivePathOrFSUserPart, HICON hIcon,
                     CPluginFSInterfaceAbstract* pluginFS);
    ~CPathHistoryItem();

    // updates top index and focused name when the same path is added again
    void ChangeData(int topIndex, const char* focusedName);

    void GetPath(char* buffer, int bufferSize);
    HICON GetIcon();
    BOOL Execute(CFilesWindow* panel); // returns TRUE if the change succeeded (FALSE leaves the panel as is)

    BOOL IsTheSamePath(CPathHistoryItem& item, CPluginFSInterfaceEncapsulation* curPluginFS); // returns TRUE if the paths match (each type compares differently)

    friend class CPathHistory;
};

class CPathHistory
{
protected:
    TIndirectArray<CPathHistoryItem> Paths;
    int ForwardIndex;            // -1 means there are no forward items, otherwise
                                 // everything from this index to the end of Paths is forward
    BOOL Lock;                   // object is "locked" (Execute uses it, our panel
                                 // changes are not stored or history would break)
    BOOL DontChangeForwardIndex; // TRUE means ForwardIndex should stay -1 (fully backward)
    CPathHistoryItem* NewItem;   // allocated when AddPathUnique is called while Lock is active

public:
    CPathHistory(BOOL dontChangeForwardIndex = FALSE);
    ~CPathHistory();

    // clears all history items
    void ClearHistory();

    // add a path to the history
    void AddPath(int type, const char* pathOrArchiveOrFSName, const char* archivePathOrFSUserPart,
                 CPluginFSInterfaceAbstract* pluginFS, CPluginFSInterfaceEncapsulation* curPluginFS);

    // add a path to history only if it's not present yet (see Alt+F12; FS paths overwrite pluginFS with the newest one)
    void AddPathUnique(int type, const char* pathOrArchiveOrFSName, const char* archivePathOrFSUserPart,
                       HICON hIcon, CPluginFSInterfaceAbstract* pluginFS,
                       CPluginFSInterfaceEncapsulation* curPluginFS);

    // modify the current path's top index and focused name only when the given
    // path matches the current entry in history
    void ChangeActualPathData(int type, const char* pathOrArchiveOrFSName,
                              const char* archivePathOrFSUserPart,
                              CPluginFSInterfaceAbstract* pluginFS,
                              CPluginFSInterfaceEncapsulation* curPluginFS,
                              int topIndex, const char* focusedName);

    // remove the current path from history only if the specified path matches
    // the current entry
    void RemoveActualPath(int type, const char* pathOrArchiveOrFSName,
                          const char* archivePathOrFSUserPart,
                          CPluginFSInterfaceAbstract* pluginFS,
                          CPluginFSInterfaceEncapsulation* curPluginFS);

    // fill the menu with items
    // IDs start from one and correspond to the index parameter of Execute()
    void FillBackForwardPopupMenu(CMenuPopup* popup, BOOL forward);

    // fill the menu with items
    // IDs start at firstID. When Execute() is called, shift them so the first has value 1.
    // maxCount - maximum number of attached items; -1 = all available (separator not counted)
    // separator - if the menu contains at least one item, insert a separator above it
    void FillHistoryPopupMenu(CMenuPopup* popup, DWORD firstID, int maxCount, BOOL separator);

    // called when closing a FS - the history holds FS interfaces that must be set
    // to NULL after closing to avoid accidental matches by identical addresses
    void ClearPluginFSFromHistory(CPluginFSInterfaceAbstract* fs);

    // index of the selected forward/backward menu item (forward numbering starts at one; backward at two)
    void Execute(int index, BOOL forward, CFilesWindow* panel, BOOL allItems = FALSE, BOOL removeItem = FALSE);

    BOOL HasForward() { return ForwardIndex != -1; }
    BOOL HasBackward()
    {
        int count = (ForwardIndex == -1) ? Paths.Count : ForwardIndex;
        return count > 1;
    }
    BOOL HasPaths() { return Paths.Count > 0; }

    void SaveToRegistry(HKEY hKey, const char* name, BOOL onlyClear);
    void LoadFromRegistry(HKEY hKey, const char* name);
};

//*****************************************************************************
//
// CFileHistoryItem, CFileHistory
//
// Holds a list of files that the user viewed or edited.
//

enum CFileHistoryItemTypeEnum
{
    fhitView,
    fhitEdit,
    fhitOpen,
};

class CFileHistoryItem
{
protected:
    CFileHistoryItemTypeEnum Type; // how the file was accessed
    DWORD HandlerID;               // ID of viewer/editor to repeat the action
    HICON HIcon;                   // icon associated with the file
    char* FileName;                // file name

public:
    CFileHistoryItem(CFileHistoryItemTypeEnum type, DWORD handlerID, const char* fileName);
    ~CFileHistoryItem();

    BOOL IsGood() { return FileName != NULL; }

    // returns TRUE if the object is constructed from the specified data
    BOOL Equal(CFileHistoryItemTypeEnum type, DWORD handlerID, const char* fileName);

    BOOL Execute();

    friend class CFileHistory;
};

class CFileHistory
{
protected:
    TIndirectArray<CFileHistoryItem> Files; // items with smaller index are newer

public:
    CFileHistory();

    // removes all history items
    void ClearHistory();

    // searches history and if the added item is not found, inserts it at the top
    // if the item already exists, it is moved to the top
    BOOL AddFile(CFileHistoryItemTypeEnum type, DWORD handlerID, const char* fileName);

    // fills the menu with items
    // IDs start from one and correspond to the index parameter of Execute()
    BOOL FillPopupMenu(CMenuPopup* popup);

    // index of the selected menu item (numbered from one)
    BOOL Execute(int index);

    // does the history contain any item?
    BOOL HasItem();
};

//****************************************************************************
//
// CColumn
//

class CPluginDataInterfaceAbstract;

// this set of variables is used for Salamander column callbacks
// the plugin receives pointers to these pointers
extern const CFileData* TransferFileData;                     // pointer to data used to draw the column
extern int TransferIsDir;                                     // 0 = file, 1 = directory, 2 = up-dir
extern char TransferBuffer[TRANSFER_BUFFER_MAX];              // pointer to an array with TRANSFER_BUFFER_MAX characters used as the return value
extern int TransferLen;                                       // number of returned characters
extern DWORD TransferRowData;                                 // user data; bits 0x00000001 to 0x00000080 are reserved for Salamander
extern CPluginDataInterfaceAbstract* TransferPluginDataIface; // plugin data interface of the panel where the item is drawn (belongs to TransferFileData->PluginData)
extern DWORD TransferActCustomData;                           // CustomData of the column for which text is retrieved (the callback is called for it) // FIXME_X64 - small for a pointer, maybe unnecessary?

// if the extension was already searched in Associations, this holds the result
extern int TransferAssocIndex; // -2 not searched yet, -1 not found, >=0 valid index

// functions for populating Salamander's standard columns
void WINAPI InternalGetDosName();
void WINAPI InternalGetSize();
void WINAPI InternalGetType();
void WINAPI InternalGetDate();
void WINAPI InternalGetDateOnlyForDisk();
void WINAPI InternalGetTime();
void WINAPI InternalGetTimeOnlyForDisk();
void WINAPI InternalGetAttr();
void WINAPI InternalGetDescr();

// function to obtain the index of simple icons for FS with custom icons (pitFromPlugin)
int WINAPI InternalGetPluginIconIndex();

//****************************************************************************
//
// CViews
//

#define STANDARD_COLUMNS_COUNT 9 // number of standard columns for the range
#define VIEW_TEMPLATES_COUNT 10
#define VIEW_NAME_MAX 30
// the Name column is always visible and if VIEW_SHOW_EXTENSION is not set it also contains the extension
#define VIEW_SHOW_EXTENSION 0x00000001
#define VIEW_SHOW_DOSNAME 0x00000002
#define VIEW_SHOW_SIZE 0x00000004
#define VIEW_SHOW_TYPE 0x00000008
#define VIEW_SHOW_DATE 0x00000010
#define VIEW_SHOW_TIME 0x00000020
#define VIEW_SHOW_ATTRIBUTES 0x00000040
#define VIEW_SHOW_DESCRIPTION 0x00000080

// structure describing a single standard column
struct CColumDataItem
{
    DWORD Flag;
    int NameResID;
    int DescResID;
    FColumnGetText GetText;
    unsigned SupportSorting : 1;
    unsigned LeftAlignment : 1;
    unsigned ID : 4;
};

// definition of standard columns
CColumDataItem* GetStdColumn(int i, BOOL isDisk);

//****************************************************************************
//
// CViewTemplate, CViewTemplates
//
// Serves as a template for panel views. It defines the visibility of columns
// in individual views. The templates are shared by both panels and do not hold
// panel-specific data (except column widths and elasticity).
//

struct CColumnConfig
{
    unsigned LeftWidth : 16;
    unsigned RightWidth : 16;
    unsigned LeftFixedWidth : 1;
    unsigned RightFixedWidth : 1;
};

struct CViewTemplate
{
    DWORD Mode;               // view mode (tree/brief/detailed)
    char Name[VIEW_NAME_MAX]; // name under which the view appears in config/menu;
                              // if empty, the view is not defined
    DWORD Flags;              // visibility of standard Salamander columns
                              // VIEW_SHOW_xxxx

    CColumnConfig Columns[STANDARD_COLUMNS_COUNT]; // stores column widths and elasticity

    BOOL LeftSmartMode;  // smart mode for the left panel (only the elastic Name column shrinks to avoid horizontal scrollbars)
    BOOL RightSmartMode; // smart mode for the right panel (only the elastic Name column shrinks to avoid horizontal scrollbars)
};

class CViewTemplates
{
public:
    // the first views cannot be moved or deleted; they can be renamed
    // the Mode variable is fixed for all ten views and cannot be changed
    CViewTemplate Items[VIEW_TEMPLATES_COUNT];

public:
    CViewTemplates();

    // sets the attributes
    void Set(DWORD index, DWORD viewMode, const char* name, DWORD flags, BOOL leftSmartMode, BOOL rightSmartMode);
    void Set(DWORD index, const char* name, DWORD flags, BOOL leftSmartMode, BOOL rightSmartMode);

    BOOL SwapItems(int index1, int index2); // swaps two items in the array
    BOOL CleanName(char* name);             // trims spaces and returns TRUE if the name is OK

    int SaveColumns(CColumnConfig* columns, char* buffer);  // converts the array to a string
    void LoadColumns(CColumnConfig* columns, char* buffer); // and back

    BOOL Save(HKEY hKey); // saves the entire array
    BOOL Load(HKEY hKey); // loads the entire array

    void Load(CViewTemplates& source)
    {
        memcpy(Items, source.Items, sizeof(Items));
    }
};

//****************************************************************************
//
// CDynamicStringImp
//
// dynamically created string - re-allocates itself as needed (current need + 100 characters)

class CDynamicStringImp : public CDynamicString
{
public:
    char* Text;
    int Allocated;
    int Length;

public:
    CDynamicStringImp()
    {
        Allocated = 0;
        Length = 0;
        Text = NULL;
    }
    ~CDynamicStringImp()
    {
        if (Text != NULL)
            free(Text);
    }

    // detaches data from the object (so the buffer is not deallocated in the destructor)
    void DetachData();

    // returns TRUE if the string 'str' of length 'len' was successfully added; when 'len' is -1,
    // the length is computed as strlen(str) (added without the terminating null). When 'len' is -2,
    // the length is strlen(str)+1 (added including the terminating null)
    virtual BOOL WINAPI Add(const char* str, int len = -1);
};

//****************************************************************************
//
// CTruncatedString
//
// String constructed from str="xxxx "%s" xxxx" and subStr="data.txt".
// The substring may be truncated according to the dialog/message box size.
//

class CTruncatedString
{
protected:
    char* Text;      // complete text
    int SubStrIndex; // index of the first character of the truncatable substring; -1 if it doesn't exist
    int SubStrLen;   // number of characters of the substring

    char* TruncatedText; // truncated form of the text (if truncation was needed)

public:
    CTruncatedString();
    ~CTruncatedString();

    // makes a copy
    BOOL CopyFrom(const CTruncatedString* src);

    // the contents of 'str' will be copied to the allocated Text buffer
    // if subStr is not NULL, it is inserted into str using sprintf
    // str must therefore contain the %s formatting sequence
    BOOL Set(const char* str, const char* subStr);

    // truncates the string according to the dimensions of the window identified by ctrlID
    // if forMessageBox is set, the substring is shortened so the message box stays on screen
    BOOL TruncateText(HWND hWindow, BOOL forMessageBox = FALSE);

    // returns the truncated version of the text (if truncation was needed)
    const char* Get();

    // returns TRUE if the string can be truncated
    BOOL NeedTruncate() { return SubStrIndex != -1; }
};

//****************************************************************************
//
// CShares
//
// list of shared directories

struct CSharesItem
{
    char* LocalPath;  // allocated local path for the share
    char* LocalName;  // points into LocalPath and designates the shared directory name
                      // equals LocalPath for a root path
    char* RemoteName; // name of the shared resource
    char* Comment;    // optional description of the shared resource

    CSharesItem(const char* localPath, const char* remoteName, const char* comment);
    ~CSharesItem();

    void Cleanup(); // initializes pointers and variables
    void Destroy(); // destroys allocated data

    BOOL IsGood() { return LocalPath != NULL; } // if LocalPath is allocated, the rest will be too
};

class CShares
{
protected:
    CRITICAL_SECTION CS;                // section used to synchronize object data
    TIndirectArray<CSharesItem> Data;   // list of shares
    TIndirectArray<CSharesItem> Wanted; // list of attractive references into Data for searching
    BOOL SubsetOnly;

public:
    CShares(BOOL subsetOnly = TRUE); // subsetOnly means no "special" shares are added
                                     // we could also leave Comment empty, but that's minimal loss
    ~CShares();

    void Refresh(); // reload shares from the system

    // prepares for Search(); 'path' specifies the location we are interested in ("" = this_computer)
    void PrepareSearch(const char* path);

    // returns TRUE if the 'path' from PrepareSearch contains the shared subdirectory (or root) 'name'
    BOOL Search(const char* name);

    // returns TRUE if 'path' is a shared directory or one of its subdirectories
    // returns FALSE if no such share is found
    // call without PrepareSearch; searches all shares linearly
    // NOTE: not optimized for speed like PrepareSearch/Search
    BOOL GetUNCPath(const char* path, char* uncPath, int uncPathMax);

    // returns the number of shared directories
    int GetCount() { return Data.Count; }

    // returns information about a specific item; localPath, remoteName or comment
    // may be NULL and then won't be returned
    BOOL GetItem(int index, const char** localPath, const char** remoteName, const char** comment);

protected:
    // returns TRUE if it finds 'name' in Wanted along with its 'index', otherwise FALSE plus the index where it should be inserted
    BOOL GetWantedIndex(const char* name, int& index);
};

class CSalamanderHelp : public CWinLibHelp
{
public:
    virtual void OnHelp(HWND hWindow, UINT helpID, HELPINFO* helpInfo,
                        BOOL ctrlPressed, BOOL shiftPressed);
    virtual void OnContextMenu(HWND hWindow, WORD xPos, WORD yPos);
};

extern CSalamanderHelp SalamanderHelp;

//****************************************************************************
//
// CLanguage
//

class CLanguage
{
public:
    // SLG file name (filename only, .spl)
    char* FileName;

    // data extracted from the SLG file
    WORD LanguageID;
    WCHAR* AuthorW;
    char* Web;
    WCHAR* CommentW;
    char* HelpDir;

public:
    CLanguage();

    BOOL Init(const char* fileName, WORD languageID, const WCHAR* authorW,
              const char* web, const WCHAR* commentW, const char* helpdir);
    BOOL Init(const char* fileName, HINSTANCE modul);
    void Free();
    BOOL GetLanguageName(char* buffer, int bufferSize);
};

BOOL IsSLGFileValid(HINSTANCE hModule, HINSTANCE hSLG, WORD& slgLangID, char* isIncomplete);

//*****************************************************************************
//
// CSystemPolicies
//
// Information is loaded at Salamander startup and adjusts some program
// features and functions.
//

class CSystemPolicies
{
private:
    DWORD NoRun;
    DWORD NoDrives;
    // DWORD NoViewOnDrive;
    DWORD NoFind;
    DWORD NoShellSearchButton;
    DWORD NoNetHood;
    // DWORD NoEntireNetwork;
    // DWORD NoComputersNearMe;
    DWORD NoNetConnectDisconnect;
    DWORD RestrictRun;
    DWORD DisallowRun;
    DWORD NoDotBreakInLogicalCompare;
    TDirectArray<char*> RestrictRunList;
    TDirectArray<char*> DisallowRunList;

public:
    CSystemPolicies();
    ~CSystemPolicies();

    // loads the settings from the registry
    void LoadFromRegistry();

    // If your application has a "run" function that allows a user to start a program
    // by typing in its name and path in a dialog, then your application must disable
    // that functionality when this policy is enabled
    //
    // 0 - The policy is disabled or not configured. The Run command appears.
    // 1 - The policy is enabled. The Run command is removed.
    DWORD GetNoRun() { return NoRun; }

    // Your application must hide any drives that are hidden by the system when this
    // policy is enabled; this includes any buttons, menu options, icons, or any other
    // visual representation of drives in your application. This does not preclude the
    // user from accessing drives by manually entering drive letters in dialogs
    //
    // 0x00000000 - Do not restrict drives. All drives appear.
    // 0x00000001 - Restrict A drive only.
    // 0x00000002 - Restrict B drive only.
    // ....
    // 0x0000000F - Restrict A, B, C, and D drives only.
    // ....
    // 0x03FFFFFF - Restrict all drives.
    DWORD GetNoDrives() { return NoDrives; }

    // When a drive is represented in the value of this entry, users cannot view the
    // contents of the selected drives in Computer or Windows Explorer. Also, they
    // cannot use the Run dialog box, the Map Network Drive dialog box, or the Dir command
    // to view the directories on these drives.
    //
    // 0x00000000 - Do not restrict drives. All drives appear.
    // 0x00000001 - Restrict A drive only.
    // 0x00000002 - Restrict B drive only.
    // ....
    // 0x0000000F - Restrict A, B, C, and D drives only.
    // ....
    // 0x03FFFFFF - Restrict all drives.
    // DWORD GetNoViewOnDrive() {return NoViewOnDrive;}

    // When the value of this entry is 1, the following features are removed or disabled.
    // When the value is 0, these features operate normally.
    // The Search item is removed from the Start menu and from the context menu that
    // appears when you right-click the Start menu.
    // The system does not respond when users press F3 or the Application key (the key
    // with the Windows logo) + F.
    // In Windows Explorer, the Search item still appears on the Standard buttons toolbar,
    // but the system does not respond when the user presses Ctrl + F.
    // In Windows Explorer, the Search item does not appear in the context menu when you
    // right-click an icon representing a drive or a folder.
    //
    // 0 - The policy is disabled or not configured.
    // 1 - The policy is enabled.
    DWORD GetNoFind() { return NoFind; }

    // Removes the Search button from the Standard Buttons toolbar that appears in
    // Windows Explorer and other programs that use the Windows Explorer window,
    // such as Computer and Network.
    //
    // 0 - The policy is disabled or not configured. The Search button appears on the Windows Explorer toolbar.
    // 1 - The policy is enabled. The Search button is removed from the Windows Explorer toolbar.
    DWORD GetNoShellSearchButton() { return NoShellSearchButton; }

    // Removes the My Network Places icon from the desktop.
    //
    // 0 - The policy is disabled or not configured. The My Network Places icon appears on the desktop.
    // 1 - The policy is enabled. The My Network Places icon is hidden.
    DWORD GetNoNetHood() { return NoNetHood; }

    // Removes the Entire Network option and the icons representing networked computers
    // from My Network Places and from the Map Network Drive browser. As a result, users
    // cannot view computers outside of their workgroup or local domain in lists of network
    // resources in Windows Explorer and My Network Places.
    //
    // 0 - The policy is disabled or not configured. The Entire Network option appears.
    // 1 - The policy is enabled. The Entire Network option is removed.
    // DWORD GetNoEntireNetwork() {return NoEntireNetwork;}

    // Removes the Computers Near Me icon, and the icons representing computers in the
    // workgroup, from My Network Places and from the Map Network Drive browser window.
    //
    // 0 - The policy is disabled or not configured. The Computers Near Me icon appears
    //     when the computer is a member of a workgroup.
    // 1 - The policy is enabled.The Computers Near Me icon never appears.
    // DWORD GetNoComputersNearMe() {return NoComputersNearMe;}

    // Prevents users from using Windows Explorer or My Network Places to map or
    // disconnect network drives.
    //
    // 0 - The policy is disabled or not configured. Users can map and disconnect network drives.
    // 1 - The policy is enabled. Users can't map and disconnect network drives.
    DWORD GetNoNetConnectDisconnect() { return NoNetConnectDisconnect; }

    // are there restrictions on running applications?
    BOOL GetMyRunRestricted() { return RestrictRun != 0 || DisallowRun != 0; }
    // is there a restriction on the file 'fileName' (it may also be a full path)
    BOOL GetMyCanRun(const char* fileName);

    // 1 = our StrCmpLogicalEx and the system StrCmpLogicalW under Vista do not treat the dot
    // as a separator in names ("File.txt" is greater than "File (4).txt")
    DWORD GetNoDotBreakInLogicalCompare() { return NoDotBreakInLogicalCompare; }

private:
    // sets all values to the allowed state and destroys the list of strings
    void EnableAll();
    // loads all keys and adds them to the list
    // returns FALSE if there was not enough memory to allocate the list
    BOOL LoadList(TDirectArray<char*>* list, HKEY hRootKey, const char* keyName);
    // returns TRUE if 'name' is in the list
    BOOL FindNameInList(TDirectArray<char*>* list, const char* name);
};

extern CSystemPolicies SystemPolicies;

//
// ****************************************************************************
//
// horizontally and vertically centered dialog
// base of all dialogs in Salamander
// ensures ArrangeHorizontalLines is called for every dialog
//
// If 'HCenterAgains' is not NULL, it is centered against it, otherwise against the parent.
// Sets the msgbox parent for plug-ins to this dialog (only while it exists)
//

class CCommonDialog : public CDialog
{
protected:
    HWND HCenterAgains;
    HWND HOldPluginMsgBoxParent;
    BOOL CallEndStopRefresh; // EndStopRefresh must be called

public:
    CCommonDialog(HINSTANCE modul, int resID, HWND parent,
                  CObjectOrigin origin = ooStandard, HWND hCenterAgains = NULL)
        : CDialog(modul, resID, parent, origin)
    {
        HCenterAgains = hCenterAgains;
        HOldPluginMsgBoxParent = NULL;
        CallEndStopRefresh = FALSE;
    }
    CCommonDialog(HINSTANCE modul, int resID, UINT helpID, HWND parent,
                  CObjectOrigin origin = ooStandard, HWND hCenterAgains = NULL)
        : CDialog(modul, resID, helpID, parent, origin)
    {
        HCenterAgains = hCenterAgains;
        HOldPluginMsgBoxParent = NULL;
        CallEndStopRefresh = FALSE;
    }
    ~CCommonDialog()
    {
        if (CallEndStopRefresh)
            TRACE_E("CCommonDialog::~CCommonDialog(): EndStopRefresh() was not called!");
    }

protected:
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    virtual void NotifDlgJustCreated();
};

//
// ****************************************************************************
//
// base of all property-sheet pages in Salamander
// ensures ArrangeHorizontalLines is called for every page
//

class CCommonPropSheetPage : public CPropSheetPage
{
public:
    CCommonPropSheetPage(TCHAR* title, HINSTANCE modul, int resID,
                         DWORD flags /* = PSP_USETITLE*/, HICON icon,
                         CObjectOrigin origin = ooStatic)
        : CPropSheetPage(title, modul, resID, flags, icon, origin) {}
    CCommonPropSheetPage(TCHAR* title, HINSTANCE modul, int resID, UINT helpID,
                         DWORD flags /* = PSP_USETITLE*/, HICON icon,
                         CObjectOrigin origin = ooStatic)
        : CPropSheetPage(title, modul, resID, helpID, flags, icon, origin) {}

protected:
    virtual void NotifDlgJustCreated();
};

//****************************************************************************
//
// CMessagesKeeper
//
// Circular queue that stores the last X MSG structures
// captured in the hook
//
// If Salamander crashes we insert this list into the bug report
//

// number of stored messages
#define MESSAGES_KEEPER_COUNT 30

class CMessagesKeeper
{
private:
    MSG Messages[MESSAGES_KEEPER_COUNT]; // actual messages
    int Index;                           // index of free slot in Messages
    int Count;                           // number of valid items

public:
    CMessagesKeeper();

    // adds a message to the queue
    void Add(const MSG* msg);

    // returns the number of valid messages
    int GetCount() { return Count; }

    // writes the item at 'index' into 'buffer' (buffer size given by 'buffMax')
    // index 0 is the oldest item, Count is the last added one
    // if the index is out of range, writes the text "error"
    void Print(char* buffer, int buffMax, int index);
};

// for the main application loop
extern CMessagesKeeper MessagesKeeper;

//****************************************************************************
//
// CWayPointsKeeper
//
// Circular queue holding the last X waypoints (id, custom data and insertion time)
// that we placed throughout the code
//
// If Salamander crashes we include this list in the bug report
//

// number of stored waypoints
#define WAYPOINTS_KEEPER_COUNT 100

struct CWayPoint
{
    DWORD WayPoint;     // value defined in the code
    WPARAM CustomData1; // user value
    LPARAM CustomData2; // user value
    DWORD Time;         // insertion time
};

class CWayPointsKeeper
{
private:
    CWayPoint WayPoints[WAYPOINTS_KEEPER_COUNT]; // actual waypoints
    int Index;                                   // index of free slot in WayPoints
    int Count;                                   // number of valid items
    BOOL Stopped;                                // TRUE/FALSE = storing of waypoints enabled/disabled
    CRITICAL_SECTION CS;                         // section used to synchronize object data

public:
    CWayPointsKeeper();
    ~CWayPointsKeeper();

    // inserts a waypoint into the queue
    void Add(DWORD waypoint, WPARAM customData1 = 0, LPARAM customData2 = 0);

    // if 'stop' is TRUE, stop storing waypoints (calls to Add are ignored);
    // if 'stop' is FALSE, storing waypoints is enabled again
    void StopStoring(BOOL stop);

    // returns the number of valid waypoints
    int GetCount()
    {
        HANDLES(EnterCriticalSection(&CS));
        int count = Count;
        HANDLES(LeaveCriticalSection(&CS));
        return count;
    }

    // writes the item at 'index' into 'buffer' (buffer size passed in 'buffMax')
    // index 0 is the oldest item, Count is the last added waypoint
    // if the index is out of range, writes the text "error"
    void Print(char* buffer, int buffMax, int index);
};

//******************************************************************************
//
// CITaskBarList3
//
// Encapsulation of the ITaskBarList3 interface introduced by Microsoft in Windows 7
//

class CITaskBarList3
{
public:
    ITaskbarList3* Iface;
    HWND HWindow;

public:
    CITaskBarList3()
    {
        Iface = NULL;
        HWindow = NULL;
    }

    ~CITaskBarList3()
    {
        if (Iface != NULL)
        {
            Iface->Release();
            Iface = NULL;
            CoUninitialize();
        }
    }

    BOOL Init(HWND hWindow) // call after receiving the TaskbarBtnCreatedMsg message
    {
        // Initialize COM for this thread...
        CoInitialize(NULL);
        CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_ITaskbarList3, (void**)&Iface);
        if (Iface == NULL)
        {
            TRACE_E("CoCreateInstance() failed for IID_ITaskbarList3!");
            CoUninitialize();
            return FALSE;
        }
        HWindow = hWindow;
        return TRUE;
    }

    void SetProgress2(const CQuadWord& progressCurrent, const CQuadWord& progressTotal)
    {
        // progressTotal can be 1 and progressCurrent a large number; the computation
        // then makes no sense (and even crashes due to RTC), so explicitly set 0% or 100% (value 1000)
        SetProgressValue(progressCurrent >= progressTotal ? (progressTotal.Value == 0 ? 0 : 1000) : (DWORD)((progressCurrent * CQuadWord(1000, 0)) / progressTotal).Value,
                         1000);
    }

    void SetProgressValue(ULONGLONG ullCompleted, ULONGLONG ullTotal)
    {
        if (Iface != NULL)
        {
            HRESULT hres = Iface->SetProgressValue(HWindow, ullCompleted, ullTotal);
            if (hres != S_OK)
                TRACE_E("SetProgressValue failed! hres=" << hres);
        }
    }

    void SetProgressState(TBPFLAG tbpFlags)
    {
        if (Iface != NULL)
        {
            HRESULT hres = Iface->SetProgressState(HWindow, tbpFlags);
            if (hres != S_OK)
                TRACE_E("SetProgressState failed! hres=" << hres);
        }
    }
};

//****************************************************************************
//
// CShellExecuteWnd
//
// window used as the parent when calling InvokeCommand, SHFileOperation, etc.
// if someone calls DestroyWindow on this handle before the destructor,
// a MessageBox is shown informing that a shell extension injured us and asks
// for sending the following Break bug report; its call stack can be seen there
//
class CShellExecuteWnd : public CWindow
{
protected:
    BOOL CanClose;

public:
    CShellExecuteWnd();
    ~CShellExecuteWnd();

    // 'format' is a formatting string for sprintf; pointers to strings may be NULL and are translated to "(null)"
    // on success returns the handle of the created window
    // WARNING: on failure returns hParent
    HWND Create(HWND hParent, const char* format, ...);

    virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

// enumerates all children of 'hParent' and searches for CShellExecuteWnd windows
// their names are extracted and inserted into 'text', separated by "\r\n"
// does not exceed the size 'textMax' and terminates the string with 0
// returns the number of found windows
int EnumCShellExecuteWnd(HWND hParent, char* text, int textMax);

//
// ****************************************************************************

class CSalamanderSafeFile : public CSalamanderSafeFileAbstract
{
public:
    virtual BOOL WINAPI SafeFileOpen(SAFE_FILE* file,
                                     const char* fileName,
                                     DWORD dwDesiredAccess,
                                     DWORD dwShareMode,
                                     DWORD dwCreationDisposition,
                                     DWORD dwFlagsAndAttributes,
                                     HWND hParent,
                                     DWORD flags,
                                     DWORD* pressedButton,
                                     DWORD* silentMask);

    virtual HANDLE WINAPI SafeFileCreate(const char* fileName,
                                         DWORD dwDesiredAccess,
                                         DWORD dwShareMode,
                                         DWORD dwFlagsAndAttributes,
                                         BOOL isDir,
                                         HWND hParent,
                                         const char* srcFileName,
                                         const char* srcFileInfo,
                                         DWORD* silentMask,
                                         BOOL allowSkip,
                                         BOOL* skipped,
                                         char* skipPath,
                                         int skipPathMax,
                                         CQuadWord* allocateWholeFile,
                                         SAFE_FILE* file);

    virtual void WINAPI SafeFileClose(SAFE_FILE* file);

    virtual BOOL WINAPI SafeFileSeek(SAFE_FILE* file,
                                     CQuadWord* distance,
                                     DWORD moveMethod,
                                     DWORD* error);

    virtual BOOL WINAPI SafeFileSeekMsg(SAFE_FILE* file,
                                        CQuadWord* distance,
                                        DWORD moveMethod,
                                        HWND hParent,
                                        DWORD flags,
                                        DWORD* pressedButton,
                                        DWORD* silentMask,
                                        BOOL seekForRead);

    virtual BOOL WINAPI SafeFileGetSize(SAFE_FILE* file,
                                        CQuadWord* fileSize,
                                        DWORD* error);

    virtual BOOL WINAPI SafeFileRead(SAFE_FILE* file,
                                     LPVOID lpBuffer,
                                     DWORD nNumberOfBytesToRead,
                                     LPDWORD lpNumberOfBytesRead,
                                     HWND hParent,
                                     DWORD flags,
                                     DWORD* pressedButton,
                                     DWORD* silentMask);

    virtual BOOL WINAPI SafeFileWrite(SAFE_FILE* file,
                                      LPVOID lpBuffer,
                                      DWORD nNumberOfBytesToWrite,
                                      LPDWORD lpNumberOfBytesWritten,
                                      HWND hParent,
                                      DWORD flags,
                                      DWORD* pressedButton,
                                      DWORD* silentMask);
};

extern BOOL GotMouseWheelScrollLines;

// An OS independent method to retrieve the number of wheel scroll lines.
// Returns: Number of scroll lines where WHEEL_PAGESCROLL indicates to scroll a page at a time.
UINT GetMouseWheelScrollLines();
UINT GetMouseWheelScrollChars(); // for horizontal scrolling

BOOL InitializeMenuWheelHook();
BOOL ReleaseMenuWheelHook();

#define BUG_REPORT_REASON_MAX 1000
extern char BugReportReasonBreak[BUG_REPORT_REASON_MAX]; // text shown when Salamander breaks into the bug report (reason)

extern CShares Shares; // loaded shared directories are stored here

extern CSalamanderSafeFile SalSafeFile; // interface for convenient file work

extern const char* SalamanderConfigurationRoots[];                                                           // see mainwnd2.cpp
BOOL GetUpgradeInfo(BOOL* autoImportConfig, char* autoImportConfigFromKey, int autoImportConfigFromKeySize); // see mainwnd2.cpp
BOOL FindLatestConfiguration(BOOL* deleteConfigurations, const char*& loadConfiguration);                    // see mainwnd2.cpp
BOOL FindLanguageFromPrevVerOfSal(char* slgName);                                                            // see mainwnd2.cpp

// creates and attaches a special class to the edit line/combobox 'ctrlID'
// enabling key interception and sending the WM_USER_KEYDOWN message to the dialog 'hDialog'
// LOWORD(wParam) contains ctrlID and lParam the pressed key (wParam from WM_KEYDOWN/WM_SYSKEYDOWN)
BOOL CreateKeyForwarder(HWND hDialog, int ctrlID);
// call after WM_USER_KEYDOWN is received; returns TRUE if the key was handled
DWORD OnDirectoryKeyDown(DWORD keyCode, HWND hDialog, int editID, int editBufSize, int buttonID);
// call after WM_USER_BUTTON is received, opens the menu for button 'buttonID'
// and then fills the edit line 'editID'
void OnDirectoryButton(HWND hDialog, int editID, int editBufSize, int buttonID, WPARAM wParam, LPARAM lParam);

// call after WM_USER_BUTTON is received; enables Ctrl+A on systems prior to Windows Vista where it didn't work everywhere
DWORD OnKeyDownHandleSelectAll(DWORD keyCode, HWND hDialog, int editID);

//  returns TRUE if the hotKey belongs to Salamander
BOOL IsSalHotKey(WORD hotKey);

void GetNetworkDrives(DWORD& netDrives, char (*netRemotePath)[MAX_PATH]);
void GetNetworkDrivesBody(DWORD& netDrives, char (*netRemotePath)[MAX_PATH], char* buffer); // for internal use in bug reports only

// returns the SID (as a string) for our process
// the returned SID must be freed using LocalFree
//   LPTSTR sid;
//   if (GetStringSid(&sid))
//     LocalFree(sid);
BOOL GetStringSid(LPTSTR* stringSid);

// returns an MD5 hash computed from the SID, producing a 16-byte array
// 'sidMD5' must point to a 16-byte array
// returns TRUE on success, otherwise FALSE and zeroes the entire 'sidMD5'
BOOL GetSidMD5(BYTE* sidMD5);

// prepares SECURITY_ATTRIBUTES so that an object (mutex, mapped memory) created with them is secure
// this removes WRITE_DAC | WRITE_OWNER from Everyone; otherwise everything is allowed
// this is better security than "NULL DACL", where the object is fully open
// can be called on any OS; returns a pointer from W2K upward, otherwise NULL
// if 'psidEveryone' or 'paclNewDacl' are returned non-NULL, they must be destroyed
SECURITY_ATTRIBUTES* CreateAccessableSecurityAttributes(SECURITY_ATTRIBUTES* sa, SECURITY_DESCRIPTOR* sd,
                                                        DWORD allowedAccessMask, PSID* psidEveryone, PACL* paclNewDacl);

// On success returns TRUE and fills the DWORD referenced by 'integrityLevel'
// otherwise (on failure or on OS older than Vista) returns FALSE
BOOL GetProcessIntegrityLevel(DWORD* integrityLevel);

// same function as the API GetProcessId(), but works under W2K as well
DWORD SalGetProcessId(HANDLE hProcess);

// Call after launching a process; stores environment variable differences
// so RegenEnvironmentVariables() can work later
void InitEnvironmentVariablesDifferences();

// loads current environment variables and applies the differences
void RegenEnvironmentVariables();

// attempt to detect an SSD, see CSalamanderGeneralAbstract::IsPathOnSSD() for more
BOOL IsPathOnSSD(const char* path);
