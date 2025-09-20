// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

//******************************************************************************
//
// CShrinkImage
//

class CShrinkImage
{
protected:
    DWORD NormCoeffX, NormCoeffY;
    DWORD* RowCoeff;
    DWORD* ColCoeff;
    DWORD* YCoeff;
    DWORD NormCoeff;
    DWORD Y, YBndr;
    DWORD* OutLine;
    DWORD* Buff;
    DWORD OrigHeight;
    WORD NewWidth;
    BOOL ProcessTopDown;

public:
    CShrinkImage();
    ~CShrinkImage();

    // allocates internal data for resizing and returns TRUE on success
    // returns FALSE if any allocation fails
    BOOL Alloc(DWORD origWidth, DWORD origHeight,
               WORD newWidth, WORD newHeight,
               DWORD* outBuff, BOOL processTopDown);

    // destroys allocated buffers and resets variables
    void Destroy();

    void ProcessRows(DWORD* inBuff, DWORD rowCount);

protected:
    DWORD* CreateCoeff(DWORD origLen, WORD newLen, DWORD& norm);
    void Cleanup();
};

//******************************************************************************
//
// CSalamanderThumbnailMaker
//
// Used to scale the original image down to a thumbnail.
//

class CSalamanderThumbnailMaker : public CSalamanderThumbnailMakerAbstract
{
protected:
    CFilesWindow* Window; // files panel whose icon reader we operate in

    DWORD* Buffer;  // buffer for row data from the plugin
    int BufferSize; // size of 'Buffer'
    BOOL Error;     // TRUE if an error occurred while processing the thumbnail (output is unusable)
    int NextLine;   // index of the next row to process

    DWORD* ThumbnailBuffer;    // downscaled image
    DWORD* AuxTransformBuffer; // helper buffer equal to ThumbnailBuffer size (used to transfer data during transformation; buffers swap afterwards)
    int ThumbnailMaxWidth;     // maximal theoretical thumbnail width in pixels
    int ThumbnailMaxHeight;
    int ThumbnailRealWidth;  // actual width of the scaled image in pixels
    int ThumbnailRealHeight; // actual height

    // parameters of the image being processed
    int OriginalWidth;
    int OriginalHeight;
    DWORD PictureFlags;
    BOOL ProcessTopDown;

    CShrinkImage Shrinker; // performs the downscaling
    BOOL ShrinkImage;

public:
    CSalamanderThumbnailMaker(CFilesWindow* window);
    ~CSalamanderThumbnailMaker();

    // Cleans up the object before processing another thumbnail or when the
    // thumbnail (finished or not) is no longer needed. 'thumbnailMaxSize'
    // specifies the maximum width and height in pixels; if it is -1 the value
    // is ignored.
    void Clear(int thumbnailMaxSize = -1);

    // Returns TRUE when a complete thumbnail has been obtained from the plugin
    BOOL ThumbnailReady();

    // Applies transformations according to PictureFlags. SSTHUMB_MIRROR_VERT is
    // already handled; remaining flags include SSTHUMB_MIRROR_HOR and
    // SSTHUMB_ROTATE_90CW.
    void TransformThumbnail();

    // Converts the completed thumbnail to a DDB and stores its size and raw data in 'data'
    BOOL RenderToThumbnailData(CThumbnailData* data);

    // If the thumbnail was not fully created and no error occurred (see
    // 'Error'), fill the remaining area with white so remnants of the previous
    // thumbnail do not appear. If fewer than three rows were created, nothing is
    // filled because the thumbnail would be useless anyway.
    void HandleIncompleteImages();

    BOOL IsOnlyPreview() { return (PictureFlags & SSTHUMB_ONLY_PREVIEW) != 0; }

    // *********************************************************************************
    // methods of the CSalamanderThumbnailMakerAbstract interface
    // *********************************************************************************

    virtual BOOL WINAPI SetParameters(int picWidth, int picHeight, DWORD flags);
    virtual BOOL WINAPI ProcessBuffer(void* buffer, int rowsCount);
    virtual void* WINAPI GetBuffer(int rowsCount);
    virtual void WINAPI SetError() { Error = TRUE; }
    virtual BOOL WINAPI GetCancelProcessing();
};
