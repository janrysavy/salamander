// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

//****************************************************************************
//
// Copyright (c) 2023 Open Salamander Authors
//
// This is a part of the Open Salamander SDK library.
//
//****************************************************************************

#pragma once

#ifdef _MSC_VER
#pragma pack(push, enter_include_spl_com) // so that the structures are independent of the alignment settings
#pragma pack(4)
#pragma warning(3 : 4706) // warning C4706: assignment within conditional expression
#endif                    // _MSC_VER
#ifdef __BORLANDC__
#pragma option -a4
#endif // __BORLANDC__

// The plugin must define the SalamanderVersion (int) variable and initialize it in SalamanderPluginEntry:
// SalamanderVersion = salamander->GetVersion();

// Global variable with the version of the Salamander in which this plugin is loaded
extern int SalamanderVersion;

//
// ****************************************************************************
// CSalamanderDirectoryAbstract
//
// The class represents a directory structure—files and directories on requested paths, the root path is "",
// separators in the path are backslashes ('\\')
//

// CQuadWord - 64-bit unsigned integer for file sizes
// tips:
//  -faster passing of an input parameter of type CQuadWord: const CQuadWord&
//  -assign a 64-bit integer: quadWord.Value = XXX;
//  -compute a size ratio: quadWord1.GetDouble() / quadWord2.GetDouble()  // the loss of precision before the division is minimal (max. 1e-15)
//  -truncate to DWORD: (DWORD)quadWord.Value
//  -convert (unsigned) __int64 to CQuadWord: CQuadWord().SetUI64(XXX)

struct CQuadWord
{
    union
    {
        struct
        {
            DWORD LoDWord;
            DWORD HiDWord;
        };
        unsigned __int64 Value;
    };

    // WARNING: Do not add an assignment operator or a constructor that takes a single DWORD here,
    //          otherwise the use of 8-byte numbers becomes completely uncontrollable (C++ converts
    //          everything implicitly, which might not be what you want)

    CQuadWord() {}
    CQuadWord(DWORD lo, DWORD hi)
    {
        LoDWord = lo;
        HiDWord = hi;
    }
    CQuadWord(const CQuadWord& qw)
    {
        LoDWord = qw.LoDWord;
        HiDWord = qw.HiDWord;
    }

    CQuadWord& Set(DWORD lo, DWORD hi)
    {
        LoDWord = lo;
        HiDWord = hi;
        return *this;
    }
    CQuadWord& SetUI64(unsigned __int64 val)
    {
        Value = val;
        return *this;
    }
    CQuadWord& SetDouble(double val)
    {
        Value = (unsigned __int64)val;
        return *this;
    }

    CQuadWord& operator++()
    {
        ++Value;
        return *this;
    } // prefix ++
    CQuadWord& operator--()
    {
        --Value;
        return *this;
    } // prefix --

    CQuadWord operator+(const CQuadWord& qw) const
    {
        CQuadWord qwr;
        qwr.Value = Value + qw.Value;
        return qwr;
    }
    CQuadWord operator-(const CQuadWord& qw) const
    {
        CQuadWord qwr;
        qwr.Value = Value - qw.Value;
        return qwr;
    }
    CQuadWord operator*(const CQuadWord& qw) const
    {
        CQuadWord qwr;
        qwr.Value = Value * qw.Value;
        return qwr;
    }
    CQuadWord operator/(const CQuadWord& qw) const
    {
        CQuadWord qwr;
        qwr.Value = Value / qw.Value;
        return qwr;
    }
    CQuadWord operator%(const CQuadWord& qw) const
    {
        CQuadWord qwr;
        qwr.Value = Value % qw.Value;
        return qwr;
    }
    CQuadWord operator<<(const int num) const
    {
        CQuadWord qwr;
        qwr.Value = Value << num;
        return qwr;
    }
    CQuadWord operator>>(const int num) const
    {
        CQuadWord qwr;
        qwr.Value = Value >> num;
        return qwr;
    }

    CQuadWord& operator+=(const CQuadWord& qw)
    {
        Value += qw.Value;
        return *this;
    }
    CQuadWord& operator-=(const CQuadWord& qw)
    {
        Value -= qw.Value;
        return *this;
    }
    CQuadWord& operator*=(const CQuadWord& qw)
    {
        Value *= qw.Value;
        return *this;
    }
    CQuadWord& operator/=(const CQuadWord& qw)
    {
        Value /= qw.Value;
        return *this;
    }
    CQuadWord& operator%=(const CQuadWord& qw)
    {
        Value %= qw.Value;
        return *this;
    }
    CQuadWord& operator<<=(const int num)
    {
        Value <<= num;
        return *this;
    }
    CQuadWord& operator>>=(const int num)
    {
        Value >>= num;
        return *this;
    }

    BOOL operator==(const CQuadWord& qw) const { return Value == qw.Value; }
    BOOL operator!=(const CQuadWord& qw) const { return Value != qw.Value; }
    BOOL operator<(const CQuadWord& qw) const { return Value < qw.Value; }
    BOOL operator>(const CQuadWord& qw) const { return Value > qw.Value; }
    BOOL operator<=(const CQuadWord& qw) const { return Value <= qw.Value; }
    BOOL operator>=(const CQuadWord& qw) const { return Value >= qw.Value; }

    // conversion to double (beware of precision loss for large numbers—double has only 15 significant digits)
    double GetDouble() const
    { // MSVC cannot convert unsigned __int64 to double, so we have to do it ourselves
        if (Value < CQuadWord(0, 0x80000000).Value)
            return (double)(__int64)Value; // positive number
        else
            return 9223372036854775808.0 + (double)(__int64)(Value - CQuadWord(0, 0x80000000).Value);
    }
};

#define QW_MAX CQuadWord(0xFFFFFFFF, 0xFFFFFFFF)

#define ICONOVERLAYINDEX_NOTUSED 15 // value for CFileData::IconOverlayIndex when the icon has no overlay

