// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"

#include "plugins.h"
#include "fileswnd.h"
#include "thumbnl.h"
#include "cfgdlg.h"

//******************************************************************************
//
// CShrinkImage
//

CShrinkImage::CShrinkImage()
{
    Cleanup();
}

CShrinkImage::~CShrinkImage()
{
    Destroy();
}

void CShrinkImage::Cleanup()
{
    NormCoeffX = 0;
    NormCoeffY = 0;
    RowCoeff = NULL;
    ColCoeff = NULL;
    YCoeff = NULL;
    NormCoeff = 0;
    Y = 0;
    YBndr = 0;
    OutLine = NULL;
    Buff = NULL;
    OrigHeight = 0;
    NewWidth = 0;
    ProcessTopDown = TRUE;
}

BOOL CShrinkImage::Alloc(DWORD origWidth, DWORD origHeight,
                         WORD newWidth, WORD newHeight,
                         DWORD* outBuff, BOOL processTopDown)
{
#ifdef _DEBUG
    if (RowCoeff != NULL || ColCoeff != NULL || Buff != NULL)
        TRACE_E("RowCoeff != NULL || ColCoeff != NULL || Buff != NULL");
#endif // _DEBUG
    if (origWidth == 0 || origHeight == 0 || newWidth == 0 || newHeight == 0)
    {
        TRACE_E("origWidth == 0 || origHeight == 0 || newWidth == 0 || newHeight == 0");
        return FALSE;
    }
    // allocate and initialize coefficients
    RowCoeff = CreateCoeff(origWidth, newWidth, NormCoeffX);
    ColCoeff = CreateCoeff(origHeight, newHeight, NormCoeffY);
    // allocate and clear the buffer
    Buff = (DWORD*)malloc(3 * newWidth * sizeof(DWORD));
    if (RowCoeff == NULL || ColCoeff == NULL || Buff == NULL)
    {
        TRACE_E(LOW_MEMORY);
        Destroy();
        return FALSE;
    }

    ZeroMemory(Buff, 3 * newWidth * sizeof(DWORD));

    OrigHeight = origHeight;
    NewWidth = newWidth;
    ProcessTopDown = processTopDown;

    YCoeff = ColCoeff;
    // coefficients for the middle and right pixel for a possible next pass
    NormCoeff = NormCoeffY * NormCoeffX;
    // y-section boundary
    YBndr = *YCoeff++;
    // skip the coefficient for the first line
    YCoeff++;

    // when processing bottom-up we must start at the last line
    if (!ProcessTopDown)
        OutLine = outBuff + newWidth * (newHeight - 1);
    else
        OutLine = outBuff;

    return TRUE;
}

void CShrinkImage::Destroy()
{
    if (RowCoeff != NULL)
        free(RowCoeff);
    if (ColCoeff != NULL)
        free(ColCoeff);
    if (Buff != NULL)
        free(Buff);
    Cleanup();
}

DWORD*
CShrinkImage::CreateCoeff(DWORD origLen, WORD newLen, DWORD& norm)
{
    DWORD* res = (DWORD*)malloc(3 * newLen * sizeof(DWORD));
    if (res == NULL)
        return NULL;
    DWORD* coeff = res;
    DWORD sum = 0;
    DWORD lCoeff, rCoeff = 0;
    DWORD boundary, modulo;

    norm = (newLen << 12) / origLen;
    DWORD i;
    for (i = 0; i < newLen; i++)
    {
        sum += origLen;
        // compute the pixel crossed by the new boundary
        boundary = sum / newLen;
        // how much of the previous boundary will be on the left part of this section
        lCoeff = norm - rCoeff;
        // and finally the weight of the pixel at the right edge of the section
        modulo = sum % newLen;
        if (modulo == 0)
        {
            // if the boundary falls between pixels, prefer the left pixel
            boundary--;
            rCoeff = norm;
        }
        else
            rCoeff = (modulo << 12) / origLen;
        // store into the array - first is the boundary coordinate
        *coeff++ = boundary;
        // next is the weight of the left-edge pixel
        *coeff++ = lCoeff;
        // and the weight at the right edge
        *coeff++ = rCoeff;
    }
    return res;
}

