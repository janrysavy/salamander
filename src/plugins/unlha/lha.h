// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

//**************************************************************************
//
//  Structure of an LHA archive:
//
//   +--------+------------+--------+------------+ . . . +--------+------------+
//   | header | compressed | header | compressed |       | header | compressed |
//   | file 1 |   file 1   | file 2 |   file 2   |       | file N |   file N   |
//   +--------+------------+--------+------------+ . . . +--------+------------+
//
//  An LHA archive has no global header or list of files (directories).
//  Solid archives and encryption are not supported. Each file is independent.
//  Listing consists of reading a header, skipping by hdr.packed_size
//  bytes forward, reading the header, etc. When extracting the entire archive,
//  just alternate calling LHAGetHeader and LHAUnpackFile (or skip instead of
//  LHAUnpackFile using fseek).
//

//**************************************************************************
//
//  LHA_HEADER - information about a file in the archive
//

struct LHA_TIMESTAMP
{
    unsigned second2 : 5;
    unsigned minute : 6;
    unsigned hour : 5;
    unsigned day : 5;
    unsigned month : 4;
    unsigned year : 7;
};

struct LHA_HEADER
{
    unsigned char header_size;
    int method;         // method; if LHA_UNKNOWNMETHOD, the file must be skipped
    long packed_size;   // size of compressed data immediately following the header
    long original_size; // size of the original (uncompressed) file
    LHA_TIMESTAMP last_modified_stamp;
    FILETIME last_modified_filetime; // last_modified_stamp converted to FILETIME
    unsigned char attribute;
    unsigned char header_level;
    char name[MAX_PATH];
    unsigned short crc;
    BOOL has_crc;
    unsigned char extend_type;
    unsigned char minor_version;
    unsigned short unix_mode; // can be UNIX_FILE_REGULAR, UNIX_FILE_DIRECTORY, or UNIX_FILE_SYMLINK
};

#define LHA_UNKNOWNMETHOD -1
#define LZHUFF0_METHOD_NUM 0
#define LZHUFF1_METHOD_NUM 1
#define LZHUFF2_METHOD_NUM 2
#define LZHUFF3_METHOD_NUM 3
#define LZHUFF4_METHOD_NUM 4
#define LZHUFF5_METHOD_NUM 5
#define LZHUFF6_METHOD_NUM 6
#define LZHUFF7_METHOD_NUM 7
#define LARC_METHOD_NUM 8
#define LARC5_METHOD_NUM 9
#define LARC4_METHOD_NUM 10
#define LZHDIRS_METHOD_NUM 11

#define UNIX_FILE_REGULAR 0100000
#define UNIX_FILE_DIRECTORY 0040000
#define UNIX_FILE_SYMLINK 0120000
#define UNIX_FILE_TYPEMASK 0170000

//**************************************************************************
//
//  LHAInit - must be called at the start
//

void LHAInit();

//**************************************************************************
//
//  LHAOpenArchive - opens the file and optionally skips SFX code.
//                   Returns TRUE if everything is in order, otherwise FALSE.
//

BOOL LHAOpenArchive(FILE*& f, LPCTSTR lpName);

//**************************************************************************
//
//  LHAGetHeader - reads the header from the current file position and stores it
//                 into lpHeader. Returns GH_ERROR, GH_SUCCESS, or GH_EOF.
//

int LHAGetHeader(FILE* hFile, LHA_HEADER* lpHeader);

#define GH_ERROR 0   // error
#define GH_SUCCESS 1 // success
#define GH_EOF 2     // end of file

//**************************************************************************
//
//  LHAUnpackFile - extracts the file starting at the current position in the
//                  open archive 'infile' into the open file 'outfile'. The
//                  header obtained with LHAGetHeader must be provided.
//                  Returns TRUE on success, otherwise FALSE. The CRC variable
//                  is filled with the checksum calculated during extraction, so
//                  it can be compared with the value in the header. During
//                  extraction the function stored in pfLHAProgress (if not NULL)
//                  is called.
//

BOOL LHAUnpackFile(FILE* infile, HANDLE outfile, LHA_HEADER* lpHeader, int* CRC,
                   char* fileName /* output file name for SafeWriteFile */);

/*
    Note: Why does 'infile' use CRT I/O functions and 'outfile' API functions?
          API functions are not buffered (and I did not want to write my own
          buffers, because it would be quite complex for 'infile'), so I
          decided to keep the CRT function calls in the LHA code.
          Then I found out that Salamander's SafeCreateFile, which returns a
          HANDLE, is used for creating files. To avoid creating files via
          SafeCreateFile and then reopening them with fopen(), I rewrote the
          output part of LHA to the API and also wrote my own buffer (which is
          simple - no seeking, reading headers, etc.). So the result is a hybrid :(
*/

//**************************************************************************
//
//  exported global variables
//

extern int iLHAErrorStrId;              // contains the error description string ID on failure
extern BOOL (*pfLHAProgress)(int size); // pointer to a function that during extraction
                                        // receives values in the range 0 .. hdr.original_size