// record of every file and directory in Salamander (basic information about the file/directory)
struct CFileData // do not add a destructor here!
{
    char* Name;                    // allocated file name (without path), must be allocated on Salamander's
                                   // heap (see CSalamanderGeneralAbstract::Alloc/Realloc/Free)
    char* Ext;                     // pointer into Name to the first dot from the right (including the dot at the start of the name,
                                   // on Windows it is considered an extension, unlike UNIX) or to the end of Name if the extension
                                   // does not exist; if the configuration has FALSE set for SALCFG_SORTBYEXTDIRSASFILES, Ext for
                                   // directories points to the end of Name (directories do not have extensions)
    CQuadWord Size;                // file size in bytes
    DWORD Attr;                    // file attributes - ORed FILE_ATTRIBUTE_XXX constants
    FILETIME LastWrite;            // time of the last write to the file (UTC-based time)
    char* DosName;                 // allocated DOS 8.3 file name, NULL if not needed, must
                                   // be allocated on Salamander's heap (see CSalamanderGeneralAbstract::Alloc/Realloc/Free)
    DWORD_PTR PluginData;          // used by the plugin via CPluginDataInterfaceAbstract, ignored by Salamander
    unsigned NameLen : 9;          // length of the Name string (strlen(Name)) - WARNING: the maximum name length is (MAX_PATH - 5)
    unsigned Hidden : 1;           // is it hidden? (if 1, the icon is 50% more transparent - ghosted)
    unsigned IsLink : 1;           // is it a link? (if 1, the icon has a link overlay) - standard filling see CSalamanderGeneralAbstract::IsFileLink(CFileData::Ext), when displayed it has priority over IsOffline, but IconOverlayIndex takes precedence
    unsigned IsOffline : 1;        // is it offline? (if 1, the icon has an offline overlay - black clock), when displayed IsLink and IconOverlayIndex take precedence
    unsigned IconOverlayIndex : 4; // index of the icon overlay (if the icon has no overlay, the value here is ICONOVERLAYINDEX_NOTUSED), when displayed it has priority over IsLink and IsOffline

    // flags for internal use in Salamander: they are reset when added to CSalamanderDirectoryAbstract
    unsigned Association : 1;     // meaningful only for displaying 'simple icons' - icon of the associated file, otherwise 0
    unsigned Selected : 1;        // read-only selection flag (0 - item not selected, 1 - item selected)
    unsigned Shared : 1;          // is the directory shared? not used for files
    unsigned Archive : 1;         // is it an archive? used to display the archive icon in the panel
    unsigned SizeValid : 1;       // has the directory size been computed?
    unsigned Dirty : 1;           // does this item need to be redrawn? (temporary validity only; between setting the bit and redrawing the panel the message queue must not be pumped, otherwise the icon (icon reader) might be redrawn and thus the bit reset! as a result the item is not redrawn)
    unsigned CutToClip : 1;       // is it CUT to the clipboard? (if 1, the icon is 50% more transparent - ghosted)
    unsigned IconOverlayDone : 1; // only for the icon-reader thread: are we obtaining or have we already obtained the icon overlay? (0 - no, 1 - yes)
};

// constants determining the validity of the data stored directly in CFileData (size, extension, etc.)
// or automatically generated from directly stored data (file type is generated from the extension);
// Name + NameLen are mandatory (must always be valid); the plugin manages the validity of PluginData itself
// (Salamander ignores this attribute)
#define VALID_DATA_EXTENSION 0x0001   // the extension is stored in Ext (without: all Ext point to the end of Name)
#define VALID_DATA_DOSNAME 0x0002     // the DOS name is stored in DosName (without: all DosName are NULL)
#define VALID_DATA_SIZE 0x0004        // the size in bytes is stored in Size (without: all Size are 0)
#define VALID_DATA_TYPE 0x0008        // the file type can be generated from Ext (without: it is not generated)
#define VALID_DATA_DATE 0x0010        // the modification date (UTC-based) is stored in LastWrite (without: all dates in LastWrite are 1.1.1602 in local time)
#define VALID_DATA_TIME 0x0020        // the modification time (UTC-based) is stored in LastWrite (without: all times in LastWrite are 0:00:00 in local time)
#define VALID_DATA_ATTRIBUTES 0x0040  // the attributes are stored in Attr (ORed Win32 API constants FILE_ATTRIBUTE_XXX) (without: all Attr are 0)
#define VALID_DATA_HIDDEN 0x0080      // the "ghosted" icon flag is stored in Hidden (without: all Hidden are 0)
#define VALID_DATA_ISLINK 0x0100      // IsLink contains 1 if it is a link, the icon has a link overlay (without: all IsLink are 0)
#define VALID_DATA_ISOFFLINE 0x0200   // IsOffline contains 1 if it is an offline file/directory, the icon has an offline overlay (without: all IsOffline are 0)
#define VALID_DATA_PL_SIZE 0x0400     // meaningful only without using VALID_DATA_SIZE: the plugin stores the size in bytes for at least some files/directories (somewhere in PluginData); to obtain this size Salamander calls CPluginDataInterfaceAbstract::GetByteSize()
#define VALID_DATA_PL_DATE 0x0800     // meaningful only without using VALID_DATA_DATE: the plugin stores the modification date for at least some files/directories (somewhere in PluginData); to obtain this value Salamander calls CPluginDataInterfaceAbstract::GetLastWriteDate()
#define VALID_DATA_PL_TIME 0x1000     // meaningful only without using VALID_DATA_TIME: the plugin stores the modification time for at least some files/directories (somewhere in PluginData); to obtain this value Salamander calls CPluginDataInterfaceAbstract::GetLastWriteTime()
#define VALID_DATA_ICONOVERLAY 0x2000 // IconOverlayIndex is the icon overlay index (no overlay = value ICONOVERLAYINDEX_NOTUSED) (without: all IconOverlayIndex are ICONOVERLAYINDEX_NOTUSED), icon assignment see CSalamanderGeneralAbstract::SetPluginIconOverlays

#define VALID_DATA_NONE 0 // helper constant - only Name and NameLen are valid

#ifdef INSIDE_SALAMANDER
// VALID_DATA_ALL and VALID_DATA_ALL_FS_ARC are for internal use in Salamander (the core) only;
// plugins OR together only the constants corresponding to the data provided by the plugin (this prevents issues
// when additional constants and corresponding data are introduced)
#define VALID_DATA_ALL 0xFFFF
#define VALID_DATA_ALL_FS_ARC (0xFFFF & ~VALID_DATA_ICONOVERLAY) // for file systems and archives: everything except icon overlays
#endif                                                           // INSIDE_SALAMANDER

// If hiding hidden and system files and directories is enabled, items with Hidden==1 and Attr containing
// FILE_ATTRIBUTE_HIDDEN and/or FILE_ATTRIBUTE_SYSTEM are not displayed in the panels.

