// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"

#include <windows.h>
#include <crtdbg.h>
#include <commctrl.h> // need LPCOLORMAP
#include <tchar.h>
#include <ostream>
#include <streambuf>

#if defined(_DEBUG) && defined(_MSC_VER) // without passing file+line to 'new' operator, list of memory leaks shows only 'crtdbg.h(552)'
#define new new (_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#pragma warning(3 : 4706) // warning C4706: assignment within conditional expression

#include "trace.h"
#include "handles.h"
#include "array.h"
#include "winlib.h"
#include "multimon.h"

HWND GetTopVisibleParent(HWND hParent)
{
    // Find the nearest ancestor that is no longer a child window (a POPUP/OVERLAPPED window).
    HWND hIterator = hParent;
    while ((GetWindowLongPtr(hIterator, GWL_STYLE) & WS_CHILD) &&
           (hIterator = ::GetParent(hIterator)) != NULL &&
           IsWindowVisible(hIterator))
        hParent = hIterator;
    return hParent;
}

void MultiMonGetClipRectByRect(const RECT* rect, RECT* workClipRect, RECT* monitorClipRect)
{
    HMONITOR hMonitor = MonitorFromRect(rect, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi;
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(hMonitor, &mi);
    *workClipRect = mi.rcWork;
    if (monitorClipRect != NULL)
        *monitorClipRect = mi.rcMonitor;
}

void MultiMonGetClipRectByWindow(HWND hByWnd, RECT* workClipRect, RECT* monitorClipRect)
{
    HMONITOR hMonitor; // target monitor that will host the window
    MONITORINFO mi;
    mi.cbSize = sizeof(mi);

    if (hByWnd != NULL && IsWindowVisible(hByWnd) && !IsIconic(hByWnd)) // note that this condition also appears in MultiMonCenterWindow
    {
        hMonitor = MonitorFromWindow(hByWnd, MONITOR_DEFAULTTONEAREST);
        // Retrieve the monitor's working area.
        GetMonitorInfo(hMonitor, &mi);
    }
    else
    {
        // If the foreground window belongs to this process, reuse its monitor.
        HWND hForegroundWnd = GetForegroundWindow();
        DWORD processID;
        GetWindowThreadProcessId(hForegroundWnd, &processID);
        if (hForegroundWnd != NULL && processID == GetCurrentProcessId())
        {
            hMonitor = MonitorFromWindow(hForegroundWnd, MONITOR_DEFAULTTONEAREST);
        }
        else
        {
            // Otherwise fall back to the primary monitor.
            POINT pt;
            pt.x = 0; // primary monitor
            pt.y = 0;
            hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
        }

        // Retrieve the monitor's working area.
        GetMonitorInfo(hMonitor, &mi);
    }
    *workClipRect = mi.rcWork;
    if (monitorClipRect != NULL)
        *monitorClipRect = mi.rcMonitor;
}

void MultiMonCenterWindow(HWND hWindow, HWND hByWnd, BOOL findTopWindow)
{
    if (hWindow == NULL)
    {
        // Passing NULL here causes unwanted window flicker.
        TRACE_E("MultiMonCenterWindow: hWindow == NULL");
        return;
    }

    if (IsZoomed(hWindow))
    {
        // Do not reposition a maximized window.
        return;
    }

    // Resolve the top-level window.
    if (findTopWindow)
    {
        if (hByWnd != NULL)
            hByWnd = GetTopVisibleParent(hByWnd);
        else
            TRACE_E("MultiMonCenterWindow: hByWnd == NULL and findTopWindow is TRUE");
    }

    RECT clipR;
    MultiMonGetClipRectByWindow(hByWnd, &clipR, NULL);
    RECT byR;
    if (hByWnd != NULL && IsWindowVisible(hByWnd) && !IsIconic(hByWnd)) // note that this condition also appears in MultiMonGetClipRectByWindow
        GetWindowRect(hByWnd, &byR);
    else
        byR = clipR;

    MultiMonCenterWindowByRect(hWindow, clipR, byR);
}

void MultiMonCenterWindowByRect(HWND hWindow, const RECT& clipR, const RECT& byR)
{
    if (hWindow == NULL)
    {
        // Passing NULL here causes unwanted window flicker.
        TRACE_E("MultiMonCenterWindowByRect: hWindow == NULL");
        return;
    }

    if (IsZoomed(hWindow))
    {
        // Do not reposition a maximized window.
        return;
    }

    RECT wndRect;
    GetWindowRect(hWindow, &wndRect);
    int wndWidth = wndRect.right - wndRect.left;
    int wndHeight = wndRect.bottom - wndRect.top;

    // Center the rectangle.
    wndRect.left = byR.left + (byR.right - byR.left - wndWidth) / 2;
    wndRect.top = byR.top + (byR.bottom - byR.top - wndHeight) / 2;
    wndRect.right = wndRect.left + wndWidth;
    wndRect.bottom = wndRect.top + wndHeight;

    // Clamp the rectangle to the clipping area.
    if (wndRect.left < clipR.left) // if the window is larger than clipR, keep its left part visible
    {
        wndRect.left = clipR.left;
        wndRect.right = wndRect.left + wndWidth;
    }

    if (wndRect.top < clipR.top) // if the window is larger than clipR, keep its top part visible
    {
        wndRect.top = clipR.top;
        wndRect.bottom = wndRect.top + wndHeight;
    }

    if (wndWidth <= clipR.right - clipR.left)
    {
        // If the window fits inside clipR, keep it within the right edge.
        if (wndRect.right >= clipR.right)
        {
            wndRect.left = clipR.right - wndWidth;
            wndRect.right = wndRect.left + wndWidth;
        }
    }
    else
    {
        // If the window is larger than clipR
        if (wndRect.left > clipR.left)
            wndRect.left = clipR.left; // keep as much of the window visible as possible
    }

    if (wndHeight <= clipR.bottom - clipR.top)
    {
        // If the window fits inside clipR, keep it within the bottom edge.
        if (wndRect.bottom >= clipR.bottom)
        {
            wndRect.top = clipR.bottom - wndHeight;
            wndRect.bottom = wndRect.top + wndHeight;
        }
    }
    else
    {
        // If the window is larger than clipR
        if (wndRect.top > clipR.top)
            wndRect.top = clipR.top; // keep as much of the window visible as possible
    }

    SetWindowPos(hWindow, NULL, wndRect.left, wndRect.top, 0, 0,
                 SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

class CGetDefWndPos : public CWindow
{
public:
    CGetDefWndPos() : CWindow(ooStatic) {}

    HWND Create(HWND hParent)
    {
        return CWindow::Create(CWINDOW_CLASSNAME,
                               NULL,
                               WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT,
                               CW_USEDEFAULT,
                               CW_USEDEFAULT,
                               CW_USEDEFAULT,
                               hParent,
                               NULL,
                               HInstance,
                               this);
    }
};

BOOL MultiMonGetDefaultWindowPos(HWND hByWnd, POINT* p)
{
    if (hByWnd != NULL)
    {
        hByWnd = GetTopVisibleParent(hByWnd);

        HMONITOR hMonitor = MonitorFromWindow(hByWnd, MONITOR_DEFAULTTONEAREST);

        MONITORINFO mi;
        mi.cbSize = sizeof(mi);
        GetMonitorInfo(hMonitor, &mi);

        if (!(mi.dwFlags & MONITORINFOF_PRIMARY))
        {
            // TRACE_I("MultiMonGetDefaultWindowPos(): parent = " << hByWnd << ", left = " << mi.rcMonitor.left);
            RECT r;
            CGetDefWndPos wnd;
            wnd.Create(hByWnd);
            GetWindowRect(wnd.HWindow, &r);
            int deltaX = 0;
            int deltaY = 0;
            HMONITOR hTmpMonitor = MonitorFromWindow(wnd.HWindow, MONITOR_DEFAULTTONEAREST);
            if (hTmpMonitor != hMonitor)
            {
                // The dummy-window trick works under MSVC, but when Salamander runs without it
                // the window opens on the primary monitor (no idea why, given it has a parent).

                // Use a workaround—move the window onto the intended monitor.
                MONITORINFO tmpInfo;
                tmpInfo.cbSize = sizeof(tmpInfo);
                GetMonitorInfo(hTmpMonitor, &tmpInfo);
                deltaX = mi.rcMonitor.left - tmpInfo.rcMonitor.left;
                deltaY = mi.rcMonitor.top - tmpInfo.rcMonitor.top;
            }

            DestroyWindow(wnd.HWindow);
            p->x = r.left + deltaX;
            p->y = r.top + deltaY;
            return TRUE;
        }
    }
    return FALSE;
}

BOOL MultiMonEnsureRectVisible(RECT* rect, BOOL partialOK)
{
    HMONITOR hMonitor = MonitorFromRect(rect, MONITOR_DEFAULTTONEAREST);

    MONITORINFO mi;
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(hMonitor, &mi);
    RECT clipRect = mi.rcWork;

    RECT intersectRect;
    BOOL intersect = IntersectRect(&intersectRect, rect, &clipRect);
    if (EqualRect(&intersectRect, rect))
        return FALSE; // the rectangle already lies entirely on one monitor, nothing to fix

    if (intersect && partialOK)
        return FALSE; // partially visible is acceptable, nothing to fix

    // Clamp the rectangle so it does not extend beyond the monitor.
    if (rect->right - rect->left > clipRect.right - clipRect.left)
        rect->right = clipRect.right - clipRect.left + rect->left;
    if (rect->bottom - rect->top > clipRect.bottom - clipRect.top)
        rect->bottom = clipRect.bottom - clipRect.top + rect->top;

    if (rect->left < clipRect.left)
    {
        int x = clipRect.left - rect->left;
        rect->left += x;
        rect->right += x;
    }

    if (rect->top < clipRect.top)
    {
        int y = clipRect.top - rect->top;
        rect->top += y;
        rect->bottom += y;
    }

    if (rect->right > clipRect.right)
    {
        int x = clipRect.right - rect->right;
        rect->left += x;
        rect->right += x;
    }

    if (rect->bottom > clipRect.bottom)
    {
        int y = clipRect.bottom - rect->bottom;
        rect->top += y;
        rect->bottom += y;
    }

    // At least one edge was adjusted, so report TRUE.
    return TRUE;
}
