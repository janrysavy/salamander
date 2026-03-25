// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

//****************************************************************************
//
// Copyright (c) 2023 Open Salamander Authors
//
// This is a part of the Open Salamander SDK library.
//
//****************************************************************************

// "light" version of WinLib

#pragma once

// macros for suppressing unnecessary parts of WinLibLT (easier compilation):
// ENABLE_PROPERTYDIALOG - if defined, it is possible to use the property sheet dialog (CPropertyDialog)

// configure custom texts for WinLib
void SetWinLibStrings(const char* invalidNumber, // "not a number" (for numeric transfer buffers)
                      const char* error);        // title "error" (for numeric transfer buffers)

// must be called before WinLib is used; 'pluginName' is the plugin name (e.g. "DEMOPLUG"),
// used to differentiate the class names of WinLib universal windows (they must differ between plugins,
// otherwise a class-name collision occurs and WinLib cannot work—the first plugin started would be the only one functioning);
// 'dllInstance' is the plugin module (used when registering WinLib universal classes)
BOOL InitializeWinLib(const char* pluginName, HINSTANCE dllInstance);
// must be called after using WinLib; 'dllInstance' is the plugin module (used when unregistering WinLib's universal classes)
void ReleaseWinLib(HINSTANCE dllInstance);

// callback type for HTML Help integration
typedef void(WINAPI* FWinLibLTHelpCallback)(HWND hWindow, UINT helpID);

// set the callback for connecting to HTML Help
void SetupWinLibHelp(FWinLibLTHelpCallback helpCallback);

// constants for WinLib strings (internal use within WinLib only)
enum CWLS
{
    WLS_INVALID_NUMBER,
    WLS_ERROR,

    WLS_COUNT
};

extern char CWINDOW_CLASSNAME[100];  // class name of the universal window
extern char CWINDOW_CLASSNAME2[100]; // char array holding the window class name for the universal window - without CS_VREDRAW | CS_HREDRAW

// ****************************************************************************

enum CObjectOrigin // used when windows and dialogs are destroyed
{
    ooAllocated, // will be deallocated during WM_DESTROY
    ooStatic,    // WM_DESTROY sets HWindow to NULL
    ooStandard   // for modal dlg => ooStatic, for modeless dlg => ooAllocated
};

// ****************************************************************************

enum CObjectType // used to identify the object type
{
    otBase,
    otWindow,
    otDialog,
#ifdef ENABLE_PROPERTYDIALOG
    otPropSheetPage,
#endif // ENABLE_PROPERTYDIALOG
    otLastWinLibObject
};

// ****************************************************************************

class CWindowsObject // base class of all MS-Windows objects
{
public:
    HWND HWindow;
    UINT HelpID; // -1 = no value (help not used)

    CWindowsObject(CObjectOrigin origin)
    {
        HWindow = NULL;
        ObjectOrigin = origin;
        HelpID = -1;
    }
    CWindowsObject(UINT helpID, CObjectOrigin origin)
    {
        HWindow = NULL;
        ObjectOrigin = origin;
        SetHelpID(helpID);
    }

    virtual ~CWindowsObject() {} // so derived class destructors are called

    virtual BOOL Is(int) { return FALSE; } // object identification
    virtual int GetObjectType() { return otBase; }

    virtual BOOL IsAllocated() { return ObjectOrigin == ooAllocated; }
    void SetObjectOrigin(CObjectOrigin origin) { ObjectOrigin = origin; }

    void SetHelpID(UINT helpID)
    {
        if (helpID == -1)
            TRACE_E("CWindowsObject::SetHelpID(): helpID==-1, -1 is 'empty value', you should use another helpID! If you want to set HelpID to -1, use ClearHelpID().");
        HelpID = helpID;
    }
    void ClearHelpID() { HelpID = -1; }

protected:
    CObjectOrigin ObjectOrigin;
};

// ****************************************************************************