// flag constants for CSalamanderDirectoryAbstract:
// file and directory names (including in paths) should be compared case-sensitively (without this flag the
// comparison is case-insensitive—the standard behavior on Windows)
#define SALDIRFLAG_CASESENSITIVE 0x0001
// subdirectory names within each directory will not be checked for duplicates (this test is time-consuming
// and is needed only in archives when items are added not just into the root—to allow, for example, adding
// "file1" to "dir1" followed by adding "dir1"—"dir1" is added by the first operation (a missing path is
// created automatically), the second operation only refreshes the data about "dir1" (it must not add it again))
#define SALDIRFLAG_IGNOREDUPDIRS 0x0002

class CPluginDataInterfaceAbstract;

class CSalamanderDirectoryAbstract
{
public:
    // clears the entire object and prepares it for reuse; if 'pluginData' is not NULL it is used
    // to release plugin-specific data (CFileData::PluginData) for files and directories;
    // sets the default value of the valid data mask (the sum of all VALID_DATA_XXX except
    // VALID_DATA_ICONOVERLAY) and the object flags (see the SetFlags method)
    virtual void WINAPI Clear(CPluginDataInterfaceAbstract* pluginData) = 0;

    // defines the valid data mask that determines which CFileData members are valid
    // and which should only be "zeroed" (see the comment for VALID_DATA_XXX); the 'validData'
    // mask contains ORed VALID_DATA_XXX values; the default value of the mask is the sum of all
    // VALID_DATA_XXX except VALID_DATA_ICONOVERLAY; the valid data mask must be set
    // before calling AddFile/AddDir
    virtual void WINAPI SetValidData(DWORD validData) = 0;

    // sets flags for this object; 'flags' is a combination of ORed SALDIRFLAG_XXX flags;
    // the default value of the object flags is zero for archivers (no flag is set)
    // and SALDIRFLAG_IGNOREDUPDIRS for file systems (items may only be added into the root,
    // so checking directory duplicates is pointless)
    virtual void WINAPI SetFlags(DWORD flags) = 0;

    // adds a file at the specified path (relative to this "Salamander directory"), returns success;
    // the path string is used only inside the function, the contents of the file structure are used outside the function as well
    // (do not release the memory allocated for the variables within the structure)
    // if the operation fails, the contents of the file structure must be released;
    // the 'pluginData' parameter is non-NULL only for archives (file systems use only an empty 'path' (==NULL));
    // if 'pluginData' is not NULL, 'pluginData' is used when creating new directories (if the
    // 'path' does not exist), see CPluginDataInterfaceAbstract::GetFileDataForNewDir;
    // uniqueness of the file name on the 'path' is not checked
    virtual BOOL WINAPI AddFile(const char* path, CFileData& file, CPluginDataInterfaceAbstract* pluginData) = 0;

    // adds a directory at the specified path (relative to this "Salamander directory"), returns success;
    // the path string is used only inside the function, the contents of the file structure are used outside the function as well
    // (do not release the memory allocated for the variables within the structure)
    // if the operation fails, the contents of the file structure must be released;
    // the 'pluginData' parameter is non-NULL only for archives (file systems use only an empty 'path' (==NULL));
    // if 'pluginData' is not NULL, it is used when creating new directories (if 'path' does not exist),
    // see CPluginDataInterfaceAbstract::GetFileDataForNewDir;
    // uniqueness of the directory name on the 'path' is checked; if an existing directory is added again,
    // the original data is released (if 'pluginData' is not NULL, CPluginDataInterfaceAbstract::ReleasePluginData is called to release the data)
    // and replaced with data from 'dir' (this is necessary to refresh data for directories that were created automatically when the 'path' did not exist);
    // special case for file systems (or an object allocated via CSalamanderGeneralAbstract::AllocSalamanderDirectory
    // with 'isForFS'==TRUE): if dir.Name is "..", the directory is added as an up-dir (there may be only one,
    // it is always displayed at the beginning of the listing and has a special icon)
    virtual BOOL WINAPI AddDir(const char* path, CFileData& dir, CPluginDataInterfaceAbstract* pluginData) = 0;

    // returns the number of files in the object
    virtual int WINAPI GetFilesCount() const = 0;

    // returns the number of directories in the object
    virtual int WINAPI GetDirsCount() const = 0;

    // returns the file at index 'index'; the returned data may only be used for reading
    virtual CFileData const* WINAPI GetFile(int index) const = 0;

    // returns the directory at index 'index'; the returned data may only be used for reading
    virtual CFileData const* WINAPI GetDir(int index) const = 0;

    // returns the CSalamanderDirectory object for the directory at index 'index'; the returned object may only
    // be used for reading (objects for empty directories are not allocated, a single global empty object is returned—
    // modifying this object would have a global effect)
    virtual CSalamanderDirectoryAbstract const* WINAPI GetSalDir(int index) const = 0;

    // Allows the plugin to provide the expected number of files and directories in this directory in advance.
    // Salamander adjusts its reallocation strategy so that adding items is not slowed down too much.
    // It makes sense to call this for directories containing thousands of files or directories. In the case of tens
    // of thousands, calling this method is almost mandatory; otherwise reallocations will take several seconds.
    // 'files' and 'dirs' therefore express the approximate total number of files and directories.
    // If any of the values is -1, Salamander ignores it.
    // The method should only be called if the directory is empty, i.e. AddFile or AddDir has not been called yet.
    virtual void WINAPI SetApproximateCount(int files, int dirs) = 0;
};

//
// ****************************************************************************
// SalEnumSelection and SalEnumSelection2
//

// constants returned from SalEnumSelection and SalEnumSelection2 in the 'errorOccured' parameter
#define SALENUM_SUCCESS 0 // no error occurred
#define SALENUM_ERROR 1   // an error occurred and the user wants to continue (only the faulty files/directories were skipped)
#define SALENUM_CANCEL 2  // an error occurred and the user wants to cancel the operation

