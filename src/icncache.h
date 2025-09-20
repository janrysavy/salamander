// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

//
// ****************************************************************************
// CIconData
//

class CIconData
{
public:
    char* NameAndData;           // allocated in DWORD chunks, zero-terminated for comparisons;
                                 // for Flag==3 (and for ==1 when it follows ==3) an icon-location string is appended;
                                 // for Flag==4,5,6 the file signature (CQuadWord Size + FILETIME LastWrite) is appended
                                 //   together with a list of CPluginInterfaceForThumbLoaderEncapsulation interfaces
                                 //   of all plugins able to create a thumbnail for file 'NameAndData'; the list ends
                                 //   with NULL
    const CFileData* FSFileData; // pointer to the CFileData of the file (only for FS with icon type pitFromPlugin), otherwise NULL

private:
    DWORD Index : 28;      // >=0 index in the icon or thumbnail cache (must be < 134217728); -1 means not loaded;
                           //   for Flag==0,1,2,3 this is an icon-cache index;
                           //   for Flag==4,5,6 this is a thumbnail-cache index
    DWORD ReadingDone : 1; // 1 = we have already tried to load (even if it failed), 0 = not attempted yet
    DWORD Flag : 3;        // flag for this type in CIconCache:
                           //   icons: 0 - not loaded, 1 - ok, 2 - old version, 3 - icon specified via icon-location
                           //   thumbnails: 4 - not loaded, 5 - ok, 6 - old version (or low quality/smaller)

public:
    int GetIndex()
    {
        int index = Index;
        if (index & 0x08000000)
            index |= 0xF0000000; // convert 28-bit int to signed 32-bit int
        return index;
    }

    int SetIndex(int index)
    {
        return Index = index;
    }

    DWORD GetFlag() { return Flag; }
    DWORD SetFlag(DWORD f) { return Flag = f; }

    DWORD GetReadingDone() { return ReadingDone; }
    DWORD SetReadingDone(DWORD r) { return ReadingDone = r; }

    const CFileData* GetFSFileData() { return FSFileData; }
};

//
// ****************************************************************************
// CThumbnailData
//

//
// represents a single thumbnail in CIconCache::ThumbnailsCache
// because a large number of bitmap handles causes the process to freeze,
// it is better to keep bitmaps as RAW data
//
struct CThumbnailData
{
    WORD Width; // thumbnail dimensions
    WORD Height;
    WORD Planes;       // define the data "geometry" (we could omit these two parameters,
    WORD BitsPerPixel; // but that would pose a risk when switching color depth)
    DWORD* Bits;       // raw data of a device-dependent bitmap; format unknown
};

//
// ****************************************************************************
// CIconCache
//

class CIconCache : public TDirectArray<CIconData>
{
protected:
    //
    // Icons
    //
    TIndirectArray<CIconList> IconsCache; // array of bitmaps serving as an icon cache
    int IconsCount;                       // number of occupied slots in bitmaps (icons)
    CIconSizeEnum IconSize;               // which icon size do we keep?

    //
    // Thumbnails
    //
    TDirectArray<CThumbnailData> ThumbnailsCache; // array of bitmaps serving as the thumbnail cache

    CPluginDataInterfaceEncapsulation* DataIfaceForFS; // internal use for SortArray()

public:
    // 'forAssociations' defines the base/delta size of the array; associations are expected to need a bigger one
    CIconCache();
    ~CIconCache();

    void Release(); // free the entire array and invalidate the cache
    void Destroy(); // free the entire array and the cache

    // sorts the array for faster search; 'dataIface' is NULL except when handling
    // ptPluginFS with icons of type pitFromPlugin
    void SortArray(int left, int right, CPluginDataInterfaceEncapsulation* dataIface);

    // returns "found?" and the item index or where it should be inserted (sorted array);
    // 'name' must be DWORD-aligned (used only if 'dataIface' is NULL);
    // 'file' represents the file data of 'name' (used only if 'dataIface' is not NULL);
    // 'dataIface' is NULL except for ptPluginFS with icons of type pitFromPlugin
    BOOL GetIndex(const char* name, int& index, CPluginDataInterfaceEncapsulation* dataIface,
                  const CFileData* file);

    // copies known icons and thumbnails (old and new caches must be sorted)
    // when transferring thumbnails, geometry and raw image data (CThumbnailData::Bits)
    // are passed to the new cache; the old cache sets Bits=NULL to avoid freeing them
    // during destruction; 'dataIface' is NULL except when both caches are ptPluginFS
    // with icons of type pitFromPlugin
    void GetIconsAndThumbsFrom(CIconCache* icons, CPluginDataInterfaceEncapsulation* dataIface,
                               BOOL transferIconsAndThumbnailsAsNew = FALSE,
                               BOOL forceReloadThumbnails = FALSE);

    // must redraw the default icon set with new background color
    void ColorsChanged();

    ////////////////
    //
    // Icons methods
    //

    // allocates space for an icon; returns its index or -1 on error
    // variables 'iconList' and 'iconListIndex' may be NULL (no output)
    // otherwise 'iconList' receives a pointer to the CIconList carrying the icon and
    // 'iconListIndex' is the index within that image list
    int AllocIcon(CIconList** iconList, int* imageIconIndex);

