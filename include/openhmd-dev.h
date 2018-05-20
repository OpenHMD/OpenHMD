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

#include <inttypes.h>
#include <stddef.h>
#include <openhmd.h>

#include "omath.h"

// olist - linked list
typedef struct olist olist;

olist* olist_create(ohmd_context* ctx, size_t elem_size);
void* olist_insert(olist* list, void* elem, void* after);
void* olist_get_next(olist* list, void* elem);
void* olist_get_first(olist* list);
void* olist_append(olist* list, void* elem);

// modules and connections
typedef struct omodule omodule;
typedef struct ooutput ooutput;
typedef struct oinput oinput;

typedef enum
{
	oct_quatf,
	oct_vec3f
} oconnection_type;

typedef enum 
{
	omt_imu,
} omodule_type;

struct omodule
{
	omodule_type type;
	ohmd_context* ctx;
	char* name;
	uint64_t id;
};

struct oimu_module
{
	omodule base;

	ooutput* accelerator;
	ooutput* gyro;
	ooutput* rotation;
};

typedef struct 
{
	uint64_t ts;
	void* user_data;
  	void* source;
}  ooutput_data;

typedef struct
{
	ooutput_data base;
	quatf value;
} oquatf_data;

typedef struct
{
	ooutput_data base;
	vec3f value;
} ovec3f_data;

struct ooutput
{
	oconnection_type type;
	olist* output_list;
	omodule* module;
};

struct oinput
{
	oconnection_type type;
	olist* input_list;
	omodule* module;
};

typedef void (*oconnection_callback)(omodule* source, ooutput_data* value, void* user_data);

typedef struct 
{
	oconnection_callback callback;
	void* user_data;

	omodule* from;
	omodule* to;
} oconnection;

ohmd_status omodule_connect(oinput* input, ooutput* output, void* user_data);
void ooutput_send(ooutput* output, ooutput_data* data);

#ifdef __cplusplus
}
#endif

#endif