void CShrinkImage::ProcessRows(DWORD* inBuff, DWORD rowCount)
{
    DWORD* ptrXCoeff;
    DWORD xCoeff, yCoeff, xNewCoeff;
    DWORD x1, x2, xBndr;
    DWORD* currPix;
    BYTE r, g, b;
    DWORD rgb;

    // iterate through all rows
    DWORD y;
    for (y = Y; y < Y + rowCount; y++)
    {
        // initialize pointers into the buffer
        currPix = Buff;
        // initialize pointer to the coefficient array
        ptrXCoeff = RowCoeff;
        // maximum x-coordinate
        xBndr = *ptrXCoeff++;
        // the left coefficient equals the middle one at the start of the line
        ptrXCoeff++;
        // right coefficient
        xCoeff = *ptrXCoeff++;

        x2 = 0;
        // branch based on the row position in the section (middle or last)
        if (y == YBndr)
        {
            // retrieve the coefficient for the last line
            DWORD yLastCoeff = *YCoeff++;
            // retrieve the coefficient for the first line of the next section (if any)
            if (y + 1 < OrigHeight)
            {
                YBndr = *YCoeff++; // new y boundary of the section
                yCoeff = *YCoeff++;
            }
            else
            {
                YBndr = 0; // new y boundary of the section
                yCoeff = 0;
            }
            // coefficients for the middle and right pixel
            xNewCoeff = yCoeff * xCoeff;
            xCoeff *= yLastCoeff;
            // coefficients for the next line
            DWORD midNewCoeff = yCoeff * NormCoeffX;
            DWORD midCoeff = yLastCoeff * NormCoeffX;
            // helper variables for the next line pixel
            DWORD nextR = 0;
            DWORD nextG = 0;
            DWORD nextB = 0;
            // and precompute the next one
            for (x1 = 0; x1 + 1 < NewWidth; x1++)
            {
                // if we are on the last line, store the current one into the result
                // process the middle part
                for (; x2 < xBndr; x2++)
                {
                    // get the pixel
                    rgb = *inBuff++;
                    r = GetRValue(rgb);
                    g = GetGValue(rgb);
                    b = GetBValue(rgb);
                    // add it to the buffer
                    currPix[0] += midCoeff * r;
                    currPix[1] += midCoeff * g;
                    currPix[2] += midCoeff * b;
                    // and prepare the pixel from the next line
                    nextR += midNewCoeff * r;
                    nextG += midNewCoeff * g;
                    nextB += midNewCoeff * b;
                }
                // get the rightmost pixel
                rgb = *inBuff++;
                r = GetRValue(rgb);
                g = GetGValue(rgb);
                b = GetBValue(rgb);
                // the calculated pixel can now be sent to the output
                *OutLine++ = RGB((currPix[0] + xCoeff * r) >> 24,
                                 (currPix[1] + xCoeff * g) >> 24,
                                 (currPix[2] + xCoeff * b) >> 24);
                // prepare the pixel for the next line
                currPix[0] = nextR + xNewCoeff * r;
                currPix[1] = nextG + xNewCoeff * g;
                currPix[2] = nextB + xNewCoeff * b;
                // increase the coordinate
                x2++;
                // move in the output to the next pixel
                currPix += 3;
                // new maximum x-coordinate
                xBndr = *ptrXCoeff++;
                // new left coefficient for both rows
                xNewCoeff = yCoeff * *ptrXCoeff;
                xCoeff = yLastCoeff * *ptrXCoeff++;
                // and add it to the buffer for the next pixel
                currPix[0] += xCoeff * r;
                currPix[1] += xCoeff * g;
                currPix[2] += xCoeff * b;
                // and prepare the pixel from the next line
                nextR = xNewCoeff * r;
                nextG = xNewCoeff * g;
                nextB = xNewCoeff * b;
                // and a new right coefficient
                xNewCoeff = yCoeff * *ptrXCoeff;
                xCoeff = yLastCoeff * *ptrXCoeff++;
            }
            // for the last pixel we must skip computing the left part
            // of the next pixel (there is none)
            for (; x2 < xBndr; x2++)
            {
                // get the pixel
                rgb = *inBuff++;
                r = GetRValue(rgb);
                g = GetGValue(rgb);
                b = GetBValue(rgb);
                // add it to the buffer
                currPix[0] += midCoeff * r;
                currPix[1] += midCoeff * g;
                currPix[2] += midCoeff * b;
                // and prepare the pixel from the next line
                nextR += midNewCoeff * r;
                nextG += midNewCoeff * g;
                nextB += midNewCoeff * b;
            }
            // get the rightmost pixel
            rgb = *inBuff++;
            r = GetRValue(rgb);
            g = GetGValue(rgb);
            b = GetBValue(rgb);
            // the calculated pixel can now be sent to the output
            *OutLine++ = RGB((currPix[0] + xCoeff * r) >> 24,
                             (currPix[1] + xCoeff * g) >> 24,
                             (currPix[2] + xCoeff * b) >> 24);
            // prepare the pixel for the next line
            currPix[0] = nextR + xNewCoeff * r;
            currPix[1] = nextG + xNewCoeff * g;
            currPix[2] = nextB + xNewCoeff * b;
            // the entire line is finished

            // if processing bottom-up, move one line up
            if (!ProcessTopDown)
                OutLine -= NewWidth * 2;
        }
        else
        {
            // right coefficient
            xCoeff *= NormCoeffY;
            // when on middle lines, compute normally
            for (x1 = 0; x1 + 1 < NewWidth; x1++)
            {
                // process the middle section
                for (; x2 < xBndr; x2++)
                {
                    // grab the pixel
                    rgb = *inBuff++;
                    r = GetRValue(rgb);
                    g = GetGValue(rgb);
                    b = GetBValue(rgb);
                    // add it to the buffer
                    currPix[0] += NormCoeff * r;
                    currPix[1] += NormCoeff * g;
                    currPix[2] += NormCoeff * b;
                }
                // get the rightmost pixel
                rgb = *inBuff++;
                r = GetRValue(rgb);
                g = GetGValue(rgb);
                b = GetBValue(rgb);
                // and also add it to the buffer
                currPix[0] += xCoeff * r;
                currPix[1] += xCoeff * g;
                currPix[2] += xCoeff * b;
                // increase the coordinate
                x2++;
                // move forward in the output to the next pixel
                currPix += 3;
                // new maximum x-coordinate
                xBndr = *ptrXCoeff++;
                // new left coefficient
                xCoeff = NormCoeffY * *ptrXCoeff++;
                // and also add it to the buffer for the next pixel
                currPix[0] += xCoeff * r;
                currPix[1] += xCoeff * g;
                currPix[2] += xCoeff * b;
                // and a new right coefficient
                xCoeff = NormCoeffY * *ptrXCoeff++;
            }
            // for the last pixel we must skip computing the left part
            for (; x2 < xBndr; x2++)
            {
                // get the pixel
                rgb = *inBuff++;
                r = GetRValue(rgb);
                g = GetGValue(rgb);
                b = GetBValue(rgb);
                // add it to the buffer
                currPix[0] += NormCoeff * r;
                currPix[1] += NormCoeff * g;
                currPix[2] += NormCoeff * b;
            }
            // get the rightmost pixel
            rgb = *inBuff++;
            r = GetRValue(rgb);
            g = GetGValue(rgb);
            b = GetBValue(rgb);
            // and also add it to the buffer
            currPix[0] += xCoeff * r;
            currPix[1] += xCoeff * g;
            currPix[2] += xCoeff * b;
            // we finished the entire line
        }
    }
    Y += rowCount;
}

