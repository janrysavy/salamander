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
#pragma pack(push, enter_include_spl_file) // to make structures independent of the alignment setting
#pragma pack(4)
#endif // _MSC_VER
#ifdef __BORLANDC__
#pragma option -a4
#endif // __BORLANDC__

//****************************************************************************
//
// CSalamanderSafeFileAbstract
//
// The SafeFile method family provides guarded file handling. The methods check
// API call error states and display the corresponding error messages. The error
// messages can contain various button combinations, from OK through Retry/Cancel
// up to Retry/Skip/Skip all/Cancel. The caller determines the button combination
// through one of the parameters.
//
// While resolving problematic states, the methods need to know the file name so
// that they can show a proper error message. They also have to know the
// parameters of the file being opened (such as dwDesiredAccess, dwShareMode,
// etc.) so that in case of an error they can close the handle and open it again.
// If, for example, a network layer interruption occurs during a ReadFile or
// WriteFile operation and the user removes the cause of the problem and presses
// Retry, the old file handle cannot be reused. The old handle must be closed, the
// file reopened, the file pointer set, and the operation repeated. Therefore,
// ATTENTION: when handling error states, the SafeFileRead and SafeFileWrite
// methods can change the value of SAFE_FILE::HFile.
//
// For these reasons, a regular HANDLE was not sufficient to hold the context, so
// it is replaced by the SAFE_FILE structure. With the SafeFileOpen method it is
// a mandatory parameter, whereas with the SafeFileCreate method it remains
// optional. This is necessary to preserve compatible behavior of
// SafeFileCreate for older plugins.
//
// Methods that support the Skip All/Overwrite All buttons use the 'silentMask'
// parameter. It is a pointer to a bit array composed of SILENT_SKIP_xxx and
// SILENT_OVERWRITE_xxx flags. If the pointer is not NULL, the bit array serves
// two purposes:
// (1) input: if the corresponding bit is set, the method suppresses the error
//            message and answers silently without user interaction when an error
//            occurs.
// (2) output: if the user answers an error prompt with Skip all or Overwrite
//             all, the method sets the relevant bit in the bit array.
// This bit array acts as the context passed into individual methods. Within one
// logical group of operations (for example, when extracting multiple files from
// an archive) the caller passes the same bit array and initializes it to 0 at the
// start. The caller can also set bits in the array explicitly to suppress the
// respective prompts.
// Salamander reserves part of the bit array for plugin-specific state. Those are
// the bits set to one in SILENT_RESERVED_FOR_PLUGINS.
//
// Unless stated otherwise, pointers passed to the interface methods must not be
// NULL.
//

struct SAFE_FILE
{
    HANDLE HFile;                // handle of the open file (note: it is stored under Salamander core HANDLES)
    char* FileName;              // fully qualified name of the open file
    HWND HParentWnd;             // window handle hParent from the SafeFileOpen/SafeFileCreate call; used
                                 // if hParent is set to HWND_STORED in the following calls
    DWORD dwDesiredAccess;       // > backup of the CreateFile API parameters
    DWORD dwShareMode;           // > for a possible repeated call
    DWORD dwCreationDisposition; // > in case of errors during reading or writing
    DWORD dwFlagsAndAttributes;  // >
    BOOL WholeFileAllocated;     // TRUE if SafeFileCreate preallocated the entire file
};

#define HWND_STORED ((HWND) - 1)

#define SAFE_FILE_CHECK_SIZE 0x00010000 // FIXME: verify that it does not conflict with BUTTONS_xxx

// bits of the silentMask mask
// skip section
#define SILENT_SKIP_FILE_NAMEUSED 0x00000001 // skips files that cannot be created because a directory with the
                                             // same name already exists (old CNFRM_MASK_NAMEUSED)
#define SILENT_SKIP_DIR_NAMEUSED 0x00000002  // skips directories that cannot be created because a file with the
                                             // same name already exists (old CNFRM_MASK_NAMEUSED)
