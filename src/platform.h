/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* Internal Interface for Platform Specific Functions */

#ifndef PLATFORM_H
#define PLATFORM_H

#include "openhmd.h"

// Time

double ohmd_get_tick();
void ohmd_sleep(double seconds);

// Sockets

typedef struct ohmd_socket ohmd_socket;

ohmd_socket* ohmd_create_socket(ohmd_context* ctx);
void ohmd_destroy_socket(ohmd_socket* socket);

bool ohmd_socket_connect(ohmd_socket* socket, const char* address, uint16_t port);
ssize_t ohmd_socket_send(ohmd_socket* socket, const void *buf, size_t len, int flags);
ssize_t ohmd_socket_recv(ohmd_socket* socket, void *buf, size_t len, int flags);
void ohmd_socket_close(ohmd_socket* socket);

// Mutices

typedef struct ohmd_mutex ohmd_mutex;

ohmd_mutex* ohmd_create_mutex(ohmd_context* ctx);
void ohmd_destroy_mutex(ohmd_mutex* mutex);

void ohmd_lock_mutex(ohmd_mutex* mutex);
void ohmd_unlock_mutex(ohmd_mutex* mutex);

// Threads

typedef struct ohmd_thread ohmd_thread;

ohmd_thread* ohmd_create_thread(ohmd_context* ctx, unsigned int (*routine)(void* arg), void* arg);
void ohmd_destroy_thread(ohmd_thread* thread);

#endif