    // returns a pointer to IconList in 'iconList' and the icon position in 'iconListIndex'
    // for 'iconIndex' returned from AllocIcon
    BOOL GetIcon(int iconIndex, CIconList** iconList, int* iconListIndex);

    ////////////////
    //
    // Thumbnails methods
    //

    // allocates space for a thumbnail at the end of ThumbnailsCache
    // if successful, returns the index matching the thumbnail
    // returns -1 on error
    int AllocThumbnail();

    // returns a pointer to the entry in 'thumbnailData'
    // addressed by 'index' (returned from AllocThumbnail)
    BOOL GetThumbnail(int index, CThumbnailData** thumbnailData);

    void SetIconSize(CIconSizeEnum iconSize);
    CIconSizeEnum GetIconSize() { return IconSize; }

protected:
    // for internal use only
    void SortArrayInt(int left, int right);
    // internal use only
    void SortArrayForFSInt(int left, int right);
};

//
// ****************************************************************************
// CAssociationData
//

struct CAssociationIndexAndFlag
{
    DWORD Index : 31; // >= 0 index; -1 not loaded; -2 dynamic (icon inside file); -3 loading (-1 -> -3)
    DWORD Flag : 1;   // can *.ExtensionAndData be opened?
};

class CAssociationData
{
public:
    char* ExtensionAndData; // allocated on DWORD boundaries, ends zeroed for comparisons;
                            // extension plus an appended icon-location string
    char* Type;             // file-type string; NULL instead of empty string saves memory

private:
    // for each icon size we need an Index+Flag pair
    CAssociationIndexAndFlag IndexAndFlag[ICONSIZE_COUNT];

public:
    int GetIndex(CIconSizeEnum iconSize)
    {
        if (iconSize >= ICONSIZE_COUNT)
        {
            TRACE_E("CAssociationData::GetIndex() unexpected iconSize=" << iconSize);
            iconSize = ICONSIZE_16;
        }
        DWORD index = IndexAndFlag[iconSize].Index;
        if (index & 0x40000000)
            index |= 0x80000000; // convert 31-bit int to signed 32-bit int
        return index;
    }

    int SetIndex(int index, CIconSizeEnum iconSize)
    {
        if (iconSize >= ICONSIZE_COUNT)
        {
            TRACE_E("CAssociationData::SetIndex() unexpected iconSize=" << iconSize);
            iconSize = ICONSIZE_16;
        }
        return IndexAndFlag[iconSize].Index = index;
    }

    int SetIndexAll(int index)
    {
        int i;
        for (i = 0; i < ICONSIZE_COUNT; i++)
            IndexAndFlag[i].Index = index;
        return index;
    }

    DWORD GetFlag() { return IndexAndFlag[0].Flag; }
    DWORD SetFlag(DWORD f) { return IndexAndFlag[0].Flag = f; }
};

//
// ****************************************************************************
// CAssociations
//

#define ASSOC_ICON_NO_ASSOC 0 // fixed icons in the CAssociations cache bitmap
#define ASSOC_ICON_SOME_FILE 1
#define ASSOC_ICON_SOME_EXE 2
#define ASSOC_ICON_SOME_DIR 3
#define ASSOC_ICON_COUNT 4

struct CAssociationsIcons
{
public:
    TIndirectArray<CIconList> IconsCache; // array of bitmaps serving as an icon cache
    int IconsCount;                       // number of occupied slots in bitmaps (icons)

public:
    CAssociationsIcons() : IconsCache(10, 5)
    {
        IconsCount = 0;
    }
};

class CAssociations : public TDirectArray<CAssociationData>
{
protected:
    CAssociationsIcons Icons[ICONSIZE_COUNT];

public:
    CAssociations();
    ~CAssociations();

    void Release(); // free the entire array and invalidate the cache
    void Destroy(); // free the entire array and the cache

    // turn all -3 states into -1
    //    void SetAllReadingToUnread();

    // sorts the array for quick searches
    void SortArray(int left, int right);

    // returns "found?" and the item index or where it should be inserted (sorted array);
    // 'name' must be DWORD-aligned;
    BOOL GetIndex(const char* name, int& index);

    // allocates space for an icon; returns its index or -1 on error
    // variables 'iconList' and 'iconListIndex' may be NULL (no output)
    // otherwise 'iconList' returns a pointer to CIconList carrying the icon and
    // 'iconListIndex' is the index within that image list
    int AllocIcon(CIconList** iconList, int* imageIconIndex, CIconSizeEnum iconSize);

    // returns a pointer to IconList in 'iconList' and the icon position in 'iconListIndex'
    // for 'iconIndex' returned from AllocIcon
    BOOL GetIcon(int iconIndex, CIconList** iconList, int* iconListIndex, CIconSizeEnum iconSize);

    // redraw the default icon set with a new background
    void ColorsChanged();

    void ReadAssociations(BOOL showWaitWnd);

    // 'ext' must be DWORD-aligned
    BOOL IsAssociated(char* ext, BOOL& addtoIconCache, CIconSizeEnum iconSize);
    BOOL IsAssociatedStatic(char* ext, const char*& iconLocation, CIconSizeEnum iconSize);
    BOOL IsAssociated(char* ext);

protected:
    // helper method
    void InsertData(const char* origin, int index, BOOL overwriteItem, char* e, char* s,
                    CAssociationData& data, LONG& size, const char* iconLocation, const char* type);
};

extern CAssociations Associations; // loaded associations are stored here
