// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

//******************************************************************************
//
// CComboboxEdit
//
// The standard CB_GETEDITSEL message cannot determine the selection
// after the combobox loses focus because the control is buggy.
// This helper control works around that problem.
//

class CComboboxEdit : public CWindow
{
protected:
    DWORD SelStart;
    DWORD SelEnd;

public:
    CComboboxEdit();

    void GetSel(DWORD* start, DWORD* end);

    void ReplaceText(const char* text);

protected:
    virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

//******************************************************************************
//
// Keywords
//

extern const char* EXECUTE_DRIVE;
extern const char* EXECUTE_PATH;
extern const char* EXECUTE_DOSPATH;
extern const char* EXECUTE_NAME;
extern const char* EXECUTE_DOSNAME;

extern const char* EXECUTE_ENV; // Environment Variable

extern const char* EXECUTE_SEPARATOR;    // Insert a separator into the menu
extern const char* EXECUTE_BROWSE;       // Insert a Browse command into the menu
extern const char* EXECUTE_HELP;         // Display help
extern const char* EXECUTE_TERMINATOR;   // End of menu definition
extern const char* EXECUTE_SUBMENUSTART; // Start of a submenu (only one level supported)
extern const char* EXECUTE_SUBMENUEND;   // End of a submenu (only one level supported)

//******************************************************************************
//
// CExecuteItem
//

// By default the selected item replaces the current text in the edit line,
// the caret is positioned after the inserted text,
// and the edit line receives focus.
// The following flags modify this default behavior:
#define EIF_REPLACE_ALL 0x01 // replace the entire contents of the edit line
#define EIF_CURSOR_1 0x02    // place the caret one character before the end of the inserted text
#define EIF_CURSOR_2 0x04    // place the caret two characters before the end of the inserted text
#define EIF_VARIABLE 0x08    // wrap the inserted text in $(text)
#define EIF_DONT_FOCUS 0x10  // do not move focus back to the edit line after the action

struct CExecuteItem
{
    const char* Keyword; // string inserted into the edit line
    int NameResID;       // resource string displayed in the menu
    BYTE Flags;          // EIF_xxxx
};

//******************************************************************************
//
// Predefined arrays
//

extern CExecuteItem UserMenuArgsExecutes[];
extern CExecuteItem HotPathItems[];
extern CExecuteItem CommandExecutes[];
extern CExecuteItem ArgumentsExecutes[];
extern CExecuteItem InitDirExecutes[];
extern CExecuteItem InfoLineContentItems[];
extern CExecuteItem MakeFileListItems[];
extern CExecuteItem RegularExpressionItems[];

//******************************************************************************
//
// Helper data structures
//

struct CUserMenuValidationData // additional data used to validate User Menu arguments
{
    BOOL UsesListOfSelNames;     // TRUE if this parameter is used; check that it is not too long
    BOOL UsesListOfSelFullNames; // TRUE if this parameter is used; check that it is not too long
    BOOL UsesFullPathLeft;       // TRUE if this parameter is used; verify it is defined
    BOOL UsesFullPathRight;      // TRUE if this parameter is used; verify it is defined
    BOOL UsesFullPathInactive;   // TRUE if this parameter is used; verify it is defined