#define SILENT_SKIP_FILE_CREATE 0x00000004   // skips files that cannot be created for another reason (old CNFRM_MASK_ERRCREATEFILE)
#define SILENT_SKIP_DIR_CREATE 0x00000008    // skips directories that cannot be created for another reason (old CNFRM_MASK_ERRCREATEDIR)
#define SILENT_SKIP_FILE_EXIST 0x00000010    // skips files that already exist (old CNFRM_MASK_FILEOVERSKIP)
                                             // mutually exclusive with SILENT_OVERWRITE_FILE_EXIST
#define SILENT_SKIP_FILE_SYSHID 0x00000020   // skips System/Hidden files that already exist (old CNFRM_MASK_SHFILEOVERSKIP)
                                             // mutually exclusive with SILENT_OVERWRITE_FILE_SYSHID
#define SILENT_SKIP_FILE_READ 0x00000040     // skips files whose reading failed
#define SILENT_SKIP_FILE_WRITE 0x00000080    // skips files whose writing failed
#define SILENT_SKIP_FILE_OPEN 0x00000100     // skips files that cannot be opened

// overwrite section
#define SILENT_OVERWRITE_FILE_EXIST 0x00001000  // overwrites files that already exist (old CNFRM_MASK_FILEOVERYES)
                                                // mutually exclusive with SILENT_SKIP_FILE_EXIST
#define SILENT_OVERWRITE_FILE_SYSHID 0x00002000 // overwrites System/Hidden files that already exist (old CNFRM_MASK_SHFILEOVERYES)
                                                // mutually exclusive with SILENT_SKIP_FILE_SYSHID
#define SILENT_RESERVED_FOR_PLUGINS 0xFFFF0000  // this space is available to plugins for their own flags

class CSalamanderSafeFileAbstract
{
public:
    //
    // SafeFileOpen
    //   Opens an existing file.
    //
    // Parameters
    //   'file'
    //      [out] Pointer to a 'SAFE_FILE' structure that receives information about
    //      the open file. This structure acts as the context for other methods in the
    //      SafeFile family. The structure values are meaningful only if SafeFileOpen
    //      returned TRUE. To close the file, call SafeFileClose.
    //
    //   'fileName'
    //      [in] Pointer to a null-terminated string containing the name of the file
    //      being opened.
    //
    //   'dwDesiredAccess'
    //   'dwShareMode'
    //   'dwCreationDisposition'
    //   'dwFlagsAndAttributes'
    //      [in] see the CreateFile API.
    //
    //   'hParent'
    //      [in] Handle of the window to which the error messages will be shown
    //      modally.
    //
    //   'flags'
    //      [in] One of the BUTTONS_xxx values; determines the buttons displayed in
    //      the error messages.
    //
    //   'pressedButton'
    //      [out] Pointer to a variable that receives the button pressed while the error
    //      message was shown. The variable is meaningful only if SafeFileOpen returns
    //      FALSE; otherwise its value is undefined. Returns one of the DIALOG_xxx
    //      values. In case of errors returns the value DIALOG_CANCEL.
    //      If a particular error message is suppressed thanks to 'silentMask', it
    //      returns the corresponding button value (for example DIALOG_SKIP or
    //      DIALOG_YES).
    //
    //      'pressedButton' can be NULL (for example for BUTTONS_OK or
    //      BUTTONS_RETRYCANCEL it makes no sense to check the pressed button).
    //
    //   'silentMask'
    //      [in/out] Pointer to a variable containing a bit array of SILENT_xxx values.
    //      For SafeFileOpen only the SILENT_SKIP_FILE_OPEN value is relevant.
    //
    //      If the SILENT_SKIP_FILE_OPEN bit is set in the bit array, the displayed
    //      message should also have a Skip button (controlled by the 'flags'
    //      parameter) and an error occurs while opening the file, the error message
    //      will be suppressed. SafeFileOpen then returns FALSE and, if 'pressedButton'
    //      is not NULL, sets it to DIALOG_SKIP.
    //
    // Return Values
    //   Returns TRUE if the file was opened successfully. The 'file' structure is
    //   initialized and you must call SafeFileClose to close the file.
    //
    //   In case of an error returns FALSE and sets the 'pressedButton' and
    //   'silentMask' variables if they are not NULL.
    //
    // Remarks
    //   The method can be called from any thread.
    //
    virtual BOOL WINAPI SafeFileOpen(SAFE_FILE* file,
                                     const char* fileName,
                                     DWORD dwDesiredAccess,
                                     DWORD dwShareMode,
                                     DWORD dwCreationDisposition,
                                     DWORD dwFlagsAndAttributes,
                                     HWND hParent,
                                     DWORD flags,
                                     DWORD* pressedButton,
                                     DWORD* silentMask) = 0;

