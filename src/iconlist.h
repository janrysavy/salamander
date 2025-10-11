// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

/************************************************************************************

// What can we extract from an HICON provided by the OS?

// Using GetIconInfo() the OS returns copies of the MASK and COLOR bitmaps. These can
// be examined via GetObject() to obtain geometry and color arrangement. They are only
// copies, not the original bitmaps stored inside the OS. MASK is always a 1-bit bitmap
// while COLOR is compatible with the screen DC, so there's no way to learn the real
// color depth of COLOR from these data.

// A special case are purely black-and-white icons. They are delivered entirely in MASK,
// which is then twice as high. COLOR is NULL and the upper half of MASK is the AND part
// while the lower half is the XOR part. This can be detected simply by checking COLOR == NULL.

// Since Windows XP there is another special case: icons with an ALPHA channel. These are
// DI B bitmaps with a depth of 32 bits where each pixel consists of ARGB components.

  




************************************************************************************/

//
// There is potential room for optimization in our ImageList implementation.
// We could keep the DIB in the same format that the screen uses. According to
// MSDN this would allegedly make BitBlt faster (I did not verify it):
//   http://support.microsoft.com/default.aspx?scid=kb;EN-US;230492
//   (HOWTO: Retrieving an Optimal DIB Format for a Device)
//
// Several points speak against this optimization:
//   - the code would need to support many data formats (15, 16, 24, 32 bit)
//   - because we draw at most a few dozen icons at once, drawing speed is not
//     critical for us; these were the measured drawing times when
//     BitBlt-ing a 16x16 32bpp DIB 100 000 times:
//     screen resolution    total time    (W2K, Matrox G450)
//     32 bpp                  0.40 s
//     24 bpp                  0.80 s
//     16 bpp                  0.65 s
//      8 bpp                  1.16 s
//   - We would still have to keep icons with an ALPHA channel, which are 32 bpp anyway
//

//
// Why we need our own ImageList implementation:
//
// The CommonControls ImageList has one major problem: when asked to hold DeviceDependentBitmaps
// it can't display blended items and instead fills them with a pattern.
//
// When a DIB bitmap is stored, blending works fine but drawing a normal item is much slower
// due to the DIB->screen conversion.
//
// There is also a risk that some implementations of ImageList_SetBkColor don't physically
// modify the stored bitmap using the mask and only set an internal variable. Drawing then
// becomes slower because masking must be performed. Tested under W2K the function behaved correctly.
//
// The only option would be to keep the ImageList for storing data and reimplement blending.
// The issue is that ImageList_GetImageInfo exposes the internal Image/Mask bitmaps, which are
// always selected into a MemDC. According to MSDN (Q131279: SelectObject() Fails After
// ImageList_GetImageInfo()) you must call CopyImage first before working with the bitmap,
// leading to incredibly slow drawing of blended items.
//
// Another issue for ImageList are icons with invert bodies. An icon consists of two bitmaps:
// MASK and COLORS. MASK is ANDed into the destination and COLORS are XORed over it. Thanks to
// the XOR step icons can invert some parts of themselves. Cursors use this heavily; see WINDOWS\Cursors.
//

//******************************************************************************
//
// CIconList
//
//
// Following the W2K approach we store items in a bitmap that is four icons wide.
// Operations on such a bitmap orientation are likely faster.

#define IL_DRAW_BLEND 0x00000001       // use 50% of the blendClr color
#define IL_DRAW_TRANSPARENT 0x00000002 // preserve the original background during \
                                       // drawing (or fill with a defined color)
#define IL_DRAW_ASALPHA 0x00000004     // use the (inverted) BLUE channel as \
                                       // alpha to mix the foreground color; \
                                       // currently used for the throbber
#define IL_DRAW_MASK 0x00000010        // draw the mask

class CIconList : public CGUIIconListAbstract
{
private:
    int ImageWidth; // size of a single icon
    int ImageHeight;
    int ImageCount;  // number of icons in the bitmap
    int BitmapWidth; // width of stored bitmaps
    int BitmapHeight;

    // images are arranged left-to-right and top-to-bottom
    HBITMAP HImage;   // DIB, raw data stored in ImageRaw
    DWORD* ImageRaw;  // ARGB values; Alpha: 0x00=transparent, 0xFF=opaque,
                      // others=partial transparency (only for IL_TYPE_ALPHA)
    BYTE* ImageFlags; // array with 'imageCount' elements (IL_TYPE_xxx)

    COLORREF BkColor; // current background color (for pixels with Alpha == 0x00)

    // shared variables for all image lists to save memory
    static HDC HMemDC;                       // shared memory DC
    static HBITMAP HOldBitmap;               // original bitmap
    static HBITMAP HTmpImage;                // cache for painting and temporary masks
    static DWORD* TmpImageRaw;               // raw data from HTmpImage
    static int TmpImageWidth;                // width of HTmpImage in pixels
    static int TmpImageHeight;               // height of HTmpImage in pixels
    static int MemDCLocks;                   // used when destroying HMemDC
    static CRITICAL_SECTION CriticalSection; // access synchronization
    static int CriticalSectionLocks;         // for constructing/destructing the section

public:
    //    BOOL     Dump; // if TRUE, raw data are dumped to TRACE

public:
    CIconList();
    ~CIconList();

