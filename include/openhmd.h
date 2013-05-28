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
	OHMD_VENDOR,
	OHMD_PRODUCT,
	OHMD_PATH,
} ohmd_string_value;

typedef enum {
	OHMD_ROTATION_EULER,
	OHMD_ROTATION_QUAT,

	OHMD_MAT4X4_LEFT_EYE_GL_MODELVIEW,
	OHMD_MAT4X4_RIGHT_EYE_GL_MODELVIEW,

	OHMD_MAT4X4_LEFT_EYE_GL_PROJECTION,
	OHMD_MAT4X4_RIGHT_EYE_GL_PROJECTION,

	OHMD_POSITION_VEC
} ohmd_float_value;

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

#ifdef __cplusplus
}
#endif

#endif
