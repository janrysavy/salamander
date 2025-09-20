// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

//*****************************************************************************
//
// CToolTip
//
// This tooltip eliminates the basic drawback of the original implementation.
// Each window used to create its own tooltip object. Another drawback was
// the need to pass this object a list of areas where tooltips should appear.
//
// The new approach means CMainWindow owns just one tooltip instance.
// The tooltip window is created only when needed and in the thread that
// requested to show it. The reason is that the tooltip must run in this
// thread - until version 2.6b6 it ran in Salamander's main thread and when
// that thread was blocked, the tooltip never showed up. When the mouse moves
// over a control using this tooltip, the control calls SetCurrentID when
// entering a new area.
//
// The interface for using the tooltip is in const.h so every control can
// access it without including mainwnd.h and tooltip.h.
//

// Messages used:
// WM_USER_TTGETTEXT - used to request text with a specific ID
//   wParam = ID passed to SetCurrentToolTip
//   lParam = buffer (points to the tooltip buffer); maximum length is TOOLTIP_TEXT_MAX
//            before sending the message the buffer's first character is set to a terminator
//            the text may contain \n for a new line and \t for a tab character
// If the window writes a null-terminated string into the buffer, it will be displayed
// otherwise the tooltip will not be shown
//

class CToolTip : public CWindow
{
    enum TipTimerModeEnum
    {
        ttmNone,         // no timer is running
        ttmWaitingOpen,  // waiting for the tooltip to open
        ttmWaitingClose, // waiting for the tooltip to close
        ttmWaitingKill,  // waiting to exit the display mode
    };

protected:
    char Text[TOOLTIP_TEXT_MAX];
    int TextLen;
    HWND HNotifyWindow;
    DWORD LastID;
    TipTimerModeEnum WaitingMode;
    DWORD HideCounter;
    DWORD HideCounterMax;
    POINT LastCursorPos;
    BOOL IsModal;     // is our message loop currently running?
    BOOL ExitASAP;    // close as soon as possible and stop being modal
    UINT_PTR TimerID; // returned by SetTimer; needed for KillTimer

public:
    CToolTip(CObjectOrigin origin = ooStatic);
    ~CToolTip();

    BOOL RegisterClass();

    // 'hParent' is required so that closing the parent also closes the tooltip.
    // Without it we had cases where the parent thread ended but the tooltip
    // window stayed open and could not be closed (its thread no longer existed),
    // causing crashes when Salamander exited (fortunately before release 2.5b7)
    BOOL Create(HWND hParent);

    // This method starts a timer and if it is not called again before the timer
    // expires, it asks 'hNotifyWindow' for text using the WM_USER_TTGETTEXT
    // message. The text is then displayed under the cursor at its current
    // coordinates. The 'id' variable distinguishes tooltip areas when
    // communicating with 'hNotifyWindow'. Repeated calls with the same 'id' are
    // ignored. The value 0 in 'hNotifyWindow' turns the tooltip off and cancels
    // the running timer. The 'showDelay' parameter is meaningful only when
    // 'hNotifyWindow' != NULL. If it is greater or equal to 1, it specifies the
    // delay before displaying the tooltip in milliseconds. If it equals 0, the
    // default delay is used. If it is -1, no timer is started at all.
    void SetCurrentToolTip(HWND hNotifyWindow, DWORD id, int showDelay);

    // suppress tooltip display at the current mouse position
    // useful when activating a window using tooltips to prevent unwanted
    // tooltip appearances
    void SuppressToolTipOnCurrentMousePos();

    // returns TRUE if the text was displayed; returns FALSE when no new text was supplied
    // if considerCursor==TRUE, measures the cursor and moves the tooltip under it
    // if modal==TRUE, runs a message loop watching for tooltip closing and
    // returns only after it disappears
    BOOL Show(int x, int y, BOOL considerCursor, BOOL modal, HWND hParent);

    // hides the tooltip
    void Hide();

    void OnTimer();

protected:
    virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    BOOL GetText();
    void GetNeededWindowSize(SIZE* sz);

    void MessageLoop(); // for the modal version of the tooltip

    void MySetTimer(DWORD elapse);
    void MyKillTimer();

    DWORD GetTime(BOOL init);
};