// enumerator; returns file names and ends by returning NULL
// 'enumFiles' == -1 -> resets the enumeration (after this call the enumeration starts from the beginning again), all
//                      other parameters (except for 'param') are ignored, it has no return value (sets
//                      everything to zero)
// 'enumFiles' == 0 -> enumerates files and subdirectories only from the root
// 'enumFiles' == 1 -> enumerates all files and subdirectories
// 'enumFiles' == 2 -> enumerates all subdirectories, files only from the root
// an error can occur only for 'enumFiles' == 1 or 'enumFiles' == 2 ('enumFiles' == 0 does not complete
// names and paths); 'parent' is the parent of potential error message boxes (NULL means do not show
// errors); in 'isDir' (if not NULL) it returns TRUE if the item is a directory; in 'size' (if not NULL) it returns
// the file size (for directories the size is returned only for 'enumFiles' == 0—otherwise it is zero)
// if 'fileData' is not NULL, it returns a pointer to the CFileData structure of the returned
// file/directory (if the enumerator returns NULL, 'fileData' also receives NULL)
// 'param' is the 'nextParam' parameter passed along with the pointer to a function of this
// type; in 'errorOccured' (if not NULL) SALENUM_ERROR is returned if a name that was too long was encountered
// while building the returned names and the user decided to skip only the faulty files/directories.
// WARNING: the error does not apply to the name just returned—that one is OK; in 'errorOccured' (if not NULL)
// SALENUM_CANCEL is returned if the user decided to cancel the operation when an error occurred;
// in that case the enumerator returns NULL (stops); in 'errorOccured' (if not NULL) SALENUM_SUCCESS is returned if
// no error occurred
typedef const char*(WINAPI* SalEnumSelection)(HWND parent, int enumFiles, BOOL* isDir, CQuadWord* size,
                                              const CFileData** fileData, void* param, int* errorOccured);

// enumerator; returns file names and ends by returning NULL
// 'enumFiles' == -1 -> resets the enumeration (after this call the enumeration starts from the beginning again), all
//                      other parameters (except for 'param') are ignored, it has no return value (sets
//                      everything to zero)
// 'enumFiles' == 0 -> enumerates files and subdirectories only from the root
// 'enumFiles' == 1 -> enumerates all files and subdirectories
// 'enumFiles' == 2 -> enumerates all subdirectories, files only from the root
// 'enumFiles' == 3 -> enumerates all files and subdirectories, and symbolic links to files have
//                     the size of the target file (for 'enumFiles' == 1 they have the size of the link, which is
//                     usually zero); WARNING: 'enumFiles' must stay 3 for all calls of the enumerator
// an error can occur only for 'enumFiles' == 1, 2, or 3 ('enumFiles' == 0 does not
// work with the disk at all and does not complete names or paths); 'parent' is the parent of possible
// error message boxes (NULL means do not show errors); in 'dosName' (if not NULL) it returns the DOS name
// (8.3; only if it exists, otherwise NULL); in 'isDir' (if not NULL) it returns TRUE if the item is a directory;
// in 'size' (if not NULL) it returns the file size (zero for directories); in 'attr' (if not NULL)
// it returns the file/directory attributes; in 'lastWrite' (if not NULL) it returns the time of the last write
// to the file/directory; 'param' is the 'nextParam' parameter passed along with the pointer to a function
// of this type; in 'errorOccured' (if not NULL) SALENUM_ERROR is returned if an error occurred while reading
// data from the disk or if a name that was too long was encountered while building the returned names
// and the user decided to skip only the faulty files/directories. WARNING: the error does not apply to the
// name just returned—that one is OK; in 'errorOccured' (if not NULL) SALENUM_CANCEL is returned if the
// user decided to cancel the operation when an error occurred; in that case the enumerator returns NULL
// (stops); in 'errorOccured' (if not NULL) SALENUM_SUCCESS is returned if no error occurred
typedef const char*(WINAPI* SalEnumSelection2)(HWND parent, int enumFiles, const char** dosName,
                                               BOOL* isDir, CQuadWord* size, DWORD* attr,
                                               FILETIME* lastWrite, void* param, int* errorOccured);

//
// ****************************************************************************
// CSalamanderViewAbstract
//
// set of Salamander methods for working with panel columns (turning them off/on/adding/configuring)

// panel view modes
#define VIEW_MODE_TREE 1
#define VIEW_MODE_BRIEF 2
#define VIEW_MODE_DETAILED 3
#define VIEW_MODE_ICONS 4
#define VIEW_MODE_THUMBNAILS 5
#define VIEW_MODE_TILES 6

#define TRANSFER_BUFFER_MAX 1024 // buffer size for transferring column contents from the plugin to Salamander
#define COLUMN_NAME_MAX 30
#define COLUMN_DESCRIPTION_MAX 100

// Column identifiers. Columns inserted by a plugin have ID == COLUMN_ID_CUSTOM.
// Salamander's standard columns use the remaining IDs.
#define COLUMN_ID_CUSTOM 0 // column provided by the plugin—the plugin is responsible for storing its data
#define COLUMN_ID_NAME 1   // left-aligned, supports FixedWidth
// left-aligned, supports FixedWidth; standalone "Ext" column, may only be at index == 1
// if the column does not exist and VALID_DATA_EXTENSION is set in the panel data
// (see CSalamanderDirectoryAbstract::SetValidData()), the "Ext" column is shown within the "Name" column
#define COLUMN_ID_EXTENSION 2
#define COLUMN_ID_DOSNAME 3     // left-aligned
#define COLUMN_ID_SIZE 4        // right-aligned
#define COLUMN_ID_TYPE 5        // left-aligned, supports FixedWidth
#define COLUMN_ID_DATE 6        // right-aligned
#define COLUMN_ID_TIME 7        // right-aligned
#define COLUMN_ID_ATTRIBUTES 8  // right-aligned
#define COLUMN_ID_DESCRIPTION 9 // left-aligned, supports FixedWidth

// Callback for filling the buffer with characters that should be shown in the respective column.
// For optimization the function does not receive/return values via parameters,
// but through global variables (CSalamanderViewAbstract::GetTransferVariables).
typedef void(WINAPI* FColumnGetText)();

// Callback for obtaining indices of simple icons for file systems with custom icons (pitFromPlugin).
// For optimization the function does not receive/return values via parameters,
// but through global variables (CSalamanderViewAbstract::GetTransferVariables).
// From the global variables the callback uses only TransferFileData and TransferIsDir.
typedef int(WINAPI* FGetPluginIconIndex)();

// A column can be created in two ways:
// 1) Salamander creates the column according to the template of the current view.
//    In this case the 'GetText' pointer (to the filling function) points into Salamander
//    and obtains texts directly from CFileData.
//    The 'ID' member then has a value other than COLUMN_ID_CUSTOM.
//
// 2) The plugin adds the column according to its own needs.
//    'GetText' points into the plugin and 'ID' equals COLUMN_ID_CUSTOM.

