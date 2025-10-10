// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// ****************************************************************************

class CPropertyDialog;
class CTreePropHolderDlg;

struct CElasticLayoutCtrl
{
    HWND HCtrl; // handle of the control that will be repositioned or resized
    POINT Pos;  // cached position or offset relative to the moving envelope's bottom edge
};

// helper that keeps dialog controls aligned with the dialog's current size
class CElasticLayout
{
public:
    CElasticLayout(HWND hWindow);
    void AddResizeCtrl(int resID);
    // perform the layout pass using the collected metrics
    void LayoutCtrls();

protected:
    static BOOL CALLBACK FindMoveControls(HWND hChild, LPARAM lParam);

    void FindMoveCtrls();

protected:
    // handle of the dialog whose layout we manage
    HWND HWindow;
    // divider from which controls begin to move (aligned with the bottom edge of the resizable controls)
    // stored in client coordinates (pixels)
    int SplitY;
    // controls stretched as the window grows (commonly a list view)
    TDirectArray<CElasticLayoutCtrl> ResizeCtrls;
    // temporary array populated by FindMoveCtrls; ideally it would be local, but
    // we keep it as a member so the FindMoveControls callback invoked by EnumChildWindows can fill it
    TDirectArray<CElasticLayoutCtrl> MoveCtrls;
};

class CPropSheetPage : protected CDialog
{
public:
    CDialog::SetObjectOrigin; // expose selected base-class methods
    CDialog::Transfer;
    CDialog::HWindow; // keep HWindow accessible too

    CPropSheetPage(const TCHAR* title, HINSTANCE modul, int resID,
                   DWORD flags /* = PSP_USETITLE*/, HICON icon,
                   CObjectOrigin origin = ooStatic);
    CPropSheetPage(const TCHAR* title, HINSTANCE modul, int resID, UINT helpID,
                   DWORD flags /* = PSP_USETITLE*/, HICON icon,
                   CObjectOrigin origin = ooStatic);
    ~CPropSheetPage();

    void Init(const TCHAR* title, HINSTANCE modul, int resID,
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
    BOOL ElasticVerticalLayout(int count, ...);

    TCHAR* Title;
    DWORD Flags;
    HICON Icon;

    CPropertyDialog* ParentDialog; // owner of this page
    // references used by the tree property dialog variant
    CPropSheetPage* ParentPage;
    HTREEITEM HTreeItem;
    BOOL* Expanded;

    // when set, reflows the controls whenever the dialog changes size
    CElasticLayout* ElasticLayout;

    friend class CPropertyDialog;
    friend class CTreePropDialog;
    friend class CTreePropHolderDlg;
};

// ****************************************************************************

class CPropertyDialog : public TIndirectArray<CPropSheetPage>
{
public:
    CPropertyDialog(HWND parent, HINSTANCE modul, const TCHAR* caption,
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
    HWND Parent; // parameters used when creating the dialog
    HWND HWindow;
    HINSTANCE Modul;
    HICON Icon;
    const TCHAR* Caption;
    int StartPage;
    DWORD Flags;
    PFNPROPSHEETCALLBACK Callback;

    DWORD* LastPage; // last selected page (can be NULL if not needed)

    friend class CPropSheetPage;
};

// ****************************************************************************

class CTreePropDialog;

// gray shaded strip above the property sheet in the tree variant of PropertyDialog
// that displays the title of the active page
class CTPHCaptionWindow : protected CWindow
{
protected:
    TCHAR* Text;
    int Allocated;

public:
    CTPHCaptionWindow(HWND hDlg, int ctrlID);
    ~CTPHCaptionWindow();

    void SetText(const TCHAR* text);

protected:
    virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

// restrict the grip control to a vertical resize cursor
class CTPHGripWindow : public CWindow
{
public:
    CTPHGripWindow(HWND hDlg, int ctrlID) : CWindow(hDlg, ctrlID) {};

protected:
    virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

// host dialog that contains the tree view, shaded caption, and current property sheet
class CTreePropHolderDlg : public CDialog
{
protected:
    CTreePropDialog* TPD;
    HWND HTreeView;
    CTPHCaptionWindow* CaptionWindow;
    CTPHGripWindow* GripWindow;
    RECT ChildDialogRect;
    int CurrentPageIndex;
    CPropSheetPage* ChildDialog;
    int ExitButton; // ID of the button that closed the dialog

    // layout metrics in pixels
    SIZE MinWindowSize;  // minimum dialog dimensions (determined by the largest child dialog)
    DWORD* WindowHeight; // pointer to the current dialog height
    int TreeWidth;       // tree-view width derived from the content
    int CaptionHeight;   // caption height
    SIZE ButtonSize;     // button dimensions along the dialog's bottom edge
    int ButtonMargin;    // spacing between buttons
    SIZE GripSize;       // resize grip dimensions for the bottom-right corner of the dialog
    SIZE MarginSize;     // horizontal and vertical dialog margins

public:
    CTreePropHolderDlg(HWND hParent, DWORD* windowHeight);

    int ExecuteIndirect(LPCDLGTEMPLATE hDialogTemplate);

protected:
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    void OnCtrlTab(BOOL shift);
    void LayoutControls();
    int BuildAndMeasureTree();
    void EnableButtons();
    BOOL SelectPage(int pageIndex);

    friend class CTreePropDialog;
};

// container for pages in the tree variant of PropertyDialog
class CTreePropDialog : public CPropertyDialog
{
protected:
    CTreePropHolderDlg Dialog;

public:
    CTreePropDialog(HWND hParent, HINSTANCE hInstance, TCHAR* caption,
                    int startPage, DWORD flags, DWORD* lastPage,
                    DWORD* windowHeight)
        : CPropertyDialog(hParent, hInstance, caption, startPage, flags, NULL, lastPage),
          Dialog(hParent, windowHeight)
    {
        Dialog.TPD = this;
    }

    virtual int Execute(const TCHAR* buttonOK,
                        const TCHAR* buttonCancel,
                        const TCHAR* buttonHelp);
    virtual int GetCurSel();
    int Add(CPropSheetPage* page, CPropSheetPage* parent = NULL, BOOL* expanded = NULL);

protected:
    WORD* lpdwAlign(WORD* lpIn);
    int AddItemEx(LPWORD& lpw, const TCHAR* className, WORD id, int x, int y, int cx, int cy,
                  UINT style, UINT exStyle, const TCHAR* text);

    //    DLGTEMPLATE *DoLockDlgRes(int page);
    friend class CTreePropHolderDlg;

    // used solely to forward messages from CTreePropHolderDlg
    virtual void DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {};
};
