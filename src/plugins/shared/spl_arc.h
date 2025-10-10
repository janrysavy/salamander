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
#pragma pack(push, enter_include_spl_arc) // so that the structures are independent of the configured alignment
#pragma pack(4)
#endif // _MSC_VER
#ifdef __BORLANDC__
#pragma option -a4
#endif // __BORLANDC__

class CSalamanderDirectoryAbstract;
class CSalamanderForOperationsAbstract;
class CPluginDataInterfaceAbstract;

//
// ****************************************************************************
// CPluginInterfaceForArchiverAbstract
//

class CPluginInterfaceForArchiverAbstract
{
#ifdef INSIDE_SALAMANDER
private: // protection against incorrect direct method calls (see CPluginInterfaceForArchiverEncapsulation)
    friend class CPluginInterfaceForArchiverEncapsulation;
#else  // INSIDE_SALAMANDER
public:
#endif // INSIDE_SALAMANDER

    // function used for the "panel archiver view"; called to load the contents of the
    // archive 'fileName'; the content is stored in the 'dir' object; Salamander obtains
    // the values for columns added by the plugin through the 'pluginData' interface (if the
    // plugin does not add columns, it returns 'pluginData' == NULL); returns TRUE when the
    // archive content is loaded successfully; if it returns FALSE, the return value of
    // 'pluginData' is ignored (the data in 'dir' must be released with 'dir.Clear(pluginData)',
    // otherwise only the Salamander-owned part is released);
    // 'salamander' is a set of helper methods exported from Salamander;
    // NOTE: the file 'fileName' may not exist (for example when it is open in the panel and
    // removed elsewhere); ListArchive is not called for zero-length files—they automatically
    // have empty content; when packing into such files, the file is deleted before calling
    // PackToArchive (for compatibility with external packing tools)
    virtual BOOL WINAPI ListArchive(CSalamanderForOperationsAbstract* salamander, const char* fileName,
                                    CSalamanderDirectoryAbstract* dir,
                                    CPluginDataInterfaceAbstract*& pluginData) = 0;

    // function for the "panel archiver view"; called when requested to unpack files/directories
    // from archive 'fileName' to directory 'targetDir' using the archive path 'archiveRoot';
    // 'pluginData' is the interface for working with file/directory information specific to the plugin
    // (for example, data for added columns; it is the same interface returned by ListArchive
    // in the 'pluginData' parameter—so it may also be NULL); the files/directories are supplied
    // through the enumeration function 'next', whose parameter is 'nextParam'; returns TRUE when
    // unpacking succeeds (Cancel was not used; Skip may have been) and the source items in the panel
    // are deselected; otherwise returns FALSE (the selection remains marked);
    // 'salamander' is a set of helper methods exported from Salamander
    virtual BOOL WINAPI UnpackArchive(CSalamanderForOperationsAbstract* salamander, const char* fileName,
                                      CPluginDataInterfaceAbstract* pluginData, const char* targetDir,
                                      const char* archiveRoot, SalEnumSelection next,
                                      void* nextParam) = 0;

    // function for the "panel archiver view"; called when requested to unpack a single file for viewing/editing
    // from archive 'fileName' into directory 'targetDir'; the file name in the archive is 'nameInArchive';
    // 'pluginData' is the interface for working with file information specific to the plugin
    // (for example, data for added columns; it is the same interface returned by ListArchive
    // in the 'pluginData' parameter—so it may also be NULL); 'fileData' is a pointer to the CFileData
    // structure of the file being unpacked (the structure was built by the plugin when listing the archive);
    // 'newFileName' (if not NULL) is a new name for the extracted file (used when the original archive name
    // cannot be written to disk, for example "aux", "prn", etc.); set 'renamingNotSupported' to TRUE
    // (only if 'newFileName' is not NULL) when the plugin does not support renaming during extraction
    // (Salamander then shows the standard error message "renaming not supported");
    // returns TRUE when the file is unpacked successfully (the file exists at the requested path and neither
    // Cancel nor Skip was used); 'salamander' is a set of helper methods exported from Salamander
    virtual BOOL WINAPI UnpackOneFile(CSalamanderForOperationsAbstract* salamander, const char* fileName,
                                      CPluginDataInterfaceAbstract* pluginData, const char* nameInArchive,
                                      const CFileData* fileData, const char* targetDir,
                                      const char* newFileName, BOOL* renamingNotSupported) = 0;

