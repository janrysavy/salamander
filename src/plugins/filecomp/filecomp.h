// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// menu ID definitions
#define MID_COMPAREFILES 1

#define CURRENT_CONFIG_VERSION_PRESEPARATEOPTIONS 6
#define CURRENT_CONFIG_VERSION_NORECOMPAREBUTTON 7
#define CURRENT_CONFIG_VERSION 8

// plugin interface object, its methods are called from Salamander
class CPluginInterface;
extern CPluginInterface PluginInterface;
extern BOOL AlwaysOnTop;

extern BOOL LoadOnStart;

// ****************************************************************************
//
// plugin interface
//

class CPluginInterface : public CPluginInterfaceAbstract
{
public:
    virtual void WINAPI About(HWND parent);

    virtual BOOL WINAPI Release(HWND parent, BOOL force);

    virtual void WINAPI LoadConfiguration(HWND parent, HKEY regKey, CSalamanderRegistryAbstract* registry);
    virtual void WINAPI SaveConfiguration(HWND parent, HKEY regKey, CSalamanderRegistryAbstract* registry);
    virtual void WINAPI Configuration(HWND parent);

    virtual void WINAPI Connect(HWND parent, CSalamanderConnectAbstract* salamander);

    virtual void WINAPI ReleasePluginDataInterface(CPluginDataInterfaceAbstract* pluginData) { return; }

    virtual CPluginInterfaceForArchiverAbstract* WINAPI GetInterfaceForArchiver() { return NULL; };
    virtual CPluginInterfaceForViewerAbstract* WINAPI GetInterfaceForViewer() { return NULL; }
    virtual CPluginInterfaceForMenuExtAbstract* WINAPI GetInterfaceForMenuExt();
    virtual CPluginInterfaceForFSAbstract* WINAPI GetInterfaceForFS() { return NULL; }
    virtual CPluginInterfaceForThumbLoaderAbstract* WINAPI GetInterfaceForThumbLoader() { return NULL; }
    virtual void WINAPI Event(int event, DWORD param);
    virtual void WINAPI ClearHistory(HWND parent);
    virtual void WINAPI AcceptChangeOnPathNotification(const char* path, BOOL includingSubdirs) {}
    virtual void WINAPI PasswordManagerEvent(HWND parent, int event) {}
};

class CPluginInterfaceForMenu : public CPluginInterfaceForMenuExtAbstract
{
public:
    // returns the state of the menu item with identifier 'id'; the return value is a combination
    // of flags (see MENU_ITEM_STATE_XXX); 'eventMask' see CSalamanderConnectAbstract::AddMenuItem
    virtual DWORD WINAPI GetMenuItemState(int id, DWORD eventMask) { return 0; }

    // runs the menu command identified by 'id'; see CSalamanderConnectAbstract::AddMenuItem for 'eventMask'.
    // 'salamander' is the set of usable Salamander methods for performing operations, 'parent' is the
    // parent of the message box. Returns TRUE if the panel selection should be cleared (Cancel was not
    // used, Skip might have been), otherwise returns FALSE (do not clear the selection);
    // NOTE: If the command makes changes on any path (disk/FS), it should call
    //       CSalamanderGeneralAbstract::PostChangeOnPathNotification to notify panels without automatic refresh
    //       and open FS (both active and detached)
    virtual BOOL WINAPI ExecuteMenuItem(CSalamanderForOperationsAbstract* salamander, HWND parent,
                                        int id, DWORD eventMask);
    virtual BOOL WINAPI HelpForMenuItem(HWND parent, int id);
    virtual void WINAPI BuildMenu(HWND parent, CSalamanderBuildMenuAbstract* salamander) {}
};

// ****************************************************************************
//
// CFileCompThread
//

class CFilecompThread : public CThread
{
public:
    char Path1[MAX_PATH];
    char Path2[MAX_PATH];
    BOOL DontConfirmSelection;
    char ReleaseEvent[20];

    CFilecompThread(const char* file1, const char* file2, BOOL dontConfirmSelection,
                    const char* releaseEvent) : CThread("Filecomp Thread")
    {
        strcpy(Path1, file1);
        strcpy(Path2, file2);
        DontConfirmSelection = dontConfirmSelection;
        strcpy(ReleaseEvent, releaseEvent);
    }

    virtual unsigned Body();
};

extern CWindowQueue MainWindowQueue; // list of all filecomp windows
extern CThreadQueue ThreadQueue;     // list of all filecomp windows and workers plus remote control