    //
    // SafeFileCreate
    //   Creates a new file including its path if it does not yet exist. If the file
    //   already exists, it offers to overwrite it. The method is primarily intended
    //   for creating files and directories extracted from an archive.
    //
    // Parameters
    //   'fileName'
    //      [in] Pointer to a null-terminated string specifying the name of the file
    //      being created.
    //
    //   'dwDesiredAccess'
    //   'dwShareMode'
    //   'dwFlagsAndAttributes'
    //      [in] see the CreateFile API.
    //
    //   'isDir'
    //      [in] Specifies whether the last component of the 'fileName' path should be
    //      a directory (TRUE) or a file (FALSE). If 'isDir' is TRUE, the variables
    //      'dwDesiredAccess', 'dwShareMode', 'dwFlagsAndAttributes', 'srcFileName',
    //      'srcFileInfo', and 'file' are ignored.
    //
    //   'hParent'
    //      [in] Handle of the window to which the error messages will be shown
    //      modally.
    //
    //   'srcFileName'
    //      [in] Pointer to a null-terminated string specifying the name of the source
    //      file. This name will be displayed together with the size and time
    //      ('srcFileInfo') in the prompt for overwriting an existing file if the file
    //      'fileName' already exists.
    //      'srcFileName' can be NULL, in which case 'srcFileInfo' is ignored.
    //      In that case the overwrite prompt will contain the text "a newly created file"
    //      instead of the source file name.
    //
    //   'srcFileInfo'
    //      [in] Pointer to a null-terminated string containing the size, date, and
    //      time of the source file. This information is displayed together with the
    //      source file name 'srcFileName' in the prompt for overwriting an existing
    //      file. Format: "size, date, time".
    //      Obtain the size using CSalamanderGeneralAbstract::NumberToStr, the date via
    //      GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, ... and the time via
    //      GetTimeFormat(LOCALE_USER_DEFAULT, 0, ... See the implementation of the
    //      GetFileInfo method in the UnFAT plugin.
    //      'srcFileInfo' can be NULL if 'srcFileName' is also NULL.
    //
    //    'silentMask'
    //      [in/out] Pointer to a bit array composed of SILENT_SKIP_xxx and
    //      SILENT_OVERWRITE_xxx; see the introduction at the beginning of this file.
    //      If 'silentMask' is NULL, it is ignored. The SafeFileCreate method tests
    //      and sets these constants:
    //        SILENT_SKIP_FILE_NAMEUSED
    //        SILENT_SKIP_DIR_NAMEUSED
    //        SILENT_OVERWRITE_FILE_EXIST
    //        SILENT_SKIP_FILE_EXIST
    //        SILENT_OVERWRITE_FILE_SYSHID
    //        SILENT_SKIP_FILE_SYSHID
    //        SILENT_SKIP_DIR_CREATE
    //        SILENT_SKIP_FILE_CREATE
    //
    //      If 'srcFileName' is not NULL, meaning this is a COPY/MOVE operation, the
    //      following applies:
    //        If the Salamander configuration (Confirmations page) has "Confirm on file
    //        overwrite" disabled, the method behaves as if 'silentMask' contained
    //        SILENT_OVERWRITE_FILE_EXIST.
    //        If "Confirm on system or hidden file overwrite" is disabled, the method
    //        behaves as if 'silentMask' contained SILENT_OVERWRITE_FILE_SYSHID.
    //
    //    'allowSkip'
    //      [in] Specifies whether prompts and error messages will also contain the
    //      "Skip" and "Skip all" buttons.
    //
    //    'skipped'
    //      [out] Returns TRUE if the user clicked the "Skip" or "Skip all" button in a
    //      prompt or error message. Otherwise returns FALSE. The 'skipped' variable
    //      can be NULL. It is meaningful only if SafeFileCreate returns
    //      INVALID_HANDLE_VALUE.
    //
    //    'skipPath'
    //      [out] Pointer to a buffer that receives the path the user chose to skip with
    //      the "Skip" or "Skip all" button in one of the prompts. The buffer size is
    //      given by the skipPathMax variable, which will not be exceeded. The path is
    //      null-terminated. At the start of SafeFileCreate the buffer is set to an empty
    //      string. 'skipPath' can be NULL; in that case 'skipPathMax' is ignored.
    //
    //    'skipPathMax'
    //      [in] Size of the 'skipPath' buffer in characters. Must be set if 'skipPath'
    //      is not NULL.
    //
    //    'allocateWholeFile'
    //      [in/out] Pointer to a CQuadWord specifying the size to which the file
    //      should be preallocated using SetEndOfFile. If the pointer is NULL, it is
    //      ignored and SafeFileCreate will not attempt preallocation. If the pointer
    //      is not NULL, the function attempts preallocation. The requested size must
    //      be greater than CQuadWord(2, 0) and less than CQuadWord(0, 0x80000000)
    //      (8EB).
    //
    //      If SafeFileCreate should also perform a test (the preallocation mechanism
    //      may not always work), the highest bit of the size must be set, meaning
    //      CQuadWord(0, 0x80000000) must be added to the value.
    //
    //      If the file is created (SafeFileCreate returns a handle other than
    //      INVALID_HANDLE_VALUE), the 'allocateWholeFile' variable is set to one of
    //      the following values:
    //       CQuadWord(0, 0x80000000): the file could not be preallocated and for the
    //                                 next SafeFileCreate call for files destined for
    //                                 the same location, 'allocateWholeFile' should be
    //                                 NULL
    //       CQuadWord(0, 0):          the file could not be preallocated, but it is not
    //                                 fatal and during subsequent SafeFileCreate calls
    //                                 for files with this destination you may request
    //                                 their preallocation
    //       other:                    preallocation succeeded
    //                                 In this case SAFE_FILE::WholeFileAllocated is set
    //                                 to TRUE and SafeFileClose calls SetEndOfFile to
    //                                 shrink the file and avoid storing unnecessary
    //                                 data.
    //
    //    'file'
    //      [out] Pointer to a 'SAFE_FILE' structure that receives information about
    //      the open file. This structure acts as the context for other methods in the
    //      SafeFile family. The structure values are meaningful only if SafeFileCreate
    //      returned a value different from INVALID_HANDLE_VALUE. To close the file call
    //      SafeFileClose. If 'file' is not NULL, SafeFileCreate registers the created
    //      handle in the Salamander HANDLES. If 'file' is NULL, the handle will not be
    //      registered in HANDLES. If 'isDir' is TRUE, the 'file' variable is ignored.
    //
    // Return Values
    //   If 'isDir' is TRUE, a successful call returns a value other than
    //   INVALID_HANDLE_VALUE. Note that this is not a valid handle of the created
    //   directory. On failure it returns INVALID_HANDLE_VALUE and sets the
    //   'silentMask', 'skipped', and 'skipPath' variables.
    //
    //   If 'isDir' is FALSE, a successful call returns the handle of the created file
    //   and, if 'file' is not NULL, fills the SAFE_FILE structure.
    //   On failure it returns INVALID_HANDLE_VALUE and sets the 'silentMask',
    //   'skipped', and 'skipPath' variables.
    //
    // Remarks
    //   The method can be called only from the main thread. (it may call
    //   FlashWindow(MainWindow), which must be called from the window's thread,
    //   otherwise it causes a deadlock)
    //
    virtual HANDLE WINAPI SafeFileCreate(const char* fileName,
                                         DWORD dwDesiredAccess,
                                         DWORD dwShareMode,
                                         DWORD dwFlagsAndAttributes,
                                         BOOL isDir,
                                         HWND hParent,
                                         const char* srcFileName,
                                         const char* srcFileInfo,
                                         DWORD* silentMask,
                                         BOOL allowSkip,
                                         BOOL* skipped,
                                         char* skipPath,
                                         int skipPathMax,
                                         CQuadWord* allocateWholeFile,
                                         SAFE_FILE* file) = 0;

