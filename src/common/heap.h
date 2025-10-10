// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// Add the _CRTDBG_MAP_ALLOC macro to the DEBUG version of the project; otherwise, the leak source is not shown.

#if defined(_DEBUG) && !defined(HEAP_DISABLE)

#define GCHEAP_MAX_USED_MODULES 100 // maximum number of modules to remember for loading before dumping leaks

// Called for modules in which memory leaks can be reported; if leaks are detected,
// all registered modules are loaded "as image" (without module initialization) (during the leak
// inspection these modules are already unloaded), and only then are the memory leaks dumped =
// the .cpp module names are shown instead of "#File Error#" messages, and MSVC is not flooded
// with numerous generated exceptions (the module names are available).
// May be called from any thread.
void AddModuleWithPossibleMemoryLeaks(const TCHAR* fileName);

#endif // defined(_DEBUG) && !defined(HEAP_DISABLE)
