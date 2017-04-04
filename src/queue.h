/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2016 Fredrik Hultin.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* Naive Thread Safe Circular Queue */

#ifndef OHMDQUEUE_H
#define OHMDQUEUE_H

#include <stdbool.h>

typedef struct ohmdq ohmdq;
typedef struct ohmd_context ohmd_context;

ohmdq* ohmdq_create(ohmd_context* ctx, unsigned elem_size, unsigned max);
void ohmdq_destroy(ohmdq* me);

bool ohmdq_push(ohmdq* me, const void* elem);
bool ohmdq_pop(ohmdq* me, void* out_elem);
unsigned ohmdq_get_size(ohmdq* me);
unsigned ohmdq_get_max(ohmdq* me);

#endif
