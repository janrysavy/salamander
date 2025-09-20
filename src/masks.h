// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// functions used by the quick-search feature
BOOL IsQSWildChar(char ch);
void PrepareQSMask(char* mask, const char* src);
BOOL AgreeQSMask(const char* filename, BOOL hasExtension, const char* mask, BOOL wholeString, int& offset);

// wildcards '*' (any string) + '?' (any character) <+ '#' (a digit) if extendedMode==TRUE>
void PrepareMask(char* mask, const char* src);                                                // converts the mask into the chosen format
BOOL AgreeMask(const char* filename, const char* mask, BOOL hasExtension, BOOL extendedMode); // does it match the mask ??

// adjusts the name according to the mask and stores the result in 'buffer' of size 'bufSize'
// 'name' is the name to adjust; 'mask' is the mask (unmodified - do not call PrepareMask on it)
// returns 'buffer' even if the name was truncated because the buffer was too small,
// otherwise NULL; NOTE: behaves like the "copy" command in Win2K
char* MaskName(char* buffer, int bufSize, const char* name, const char* mask);

//*****************************************************************************
//
// CMaskGroup
//
// Life cycle:
//   1) Pass the mask group in the constructor or SetMasksString.
//   2) Call PrepareMasks to build internal data; if it fails, show the problematic
//      position and after fixing the mask return to step (2).
//   3) Call AgreeMasks at any time to check whether a name matches the mask group.
//   4) If SetMasksString is called again, continue from step (2).
//
// Mask:
//   '?' - any character
//   '*' - any string (including empty)
//   '#' - any digit (only if 'extendedMode'==TRUE)
//
//   Examples:
//     *     - all names
//     *.*   - all names
//     *.exe - names with the "exe" extension
//     *.t?? - names with an extension starting with 't' and having two additional characters
//     *.r## - names with an extension starting with 'r' and two additional digits
//
// Mask group:
//   Masks are separated by ';'. The '|' character can also be used as a separator
//   but has a special meaning. All masks following '|' are treated inversely,
//   meaning AgreeMasks returns FALSE if a name matches them.
//   The '|' separator may appear only once and must be followed by at least one mask.
//   If nothing precedes '|', a "*" mask is automatically inserted.
//
//   Examples:
//     *.txt;*.cpp - all names with the txt or cpp extension
//     *.h*|*.html - all names whose extension starts with 'h' but not the extension "html"
//     |*.txt      - all names with an extension other than "txt"
//

#define MASK_OPTIMIZE_NONE 0      // no optimization
#define MASK_OPTIMIZE_ALL 1       // mask satisfies all requests (*.* or *)
#define MASK_OPTIMIZE_EXTENSION 2 // mask is in form (*.xxxx) where xxxx is the extension

struct CMaskItemFlags
{
    unsigned Optimize : 7; // MASK_OPTIMIZE_xxx
    unsigned Exclude : 1;  // when set, this is an exclude mask, otherwise include
                           // exclude masks are stored before include masks in PreparedMasks
};

struct CMasksHashEntry
{
    CMaskItemFlags* Mask;  // internal mask representation, see CMaskItemFlags for the format
    CMasksHashEntry* Next; // next entry with the same hash
};

class CMaskGroup
{
protected:
    char MasksString[MAX_GROUPMASK];   // mask group provided in the constructor or by PrepareMasks
    TDirectArray<char*> PreparedMasks; // internal mask representation; some masks may be in MasksHashArray instead
    BOOL NeedPrepare;                  // whether PrepareMasks must be called before using 'PreparedMasks'
    BOOL ExtendedMode;

    CMasksHashEntry* MasksHashArray; // if not NULL, hash table holding all masks with MASK_OPTIMIZE_EXTENSION and Exclude==0
    int MasksHashArraySize;          // size of MasksHashArray (twice the number of stored masks)

public:
    CMaskGroup();
    CMaskGroup(const char* masks, BOOL extendedMode = FALSE);
    ~CMaskGroup();
    void Release();

    CMaskGroup& operator=(const CMaskGroup& s);

    // sets the mask string 'masks'; maximum length including terminator is MAX_GROUPMASK
    void SetMasksString(const char* masks, BOOL extendedMode = FALSE);

    // returns the mask string; 'buffer' must be at least MAX_GROUPMASK long
    const char* GetMasksString();

    // returns a writable mask string; 'buffer' must be at least MAX_GROUPMASK long
    char* GetWritableMasksString();

    BOOL GetExtendedMode();

    // Converts the mask group passed to the constructor or SetMasksString
    // into an internal form used by AgreeMasks. If 'extendedMode' is TRUE, the
    // syntax is extended with '#' representing any digit.
    // Returns TRUE on success or FALSE on error. On error 'errorPos' contains
    // the index of the problematic character. If memory is low 'errorPos' is 0.
    // If masksString == NULL, CMaskGroup::MasksString is used; otherwise the
    // provided 'masksString' is used (CMaskGroup::MasksString is ignored).
    BOOL PrepareMasks(int& errorPos, const char* masksString = NULL);

    // Determines whether 'fileName' matches the mask group.
    // NOTE: 'fileName' must not be a full path, only name.ext
    // fileExt must point either to the terminator of fileName or to the extension (if any)
    // if fileExt == NULL, the extension will be searched for - slower
    BOOL AgreeMasks(const char* fileName, const char* fileExt);

protected:
    // releases the hash table MasksHashArray
    void ReleaseMasksHashArray();
};
