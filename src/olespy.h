// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// OLESPY is used to detect COM/OLE leaks
// the bodies of the following functions are empty in release builds
// (when _DEFINE is not defined)
//
// Details about searching for OLE leaks are described in OLESPY.CPP

// Connect our IMallocSpy to OLE; COM must be initialized first
// Returns TRUE if the following functions can be called
BOOL OleSpyRegister();

// Disconnect the SPY from OLE; OleSpyDump can still be called afterwards
void OleSpyRevoke();

// Used to break the application when allocation number 'alloc' is reached
// Call sometime between OleSpyRegister and OleSpyRevoke
void OleSpySetBreak(int alloc);

// Prints statistics and leaks to the Debug window and TRACE_I
// For leaks it shows the allocation order [n] which can be used for OleSpySetBreak
void OleSpyDump();

// stress test of IMallocSpy implementation
// meant for debugging purposes
// void OleSpyStressTest();
