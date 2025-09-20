// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

void InitShellIconOverlays();
void ReleaseShellIconOverlays();

struct CSQLite3DynLoadBase
{
    BOOL OK; // TRUE if SQLite3 was loaded successfully and is ready for use
    HINSTANCE SQLite3DLL;

    CSQLite3DynLoadBase()
    {
        OK = FALSE;
        SQLite3DLL = NULL;
    }
    ~CSQLite3DynLoadBase()
    {
        if (SQLite3DLL != NULL)
            HANDLES(FreeLibrary(SQLite3DLL));
    }
};

struct CShellIconOverlayItem
{
    char IconOverlayName[MAX_PATH];          // registry key name under HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\Explorer\ShellIconOverlayIdentifiers
    IShellIconOverlayIdentifier* Identifier; // IShellIconOverlayIdentifier object, WARNING: usable only in the main thread
    CLSID IconOverlayIdCLSID;                // CLSID of the corresponding IShellIconOverlayIdentifier object IShellIconOverlayIdentifier
    int Priority;                            // priority of this icon overlay (0-100, highest priority is zero)
    HICON IconOverlay[ICONSIZE_COUNT];       // icon overlay in all sizes
    BOOL GoogleDriveOverlay;                 // TRUE = Google Drive handler (they crash so we synchronize extra)

    void Cleanup();

    CShellIconOverlayItem();
    ~CShellIconOverlayItem();
};

class CShellIconOverlays
{
protected:
    TIndirectArray<CShellIconOverlayItem> Overlays; // priority-sorted list of icon overlays
    CRITICAL_SECTION GD_CS;                         // Google Drive requires mutual exclusion of IsMemberOf calls from both readers (otherwise it crashes)
    BOOL GetGDAlreadyCalled;                        // TRUE if the Google Drive folder path has already been detected
    char GoogleDrivePath[MAX_PATH];                 // Google Drive folder; the handler is not called elsewhere because it is extremely slow and crashes without extra synchronization
    BOOL GoogleDrivePathIsFromCfg;                  // was the path retrieved from Google Drive configuration? (FALSE = only default path may exist and Google Drive might not be installed)
    BOOL GoogleDrivePathExists;                     // does the Google Drive folder exist on disk?

public:
    CShellIconOverlays() : Overlays(1, 5)
    {
        HANDLES(InitializeCriticalSection(&GD_CS));
        GoogleDrivePath[0] = 0;
        GetGDAlreadyCalled = FALSE;
        GoogleDrivePathIsFromCfg = FALSE;
        GoogleDrivePathExists = FALSE;
    }
    ~CShellIconOverlays() { HANDLES(DeleteCriticalSection(&GD_CS)); }

    // adds 'item' to the array (previously insertion by 'priority' was wrong)
    BOOL Add(CShellIconOverlayItem* item /*, int priority*/);

    // releases all icon overlays
    void Release() { Overlays.Destroy(); }

    // allocates an array of IShellIconOverlayIdentifier objects for the calling thread (we use COM
    // in STA threading model, so the object must be created and used only in one thread)
    IShellIconOverlayIdentifier** CreateIconReadersIconOverlayIds();

    // releases the array of IShellIconOverlayIdentifier objects
    void ReleaseIconReadersIconOverlayIds(IShellIconOverlayIdentifier** iconReadersIconOverlayIds);

    // returns the icon-overlay index for the file/directory "wPath+name"
    DWORD GetIconOverlayIndex(WCHAR* wPath, WCHAR* wName, char* aPath, char* aName, char* name,
                              DWORD fileAttrs, int minPriority,
                              IShellIconOverlayIdentifier** iconReadersIconOverlayIds,
                              BOOL isGoogleDrivePath);

    HICON GetIconOverlay(int iconOverlayIndex, CIconSizeEnum iconSize)
    {
        return Overlays[iconOverlayIndex]->IconOverlay[iconSize];
    }

    // called when the display color depth changes, all overlay icons must be reloaded
    // WARNING: may be called only from the main thread
    void ColorsChanged();

    // if not done yet, determine where Google Drive resides; 'sqlite3_Dyn_InOut'
    // serves as a cache for sqlite.dll (if it's already loaded we reuse it, and if loaded
    // in this function we return it)
    void InitGoogleDrivePath(CSQLite3DynLoadBase** sqlite3_Dyn_InOut, BOOL debugTestOverlays);

    BOOL HasGoogleDrivePath();

    BOOL GetPathForGoogleDrive(char* path, int pathLen)
    {
        strcpy_s(path, pathLen, GoogleDrivePath);
        return GoogleDrivePath[0] != 0;
    }

    void SetGoogleDrivePath(const char* path, BOOL pathIsFromConfig)
    {
        strcpy_s(GoogleDrivePath, path);
        GoogleDrivePathIsFromCfg = pathIsFromConfig;
        GoogleDrivePathExists = FALSE;
    }

    BOOL IsGoogleDrivePath(const char* path) { return GoogleDrivePath[0] != 0 && SalPathIsPrefix(GoogleDrivePath, path); }
};

struct CShellIconOverlayItem2 // only a list of icon overlay handlers (for the configuration dialog, Icon Overlays page)
{
    char IconOverlayName[MAX_PATH];  // registry key name under HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\Explorer\ShellIconOverlayIdentifiers
    char IconOverlayDescr[MAX_PATH]; // description of the icon overlay handler COM object
};

extern CShellIconOverlays ShellIconOverlays;                           // array of all available icon overlays
extern TIndirectArray<CShellIconOverlayItem2> ListOfShellIconOverlays; // list of all icon overlay handlers
