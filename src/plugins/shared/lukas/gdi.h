// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// ****************************************************************************
//
// CBackbufferedDC -- device context with a back buffer for smooth rendering of
// more complex graphics
//

class CBackbufferedDC
{
public:
    CBackbufferedDC();
    CBackbufferedDC(HWND window);
    ~CBackbufferedDC();
    void Destroy();

    // binds the device context to the specified window
    void SetWindow(HWND window);

    // refreshes the internal state when the window size or screen resolution
    // changes; do not call between BeginPaint and EndPaint
    void Update();

    // begins drawing; _must_ be paired with EndPaint and cannot be called
    // repeatedly
    void BeginPaint();

    // finishes drawing and copies the back-buffer contents to the screen
    void EndPaint();

    // returns the device context used for drawing; valid only between BeginPaint
    // and EndPaint
    operator HDC();

    // returns the rectangle that describes the back-buffer dimensions
    const RECT& GetRect() { return ClientRect; }

private:
    HDC DC;
    HWND HWindow;
    HBITMAP HBitmap;
    HBITMAP OldHBitmap;
    PAINTSTRUCT PS;
    RECT ClientRect;
};

inline BOOL FastFillRect(HDC hdc, const RECT& r)
{
    return ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &r, NULL, 0, 0);
}

inline BOOL FastFillRect(HDC hdc, int x1, int y1, int x2, int y2)
{
    RECT r;
    r.left = x1;
    r.top = y1;
    r.right = x2;
    r.bottom = y2;
    return FastFillRect(hdc, r);
}
