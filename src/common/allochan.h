// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// Introduces a handler for addressing the situation when memory runs out during a call to operator new or
// the malloc function (used by calloc, realloc, and others; see help). It guarantees that neither
// operator new nor malloc will ever return NULL without the user's knowledge. It displays a
// message box with an "out of memory" error and the user can retry the memory allocation after closing
// other applications. The user can also terminate the process or let the allocation error propagate to the application
// (operator new or malloc returns NULL; allocations of large memory blocks should be prepared for that,
// otherwise it will crash - the user is informed about this).

// localized settings of the out-of-memory message and warning prompts
// (use NULL if the string should not change); the expected content reads:
// message:
// Insufficient memory to allocate %u bytes. Try to release some memory (e.g.
// close some running application) and click Retry. If it does not help, you can
// click Ignore to pass memory allocation error to this application or click Abort
// to terminate this application.
// title: (used for both "message" and "warning")
// we recommend using the application name so the user knows which application is complaining
// warningIgnore:
// Do you really want to pass memory allocation error to this application?\n\n
// WARNING: Application may crash and then all unsaved data will be lost!\n
// HINT: We recommend to risk this action only if the application is trying to
// allocate extra large block of memory (i.e. more than 500 MB).
// warningAbort:
// Do you really want to terminate this application?\n\nWARNING: All unsaved data will be lost!
void SetAllocHandlerMessage(const TCHAR* message, const TCHAR* title,
                            const TCHAR* warningIgnore, const TCHAR* warningAbort);