    // function for "panel archiver edit" and "custom archiver pack"; called when requested to pack
    // files/directories into archive 'fileName' at path 'archiveRoot'; the files/directories are provided by
    // the source path 'sourcePath' and the enumeration function 'next' with parameter 'nextParam';
    // if 'move' is TRUE, the packed files/directories should be removed from disk; returns TRUE
    // when it succeeds in packing/removing all files/directories (Cancel was not used; Skip may have been)
    // and the source items in the panel are deselected; otherwise it returns FALSE (the selection remains);
    // 'salamander' is a set of helper methods exported from Salamander
    virtual BOOL WINAPI PackToArchive(CSalamanderForOperationsAbstract* salamander, const char* fileName,
                                      const char* archiveRoot, BOOL move, const char* sourcePath,
                                      SalEnumSelection2 next, void* nextParam) = 0;

    // function for "panel archiver edit"; called when requested to delete files/directories from archive
    // 'fileName'; the files/directories are provided by the path 'archiveRoot' and the enumeration function
    // 'next' with parameter 'nextParam'; 'pluginData' is the interface for working with file/directory
    // information specific to the plugin (for example, data for added columns; it is the same interface
    // returned by ListArchive in the 'pluginData' parameter—so it may also be NULL); returns TRUE when it
    // succeeds in deleting all files/directories (Cancel was not used; Skip may have been) and the source
    // items in the panel are deselected; otherwise it returns FALSE (the selection remains);
    // 'salamander' is a set of helper methods exported from Salamander
    virtual BOOL WINAPI DeleteFromArchive(CSalamanderForOperationsAbstract* salamander, const char* fileName,
                                          CPluginDataInterfaceAbstract* pluginData, const char* archiveRoot,
                                          SalEnumSelection next, void* nextParam) = 0;

    // function for "custom archiver unpack"; called when requested to extract files/directories from
    // archive 'fileName' into directory 'targetDir'; the files/directories are specified by the mask 'mask';
    // returns TRUE when it succeeds in extracting all files/directories (Cancel was not used; Skip may have been);
    // if 'delArchiveWhenDone' is TRUE, add all archive volumes to 'archiveVolumes' (including the terminating
    // null; if the archive is not multi-volume, only 'fileName' is added); if the function returns TRUE
    // (extraction succeeded), every file listed in 'archiveVolumes' is deleted afterwards;
    // 'salamander' is a set of helper methods exported from Salamander
    virtual BOOL WINAPI UnpackWholeArchive(CSalamanderForOperationsAbstract* salamander, const char* fileName,
                                           const char* mask, const char* targetDir, BOOL delArchiveWhenDone,
                                           CDynamicString* archiveVolumes) = 0;

    // function for "panel archiver view/edit"; called before closing the panel with the archive
    // NOTE: if a new path cannot be opened, the archive may remain displayed in the panel (regardless of
    //        what CanCloseArchive returns); therefore the method cannot be used to destroy the context;
    //        it is meant, for example, to optimize the Delete operation from the archive, where it can
    //        offer to "compact" the archive when leaving it;
    //        to destroy the context, use the method CPluginInterfaceAbstract::ReleasePluginDataInterface—
    //        see the document archivatory.txt
    // 'fileName' is the archive name; 'salamander' is a set of helper methods exported from Salamander;
    // 'panel' denotes the panel where the archive is open (PANEL_LEFT or PANEL_RIGHT);
    // returns TRUE if closing is possible; if 'force' is TRUE, it always returns TRUE; when a critical shutdown
    // is in progress (see more in CSalamanderGeneralAbstract::IsCriticalShutdown), it makes no sense
    // to ask the user anything
    virtual BOOL WINAPI CanCloseArchive(CSalamanderForOperationsAbstract* salamander, const char* fileName,
                                        BOOL force, int panel) = 0;