class CWindow : public CWindowsObject
{
public:
    CWindow(CObjectOrigin origin = ooAllocated) : CWindowsObject(origin) { DefWndProc = DefWindowProc; }
    CWindow(HWND hDlg, int ctrlID, CObjectOrigin origin = ooAllocated)
        : CWindowsObject(origin)
    {
        DefWndProc = DefWindowProc;
        AttachToControl(hDlg, ctrlID);
    }
    CWindow(HWND hDlg, int ctrlID, UINT helpID, CObjectOrigin origin = ooAllocated)
        : CWindowsObject(helpID, origin)
    {
        DefWndProc = DefWindowProc;
        AttachToControl(hDlg, ctrlID);
    }

    virtual BOOL Is(int type) { return type == otWindow; }
    virtual int GetObjectType() { return otWindow; }

    // registers the WinLib universal classes, called automatically (unregistration is also automatic)
    static BOOL RegisterUniversalClass(HINSTANCE dllInstance);

    // registers a custom universal class; WARNING: when the plugin is unloaded, the class must be unregistered,
    // otherwise registering it again when the plugin is reloaded will fail because of a conflict with the old class
    static BOOL RegisterUniversalClass(UINT style,
                                       int cbClsExtra,
                                       int cbWndExtra,
                                       HINSTANCE dllInstance,
                                       HICON hIcon,
                                       HCURSOR hCursor,
                                       HBRUSH hbrBackground,
                                       LPCTSTR lpszMenuName,
                                       LPCTSTR lpszClassName,
                                       HICON hIconSm);

    HWND Create(LPCTSTR lpszClassName,  // address of registered class name
                LPCTSTR lpszWindowName, // address of window name
                DWORD dwStyle,          // window style
                int x,                  // horizontal position of window
                int y,                  // vertical position of window
                int nWidth,             // window width
                int nHeight,            // window height
                HWND hwndParent,        // handle of parent or owner window
                HMENU hmenu,            // handle of menu or child-window identifier
                HINSTANCE hinst,        // handle of application instance
                LPVOID lpvParam);       // pointer to the window object being created

    HWND CreateEx(DWORD dwExStyle,        // extended window style
                  LPCTSTR lpszClassName,  // address of registered class name
                  LPCTSTR lpszWindowName, // address of window name
                  DWORD dwStyle,          // window style
                  int x,                  // horizontal position of window
                  int y,                  // vertical position of window
                  int nWidth,             // window width
                  int nHeight,            // window height
                  HWND hwndParent,        // handle of parent or owner window
                  HMENU hmenu,            // handle of menu or child-window identifier
                  HINSTANCE hinst,        // handle of application instance
                  LPVOID lpvParam);       // pointer to the window object being created

    void AttachToWindow(HWND hWnd);
    void AttachToControl(HWND dlg, int ctrlID);
    void DetachWindow();

    static LRESULT CALLBACK CWindowProc(HWND hwnd, UINT uMsg,
                                        WPARAM wParam, LPARAM lParam);

protected:
    virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    WNDPROC DefWndProc;
};

// ****************************************************************************

enum CTransferType
{
    ttDataToWindow,  // data is transferred to the window
    ttDataFromWindow // data come from the window
};

// ****************************************************************************

class CTransferInfo
{
public:
    int FailCtrlID; // INT_MAX - everything is OK; otherwise, the ID of the control with the error
    CTransferType Type;

    CTransferInfo(HWND hDialog, CTransferType type)
    {
        HDialog = hDialog;
        FailCtrlID = INT_MAX;
        Type = type;
    }

    BOOL IsGood() { return FailCtrlID == INT_MAX; }
    void ErrorOn(int ctrlID) { FailCtrlID = ctrlID; }
    BOOL GetControl(HWND& ctrlHWnd, int ctrlID, BOOL ignoreIsGood = FALSE);
    void EnsureControlIsFocused(int ctrlID);

    void EditLine(int ctrlID, char* buffer, DWORD bufferSize, BOOL select = TRUE);
    void RadioButton(int ctrlID, int ctrlValue, int& value);
    void CheckBox(int ctrlID, int& value); // 0-unchecked, 1-checked, 2-grayed

