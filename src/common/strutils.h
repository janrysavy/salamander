// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// The SAFE_ALLOC macro removes the code that checks whether a memory allocation succeeded (see allochan.*)

// Conversion of a Unicode string (UTF-16) to an ANSI multibyte string. 'src' is the Unicode string.
// 'srcLen' is the length of that string (without the terminating null; when -1 is provided the length is determined
// from the terminating null). 'bufSize' (must be greater than 0) is the size of the target buffer 'buf' for the ANSI
// string. If 'compositeCheck' is TRUE, the call uses the WC_COMPOSITECHECK flag (see MSDN); do not use it for file
// names (NTFS distinguishes names written as precomposed and composite, i.e. it does not normalize names).
// 'codepage' is the ANSI code page. The function returns the number of characters written to 'buf' (including the
// terminating null); on error it returns zero (for details see GetLastError()). It always ensures that 'buf' is
// null-terminated (even on error). If 'buf' is too small, the function returns zero but leaves at least part of the
// converted string in 'buf'.
int ConvertU2A(const WCHAR* src, int srcLen, char* buf, int bufSize,
               BOOL compositeCheck = FALSE, UINT codepage = CP_ACP);

// Conversion of a Unicode string (UTF-16) to a newly allocated ANSI multibyte string (the caller is responsible for
// freeing the result). 'src' is the Unicode string. 'srcLen' is the length of the Unicode string (without the
// terminating null; when -1 is provided the length is determined from the terminating null). If 'compositeCheck' is
// TRUE, the call uses the WC_COMPOSITECHECK flag (see MSDN); do not use it for file names (NTFS distinguishes names
// written as precomposed and composite, i.e. it does not normalize names). 'codepage' is the ANSI code page. The
// function returns the allocated ANSI string; on error it returns NULL (for details see GetLastError()).
char* ConvertAllocU2A(const WCHAR* src, int srcLen, BOOL compositeCheck = FALSE, UINT codepage = CP_ACP);

// Conversion of an ANSI multibyte string to a Unicode string (UTF-16). 'src' is the ANSI string.
// 'srcLen' is the length of that string (without the terminating null; when -1 is provided the length is determined
// from the terminating null). 'bufSize' (must be greater than 0) is the size of the target buffer 'buf' for the
// Unicode string. 'codepage' is the ANSI code page. The function returns the number of characters written to 'buf'
// (including the terminating null); on error it returns zero (for details see GetLastError()). It always ensures that
// 'buf' is null-terminated (even on error). If 'buf' is too small, the function returns zero but leaves at least part
// of the converted string in 'buf'.
int ConvertA2U(const char* src, int srcLen, WCHAR* buf, int bufSizeInChars,
               UINT codepage = CP_ACP);

// Conversion of an ANSI multibyte string to a newly allocated Unicode string (UTF-16) (the caller is responsible for
// freeing the result). 'src' is the ANSI string. 'srcLen' is the length of the ANSI string (without the terminating
// null; when -1 is provided the length is determined from the terminating null). 'codepage' is the ANSI code page.
// The function returns the allocated Unicode string; on error it returns NULL (for details see GetLastError()).
WCHAR* ConvertAllocA2U(const char* src, int srcLen, UINT codepage = CP_ACP);

// Copies the string 'txt' into a newly allocated string; returns NULL on out-of-memory (only a risk when allochan.*
// is not used) or when 'txt' == NULL.
WCHAR* DupStr(const WCHAR* txt);

// Holds a pointer to allocated memory, releasing it when overwritten by another pointer to allocated memory and when
// the wrapper itself is destroyed.
template <class PTR_TYPE>
class CAllocP
{
public:
    PTR_TYPE* Ptr;

public:
    CAllocP(PTR_TYPE* ptr = NULL) { Ptr = ptr; }
    ~CAllocP()
    {
        if (Ptr != NULL)
            free(Ptr);
    }

    PTR_TYPE* GetAndClear()
    {
        PTR_TYPE* p = Ptr;
        Ptr = NULL;
        return p;
    }

    operator PTR_TYPE*() { return Ptr; }
    PTR_TYPE* operator=(PTR_TYPE* p)
    {
        if (Ptr != NULL)
            free(Ptr);
        return Ptr = p;
    }
};

// Holds an allocated string, releasing it when overwritten by another allocated string and when the wrapper itself is
// destroyed.
typedef CAllocP<WCHAR> CStrP;