struct CColumn
{
    char Name[COLUMN_NAME_MAX]; // "Name", "Ext", "Size", ... column title
                                // under which the column appears in the view and the menu
                                // Must not be an empty string.
                                // WARNING: It may contain (after the first null terminator)
                                // also the title of the "Ext" column—this happens if a standalone
                                // "Ext" column is missing and VALID_DATA_EXTENSION is set
                                // in the panel data (see CSalamanderDirectoryAbstract::SetValidData()).
                                // Use CSalamanderGeneralAbstract::AddStrToStr() to concatenate the two strings.

    char Description[COLUMN_DESCRIPTION_MAX]; // Tooltip in the header line
                                              // Must not be an empty string.
                                              // WARNING: It may contain (after the first null terminator)
                                              // also the description of the "Ext" column—this happens if a standalone
                                              // "Ext" column is missing and VALID_DATA_EXTENSION is set
                                              // in the panel data (see CSalamanderDirectoryAbstract::SetValidData()).
                                              // Use CSalamanderGeneralAbstract::AddStrToStr() to concatenate the two strings.

    FColumnGetText GetText; // callback for obtaining the text (see the FColumnGetText type declaration)

    // FIXME_X64 - small for a pointer, is it ever needed?
    DWORD CustomData; // Not used by Salamander; the plugin can
                      // use it to distinguish its added columns.

    unsigned SupportSorting : 1; // can the column be sorted?

    unsigned LeftAlignment : 1; // TRUE for left alignment; otherwise right-aligned

    unsigned ID : 4; // column identifier
                     // For standard columns provided by Salamander
                     // contains values different from COLUMN_ID_CUSTOM.
                     // For columns added by the plugin it always
                     // contains the value COLUMN_ID_CUSTOM.

    // Members Width and FixedWidth can be changed by the user while working with the panel.
    // Standard columns provided by Salamander automatically store/load
    // these values.
    // The plugin must store/load the values for columns it provides
    // itself.
    // Columns whose width is computed by Salamander based on their content
    // and cannot be changed by the user are called 'elastic'. Columns whose width
    // the user can set are called 'fixed'.
    unsigned Width : 16;     // Column width if it is in the fixed (user-adjustable) width mode.
    unsigned FixedWidth : 1; // Is the column in the fixed (user-adjustable) width mode?

    // working variables (not stored anywhere and no initialization is needed)
    // intended for Salamander's internal use; plugins ignore them
    // because their values are not guaranteed when the plugin is called
    unsigned MinWidth : 16; // Minimum width to which the column may be shrunk.
                            // Calculated from the column title and whether it supports sorting
                            // so that the column header is always readable
};

// Through this interface the plugin can change the panel view mode when the path changes.
// All column-related work concerns only the detailed modes
// (Detailed + Types + the three optional Alt+8/9/0 modes). When the path changes
// the plugin receives the standard column set generated from the template
// of the current view. The plugin may modify this set. The modification is not permanent
// and on the next path change the plugin receives the standard set again. This allows it
// to remove a standard column, for example. Before Salamander repopulates the standard columns
// the plugin gets a chance to store information about its columns (COLUMN_ID_CUSTOM).
// That way it can store their 'Width' and 'FixedWidth' values that the user might have set in the panel
// (see ColumnFixedWidthShouldChange() and ColumnWidthWasChanged() in the
// CPluginDataInterfaceAbstract interface). If the plugin changes the view mode, the change is permanent
// (for example, switching to the Thumbnails mode persists even after leaving the plugin's path).

class CSalamanderViewAbstract
{
public:
    // -------------- panel ----------------

    // returns the mode in which the panel is displayed (tree/brief/detailed/icons/thumbnails/tiles)
    // returns one of the VIEW_MODE_xxxx values (the Detailed, Types, and the three optional modes
    // are all VIEW_MODE_DETAILED)
    virtual DWORD WINAPI GetViewMode() = 0;

    // Sets the panel mode to 'viewMode'. If it is one of the detailed modes, it can
    // remove some of the standard columns (see 'validData'). Therefore this function
    // should be called first—before the other interface methods that modify columns.
    //
    // 'viewMode' is one of the VIEW_MODE_xxxx values
    // The panel mode cannot be switched directly to Types or one of the three optional detailed modes
    // (they are all represented by VIEW_MODE_DETAILED, the constant used for the Detailed panel mode).
    // However, if one of these four modes is currently selected and 'viewMode' equals
    // VIEW_MODE_DETAILED, the existing mode stays selected (it does not switch to the plain Detailed mode).
    // The change of the panel mode is persistent (it remains even after leaving the plugin path).
    //
    // 'validData' specifies which data the plugin wants to show in detailed mode; the value is ANDed
    // with the valid data mask set via CSalamanderDirectoryAbstract::SetValidData
    // (there is no point in showing columns whose values are all zeroed).
    virtual void WINAPI SetViewMode(DWORD viewMode, DWORD validData) = 0;

    // Retrieves from Salamander the locations of variables that replace the parameters of the
    // CColumn::GetText callback. Salamander stores these as global variables. The plugin saves
    // the pointers to its own global variables.
    //
    // variables:
    //   transferFileData        [IN]     data describing the item that should be rendered
    //   transferIsDir           [IN]     equals 0 for a file (entry from the Files array),
    //                                    equals 1 for a directory (entry from the Dirs array),
    //                                    equals 2 for the up-directory symbol
    //   transferBuffer          [OUT]    buffer to receive the data, at most TRANSFER_BUFFER_MAX characters
    //                                    there is no need to terminate it with zero
    //   transferLen             [OUT]    before returning from the callback this variable is set
    //                                    to the number of characters written without the terminator
    //                                    (there is no need to store the terminator in the buffer)
    //   transferRowData         [IN/OUT] points to a DWORD that is zeroed for every row before drawing the columns;
    //                                    can be used for optimizations
    //                                    Salamander reserves bits 0x00000001 to 0x00000008.
    //                                    The remaining bits are available to the plugin.
    //   transferPluginDataIface [IN]     plugin-data interface of the panel where the item
    //                                    is drawn (matches (*transferFileData)->PluginData)
    //   transferActCustomData   [IN]     CustomData of the column whose text is being retrieved (the callback target)
    virtual void WINAPI GetTransferVariables(const CFileData**& transferFileData,
                                             int*& transferIsDir,
                                             char*& transferBuffer,
                                             int*& transferLen,
                                             DWORD*& transferRowData,
                                             CPluginDataInterfaceAbstract**& transferPluginDataIface,
                                             DWORD*& transferActCustomData) = 0;

