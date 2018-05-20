/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2018 Fredrik Hultin.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/**
 * \file openhmd-dev.h
 * Header for the unstable OpenHMD development API.
 **/

#ifndef OPENHMD_DEV_H
#define OPENHMD_DEV_H

#ifdef __cplusplus
extern "C" {
#endif

#include <openhmd.h>

typedef struct olist olist;

olist* olist_create(ohmd_context* ctx, size_t elem_size);
void* olist_insert(olist* list, void* elem, void* after);
void* olist_get_next(olist* list, void* elem);
void* olist_get_first(olist* list);

#ifdef __cplusplus
}
#endif

#endif
