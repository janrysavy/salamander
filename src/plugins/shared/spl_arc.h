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

    // function for the "panel archiver view"; called to load the contents of archive
    // 'fileName'; the contents are stored in the 'dir' object; Salamander gets
    // the values of columns added by the plugin through the 'pluginData' interface (if the
    // plugin does not add columns, it sets 'pluginData' == NULL); returns TRUE if the
    // archive contents were loaded successfully; if it returns FALSE, the value of
    // 'pluginData' is ignored (the data in 'dir' must be released using 'dir.Clear(pluginData)',
    // otherwise only the Salamander-owned part of the data is released);
    // 'salamander' is a set of useful methods exported by Salamander;
    // WARNING: the file 'fileName' may also not exist (for example if it is open in the panel
    // and deleted from elsewhere); ListArchive is not called for zero-length files; those
    // automatically have empty contents; when packing into such files, the file is deleted
    // before calling PackToArchive (for compatibility with external packers)
    virtual BOOL WINAPI ListArchive(CSalamanderForOperationsAbstract* salamander, const char* fileName,
                                    CSalamanderDirectoryAbstract* dir,
                                    CPluginDataInterfaceAbstract*& pluginData) = 0;

    // function for the "panel archiver view"; called when a request is made to unpack files/directories
    // from archive 'fileName' to directory 'targetDir' from the path in the archive 'archiveRoot';
    // 'pluginData' is an interface for working with file/directory information specific to the plugin
    // (for example, data from added columns; it is the same interface returned by ListArchive
    // in the 'pluginData' parameter, so it may also be NULL); the files/directories are specified
    // by the enumeration function 'next', whose parameter is 'nextParam'; returns TRUE on
    // successful unpacking (Cancel was not used; Skip may have been) - the source items in the panel
    // are deselected; otherwise returns FALSE (they are not deselected);
    // 'salamander' is a set of useful methods exported by Salamander
    virtual BOOL WINAPI UnpackArchive(CSalamanderForOperationsAbstract* salamander, const char* fileName,
                                      CPluginDataInterfaceAbstract* pluginData, const char* targetDir,
                                      const char* archiveRoot, SalEnumSelection next,
                                      void* nextParam) = 0;

    // function for the "panel archiver view"; called when requested to unpack a single file for viewing/editing
    // from archive 'fileName' into directory 'targetDir'; the file name in the archive is 'nameInArchive';
    // 'pluginData' is the interface for working with file information specific to the plugin
    // (for example, data from added columns; it is the same interface returned by the ListArchive method
    // in the 'pluginData' parameter, so it may also be NULL); 'fileData' is a pointer to the CFileData
    // structure of the unpacked file (the structure was built by the plugin when listing the archive);
    // 'newFileName' (if not NULL) is a new name for the extracted file (used when the original archive name
    // cannot be unpacked to disk, for example "aux", "prn", etc.); set 'renamingNotSupported' to TRUE
    // (only if 'newFileName' is not NULL) if the plugin does not support renaming during unpacking
    // (the standard error message "renaming not supported" is then displayed by Salamander);
    // returns TRUE if the file is unpacked successfully (the file is at the specified path and neither
    // Cancel nor Skip was used); 'salamander' is a set of useful methods exported by Salamander
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

    // function for "panel archiver edit"; called when a request is made to delete files/directories from the archive
    // 'fileName'; the files/directories are specified by the path 'archiveRoot' and the enumeration function
    // 'next' with parameter 'nextParam'; 'pluginData' is an interface for working with file/directory
    // information specific to the plugin (for example, data from added columns; it is the same interface
    // returned by the ListArchive method in the 'pluginData' parameter, so it may also be NULL); returns TRUE if it
    // succeeds in deleting all files/directories (Cancel was not used; Skip may have been) - the source
    // items in the panel are deselected; otherwise it returns FALSE (they are not deselected);
    // 'salamander' is a set of useful methods exported by Salamander
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
    // NOTE: if a new path cannot be opened, the archive may remain open in the panel (regardless of
    //        what CanCloseArchive returns); therefore this method cannot be used to destroy the context;
    //        it is intended, for example, to optimize the Delete operation from the archive, when it can
    //        offer to "compact" the archive when leaving it;
    //        to destroy the context, use the method CPluginInterfaceAbstract::ReleasePluginDataInterface;
    //        see the document archivatory.txt
    // 'fileName' is the archive name; 'salamander' is a set of helper methods exported from Salamander;
    // 'panel' denotes the panel in which the archive is open (PANEL_LEFT or PANEL_RIGHT);
    // returns TRUE if closing is possible; if 'force' is TRUE, it always returns TRUE; if a critical shutdown
    // is in progress (for more, see CSalamanderGeneralAbstract::IsCriticalShutdown), there is no point
    // in asking the user anything
    virtual BOOL WINAPI CanCloseArchive(CSalamanderForOperationsAbstract* salamander, const char* fileName,
                                        BOOL force, int panel) = 0;

    // determines the required disk-cache settings (the disk cache is used for temporary copies of files
    // when opening files from an archive in viewers, editors, and through system associations); normally
    // (if allocating a copy of 'tempPath' after the call succeeds) it is called only once, namely before
    // the first use of the disk cache (for example before the first file from an archive is opened in a
    // viewer/editor); if it returns FALSE, the standard settings are used (files in the TEMP directory,
    // copies are deleted using the Win32 API function DeleteFile() only after the cache size limit is
    // exceeded or when the archive is closed) and all other output values are ignored; if it returns TRUE,
    // the following output values are used: if 'tempPath' (a buffer of size MAX_PATH) is not an empty string,
    // all temporary copies extracted by the plugin from the archive are stored in subdirectories of this path
    // (these subdirectories are removed by the disk cache when Salamander exits, but nothing prevents the
    // plugin from deleting them earlier, for example during unload; at the same time, when loading the first
    // instance of the plugin (not only within one running Salamander), it is advisable to clean out the
    // "SAL*.tmp" subdirectories on this path - this solves problems caused by locked files and software
    // crashes) + if 'ownDelete' is TRUE, the methods DeleteTmpCopy and PrematureDeleteTmpCopy will be called
    // to delete the copies + if 'cacheCopies' is FALSE, the copies will be deleted as soon as they are
    // released (for example as soon as the viewer is closed); if 'cacheCopies' is TRUE, the copies will be
    // deleted only after the cache size limit is exceeded or when the archive is closed
    virtual BOOL WINAPI GetCacheInfo(char* tempPath, BOOL* ownDelete, BOOL* cacheCopies) = 0;

    // used only if GetCacheInfo returns TRUE in the 'ownDelete' parameter:
    // deletes a temporary copy extracted from this archive (watch out for read-only files;
    // their attributes must be changed first before they can be deleted); if possible,
    // it should not display any windows (the user did not trigger this action directly, so
    // it could distract them from other work); for longer actions, it is useful to use a
    // wait window (see CSalamanderGeneralAbstract::CreateSafeWaitWindow); 'fileName' is the
    // name of the file containing the copy; if multiple files are deleted at once (this can
    // happen, for example, after closing an edited archive), 'firstFile' is TRUE for the
    // first file and FALSE for the others (used for correct wait-window display - see
    // DEMOPLUG)
    //
    // WARNING: called in the main thread when a message from the disk cache is delivered
    // to the main window - a message requesting that a temporary copy be released
    // (typically when the viewer or the "edited" archive in the panel is closed), so
    // re-entry into the plugin may occur (if the message is dispatched by a message loop
    // inside the plugin); another entry into DeleteTmpCopy is excluded, because until the
    // DeleteTmpCopy call finishes, the disk cache does not send any further messages
    virtual void WINAPI DeleteTmpCopy(const char* fileName, BOOL firstFile) = 0;

    // used only if GetCacheInfo sets the 'ownDelete' parameter to TRUE:
    // during plugin unload, determines whether DeleteTmpCopy should be called for copies that are
    // still in use (e.g. open in the viewer) - called only if such copies exist; 'parent' is the
    // parent window of a possible message box asking the user (or possibly recommending that the
    // user close all files from the archive so that the plugin can delete them); 'copiesCount' is
    // the number of copies of archive files currently in use; returns TRUE if DeleteTmpCopy should
    // be called; if it returns FALSE, the copies remain on disk; if a critical shutdown is in
    // progress (for more details see CSalamanderGeneralAbstract::IsCriticalShutdown), there is no
    // point in asking the user anything or performing lengthy actions (e.g. file shredding)
    // NOTE: while PrematureDeleteTmpCopy is running, DeleteTmpCopy is guaranteed not to be called
    virtual BOOL WINAPI PrematureDeleteTmpCopy(HWND parent, int copiesCount) = 0;
};

#ifdef _MSC_VER
#pragma pack(pop, enter_include_spl_arc)
#endif // _MSC_VER
#ifdef __BORLANDC__
#pragma option -a
#endif // __BORLANDC__