    //
    // SafeFileClose
    //   Closes the file and frees data allocated in the 'file' structure.
    //
    // Parameters
    //   'file'
    //      [in] Pointer to the 'SAFE_FILE' structure that was initialized by a
    //      successful call to SafeFileCreate or SafeFileOpen.
    //
    // Remarks
    //   The method can be called from any thread.
    //
    virtual void WINAPI SafeFileClose(SAFE_FILE* file) = 0;

    //
    // SafeFileSeek
    //   Sets the file pointer in the open file.
    //
    // Parameters
    //   'file'
    //      [in] Pointer to the 'SAFE_FILE' structure that was initialized by a call
    //      to SafeFileOpen or SafeFileCreate.
    //
    //   'distance'
    //      [in/out] Number of bytes by which the file pointer should move. On success
    //      it receives the new pointer position.
    //
    //      The value of CQuadWord::Value is interpreted as signed for all three
    //      'moveMethod' values (beware of the MSDN error for SetFilePointerEx, which
    //      claims the value is unsigned for FILE_BEGIN). If we therefore want to move
    //      backwards from the current position (FILE_CURRENT) or from the end of the
    //      file (FILE_END), we set CQuadWord::Value to a negative number. You can
    //      assign, for example, __int64 directly to CQuadWord::Value.
    //
    //      The return value is the absolute position from the start of the file and
    //      will range from 0 to 2^63. None of the current versions of Windows support
    //      files larger than 2^63.
    //
    //   'moveMethod'
    //      [in] Starting position for the file pointer. Can be one of the values:
    //           FILE_BEGIN, FILE_CURRENT, or FILE_END.
    //
    //   'error'
    //      [out] Pointer to a DWORD variable that contains the value returned by
    //      GetLastError() if an error occurs. 'error' can be NULL.
    //
    // Return Values
    //   Returns TRUE if successful and the 'distance' variable is set to the new file
    //   pointer position.
    //
    //   On failure it returns FALSE and sets the 'error' value to GetLastError if
    //   'error' is not NULL. It does not display the error; SafeFileSeekMsg serves
    //   that purpose.
    //
    // Remarks
    //   The method calls the SetFilePointer API, so it inherits the limitations of
    //   that function.
    //
    //   It is not an error to set the file pointer past the end of the file. The
    //   file size will not increase until you call SetEndOfFile or SafeFileWrite. See
    //   the SetFilePointer API.
    //
    //   The method can be used to obtain the file size by setting 'distance' to 0 and
    //   'moveMethod' to FILE_END. The returned 'distance' value will be the file size.
    //
    //   The method can be called from any thread.
    //
    virtual BOOL WINAPI SafeFileSeek(SAFE_FILE* file,
                                     CQuadWord* distance,
                                     DWORD moveMethod,
                                     DWORD* error) = 0;