    // for file systems with custom icons only (pitFromPlugin):
    // Sets the callback for obtaining simple icon indices (see
    // CPluginDataInterfaceAbstract::GetSimplePluginIcons). If the plugin does not set this callback,
    // the icon at index 0 is always drawn.
    // From the global variables the callback uses only TransferFileData and TransferIsDir.
    virtual void WINAPI SetPluginSimpleIconCallback(FGetPluginIconIndex callback) = 0;

    // ------------- columns ---------------

    // returns the number of columns in the panel (always at least one, because the name is always shown)
    virtual int WINAPI GetColumnsCount() = 0;

    // returns a pointer to a column (read-only)
    // 'index' specifies which column is returned; returns NULL if the column does not exist
    virtual const CColumn* WINAPI GetColumn(int index) = 0;

    // Inserts a column at position 'index'. Column Name is always at position 0,
    // if the Ext column is shown it must be at position 1. Otherwise the column can be placed
    // anywhere. The 'column' structure is copied into Salamander's internal structures.
    // Returns TRUE if the column was inserted.
    virtual BOOL WINAPI InsertColumn(int index, const CColumn* column) = 0;

    // Inserts a standard column with ID 'id' at position 'index'. Column Name is always
    // at position 0; if the Ext column is inserted it must be at position 1.
    // Otherwise the column can be placed anywhere. 'id' is one of the COLUMN_ID_xxxx values,
    // except COLUMN_ID_CUSTOM and COLUMN_ID_NAME.
    virtual BOOL WINAPI InsertStandardColumn(int index, DWORD id) = 0;

    // Sets the column title and description (must not be empty strings or NULL). Lengths
    // are limited to COLUMN_NAME_MAX and COLUMN_DESCRIPTION_MAX. Returns success.
    // WARNING: The title and description of the "Name" column may contain (always after the first
    // null terminator) also the title and description of the "Ext" column—this happens if
    // there is no standalone "Ext" column and VALID_DATA_EXTENSION is set
    // in the panel data (see CSalamanderDirectoryAbstract::SetValidData()).
    // In that case you must provide a double string (two null terminators)
    // see CSalamanderGeneralAbstract::AddStrToStr().
    virtual BOOL WINAPI SetColumnName(int index, const char* name, const char* description) = 0;

    // Removes the column at position 'index'. Both plugin-added columns
    // and Salamander standard columns can be removed. Column 'Name' cannot be removed; it is always
    // at index 0. Be careful when removing column 'Ext': if VALID_DATA_EXTENSION is set
    // in the plugin data (see CSalamanderDirectoryAbstract::SetValidData()),
    // the title/description of the 'Ext' column must appear within the 'Name' column.
    virtual BOOL WINAPI DeleteColumn(int index) = 0;
};

//
// ****************************************************************************
// CPluginDataInterfaceAbstract
//
// set of plugin methods Salamander needs to obtain plugin-specific data
// for columns added by the plugin (works with CFileData::PluginData)

class CPluginInterfaceAbstract;

class CPluginDataInterfaceAbstract
{
#ifdef INSIDE_SALAMANDER
private: // protection against improper direct method calls (see CPluginDataInterfaceEncapsulation)
    friend class CPluginDataInterfaceEncapsulation;
#else  // INSIDE_SALAMANDER
public:
#endif // INSIDE_SALAMANDER

    // returns TRUE if ReleasePluginData should be called for all files associated
    // with this interface; otherwise returns FALSE
    virtual BOOL WINAPI CallReleaseForFiles() = 0;

    // returns TRUE if ReleasePluginData should be called for all directories associated
    // with this interface; otherwise returns FALSE
    virtual BOOL WINAPI CallReleaseForDirs() = 0;

    // releases plugin-specific data (CFileData::PluginData) for 'file' (file or directory—
    // 'isDir' FALSE or TRUE; the structure inserted into CSalamanderDirectoryAbstract
    // while listing archives or file systems); called for all files if CallReleaseForFiles
    // returns TRUE, and for all directories if CallReleaseForDirs returns TRUE
    virtual void WINAPI ReleasePluginData(CFileData& file, BOOL isDir) = 0;

    // archive data only (file systems do not supply an up-dir symbol):
    // adjusts the proposed content of the up-dir symbol (".." at the top of the panel); 'archivePath'
    // is the path in the archive for which the symbol is intended; 'upDir' contains the proposed
    // symbol data: name ".." (do not change), archive date & time, the rest zeroed;
    // 'upDir' returns the plugin's modifications, primarily it should update 'upDir.PluginData',
    // which will be used for the up-dir symbol when retrieving data for added columns;
    // ReleasePluginData will not be called for 'upDir'; any necessary cleanup can be performed
    // during the next GetFileDataForUpDir call or when the entire interface is released
    // (in its destructor—called from CPluginInterfaceAbstract::ReleasePluginDataInterface)
    virtual void WINAPI GetFileDataForUpDir(const char* archivePath, CFileData& upDir) = 0;

    // archive data only (file systems use only the root path in CSalamanderDirectoryAbstract):
    // when adding a file/directory to CSalamanderDirectoryAbstract the specified path might not exist
    // and therefore needs to be created; individual directories on that path are created automatically,
    // and this method lets the plugin add its specific data (for its columns) to these directories.
    // 'dirName' is the full path of the directory being added in the archive; 'dir' contains the proposed
    // data: directory name (allocated on Salamander's heap), date & time copied from the added file/directory,
    // the rest zeroed; 'dir' returns the plugin's modifications, primarily it should update
    // 'dir.PluginData'; returns TRUE if the plugin's data were added successfully, otherwise FALSE;
    // if TRUE is returned, 'dir' is released through the standard path (Salamander part +
    // ReleasePluginData), either when the entire listing is released or even during its creation
    // if the same directory is later added via CSalamanderDirectoryAbstract::AddDir
    // (overwriting the automatic creation with a regular addition); if FALSE is returned, only
    // Salamander's part of 'dir' is released
    virtual BOOL WINAPI GetFileDataForNewDir(const char* dirName, CFileData& dir) = 0;

