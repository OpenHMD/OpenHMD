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

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include "openhmdi.h"

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

// threads

struct ohmd_thread {
	HANDLE handle;
	void* arg;
	unsigned int (*routine)(void* arg);
};

struct ohmd_mutex {
	HANDLE handle;
};

__stdcall DWORD ohmd_thread_wrapper(void* t)
{
	ohmd_thread* thread = (ohmd_thread*)t;
	return thread->routine(thread->arg);
}

ohmd_thread* ohmd_create_thread(ohmd_context* ctx, unsigned int (*routine)(void* arg), void* arg)
{
	ohmd_thread* thread = ohmd_alloc(ctx, sizeof(ohmd_thread));
	if(!thread)
		return NULL;

	thread->routine = routine;
	thread->arg = arg;

	thread->handle = CreateThread(NULL, 0, ohmd_thread_wrapper, thread, 0, NULL);

	return thread;
}

ohmd_mutex* ohmd_create_mutex(ohmd_context* ctx)
{
	ohmd_mutex* mutex = ohmd_alloc(ctx, sizeof(ohmd_mutex));
	if(!mutex)
		return NULL;
	
	mutex->handle = CreateMutex(NULL, FALSE, NULL);

	return mutex;
}

void ohmd_destroy_mutex(ohmd_mutex* mutex)
{
	CloseHandle(mutex->handle);
	free(mutex);
}

void ohmd_lock_mutex(ohmd_mutex* mutex)
{
	WaitForSingleObject(mutex->handle, INFINITE);
}

void ohmd_unlock_mutex(ohmd_mutex* mutex)
{
	ReleaseMutex(mutex->handle);
}

struct ohmd_socket
{
	SOCKET fd;
};

ohmd_socket* ohmd_create_socket(ohmd_context* ctx)
{
	return ohmd_alloc(ctx, sizeof(ohmd_socket));
}

void ohmd_socket_close(ohmd_socket* sock)
{
	closesocket(sock->fd);
}

void ohmd_destroy_socket(ohmd_socket* sock)
{
	free(sock);
}

static const char *ohmd_inet_ntop(int af, const void *src, char *dst, socklen_t cnt)
{
	if (af == AF_INET)
	{
		struct sockaddr_in in;
		memset(&in, 0, sizeof(in));
		in.sin_family = AF_INET;
		memcpy(&in.sin_addr, src, sizeof(struct in_addr));
		getnameinfo((struct sockaddr *)&in, sizeof(struct sockaddr_in), dst, cnt, NULL, 0, NI_NUMERICHOST);
		return dst;
	}
	else if (af == AF_INET6)
	{
		struct sockaddr_in6 in;
		memset(&in, 0, sizeof(in));
		in.sin6_family = AF_INET6;
		memcpy(&in.sin6_addr, src, sizeof(struct in_addr6));
		getnameinfo((struct sockaddr *)&in, sizeof(struct sockaddr_in6), dst, cnt, NULL, 0, NI_NUMERICHOST);
		return dst;
	}
	return NULL;
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
			closesocket(sock->fd);
			continue;
		}

		break;
	}

	if (p == NULL)
		goto error;

	struct sockaddr* sa = (struct sockaddr*)p->ai_addr;
	void* sin_addr = sa->sa_family == AF_INET6 ? (void*)&(((struct sockaddr_in6*)sa)->sin6_addr) : (void*)&(((struct sockaddr_in*)sa)->sin_addr);

	ohmd_inet_ntop(p->ai_family, sin_addr, s, sizeof s);
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