    BOOL MustHandleItemsAsGroup;     // TRUE if items must be processed as a group: ListOfSelectedNames, ListOfSelectedFullNames, FileToCompareXXX, DirToCompareXXX
    BOOL MustHandleItemsOneByOne;    // TRUE if items must be processed individually: FullName, Name, NamePart, ExtPart, DOSFullName, DOSName, DOSNamePart, DOSExtPart
    int UsedCompareType;             // 0 = none yet, 1 = file-left-right, 2 = file-active-inactive, 3 = dir-left-right, 4 = dir-active-inactive, 5 = multiple types (invalid), 6 = file-or-dir-left-right, 7 = file-or-dir-active-inactive
    BOOL UsedCompareLeftOrActive;    // TRUE if at least one compare-left or compare-active variable is used (must use both or neither)
    BOOL UsedCompareRightOrInactive; // TRUE if at least one compare-right or compare-inactive variable is used (must use both or neither)
};

struct CUserMenuAdvancedData // additional data used only for the User Menu's Arguments array
{
    // precomputed values for some parameters:
    char ListOfSelNames[USRMNUARGS_MAXLEN];     // empty string means no or too many selections (longer than USRMNUARGS_MAXLEN); see ListOfSelNamesIsEmpty
    BOOL ListOfSelNamesIsEmpty;                 // TRUE if ListOfSelNames is empty
    char ListOfSelFullNames[USRMNUARGS_MAXLEN]; // empty string means none or too many selections (longer than USRMNUARGS_MAXLEN); see ListOfSelFullNamesIsEmpty
    BOOL ListOfSelFullNamesIsEmpty;             // TRUE if ListOfSelFullNames is empty
    char FullPathLeft[MAX_PATH];                // empty string = not defined (we are in Find or the panel shows archive/FS)
    char FullPathRight[MAX_PATH];               // empty string = not defined (we are in Find or the panel shows archive/FS)
    const char* FullPathInactive;               // points to FullPathLeft or FullPathRight: empty string = not defined (Find or archive/FS panel)
    char CompareName1[MAX_PATH];                // first full name for compare (file or directory)
    char CompareName2[MAX_PATH];                // second full name for compare (file or directory)
    BOOL CompareNamesAreDirs;                   // TRUE if CompareName1 and CompareName2 are directories; otherwise files
    BOOL CompareNamesReversed;                  // TRUE if compare names come from different panels and CompareName1 is from the right panel
};

//******************************************************************************
//
// Custom functions
//

// Displays a popup menu with the supplied list and inserts the selected text
// into the edit line.
// hParent:          dialog containing the edit line or combobox and the Browse button
// buttonResID:      resource ID of the Browse button
// editlineResID:    resource ID of the edit line or combobox
// combobox:         if TRUE, editlineResID identifies an edit line; otherwise a combobox
//                   when TRUE, the edit line should have the CComboboxEdit control attached
// executeItems:     array used to populate the menu
// filterResID:      text for the browse window opened from the menu in a special case
// replaceWholeText: if TRUE, replace the entire contents of the edit line; otherwise
//                   replace only the selection
const CExecuteItem* TrackExecuteMenu(HWND hParent, int buttonResID, int editlineResID,
                                     BOOL combobox, CExecuteItem* executeItems, int filterResID = 0);

// Opens a FileOpen dialog for *.exe
// Inserts the selected file into the edit line
// Returns TRUE if the user chose a file; otherwise FALSE
BOOL BrowseCommand(HWND hParent, int editlineResID, int filterResID);

// Validates varText containing variables from the UserMenuArgsExecutes array
// msgParent - parent message box for errors; if NULL, errors are not shown
BOOL ValidateUserMenuArguments(HWND msgParent, const char* varText, int& errorPos1, int& errorPos2,
                               CUserMenuValidationData* userMenuValidationData);

// Expands varText containing variables from UserMenuArgsExecutes and stores the result in buffer
// msgParent - parent message box for errors; if NULL, errors are not shown
// if 'fileNameUsed' is not NULL it is set to TRUE when a path or file name is used
BOOL ExpandUserMenuArguments(HWND msgParent, const char* name, const char* dosName, const char* varText,
                             char* buffer, int bufferLen, BOOL* fileNameUsed,
                             CUserMenuAdvancedData* userMenuAdvancedData,
                             BOOL ignoreEnvVarNotFoundOrTooLong);

// Validates varText containing variables from the Command array
// msgParent - parent message box for errors; if NULL, errors are not shown
BOOL ValidateCommandFile(HWND msgParent, const char* varText, int& errorPos1, int& errorPos2);

// Validates varText containing variables from the HotPath array
// msgParent - parent message box for errors; if NULL, errors are not shown
BOOL ValidateHotPath(HWND msgParent, const char* varText, int& errorPos1, int& errorPos2);

// Validates varText containing variables from the ArgumentsExecutes array
// msgParent - parent message box for errors; if NULL, errors are not shown
BOOL ValidateArguments(HWND msgParent, const char* varText, int& errorPos1, int& errorPos2);

// Expands varText containing variables from the ArgumentsExecutes array and stores the result in buffer
// msgParent - parent message box for errors; if NULL, errors are not shown
// if 'fileNameUsed' is not NULL it is set to TRUE when a path or file name is used
BOOL ExpandArguments(HWND msgParent, const char* name, const char* dosName, const char* varText,
                     char* buffer, int bufferLen, BOOL* fileNameUsed);

// Validates varText containing variables from the InfoLineContentItems array
// msgParent - parent message box for errors; if NULL, errors are not shown
BOOL ValidateInfoLineItems(HWND msgParent, const char* varText, int& errorPos1, int& errorPos2);

// Expands varText containing variables from the InfoLineContentItems array and stores the result in buffer
// msgParent - parent message box for errors; if NULL, errors are not shown
// varPlacements: array with [varPlacementsCount] items; filled with positions of variables
//                in the output buffer (LOWORD) and their lengths (HIWORD)

BOOL ExpandInfoLineItems(HWND msgParent, const char* varText, CPluginDataInterfaceEncapsulation* pluginData,
                         CFileData* fData, BOOL isDir, char* buffer, int bufferLen, DWORD* varPlacements,
                         int* varPlacementsCount, DWORD validFileData, BOOL isDisk);

// Validates varText containing variables from the MakeFileListItems array
// msgParent - parent message box for errors; if NULL, errors are not shown
BOOL ValidateMakeFileList(HWND msgParent, const char* varText, int& errorPos1, int& errorPos2);

// Expands varText containing variables from MakeFileListItems and stores the result in buffer
// msgParent - parent message box for errors; if NULL, errors are not shown
// maxVarSizes: array with [maxVarSizesCount] items. If the corresponding variable
//              uses the ":max" modifier and its length exceeds the array item,
//              the array is updated with that length.
//              When detectMaxVarSizes == TRUE, the maximum length is used for column formatting
BOOL ExpandMakeFileList(HWND msgParent, const char* varText, CPluginDataInterfaceEncapsulation* pluginData,
                        CFileData* fData, BOOL isDir, char* buffer, int bufferLen, BOOL detectMaxVarSizes,
                        int* maxVarSizes, int maxVarSizesCount, DWORD validFileData, const char* path,
                        BOOL ignoreEnvVarNotFoundOrTooLong);

// Validates varText containing variables from the InitDirExecutes array
// msgParent - parent message box for errors; if NULL, errors are not shown
BOOL ValidateInitDir(HWND msgParent, const char* varText, int& errorPos1, int& errorPos2);

// Expands varText containing variables from the InitDirExecutes array and stores the result in buffer
// msgParent - parent message box for errors; if NULL, errors are not shown
BOOL ExpandInitDir(HWND msgParent, const char* name, const char* dosName, const char* varText,
                   char* buffer, int bufferLen, BOOL ignoreEnvVarNotFoundOrTooLong);

// Expands varText containing environment variables and stores the result in buffer
// msgParent - parent message box for errors; if NULL, errors are not shown
BOOL ExpandCommand(HWND msgParent, const char* varText, char* buffer, int bufferLen,
                   BOOL ignoreEnvVarNotFoundOrTooLong);

// Expands varText containing environment variables and stores the result in buffer
// msgParent - parent message box for errors; if NULL, errors are not shown
BOOL ExpandHotPath(HWND msgParent, const char* varText, char* buffer, int bufferLen,
                   BOOL ignoreEnvVarNotFoundOrTooLong);
