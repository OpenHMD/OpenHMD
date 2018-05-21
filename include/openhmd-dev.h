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

// modules and connections
typedef struct omodule omodule;
typedef struct omessage omessage;

typedef enum
{
	omd_error = -1,
	omd_float = 0,
	omd_int = 1,
	omd_bin = 2,
	omd_string = 3,
} omessage_data_type;

typedef enum 
{
	omt_imu = 0,
	omt_imu_filter = 1,
} omodule_type;

typedef void (*omessage_callback)(omodule* source, omessage* msg, void* user_data);

ohmd_status omodule_connect(omodule* from, const char* output_name, omodule* to, const char* input_name);

omodule* omodule_create(ohmd_context* ctx, const char* name, uint64_t id);
int omodule_get_input_count(omodule* me);
int omodule_get_output_count(omodule* me);
const char* omodule_get_input_name(omodule* me, int idx);
const char* omodule_get_output_name(omodule* me, int idx);
void omodule_add_output(omodule* me, const char* name);
void omodule_add_input(omodule* me, const char* name,  omessage_callback callback, void* user_data);
ohmd_status omodule_send_message(omodule* me, const char* output_name, omessage* msg);

omessage* omessage_create(ohmd_context* ctx, const char* type_name);
void omessage_add_float_data(omessage* me, const char* name, const float* data, int count);
void omessage_add_int_data(omessage* me, const char* name, const int* data, int count);
void omessage_add_bin_data(omessage* me, const char* name, const uint8_t* data, int count);
void omessage_add_string_data(omessage* me, const char* name, const char* data, int length);
int omessage_get_field_count(omessage* me);
const char* omessage_get_field_name(omessage* me, int idx);
omessage_data_type omessage_get_field_type(omessage* me, const char* name);
const float* omessage_get_float_data(omessage* me, const char* name, int* out_count);
const int* omessage_get_int_data(omessage* me, const char* name, int* out_count);
const uint8_t* omessage_get_bin_data(omessage* me, const char* name, int* out_count);
const char* omessage_get_string_data(omessage* me, const char* name, int* out_length);

#ifdef __cplusplus
}
#endif

#endif