    // for file systems with custom icons only (pitFromPlugin):
    // returns an image list with simple icons; while drawing items in the panel the icon index
    // is retrieved through the callback into this image list; called after every new listing
    // is obtained (after CPluginFSInterfaceAbstract::ListCurrentPath),
    // so the image list may be rebuilt for each listing;
    // 'iconSize' specifies the desired icon size and is one of the SALICONSIZE_xxx values
    // the plugin is responsible for destroying the image list during the next GetSimplePluginIcons call
    // or when the entire interface is released (in its destructor—called from
    // CPluginInterfaceAbstract::ReleasePluginDataInterface)
    // if the image list cannot be created, NULL is returned and the current plugin-icons type
    // falls back to pitSimple
    virtual HIMAGELIST WINAPI GetSimplePluginIcons(int iconSize) = 0;

    // for file systems with custom icons only (pitFromPlugin):
    // returns TRUE if a simple icon should be used for the given file/directory ('isDir' FALSE/TRUE) 'file'
    // returns FALSE if the icon should be obtained by calling GetPluginIcon from the icon-loading thread
    // (loading the icon in the background);
    // this method can also precompute the simple icon index
    // (for icons loaded in the background simple icons are used until the icon is loaded)
    // and store it into CFileData (typically CFileData::PluginData);
    // limitation: only methods from CSalamanderGeneralAbstract that may be called from any thread
    // (methods independent of the panel state) may be used
    virtual BOOL WINAPI HasSimplePluginIcon(CFileData& file, BOOL isDir) = 0;

    // for file systems with custom icons only (pitFromPlugin):
    // returns an icon for the file or directory 'file', or NULL if the icon cannot be obtained;
    // if 'destroyIcon' is returned as TRUE, the Win32 API function DestroyIcon must be called to free the icon;
    // 'iconSize' specifies the desired icon size and is one of the SALICONSIZE_xxx values
    // limitation: since it is called from the icon-loading thread (not the main thread),
    // only methods from CSalamanderGeneralAbstract that may be called from any thread may be used
    virtual HICON WINAPI GetPluginIcon(const CFileData* file, int iconSize, BOOL& destroyIcon) = 0;

    // for file systems with custom icons only (pitFromPlugin):
    // compares 'file1' (file or directory) and 'file2' (file or directory);
    // it must never report that any two listing entries are equal (this guarantees a unique
    // mapping of custom icons to files/directories); if duplicate names cannot occur in the listing
    // (the usual case), the implementation can simply be:
    // {return strcmp(file1->Name, file2->Name);}
    // returns a negative value if 'file1' < 'file2', zero if 'file1' == 'file2',
    // and a positive value if 'file1' > 'file2';
    // limitation: because it is also called from the icon-loading thread (not only the main thread),
    // only methods from CSalamanderGeneralAbstract that may be called from any thread may be used
    virtual int WINAPI CompareFilesFromFS(const CFileData* file1, const CFileData* file2) = 0;

    // sets view parameters; this method is called before each new panel content is displayed
    // (when the path changes) and when the current view changes (including manual column width changes);
    // 'leftPanel' is TRUE for the left panel (FALSE for the right panel);
    // 'view' is the interface for modifying the view (setting modes, working with columns);
    // for archive data 'archivePath' contains the current path in the archive, for file-system data it is NULL;
    // for archive data 'upperDir' points to the parent directory (NULL if the current path is the archive root);
    // for file-system data it is always NULL;
    // WARNING: while this method is executing the panel must not be redrawn (icon size etc. may change),
    //          so no message loops (no dialogs, etc.)!
    // limitation: only methods from CSalamanderGeneralAbstract that may be called from any thread
    //             (methods independent of the panel state) may be used
    virtual void WINAPI SetupView(BOOL leftPanel, CSalamanderViewAbstract* view,
                                  const char* archivePath, const CFileData* upperDir) = 0;

    // sets the new value of "column->FixedWidth"—the user used the context menu
    // on a plugin-added column in the header line > "Automatic Column Width"; the plugin
    // should store the new value of column->FixedWidth provided in 'newFixedWidth'
    // (it is always the negation of column->FixedWidth) so that during subsequent SetupView() calls
    // it can add the column with the correct FixedWidth setting; if fixed width is being enabled,
    // the plugin should also store the current "column->Width" (so enabling the fixed width does not
    // change the column width)—ideally by calling
    // "ColumnWidthWasChanged(leftPanel, column, column->Width)"; 'column' identifies
    // the column to be changed; 'leftPanel' is TRUE for a column from the left panel
    // (FALSE for a column from the right panel)
    virtual void WINAPI ColumnFixedWidthShouldChange(BOOL leftPanel, const CColumn* column,
                                                     int newFixedWidth) = 0;

    // sets the new value of "column->Width"—the user changed the width of a plugin-added
    // column in the header line using the mouse; the plugin should store the new column->Width value
    // (also provided in 'newWidth') so that during subsequent SetupView() calls the column can be added
    // with the correct Width; 'column' identifies the column that changed; 'leftPanel'
    // is TRUE for a column from the left panel (FALSE for a column from the right panel)
    virtual void WINAPI ColumnWidthWasChanged(BOOL leftPanel, const CColumn* column,
                                              int newWidth) = 0;

    // obtains the Information Line content for the file/directory ('isDir' TRUE/FALSE) 'file'
    // or for the selected files and directories ('file' is NULL and the counts of selected files/directories
    // are in 'selectedFiles'/'selectedDirs') in the panel ('panel' is one of PANEL_XXX);
    // called even for an empty listing (only for file systems—archives cannot encounter this; 'file' is NULL,
    // 'selectedFiles' and 'selectedDirs' are 0); if 'displaySize' is TRUE, the size
    // of all selected directories is known (see CFileData::SizeValid; TRUE if nothing is selected);
    // 'selectedSize' contains the sum of CFileData::Size values of selected files and directories
    // (zero if nothing is selected); 'buffer' is the buffer for the returned text (size 1000 bytes);
    // 'hotTexts' is an array (size 100 DWORDs) that returns the positions of hot texts—
    // the lower WORD always contains the position of the hot text in 'buffer', the upper WORD contains
    // the length of the hot text; 'hotTextsCount' holds the size of the 'hotTexts' array (100) and returns
    // the number of hot texts written into 'hotTexts'; returns TRUE if 'buffer' + 'hotTexts' +
    // 'hotTextsCount' are filled, returns FALSE if the Information Line should be filled
    // in the standard way (as on disk)
    virtual BOOL WINAPI GetInfoLineContent(int panel, const CFileData* file, BOOL isDir, int selectedFiles,
                                           int selectedDirs, BOOL displaySize, const CQuadWord& selectedSize,
                                           char* buffer, DWORD* hotTexts, int& hotTextsCount) = 0;

