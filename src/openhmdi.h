/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* Internal interface */

#ifndef OPENHMDI_H
#define OPENHMDI_H

#include "openhmd.h"
#include "omath.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define OHMD_MAX_DEVICES 16

#define OHMD_MAX(_a, _b) ((_a) > (_b) ? (_a) : (_b))
#define OHMD_MIN(_a, _b) ((_a) < (_b) ? (_a) : (_b))

#define OHMD_STRINGIFY(_what) #_what

typedef struct ohmd_driver ohmd_driver;

typedef struct
{
	char driver[OHMD_STR_SIZE];
	char vendor[OHMD_STR_SIZE];
	char product[OHMD_STR_SIZE];
	char path[OHMD_STR_SIZE];
	int revision;
	ohmd_driver* driver_ptr;
} ohmd_device_desc;

typedef struct {
	int num_devices;
	ohmd_device_desc devices[OHMD_MAX_DEVICES];
} ohmd_device_list;

struct ohmd_driver {
	void (*get_device_list)(ohmd_driver* driver, ohmd_device_list* list);
	ohmd_device* (*open_device)(ohmd_driver* driver, ohmd_device_desc* desc);
	void (*destroy)(ohmd_driver* driver);
	ohmd_context* ctx;
};

typedef struct {
		int hres;
		int vres;
		float hsize;
		float vsize;

		float lens_sep;
		float lens_vpos;

		float fov;
		float ratio;

		float ipd;
		float zfar;
		float znear;

		int accel_only; //bool-like for setting acceleration only fallback (android driver)

		mat4x4f proj_left; // adjusted projection matrix for left screen
		mat4x4f proj_right; // adjusted projection matrix for right screen
} ohmd_device_properties;

struct ohmd_device {
	ohmd_device_properties properties;

	quatf rotation_correction;
	vec3f position_correction;

	int (*getf)(ohmd_device* device, ohmd_float_value type, float* out);
	int (*setf)(ohmd_device* device, ohmd_float_value type, float* in);
	int (*seti)(ohmd_device* device, ohmd_int_value type, int* in);
	int (*set_data)(ohmd_device* device, ohmd_data_value type, void* in);

	void (*update)(ohmd_device* device);
	void (*close)(ohmd_device* device);

	ohmd_context* ctx;
	int active_device_idx; // index into ohmd_device->active_devices[]
};

struct ohmd_context {
	ohmd_driver* drivers[16];
	int num_drivers;

	ohmd_device_list list;

	ohmd_device* active_devices[256];
	int num_active_devices;

	char error_msg[OHMD_STR_SIZE];
};

// helper functions
void ohmd_set_default_device_properties(ohmd_device_properties* props);
void ohmd_calc_default_proj_matrices(ohmd_device_properties* props);

// drivers
ohmd_driver* ohmd_create_dummy_drv(ohmd_context* ctx);
ohmd_driver* ohmd_create_oculus_rift_drv(ohmd_context* ctx);
ohmd_driver* ohmd_create_external_drv(ohmd_context* ctx);
ohmd_driver* ohmd_create_android_drv(ohmd_context* ctx);

#include "log.h"
#include "platform.h"
#include "omath.h"
#include "fusion.h"

#endif