    //
    // SafeFileSeekMsg
    //   Sets the file pointer in the open file. If an error occurs, it displays it.
    //
    // Parameters
    //   'file'
    //   'distance'
    //   'moveMethod'
    //      See the comment for SafeFileSeek.
    //
    //   'hParent'
    //      [in] Handle of the window to which the error messages will be shown
    //      modally. If it equals HWND_STORED, the 'hParent' from the
    //      SafeFileOpen/SafeFileCreate call is used.
    //
    //   'flags'
    //      [in] One of the BUTTONS_xxx values; determines the buttons displayed in the
    //      error message.
    //
    //   'pressedButton'
    //      [out] Pointer to a variable that receives the button pressed while the error
    //      message was shown. The variable is meaningful only if SafeFileSeekMsg
    //      returns FALSE. 'pressedButton' can be NULL (for example for BUTTONS_OK it
    //      makes no sense to check the pressed button)
    //
    //   'silentMask'
    //      [in/out] Pointer to a variable containing a bit array of SILENT_SKIP_xxx
    //      values. For details see the comment for SafeFileOpen.
    //      SafeFileSeekMsg tests and sets the SILENT_SKIP_FILE_READ bit if
    //      'seekForRead' is TRUE or SILENT_SKIP_FILE_WRITE if 'seekForRead' is FALSE.
    //
    //   'seekForRead'
    //      [in] Tells the method why the seek in the file was performed. The method
    //      uses this variable only in case of an error. It determines which bit is
    //      used for 'silentMask' and what the title of the error message will be:
    //      "Error Reading File" or "Error Writing File".
    //
    // Return Values
    //   Returns TRUE if successful and the 'distance' variable is set to the new file
    //   pointer position.
    //
    //   On failure it returns FALSE and sets the 'pressedButton' and 'silentMask'
    //   variables if they are not NULL.
    //
    // Remarks
    //   See the SafeFileSeek method.
    //
    //   The method can be called from any thread.
    //
    virtual BOOL WINAPI SafeFileSeekMsg(SAFE_FILE* file,
                                        CQuadWord* distance,
                                        DWORD moveMethod,
                                        HWND hParent,
                                        DWORD flags,
                                        DWORD* pressedButton,
                                        DWORD* silentMask,
                                        BOOL seekForRead) = 0;

