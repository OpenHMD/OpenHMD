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
typedef struct ooutput_data ooutput_data;

typedef void (*oconnection_callback)(omodule* source, ooutput_data* value, void* user_data);

typedef enum
{
	oct_quatf,
	oct_vec3f
} oconnection_type;

typedef enum 
{
	omt_imu,
	omt_imu_filter,
} omodule_type;

struct omodule
{
	omodule_type type;
	ohmd_context* ctx;
	char name[32];
	uint64_t id;
};

typedef enum 
{
	oimf_has_accelerator = 1,
	oimf_has_gyro = 2,
	oimf_has_rotation = 4,
} oimu_module_flags;

typedef struct
{
	omodule base;

	ooutput* accelerator;
	ooutput* gyro;
	ooutput* rotation;
} oimu_module;

struct oinput
{
	oconnection_type type;
	omodule* module;
	oconnection_callback callback;
	void* user_data;
};

typedef struct
{
	omodule base;
	
	oinput accelerator;
	oinput gyro;
	
	ooutput* rotation;
} oimu_filter_module;

struct ooutput_data 
{
	uint64_t ts;
  	void* source;
};

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
	omodule* module;
	olist* output_list;
};

typedef struct 
{
	oconnection_callback callback;
	void* user_data;

	omodule* from;
	omodule* to;
} oconnection;

void oinput_init(oinput* me, omodule* module, oconnection_type type, 
	oconnection_callback callback, void* user_data);

ohmd_status omodule_connect(ooutput* output, oinput* input);

ooutput* ooutput_create(omodule* module, oconnection_type type);
void ooutput_send(ooutput* output, ooutput_data* data);

void omodule_imu_init(oimu_module* me, ohmd_context* ctx, const char* name, 
	uint64_t id, oimu_module_flags flags);

void oimu_filter_module_init(oimu_filter_module* me, ohmd_context* ctx, 
	const char* name, uint64_t id, oconnection_callback rotation_callback, void* rotation_user_data);

#ifdef __cplusplus
}
#endif

#endif