//******************************************************************************
//
// CSalamanderThumbnailMaker
//

CSalamanderThumbnailMaker::CSalamanderThumbnailMaker(CFilesWindow* window)
{
    Window = window;
    Buffer = NULL;
    BufferSize = 0;

    ThumbnailBuffer = NULL;
    AuxTransformBuffer = NULL;
    ThumbnailMaxWidth = 0; // we must initialize because Clear() is called with -1
    ThumbnailMaxHeight = 0;

    Clear(-1);
}

CSalamanderThumbnailMaker::~CSalamanderThumbnailMaker()
{
    if (Buffer != NULL)
        free(Buffer);
    if (ThumbnailBuffer != NULL)
        free(ThumbnailBuffer);
    if (AuxTransformBuffer != NULL)
        free(AuxTransformBuffer);
}

// object cleanup - called before processing the next thumbnail or when this object's
// thumbnail is no longer needed (finished or not)
void CSalamanderThumbnailMaker::Clear(int thumbnailMaxSize)
{
    Error = FALSE;
    NextLine = -1;

    OriginalWidth = 0;
    OriginalHeight = 0;
    PictureFlags = 0;
    ProcessTopDown = TRUE;

    ThumbnailRealWidth = 0;
    ThumbnailRealHeight = 0;

    if (thumbnailMaxSize != -1)
    {
        if (thumbnailMaxSize != ThumbnailMaxWidth || thumbnailMaxSize != ThumbnailMaxHeight)
        {
            // if the user changed the thumbnail size (in the configuration) we must
            // allocate ThumbnailBuffer and AuxTransformBuffer again
            if (ThumbnailBuffer != NULL)
            {
                free(ThumbnailBuffer);
                ThumbnailBuffer = NULL;
            }
            if (AuxTransformBuffer != NULL)
            {
                free(AuxTransformBuffer);
                AuxTransformBuffer = NULL;
            }
            ThumbnailMaxWidth = thumbnailMaxSize;
            ThumbnailMaxHeight = thumbnailMaxSize;
        }
    }

    ShrinkImage = FALSE;
    Shrinker.Destroy();
}