    // archives only: the user copied files/directories from the archive to the clipboard and is now closing
    // the archive in the panel; if the method returns TRUE, this object remains open (optimization
    // for a potential Paste from the clipboard—the archive is already listed); if it returns FALSE,
    // the object is released (a subsequent Paste from the clipboard will relist the archive before
    // extracting the selected files/directories); NOTE: if the archive file itself remains open for the lifetime
    // of the object, the method should return FALSE, otherwise the file will stay open the entire time
    // otherwise the archive file would remain open for as long as the data stay on the clipboard (it could not be deleted, etc.)
    virtual BOOL WINAPI CanBeCopiedToClipboard() = 0;

    // only when VALID_DATA_PL_SIZE was specified via CSalamanderDirectoryAbstract::SetValidData():
    // returns TRUE if the size of the file/directory ('isDir' TRUE/FALSE) 'file' is known,
    // otherwise returns FALSE; the size is returned in 'size'
    virtual BOOL WINAPI GetByteSize(const CFileData* file, BOOL isDir, CQuadWord* size) = 0;

    // only when VALID_DATA_PL_DATE was specified via CSalamanderDirectoryAbstract::SetValidData():
    // returns TRUE if the date of the file/directory ('isDir' TRUE/FALSE) 'file' is known,
    // otherwise returns FALSE; the date is returned in the "date" part of the 'date' structure
    // (the "time" part should remain untouched)
    virtual BOOL WINAPI GetLastWriteDate(const CFileData* file, BOOL isDir, SYSTEMTIME* date) = 0;

    // only when VALID_DATA_PL_TIME was specified via CSalamanderDirectoryAbstract::SetValidData():
    // returns TRUE if the time of the file/directory ('isDir' TRUE/FALSE) 'file' is known,
    // otherwise returns FALSE; the time is returned in the "time" part of the 'time' structure
    // (the "date" part should remain untouched)
    virtual BOOL WINAPI GetLastWriteTime(const CFileData* file, BOOL isDir, SYSTEMTIME* time) = 0;
};

//
// ****************************************************************************
// CSalamanderForOperationsAbstract
//
// set of Salamander methods supporting operations; the interface is valid only
// for the method to which it is passed as a parameter, so it can be used only
// from that thread and within that method (the object lives on the stack, so it ends when the call returns)

class CSalamanderForOperationsAbstract
{
public:
    // PROGRESS DIALOG: the dialog contains one or two ('twoProgressBars' FALSE/TRUE) progress bars
    // opens the progress dialog with the title 'title'; 'parent' is the parent window of the dialog (if
    // NULL, the main window is used); if it contains only one progress bar it can be labeled
    // either "File" ('fileProgress' is TRUE) or "Total" ('fileProgress' is FALSE)
    //
    // the dialog does not run in its own thread; for it to function (Cancel button + internal timer)
    // the message queue must occasionally be pumped; this is done by the ProgressDialogAddText,
    // ProgressAddSize, and ProgressSetSize methods
    //
    // because real-time text updates and progress bar changes slow things down significantly,
    // the methods ProgressDialogAddText, ProgressAddSize, and ProgressSetSize have the
    // 'delayedPaint' parameter; it should be TRUE for all rapidly changing texts and values;
    // the methods then cache the texts and display them once the dialog's internal timer fires;
    // set 'delayedPaint' to FALSE for initial/final texts such as "preparing data..."
    // or "canceling operation..." after which the dialog is not given a chance to process messages
    // messages (timer); if such an operation is likely to take a long time, the dialog should be
    // "refreshed" during that time by calling ProgressAddSize(CQuadWord(0, 0), TRUE)
    // and the return value can be used to abort the action early if needed
    virtual void WINAPI OpenProgressDialog(const char* title, BOOL twoProgressBars,
                                           HWND parent, BOOL fileProgress) = 0;
    // prints the text 'txt' (multiple lines allowed—the text is split into lines) into the progress dialog
    virtual void WINAPI ProgressDialogAddText(const char* txt, BOOL delayedPaint) = 0;
    // if 'totalSize1' is not CQuadWord(-1, -1), sets 'totalSize1' as 100 percent of the first progress bar
    // if 'totalSize2' is not CQuadWord(-1, -1), sets 'totalSize2' as 100 percent of the second progress bar
    // (for a progress dialog with one progress bar 'totalSize2' must be CQuadWord(-1, -1))
    virtual void WINAPI ProgressSetTotalSize(const CQuadWord& totalSize1, const CQuadWord& totalSize2) = 0;
    // if 'size1' is not CQuadWord(-1, -1), sets the value 'size1' (size1/total1*100 percent) on the first progress bar
    // if 'size2' is not CQuadWord(-1, -1), sets the value 'size2' (size2/total2*100 percent) on the second progress bar
    // (for a progress dialog with one progress bar 'size2' must be CQuadWord(-1, -1)); returns whether the action should continue
    // (FALSE = stop)
    virtual BOOL WINAPI ProgressSetSize(const CQuadWord& size1, const CQuadWord& size2, BOOL delayedPaint) = 0;
    // adds the amount 'size' (size/total*100 percent progress) to the progress bar(s),
    // returns whether the action should continue (FALSE = stop)
    virtual BOOL WINAPI ProgressAddSize(int size, BOOL delayedPaint) = 0;
    // enables/disables the Cancel button
    virtual void WINAPI ProgressEnableCancel(BOOL enable) = 0;
    // returns the HWND of the progress dialog (useful for error messages and prompts while the dialog is open)
    virtual HWND WINAPI ProgressGetHWND() = 0;
    // closes the progress dialog
    virtual void WINAPI CloseProgressDialog() = 0;

    // moves all files from the 'source' directory to the 'target' directory
    // additionally remaps the displayed name prefixes ('remapNameFrom' -> 'remapNameTo')
    // returns whether the operation succeeded
    virtual BOOL WINAPI MoveFiles(const char* source, const char* target, const char* remapNameFrom,
                                  const char* remapNameTo) = 0;
};

#ifdef _MSC_VER
#pragma pack(pop, enter_include_spl_com)
#endif // _MSC_VER
#ifdef __BORLANDC__
#pragma option -a
#endif // __BORLANDC__
