/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

#ifndef OPENHMD_H
#define OPENHMD_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#ifdef DLL_EXPORT
#define OHMD_APIENTRY __cdecl __declspec( dllexport )
#else
#ifdef OHMD_STATIC
#define OHMD_APIENTRY __cdecl
#else
#define OHMD_APIENTRY __cdecl __declspec( dllimport )
#endif
#endif
#else
#define OHMD_APIENTRY
#endif

#define OHMD_STR_SIZE 256

typedef enum {
	OHMD_VENDOR    = 0,
	OHMD_PRODUCT   = 1,
	OHMD_PATH      = 2,
} ohmd_string_value;

typedef enum {
	OHMD_ROTATION_EULER                   =  0,
	OHMD_ROTATION_QUAT                    =  1,

	OHMD_LEFT_EYE_GL_MODELVIEW_MATRIX     =  2,
	OHMD_RIGHT_EYE_GL_MODELVIEW_MATRIX    =  3,

	OHMD_LEFT_EYE_GL_PROJECTION_MATRIX    =  4,
	OHMD_RIGHT_EYE_GL_PROJECTION_MATRIX   =  5,

	OHMD_POSITION_VECTOR                  =  6,

	OHMD_SCREEN_HORIZONTAL_SIZE           =  7,
	OHMD_SCREEN_VERTICAL_SIZE             =  8,

	OHMD_LENS_HORIZONTAL_SEPERATION       =  9,
	OHMD_LENS_VERTICAL_POSITION           = 10,

	OHMD_LEFT_EYE_FOV                     = 11,
	OHMD_LEFT_EYE_ASPECT_RATIO            = 12,
	OHMD_RIGHT_EYE_FOV                    = 13,
	OHMD_RIGHT_EYE_ASPECT_RATIO           = 14,

	OHMD_EYE_IDP                          = 15,

	OHMD_PROJECTION_ZFAR                  = 16,
	OHMD_PROJECTION_ZNEAR                 = 17,

} ohmd_float_value;

typedef enum {
	OHMD_SCREEN_HORIZONTAL_RESOLUTION     =  0,
	OHMD_SCREEN_VERTICAL_RESOLUTION       =  1,

} ohmd_int_value;

typedef struct ohmd_context ohmd_context;
typedef struct ohmd_device ohmd_device;

OHMD_APIENTRY ohmd_context* ohmd_ctx_create();
OHMD_APIENTRY void ohmd_ctx_destroy(ohmd_context* ctx);
OHMD_APIENTRY const char* ohmd_ctx_get_error(ohmd_context* ctx);
OHMD_APIENTRY void ohmd_ctx_update(ohmd_context* ctx);
OHMD_APIENTRY int ohmd_ctx_probe(ohmd_context* ctx);

OHMD_APIENTRY const char* ohmd_list_gets(ohmd_context* ctx, int index, ohmd_string_value type);
OHMD_APIENTRY ohmd_device* ohmd_list_open_device(ohmd_context* ctx, int index);

OHMD_APIENTRY int ohmd_device_getf(ohmd_device* device, ohmd_float_value type, float* out);
OHMD_APIENTRY int ohmd_device_setf(ohmd_device* device, ohmd_float_value type, float* in);
OHMD_APIENTRY int ohmd_device_geti(ohmd_device* device, ohmd_int_value type, int* out);

#ifdef __cplusplus
}
#endif

#endif
