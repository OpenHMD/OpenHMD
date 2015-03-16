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
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "openhmdi.h"
#include "log.h"

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

	pthread_create(&thread->thread, NULL, pthread_wrapper, thread);

	return thread;
}

ohmd_mutex* ohmd_create_mutex(ohmd_context* ctx)
{
	pthread_mutex_t* mutex = ohmd_alloc(ctx, sizeof(pthread_mutex_t));
	if(mutex == NULL)
		return NULL;

	pthread_mutex_init(mutex, NULL);

	return (ohmd_mutex*)mutex;
}

void ohmd_destroy_thread(ohmd_thread* thread)
{
	free(thread);
}

void ohmd_destroy_mutex(ohmd_mutex* mutex)
{
	pthread_mutex_destroy((pthread_mutex_t*)mutex);
}

void ohmd_lock_mutex(ohmd_mutex* mutex)
{
	pthread_mutex_lock((pthread_mutex_t*)mutex);
}

void ohmd_unlock_mutex(ohmd_mutex* mutex)
{
	pthread_mutex_unlock((pthread_mutex_t*)mutex);
}

struct ohmd_socket
{
	int fd;
};

ohmd_socket* ohmd_create_socket(ohmd_context* ctx)
{
	return ohmd_alloc(ctx, sizeof(ohmd_socket));
}

void ohmd_socket_close(ohmd_socket* sock)
{
	close(sock->fd);
}

void ohmd_destroy_socket(ohmd_socket* sock)
{
	free(sock);
}

bool ohmd_socket_connect(ohmd_socket* sock, const char* address, uint16_t port)
{
	struct addrinfo hints, *servinfo = NULL, *p;
	char s[INET6_ADDRSTRLEN];
	int rv;
	char port_tmp[6];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	snprintf(port_tmp, 6, "%u", port);

	if ((rv = getaddrinfo(address, port_tmp, &hints, &servinfo)) != 0)
		goto error;

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sock->fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
			continue;

		if (connect(sock->fd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sock->fd);
			continue;
		}

		break;
	}

	if (p == NULL)
		goto error;

	struct sockaddr* sa = (struct sockaddr*)p->ai_addr;
	void* sin_addr = sa->sa_family == AF_INET6 ? (void*)&(((struct sockaddr_in6*)sa)->sin6_addr) : (void*)&(((struct sockaddr_in*)sa)->sin_addr);

	inet_ntop(p->ai_family, sin_addr, s, sizeof s);
	freeaddrinfo(servinfo);

	return true;

error:
	if(servinfo){
		freeaddrinfo(servinfo);
	}
	return false;
}

ssize_t ohmd_socket_send(ohmd_socket* sock, const void *buf, size_t len, int flags)
{
	return send(sock->fd, buf, len, flags);
}

ssize_t ohmd_socket_recv(ohmd_socket* sock, void *buf, size_t len, int flags)
{
	return recv(sock->fd, buf, len, flags);
}

#endif