// returns TRUE if this object already contains the entire thumbnail
// successfully obtained from a plugin
BOOL CSalamanderThumbnailMaker::ThumbnailReady()
{
    return OriginalHeight != 0 && NextLine >= OriginalHeight && !Error;
}

void CSalamanderThumbnailMaker::TransformThumbnail()
{
    // SSTHUMB_MIRROR_VERT is already done; now perform SSTHUMB_MIRROR_HOR and SSTHUMB_ROTATE_90CW
    int transformation = (PictureFlags & (SSTHUMB_MIRROR_HOR | SSTHUMB_ROTATE_90CW));
    switch (transformation)
    {
    case 0:
        break; // nothing to do

    case SSTHUMB_MIRROR_HOR:
    {
        DWORD realWidth = ThumbnailRealWidth;
        DWORD realWidth_min1 = ThumbnailRealWidth - 1;
        DWORD realHeight = ThumbnailRealHeight;
        DWORD* lineData = ThumbnailBuffer;
        DWORD* lineDataTgt = AuxTransformBuffer;
        DWORD line;
        for (line = 0; line < realHeight; line++)
        {
            DWORD i;
            for (i = 0; i < realWidth; i++)
                lineDataTgt[realWidth_min1 - i] = lineData[i];
            lineData += realWidth;
            lineDataTgt += realWidth;
        }
        DWORD* swap = ThumbnailBuffer;
        ThumbnailBuffer = AuxTransformBuffer;
        AuxTransformBuffer = swap;
        break;
    }

    case SSTHUMB_MIRROR_HOR | SSTHUMB_ROTATE_90CW:
    {
        DWORD realWidth = ThumbnailRealWidth;
        DWORD endOffset = ThumbnailRealHeight * ThumbnailRealWidth - 1;
        DWORD realHeight = ThumbnailRealHeight;
        DWORD realHeight_min1 = ThumbnailRealHeight - 1;
        DWORD* lineData = ThumbnailBuffer;
        DWORD* dataTgt = AuxTransformBuffer;
        DWORD line;
        for (line = 0; line < realHeight; line++)
        {
            DWORD offset = endOffset - line;
            DWORD i;
            for (i = 0; i < realWidth; i++)
            {
                dataTgt[offset] = lineData[i];
                offset -= realHeight;
            }
            lineData += realWidth;
        }
        DWORD* swap = ThumbnailBuffer;
        ThumbnailBuffer = AuxTransformBuffer;
        AuxTransformBuffer = swap;
        ThumbnailRealWidth = realHeight;
        ThumbnailRealHeight = realWidth;
        break;
    }

    case SSTHUMB_ROTATE_90CW:
    {
        DWORD realWidth = ThumbnailRealWidth;
        DWORD realHeight = ThumbnailRealHeight;
        DWORD realHeight_min1 = ThumbnailRealHeight - 1;
        DWORD* lineData = ThumbnailBuffer;
        DWORD* dataTgt = AuxTransformBuffer;
        DWORD line;
        for (line = 0; line < realHeight; line++)
        {
            DWORD offset = realHeight_min1 - line;
            DWORD i;
            for (i = 0; i < realWidth; i++)
            {
                dataTgt[offset] = lineData[i];
                offset += realHeight;
            }
            lineData += realWidth;
        }
        DWORD* swap = ThumbnailBuffer;
        ThumbnailBuffer = AuxTransformBuffer;
        AuxTransformBuffer = swap;
        ThumbnailRealWidth = realHeight;
        ThumbnailRealHeight = realWidth;
        break;
    }
    }
}