    // validates a double value (if it is not a number, validation fails); the decimal separator can be '.' or ',';
    // 'format' is used by sprintf when converting the number to a string (e.g. "%.2f" or "%g")
    void EditLine(int ctrlID, double& value, char* format, BOOL select = TRUE);

    // validates an integer value (if it is not numeric, validation fails)
    void EditLine(int ctrlID, int& value, BOOL select = TRUE);

protected:
    HWND HDialog; // handle of the dialog for which the transfer is performed
};

// ****************************************************************************

class CDialog : public CWindowsObject
{
public:
#ifdef ENABLE_PROPERTYDIALOG
    CWindowsObject::HWindow;         // for CPropSheetPage to compile
    CWindowsObject::SetObjectOrigin; // for CPropSheetPage to compile
#endif                               // ENABLE_PROPERTYDIALOG

    CDialog(HINSTANCE modul, int resID, HWND parent,
            CObjectOrigin origin = ooStandard) : CWindowsObject(origin)
    {
        Modal = 0;
        Modul = modul;
        ResID = resID;
        Parent = parent;
    }
    CDialog(HINSTANCE modul, int resID, UINT helpID, HWND parent,
            CObjectOrigin origin = ooStandard) : CWindowsObject(helpID, origin)
    {
        Modal = 0;
        Modul = modul;
        ResID = resID;
        Parent = parent;
    }

    virtual BOOL ValidateData();
    virtual void Validate(CTransferInfo& /*ti*/) {}
    virtual BOOL TransferData(CTransferType type);
    virtual void Transfer(CTransferInfo& /*ti*/) {}

    virtual BOOL Is(int type) { return type == otDialog; }
    virtual int GetObjectType() { return otDialog; }

    virtual BOOL IsAllocated() { return ObjectOrigin == ooAllocated ||
                                        (!Modal && ObjectOrigin == ooStandard); }

    void SetParent(HWND parent) { Parent = parent; }
    INT_PTR Execute(); // modal dialog
    HWND Create();     // modeless dialog

    static INT_PTR CALLBACK CDialogProc(HWND hwndDlg, UINT uMsg,
                                        WPARAM wParam, LPARAM lParam);

protected:
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    virtual void NotifDlgJustCreated() {}

    BOOL Modal; // determines how the dialog is destroyed
    HINSTANCE Modul;
    int ResID;
    HWND Parent;
};

// ****************************************************************************

#ifdef ENABLE_PROPERTYDIALOG

class CPropertyDialog;

class CPropSheetPage : protected CDialog
{
public:
    CDialog::HWindow; // keep HWindow available

    CDialog::SetObjectOrigin; // expose allowed base-class methods
    CDialog::Transfer;

    // tested with a property page dialog resource with the style:
    // DS_CONTROL | DS_3DLOOK | WS_CHILD | WS_CAPTION;
    // if we want to use the title directly from the resource, set 'title'==NULL and
    // 'flags'==0
    CPropSheetPage(char* title, HINSTANCE modul, int resID,
                   DWORD flags /* = PSP_USETITLE*/, HICON icon,
                   CObjectOrigin origin = ooStatic);
    CPropSheetPage(char* title, HINSTANCE modul, int resID, int helpID,
                   DWORD flags /* = PSP_USETITLE*/, HICON icon,
                   CObjectOrigin origin = ooStatic);
    ~CPropSheetPage();

    void Init(char* title, HINSTANCE modul, int resID,
              HICON icon, DWORD flags, CObjectOrigin origin);

    virtual BOOL ValidateData();
    virtual BOOL TransferData(CTransferType type);

    HPROPSHEETPAGE CreatePropSheetPage();
    virtual BOOL Is(int type) { return type == otPropSheetPage || CDialog::Is(type); }
    virtual int GetObjectType() { return otPropSheetPage; }
    virtual BOOL IsAllocated() { return ObjectOrigin == ooAllocated; }

    static INT_PTR CALLBACK CPropSheetPageProc(HWND hwndDlg, UINT uMsg,
                                               WPARAM wParam, LPARAM lParam);

protected:
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    char* Title;
    DWORD Flags;
    HICON Icon;