    // determines the required disk-cache settings (the disk cache is used for temporary copies of files
    // when opening archive files in viewers, editors, and through system associations); normally
    // (if it succeeds in allocating the copy 'tempPath' after the call) it is called only once, specifically before
    // the first use of the disk cache (for example before the first archive file is opened in the viewer/editor);
    // if it returns FALSE, the standard settings are used (files in the TEMP directory; copies are deleted via the
    // Win32 API function DeleteFile() only after exceeding the cache size limit or when the archive is closed)
    // and all other return values are ignored; if it returns TRUE, the following return values are used:
    // if 'tempPath' (a buffer of size MAX_PATH) is not an empty string, all temporary copies extracted by the
    // plugin from the archive are stored in subdirectories of this path (these subdirectories are removed by the
    // disk cache when Salamander exits, but nothing prevents the plugin from deleting them earlier, for example
    // during unload; it is also advisable, when loading the first instance of the plugin—not only within a single
    // running Salamander—to clean the "SAL*.tmp" subdirectories on this path; this solves problems caused by
    // locked files and crashes) + if 'ownDelete' is TRUE, the methods DeleteTmpCopy and PrematureDeleteTmpCopy
    // will be called to delete the copies + if 'cacheCopies' is FALSE, the copies are deleted as soon as they are
    // released (for example when the viewer closes); if 'cacheCopies' is TRUE, the copies are deleted only after
    // the cache size limit is exceeded or when the archive is closed
    virtual BOOL WINAPI GetCacheInfo(char* tempPath, BOOL* ownDelete, BOOL* cacheCopies) = 0;

    // used only if GetCacheInfo returns TRUE in the parameter 'ownDelete':
    // deletes a temporary copy extracted from this archive (watch out for read-only files; their attributes
    // must be changed first and only then can they be deleted); if possible, it should not display any windows
    // (the user did not invoke the action directly and it could distract them from other tasks); for longer
    // actions it is useful to display a wait window (see CSalamanderGeneralAbstract::CreateSafeWaitWindow);
    // 'fileName' is the name of the file with the copy; if several files are deleted at once (for example after
    // closing the edited archive), 'firstFile' is TRUE for the first file and FALSE for the others
    // (used to display the wait window correctly—see DEMOPLUG)
    //
    // WARNING: called in the main thread after the disk cache delivers a message to the main window—a message
    // about the need to release a temporary copy (typically when the viewer or the "edited" archive in the panel
    // closes), so re-entry into the plugin may occur (if the message is dispatched by the plugin's internal
    // message loop); another entry into DeleteTmpCopy is excluded, because until the DeleteTmpCopy call finishes
    // the disk cache does not send any further messages
    virtual void WINAPI DeleteTmpCopy(const char* fileName, BOOL firstFile) = 0;

    // used only if GetCacheInfo returns TRUE in the parameter 'ownDelete':
    // during plugin unload, determines whether DeleteTmpCopy should be called for copies that are still in use
    // (for example open in the viewer)—called only if such copies exist; 'parent' is the parent window of a
    // possible message box shown to the user (or a recommendation that the user close all files from the archive
    // so the plugin can delete them); 'copiesCount' is the number of in-use copies of files from the archive;
    // returns TRUE when DeleteTmpCopy should be called; if it returns FALSE, the copies remain on disk; when a
    // critical shutdown is in progress (see more in CSalamanderGeneralAbstract::IsCriticalShutdown), it makes no
    // sense to ask the user anything or to perform lengthy actions (for example file shredding)
    // NOTE: during the execution of PrematureDeleteTmpCopy it is guaranteed that DeleteTmpCopy will not be called
    virtual BOOL WINAPI PrematureDeleteTmpCopy(HWND parent, int copiesCount) = 0;
};

#ifdef _MSC_VER
#pragma pack(pop, enter_include_spl_arc)
#endif // _MSC_VER
#ifdef __BORLANDC__
#pragma option -a
#endif // __BORLANDC__
