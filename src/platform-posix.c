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
#include <pthread.h>

#include "platform.h"
#include "openhmdi.h"

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

// threads

struct ohmd_thread
{
	pthread_t thread;
	unsigned int (*routine)(void* arg);
	void* arg;
};

static void* pthread_wrapper(void* arg)
{
	ohmd_thread* my_thread = (ohmd_thread*)arg;
	my_thread->routine(my_thread->arg);
	return NULL;
}

ohmd_thread* ohmd_create_thread(ohmd_context* ctx, unsigned int (*routine)(void* arg), void* arg)
{
	ohmd_thread* thread = ohmd_alloc(ctx, sizeof(ohmd_thread));
	if(thread == NULL)
		return NULL;

	thread->arg = arg;
	thread->routine = routine;

	int ret = pthread_create(&thread->thread, NULL, pthread_wrapper, thread);

	if(ret != 0){
		free(thread);
		thread = NULL;
	}

	return thread;
}

ohmd_mutex* ohmd_create_mutex(ohmd_context* ctx)
{
	pthread_mutex_t* mutex = ohmd_alloc(ctx, sizeof(pthread_mutex_t));
	if(mutex == NULL)
		return NULL;

	int ret = pthread_mutex_init(mutex, NULL);

	if(ret != 0){
		free(mutex);
		mutex = NULL;
	}

	return (ohmd_mutex*)mutex;
}

void ohmd_destroy_thread(ohmd_thread* thread)
{
	pthread_join(thread->thread, NULL);
	free(thread);
}

void ohmd_destroy_mutex(ohmd_mutex* mutex)
{
	pthread_mutex_destroy((pthread_mutex_t*)mutex);
	free(mutex);
}

void ohmd_lock_mutex(ohmd_mutex* mutex)
{
	if(mutex)
		pthread_mutex_lock((pthread_mutex_t*)mutex);
}

void ohmd_unlock_mutex(ohmd_mutex* mutex)
{
	if(mutex)
		pthread_mutex_unlock((pthread_mutex_t*)mutex);
}

#endif
