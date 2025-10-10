// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// loads the hRsrc bitmap resource (obtained from FindResource(...)),
// remaps mapCount colors: mapColor[i] -> toColor[i]
// and creates a desktop-compatible bitmap

HBITMAP LoadBitmapAndMapColors(HINSTANCE hInst, HRSRC hRsrc, int mapCount,
                               COLORREF* mapColor, COLORREF* toColor);

DWORD DIBHeight(LPSTR lpDIB);
DWORD DIBWidth(LPSTR lpDIB);
HBITMAP DIBToBitmap(HANDLE hDIB, HPALETTE hPal);