// convert the held thumbnail to a DDB and store its data in CThumbnailData
BOOL CSalamanderThumbnailMaker::RenderToThumbnailData(CThumbnailData* data)
{
    // create a DDB and let it initialize with the thumbnail's RGB data
    HDC hDC = HANDLES(GetDC(NULL));
    BITMAPINFO srcBI;
    memset(&srcBI, 0, sizeof(BITMAPINFO));
    srcBI.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    srcBI.bmiHeader.biWidth = ThumbnailRealWidth;
    srcBI.bmiHeader.biHeight = -ThumbnailRealHeight; // we have a top-down representation
    srcBI.bmiHeader.biPlanes = 1;
    srcBI.bmiHeader.biBitCount = 32;
    srcBI.bmiHeader.biCompression = BI_RGB;
    HBITMAP hBmp = HANDLES(CreateDIBitmap(hDC, &srcBI.bmiHeader, CBM_INIT,
                                          ThumbnailBuffer, &srcBI, DIB_RGB_COLORS));
    HANDLES(ReleaseDC(NULL, hDC));

    if (hBmp == NULL)
    {
        TRACE_E("Error creating bitmap!");
        return FALSE;
    }

    // obtain the geometry of the newly created bitmap
    BITMAP bitmap;
    if (GetObject(hBmp, sizeof(BITMAP), &bitmap) == NULL)
    {
        TRACE_E("GetObject failed!");
        HANDLES(DeleteObject(hBmp));
        return FALSE;
    }

    // allocate a buffer for raw bitmap data
    // it will be freed in CIconCache::Destroy() or a few lines below
    // if this is a refresh

    DWORD rawSize = bitmap.bmWidthBytes * bitmap.bmPlanes * bitmap.bmHeight;

    DWORD* bits = (DWORD*)malloc(rawSize);
    if (bits == NULL)
    {
        TRACE_E(LOW_MEMORY);
        HANDLES(DeleteObject(hBmp));
        return FALSE;
    }

    // get the raw data into the allocated array
    if (GetBitmapBits(hBmp, rawSize, bits) == NULL)
    {
        TRACE_E("GetBitmapBits failed!");
        free(bits);
        HANDLES(DeleteObject(hBmp));
        return FALSE;
    }

    // discard the bitmap
    HANDLES(DeleteObject(hBmp));

    // if we already hold some data, we must free them for the new ones
    if (data->Bits != NULL)
        free(data->Bits);

    // store the result
    data->Width = (WORD)bitmap.bmWidth;
    data->Height = (WORD)bitmap.bmHeight;
    data->Planes = bitmap.bmPlanes;
    data->BitsPerPixel = bitmap.bmBitsPixel;
    data->Bits = bits;

    return TRUE;
}