    virtual BOOL WINAPI Create(int imageWidth, int imageHeight, int imageCount);
    virtual BOOL WINAPI CreateFromImageList(HIMAGELIST hIL, int requiredImageSize = -1);          // if 'requiredImageSize' is -1 the geometry from hIL is used
    virtual BOOL WINAPI CreateFromPNG(HINSTANCE hInstance, LPCTSTR lpBitmapName, int imageWidth); // load PNG from resources; must be a long strip one row high
    virtual BOOL WINAPI CreateFromRawPNG(const void* rawPNG, DWORD rawPNGSize, int imageWidth);
    virtual BOOL WINAPI CreateFromBitmap(HBITMAP hBitmap, int imageCount, COLORREF transparentClr); // grab a bitmap (up to 256 colors); must be a long strip one row high
    virtual BOOL WINAPI CreateAsCopy(const CIconList* iconList, BOOL grayscale);
    virtual BOOL WINAPI CreateAsCopy(const CGUIIconListAbstract* iconList, BOOL grayscale);

    // convert the icon list to a black-and-white version
    virtual BOOL WINAPI ConvertToGrayscale(BOOL forceAlphaForBW);

    // compress the bitmap into a 32-bit PNG with an alpha channel (one long row)
    // returns TRUE and an allocated buffer to free on success, FALSE on failure
    virtual BOOL WINAPI SaveToPNG(BYTE** rawPNG, DWORD* rawPNGSize);

    virtual BOOL WINAPI ReplaceIcon(int index, HICON hIcon);

    // create an icon from position 'index'; returns its handle or NULL on failure
    // Destroy the returned icon using DestroyIcon
    virtual HICON WINAPI GetIcon(int index);
    HICON GetIcon(int index, BOOL useHandles);

    // create an image list (one row, column count equals item count); returns its handle or NULL on failure
    // destroy the returned image list using ImageList_Destroy()
    virtual HIMAGELIST WINAPI GetImageList();

    // copies one item from 'srcIL' at position 'srcIndex' to position 'dstIndex'
    virtual BOOL WINAPI Copy(int dstIndex, CIconList* srcIL, int srcIndex);

    // copies one item from position 'srcIndex' to 'hDstImageList' at position 'dstIndex'
    //    BOOL CopyToImageList(HIMAGELIST hDstImageList, int dstIndex, int srcIndex);

    virtual BOOL WINAPI Draw(int index, HDC hDC, int x, int y, COLORREF blendClr, DWORD flags);

    virtual BOOL WINAPI SetBkColor(COLORREF bkColor);
    virtual COLORREF WINAPI GetBkColor();

private:
    // create HTmpImage if it does not exist
    // if HTmpImage exists and is smaller than 'width' x 'height', create a new one
    // returns TRUE on success, otherwise returns FALSE and keeps the previous HTmpImage
    BOOL CreateOrEnlargeTmpImage(int width, int height);

    // returns the handle of the bitmap currently selected in HMemDC
    // returns NULL if HMemDC does not exist
    HBITMAP GetCurrentBitmap();

    // 'index' denotes the icon position inside HImage
    // returns TRUE if the image at 'index' contained an alpha channel
    BYTE ApplyMaskToImage(int index, BYTE forceXOR);

    // debugging helper -- dumps ARGB values of the color bitmap and mask
    //    void DumpToTrace(int index, BOOL dumpMask);

    // pixel-by-pixel rendering followed by BitBlt is only about 30% slower than plain BitBlt in RELEASE

    BOOL DrawALPHA(HDC hDC, int x, int y, int index, COLORREF bkColor);
    BOOL DrawXOR(HDC hDC, int x, int y, int index, COLORREF bkColor);
    BOOL AlphaBlend(HDC hDC, int x, int y, int index, COLORREF bkColor, COLORREF fgColor);
    BOOL DrawMask(HDC hDC, int x, int y, int index, COLORREF fgColor, COLORREF bkColor);
    BOOL DrawALPHALeaveBackground(HDC hDC, int x, int y, int index);
    BOOL DrawAsAlphaLeaveBackground(HDC hDC, int x, int y, int index, COLORREF fgColor);

    void StoreMonoIcon(int index, WORD* mask);

    // helper for CreateFromBitmap(); copy from 'hSrcBitmap' the chosen number of items to 'dstIndex'
    // assumes 'hSrcBitmap' is a long strip one row high
    // 'transparentClr' defines the color considered transparent
    // assumes the source bitmap uses the same icon dimensions as the target (ImageWidth, ImageHeight)
    // a single copy can handle only one row of the destination bitmap, it cannot copy to two rows
    BOOL CopyFromBitmapIternal(int dstIndex, HBITMAP hSrcBitmap, int srcIndex, int imageCount, COLORREF transparentClr);
};

HBITMAP LoadPNGBitmap(HINSTANCE hInstance, LPCTSTR lpBitmapName, DWORD flags);
HBITMAP LoadRawPNGBitmap(const void* rawPNG, DWORD rawPNGSize, DWORD flags);

inline BYTE GetGrayscaleFromRGB(int red, int green, int blue)
{
    //  int brightness = (76*(int)red + 150*(int)green + 29*(int)blue) / 255;
    int brightness = (55 * (int)red + 183 * (int)green + 19 * (int)blue) / 255;
    //  int brightness = (40*(int)red + 175*(int)green + 60*(int)blue) / 255;
    if (brightness > 255)
        brightness = 255;
    return (BYTE)brightness;
}
