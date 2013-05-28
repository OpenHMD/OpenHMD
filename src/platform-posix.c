/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* Platform Specific Functions, Unix/Posix Implementation */

#if defined(__unix__) || defined(__unix) || defined(__APPLE__) || defined(__MACH__)

#ifdef __CYGWIN__
#define CLOCK_MONOTONIC (clockid_t)4
#endif

#define _POSIX_C_SOURCE 199309L

#include <time.h>
#include <sys/time.h>
#include <stdio.h>

// Use clock_gettime if the system implements posix realtime timers
#ifndef CLOCK_MONOTONIC
double ohmd_get_tick()
{
	struct timeval now;
	gettimeofday(&now, NULL);
	return (double)now.tv_sec * 1.0 + (double)now.tv_usec / 1000000.0;
}
#else
double ohmd_get_tick()
{
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	return (double)now.tv_sec * 1.0 + (double)now.tv_nsec / 1000000000.0;
}
#endif

void ohmd_sleep(double seconds)
{
	struct timespec sleepfor;

	sleepfor.tv_sec = (time_t)seconds;
	sleepfor.tv_nsec = (long)((seconds - sleepfor.tv_sec) * 1000000000.0);

	nanosleep(&sleepfor, NULL);
}

#endif
