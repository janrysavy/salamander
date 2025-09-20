// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

struct NSVGrasterizer;
struct NSVGimage;
void RenderSVGImage(NSVGrasterizer* rast, HDC hDC, int x, int y, const char* svgName, int iconSize, COLORREF bkColor, BOOL enabled);

// returns SysColor in the format used by the SVG library (BGR instead of Win32 RGB)
DWORD GetSVGSysColor(int index);

//*****************************************************************************
//
// CSVGSprite
//

#define SVGSTATE_ORIGINAL 0x0001 // the SVG in its original unmodified form
#define SVGSTATE_ENABLED 0x0002  // SVG colored using the enabled text color
#define SVGSTATE_DISABLED 0x0004 // SVG colored using the disabled text color
#define SVGSTATE_COUNT 3

// This object renders SVGs using a cached bitmap.
// It primarily keeps a colored version of the image rendered according to the colors in the source SVG.
// It can also hold colored variants of the bitmap (hence the Sprite name - internally it keeps a larger bitmap with multiple images),
// for example "disabled", "active", or "selected".
class CSVGSprite
{
public:
    CSVGSprite();
    ~CSVGSprite();

    // discards the bitmap and resets all members to their default values
    void Clean();

    // 'states' is a combination of SVGSTATE_* bits
    BOOL Load(int resID, int width, int height, DWORD states);

    void GetSize(SIZE* s);
    int GetWidth();
    int GetHeight();

    // 'hDC' is the target DC where the bitmap will be drawn
    // 'x' and 'y' are the target coordinates in 'hDC'
    // 'width' and 'height' are the output dimensions; if they are -1 the object's Width/Height are used
    void AlphaBlend(HDC hDC, int x, int y, int width, int height, DWORD state);

protected:
    // loads a resource into memory, allocates a buffer one byte longer and terminates the data with zero
    // returns a pointer to the allocated memory on success (caller must free it) or NULL on failure
    char* LoadSVGResource(int resID);

    // The input 'sz' defines the size in points that the SVG should fit into once converted to a bitmap.
    // If one dimension is -1 it is unspecified and computed with aspect ratio preserved.
    // If both dimensions are unspecified they are taken from the source data.
    // The resulting bitmap size in points is returned through the output parameters.
    void GetScaleAndSize(const NSVGimage* image, const SIZE* sz, float* scale, int* width, int* height);

    // creates a DIB with size 'width' x 'height', returning its handle and a pointer to the bits
    void CreateDIB(int width, int height, HBITMAP* hMemBmp, void** lpMemBits);

    // tints the SVG 'image' according to the color defined by 'state'
    void ColorizeSVG(NSVGimage* image, DWORD state);

protected:
    int Width; // size of a single image in points
    int Height;
    HBITMAP HBitmaps[SVGSTATE_COUNT];
};

//*****************************************************************************
//
// global variables
//

//extern HBITMAP HArrowRight;         // bitmap created from an SVG, used as the right-arrow button image
//extern SIZE ArrowRightSize;         // its size in points
//HBITMAP HArrowRight = NULL;
//SIZE ArrowRightSize = { 0 };

extern CSVGSprite SVGArrowRight;
extern CSVGSprite SVGArrowRightSmall;
extern CSVGSprite SVGArrowMore;
extern CSVGSprite SVGArrowLess;
extern CSVGSprite SVGArrowDropDown;
