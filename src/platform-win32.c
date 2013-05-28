/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* Platform Specific Functions, Win32 Implementation */

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN

#include <windows.h>

double ohmd_get_tick()
{
        double high, low;
        FILETIME filetime;

        GetSystemTimeAsFileTime(&filetime);

        high = filetime.dwHighDateTime;
        low = filetime.dwLowDateTime;

        return (high * 4294967296.0 + low) / 10000000;
}

// TODO higher resolution
void ohmd_sleep(double seconds)
{
        Sleep((DWORD)(seconds * 1000));
}

#endif