    //
    // SafeFileGetSize
    //   Returns the file size.
    //
    //   'file'
    //      [in] Pointer to the 'SAFE_FILE' structure that was initialized by a call
    //      to SafeFileOpen or SafeFileCreate.
    //
    //   'lpBuffer'
    //      [out] Pointer to a CQuadWord structure that receives the file size.
    //
    //   'error'
    //      [out] Pointer to a DWORD variable that contains the value returned by
    //      GetLastError() if an error occurs. 'error' can be NULL.
    //
    // Return Values
    //   Returns TRUE if successful and sets the 'fileSize' variable.
    //   On failure returns FALSE and sets the 'error' value if it is not NULL.
    //
    // Remarks
    //   The method can be called from any thread.
    //
    virtual BOOL WINAPI SafeFileGetSize(SAFE_FILE* file,
                                        CQuadWord* fileSize,
                                        DWORD* error) = 0;

    //
    // SafeFileRead
    //   Reads data from the file starting at the file pointer position. After the
    //   operation completes, the pointer is moved by the number of bytes read. The
    //   method supports only synchronous reading, meaning it does not return until the
    //   data is read or an error occurs.
    //
    // Parameters
    //   'file'
    //      [in] Pointer to the 'SAFE_FILE' structure that was initialized by a call
    //      to SafeFileOpen or SafeFileCreate.
    //
    //   'lpBuffer'
    //      [out] Pointer to the buffer that receives the data read from the file.
    //
    //   'nNumberOfBytesToRead'
    //      [in] Specifies how many bytes should be read from the file.
    //
    //   'lpNumberOfBytesRead'
    //      [out] Points to the variable that receives the number of bytes actually
    //      read into the buffer.
    //
    //   'hParent'
    //      [in] Handle of the window to which the error messages will be shown
    //      modally. If it equals HWND_STORED, the 'hParent' from the
    //      SafeFileOpen/SafeFileCreate call is used.
    //
    //   'flags'
    //      [in] One of the BUTTONS_xxx values, optionally combined with
    //      SAFE_FILE_CHECK_SIZE; determines the buttons shown in the error messages.
    //      If the SAFE_FILE_CHECK_SIZE bit is set, SafeFileRead treats it as an error
    //      when it cannot read the requested number of bytes and displays an error
    //      message. Without this bit the behavior matches the ReadFile API.
    //
    //   'pressedButton'
    //   'silentMask'
    //      See SafeFileOpen.
    //
    // Return Values
    //   Returns TRUE if successful and the 'lpNumberOfBytesRead' variable is set to
    //   the number of bytes read.
    //
    //   On failure it returns FALSE and sets the 'pressedButton' and 'silentMask'
    //   variables if they are not NULL.
    //
    // Remarks
    //   The method can be called from any thread.
    //
    virtual BOOL WINAPI SafeFileRead(SAFE_FILE* file,
                                     LPVOID lpBuffer,
                                     DWORD nNumberOfBytesToRead,
                                     LPDWORD lpNumberOfBytesRead,
                                     HWND hParent,
                                     DWORD flags,
                                     DWORD* pressedButton,
                                     DWORD* silentMask) = 0;