    CPropertyDialog* ParentDialog; // owner of this page

    friend class CPropertyDialog;
};

// ****************************************************************************

class CPropertyDialog : public TIndirectArray<CPropSheetPage>
{
public:
    // it is best to add the individual page objects to this object
    // and then add them via the Add method as "static" (the default option);
    // 'startPage' and 'lastPage' can be a single variable (value in/reference out);
    // for 'flags', see the help for 'PROPSHEETHEADER'; the most useful constants are
    // PSH_NOAPPLYNOW, PSH_USECALLBACK, and PSH_HASHELP (otherwise 'flags'==0 is sufficient)
    CPropertyDialog(HWND parent, HINSTANCE modul, char* caption,
                    int startPage, DWORD flags, HICON icon = NULL,
                    DWORD* lastPage = NULL, PFNPROPSHEETCALLBACK callback = NULL)
        : TIndirectArray<CPropSheetPage>(10, 5, dtNoDelete)
    {
        Parent = parent;
        HWindow = NULL;
        Modul = modul;
        Icon = icon;
        Caption = caption;
        StartPage = startPage;
        Flags = flags;
        LastPage = lastPage;
        Callback = callback;
    }

    virtual INT_PTR Execute();

    virtual int GetCurSel();

protected:
    HWND Parent; // parameters for creating the dialog
    HWND HWindow;
    HINSTANCE Modul;
    HICON Icon;
    char* Caption;
    int StartPage;
    DWORD Flags;
    PFNPROPSHEETCALLBACK Callback;

    DWORD* LastPage; // last selected page (can be NULL if it is not needed)

    friend class CPropSheetPage;
};

#endif // ENABLE_PROPERTYDIALOG

// ****************************************************************************

class CWindowsManager
{
public:
    int WindowsCount; // number of windows handled by WinLib (current state)

public:
    CWindowsManager() { WindowsCount = 0; }

    BOOL AddWindow(HWND hWnd, CWindowsObject* wnd);
    void DetachWindow(HWND hWnd);
    CWindowsObject* GetWindowPtr(HWND hWnd);
};

// ****************************************************************************

struct CWindowQueueItem
{
    HWND HWindow;
    CWindowQueueItem* Next;

    CWindowQueueItem(HWND hWindow)
    {
        HWindow = hWindow;
        Next = NULL;
    }
};

class CWindowQueue
{
protected:
    const char* QueueName; // queue name (debugging only)
    CWindowQueueItem* Head;

    struct CCS // access from multiple threads -> synchronization required
    {
        CRITICAL_SECTION cs;

        CCS() { InitializeCriticalSection(&cs); }
        ~CCS() { DeleteCriticalSection(&cs); }

        void Enter() { EnterCriticalSection(&cs); }
        void Leave() { LeaveCriticalSection(&cs); }
    } CS;

public:
    CWindowQueue(const char* queueName /* e.g. "DemoPlug Viewers" */)
    {
        QueueName = queueName;
        Head = NULL;
    }
    ~CWindowQueue();

    BOOL Add(CWindowQueueItem* item); // adds an item to the queue; returns TRUE on success
    void Remove(HWND hWindow);        // remove an item from the queue
    BOOL Empty();                     // returns TRUE if the queue is empty

    // posts a message to all windows (using PostMessage - the windows may be in different threads)
    void BroadcastMessage(DWORD uMsg, WPARAM wParam, LPARAM lParam);

    // broadcasts WM_CLOSE, then waits for the queue to become empty (up to either 'forceWaitTime'
    // or 'waitTime', depending on 'force'); returns TRUE if the queue is empty (all windows closed)
    // or if 'force' is TRUE; INFINITE means waiting indefinitely
    // Note: when 'force' is TRUE, it always returns TRUE, so there is no point in waiting, therefore forceWaitTime = 0
    BOOL CloseAllWindows(BOOL force, int waitTime = 1000, int forceWaitTime = 0);
};

// ****************************************************************************

extern CWindowsManager WindowsManager;