void CSalamanderThumbnailMaker::HandleIncompleteImages()
{
    if (!Error && NextLine < OriginalHeight && ThumbnailRealHeight > 0 &&
        NextLine >= (3 * OriginalHeight / ThumbnailRealHeight) &&
        !Window->ICStopWork && OriginalWidth > 0)
    {
        if (GetBuffer(1) != NULL)
        {
            memset(Buffer, 0xFF, BufferSize);
            int maxRowsInBuf = BufferSize / OriginalWidth / sizeof(DWORD);
            if (maxRowsInBuf > 0)
            {
                while (NextLine < OriginalHeight)
                {
                    if (!ProcessBuffer(Buffer, min(maxRowsInBuf, OriginalHeight - NextLine)))
                        break;
                }
            }
            else
                TRACE_E("CSalamanderThumbnailMaker::HandleIncompleteImages(): this should never happen!");
        }
    }
}

// *********************************************************************************
// methods of the CSalamanderThumbnailMakerAbstract interface
// *********************************************************************************

BOOL CSalamanderThumbnailMaker::SetParameters(int picWidth, int picHeight, DWORD flags)
{
    if (Error)
    {
        TRACE_E("CSalamanderThumbnailMaker::SetParameters(): Error == TRUE");
        return FALSE;
    }
    if (picWidth < 1 || picHeight < 1)
    {
        TRACE_E("CSalamanderThumbnailMaker::SetParameters invalid parameters: picWidth=" << picWidth << " picHeight=" << picHeight);
        Error = TRUE;
        return FALSE;
    }
    OriginalWidth = picWidth;
    OriginalHeight = picHeight;
    PictureFlags = flags;
    ProcessTopDown = (flags & SSTHUMB_MIRROR_VERT) == 0;

    int maxWidth = ThumbnailMaxWidth; // maximum thumbnail width
    int maxHeight = ThumbnailMaxHeight;

    if (maxWidth < 1 || maxHeight < 1)
    {
        TRACE_E("CSalamanderThumbnailMaker::SetParameters invalid parameters: ThumbnailMaxWidth=" << maxWidth << " or ThumbnailMaxHeight=" << maxHeight);
        Error = TRUE;
        return FALSE;
    }

    if (OriginalWidth <= maxWidth && OriginalHeight <= maxHeight)
    {
        // copy the data directly
        ThumbnailRealWidth = OriginalWidth;
        ThumbnailRealHeight = OriginalHeight;
        ShrinkImage = FALSE;
    }
    else
    {
        // keep the aspect ratio
        if ((double)maxWidth / (double)maxHeight < (double)OriginalWidth / (double)OriginalHeight)
        {
            ThumbnailRealWidth = maxWidth;
            ThumbnailRealHeight = (int)((double)maxWidth / ((double)OriginalWidth / (double)OriginalHeight));
        }
        else
        {
            ThumbnailRealHeight = maxHeight;
            ThumbnailRealWidth = (int)((double)maxHeight / ((double)OriginalHeight / (double)OriginalWidth));
        }
        // the algorithm cannot accept zero dimensions, so violate proportions if necessary
        if (ThumbnailRealWidth < 1)
            ThumbnailRealWidth = 1;
        if (ThumbnailRealHeight < 1)
            ThumbnailRealHeight = 1;
        ShrinkImage = TRUE;
    }

    if (ThumbnailBuffer == NULL)
        ThumbnailBuffer = (DWORD*)malloc(maxWidth * maxHeight * sizeof(DWORD));
    if (AuxTransformBuffer == NULL)
        AuxTransformBuffer = (DWORD*)malloc(maxWidth * maxHeight * sizeof(DWORD));
    if (ThumbnailBuffer == NULL || AuxTransformBuffer == NULL)
    {
        if (ThumbnailBuffer != NULL)
            free(ThumbnailBuffer);
        if (AuxTransformBuffer != NULL)
            free(AuxTransformBuffer);
        ThumbnailBuffer = NULL;
        AuxTransformBuffer = NULL;
        TRACE_E(LOW_MEMORY);
        Error = TRUE;
        return FALSE;
    }

    if (ShrinkImage)
    {
        Shrinker.Destroy();
        if (!Shrinker.Alloc(OriginalWidth, OriginalHeight,
                            ThumbnailRealWidth, ThumbnailRealHeight,
                            ThumbnailBuffer, ProcessTopDown))
        {
            Error = TRUE;
            return FALSE;
        }
    }

    NextLine = 0;
    return TRUE;
}