    //
    // SafeFileWrite
    //   Writes data to the file starting at the file pointer position. After the
    //   operation completes, the pointer is moved by the number of bytes written. The
    //   method supports only synchronous writing, meaning it does not return until the
    //   data is written or an error occurs.
    //
    // Parameters
    //   'file'
    //      [in] Pointer to the 'SAFE_FILE' structure that was initialized by a call
    //      to SafeFileOpen or SafeFileCreate.
    //
    //   'lpBuffer'
    //      [in] Pointer to the buffer containing the data that should be written to
    //      the file.
    //
    //   'nNumberOfBytesToWrite'
    //      [in] Specifies how many bytes should be written from the buffer to the
    //      file.
    //
    //   'lpNumberOfBytesWritten'
    //      [out] Points to the variable that receives the number of bytes actually
    //      written.
    //
    //   'hParent'
    //      [in] Handle of the window to which the error messages will be shown
    //      modally. If it equals HWND_STORED, the 'hParent' from the
    //      SafeFileOpen/SafeFileCreate call is used.
    //
    //   'flags'
    //      [in] One of the BUTTONS_xxx values; determines the buttons displayed in the
    //      error messages.
    //
    //   'pressedButton'
    //   'silentMask'
    //      See SafeFileOpen.
    //
    // Return Values
    //   Returns TRUE if successful and the 'lpNumberOfBytesWritten' variable is set to
    //   the number of bytes written.
    //
    //   On failure it returns FALSE and sets the 'pressedButton' and 'silentMask'
    //   variables if they are not NULL.
    //
    // Remarks
    //   The method can be called from any thread.
    //
    virtual BOOL WINAPI SafeFileWrite(SAFE_FILE* file,
                                      LPVOID lpBuffer,
                                      DWORD nNumberOfBytesToWrite,
                                      LPDWORD lpNumberOfBytesWritten,
                                      HWND hParent,
                                      DWORD flags,
                                      DWORD* pressedButton,
                                      DWORD* silentMask) = 0;
};

#ifdef _MSC_VER
#pragma pack(pop, enter_include_spl_file)
#endif // _MSC_VER
#ifdef __BORLANDC__
#pragma option -a
#endif // __BORLANDC__