BOOL CSalamanderThumbnailMaker::GetCancelProcessing()
{
    if (Error || NextLine >= OriginalHeight || Window->ICStopWork)
        return TRUE;
    else
        return FALSE;
}

BOOL CSalamanderThumbnailMaker::ProcessBuffer(void* buffer, int rowsCount)
{
    if (Error || NextLine >= OriginalHeight || Window->ICStopWork)
    {
        if (!Window->ICStopWork)
            TRACE_E("CSalamanderThumbnailMaker::ProcessBuffer failed. Error=" << Error << " NextLine=" << NextLine << " OriginalHeight=" << OriginalHeight);
        return FALSE; // we will exit (error, overflow or sleep-icon-cache)
    }
    if (NextLine == -1)
    {
        TRACE_E("Call SetParameters before ProcessBuffer!");
        return FALSE;
    }
#ifdef _DEBUG
    if (NextLine + rowsCount > OriginalHeight)
    {
        TRACE_E("CSalamanderThumbnailMaker::ProcessBuffer(): Too much rows (" << rowsCount << ") to process (they overlap picture)!");
        Error = TRUE;
        return FALSE;
    }
#endif // _DEBUG
    if (buffer == NULL)
    {
        buffer = Buffer;
#ifdef _DEBUG
        if (BufferSize / OriginalWidth / (int)sizeof(DWORD) < rowsCount)
        {
            TRACE_E("CSalamanderThumbnailMaker::ProcessBuffer(): Too much rows (" << rowsCount << ") in internal buffer! (insufficient size of buffer)");
            Error = TRUE;
            return FALSE;
        }
#endif // _DEBUG
    }

    if (rowsCount > 0)
    {
        if (ShrinkImage)
        {
            // reduce to thumbnail size
            Shrinker.ProcessRows((DWORD*)buffer, rowsCount);
        }
        else
        {
            // copy one-to-one
            if (ProcessTopDown)
            {
                memcpy(ThumbnailBuffer + NextLine * ThumbnailRealWidth, buffer, rowsCount * ThumbnailRealWidth * sizeof(DWORD));
            }
            else
            {
                int i;
                for (i = 0; i < rowsCount; i++)
                    memcpy(ThumbnailBuffer + (OriginalHeight - NextLine - i - 1) * ThumbnailRealWidth,
                           (DWORD*)buffer + i * ThumbnailRealWidth, ThumbnailRealWidth * sizeof(DWORD));
            }
        }
        NextLine += rowsCount;
    }

    return NextLine < OriginalHeight;
}

void* CSalamanderThumbnailMaker::GetBuffer(int rowsCount)
{
    if (Error)
    {
        TRACE_E("CSalamanderThumbnailMaker::GetBuffer(): Error == TRUE");
        return NULL;
    }
    int required = rowsCount * OriginalWidth * sizeof(DWORD);
    if (required > BufferSize)
    {
        if (Buffer != NULL)
            free(Buffer);
        Buffer = (DWORD*)malloc(required);
        if (Buffer != NULL)
            BufferSize = required;
        else
        {
            BufferSize = 0;
            TRACE_E("CSalamanderThumbnailMaker::GetBuffer(): Unable to allocate internal buffer (size=" << required << ")!");
        }
    }
    return Buffer;
}
