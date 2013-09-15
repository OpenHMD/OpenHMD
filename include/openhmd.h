/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/** \file openhmd.h
 * Main header for OpenHMD public API.
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

/** Maximum length of a string, including termination, in OpenHMD. */
#define OHMD_STR_SIZE 256

/** A collection of string value information types used for getting information with ohmd_list_gets(). */
typedef enum {
	OHMD_VENDOR    = 0,
	OHMD_PRODUCT   = 1,
	OHMD_PATH      = 2,
} ohmd_string_value;

/** A collection of float value information types used for getting and setting information with 
 ohmd_device_getf() and ohmd_device_setf(). */
typedef enum {
	/** float[4], get - Absolute rotation of the device, in space, as a quaternion. */
	OHMD_ROTATION_QUAT                    =  1,

	/** float[16], get - A "ready to use" OpenGL style 4x4 matrix with a modelview matrix for the 
	 left eye of the HMD. */
	OHMD_LEFT_EYE_GL_MODELVIEW_MATRIX     =  2,

	/** float[16], get - A "ready to use" OpenGL style 4x4 matrix with a modelview matrix for the 
	 right eye of the HMD. */
	OHMD_RIGHT_EYE_GL_MODELVIEW_MATRIX    =  3,

	/** float[16], get - A "ready to use" OpenGL style 4x4 matrix with a projection matrix for the 
	 left eye of the HMD. */
	OHMD_LEFT_EYE_GL_PROJECTION_MATRIX    =  4,
	/** float[16], get - A "ready to use" OpenGL style 4x4 matrix with a projection matrix for the 
	 right eye of the HMD. */
	OHMD_RIGHT_EYE_GL_PROJECTION_MATRIX   =  5,

	/** float[3], get - A 3-D vector representing the absolute position of the device, in space. */
	OHMD_POSITION_VECTOR                  =  6,

	/** float[1], get - Physical width of the device screen, in centimeters. */
	OHMD_SCREEN_HORIZONTAL_SIZE           =  7,
	/** float[1], get - Physical height of the device screen, in centimeters. */
	OHMD_SCREEN_VERTICAL_SIZE             =  8,

	/** float[1], get - Physical speration of the device lenses, in centimeters. */
	OHMD_LENS_HORIZONTAL_SEPARATION       =  9,
	/** float[1], get - Physical vertical position of the lenses, in centimeters. */
	OHMD_LENS_VERTICAL_POSITION           = 10,

	/** float[1], get - Physical field of view for the left eye, in degrees. */
	OHMD_LEFT_EYE_FOV                     = 11,
	/** float[1], get - Physical display aspect ratio for the left eye screen. */
	OHMD_LEFT_EYE_ASPECT_RATIO            = 12,
	/** float[1], get - Physical field of view for the left right, in degrees. */
	OHMD_RIGHT_EYE_FOV                    = 13,
	/** float[1], get
	      Physical display aspect ratio for the right eye screen. */
	OHMD_RIGHT_EYE_ASPECT_RATIO           = 14,

	/** float[1], get/set - Physical interpupilary distance of the user, in centimeters. */
	OHMD_EYE_IPD                          = 15,

	/** float[1], get/set - Z-far value for the projection matrix calculations, i.e. drawing distance. */
	OHMD_PROJECTION_ZFAR                  = 16,
	/** float[1], get/set - Z-near value for the projection matrix calculations, i.e. close clipping 
	 distance. */
	OHMD_PROJECTION_ZNEAR                 = 17,

	/** float[6], get - Device specifc distortion value. */
	OHMD_DISTORTION_K                     = 18,

} ohmd_float_value;

/** A collection of int value information types used for getting information with ohmd_device_geti()
*/
typedef enum {
	/** int[1], get
	      Physical horizontal resolution of the device screen. */
	OHMD_SCREEN_HORIZONTAL_RESOLUTION     =  0,
	/** int[1], get
	      Physical vertical resolution of the device screen. */
	OHMD_SCREEN_VERTICAL_RESOLUTION       =  1,

} ohmd_int_value;

/** An opaque pointer to a context structure. */
typedef struct ohmd_context ohmd_context;

/** An opaque pointer to a structure representing a device, such as an HMD. */
typedef struct ohmd_device ohmd_device;

/**
 * Create an OpenHMD context.
 *
 * @return a pointer to an allocated ohmd_context on success or NULL if it fails
 */
OHMD_APIENTRY ohmd_context* ohmd_ctx_create();

/**
 * Destroy an OpenHMD context. 
 *
 * ohmd_ctx_destroy de-initializes and de-allocates an OpenHMD context allocated with ohmd_ctx_create.
 *  
 * @param ctx The context to destroy.
 */
OHMD_APIENTRY void ohmd_ctx_destroy(ohmd_context* ctx);

/**
 * Get the last error as a human readable string. 
 *
 * If a function taking a context as an argument (ohmd_context "methods") returns non-successfully,
 * a human readable error message describing what went wrong can be retreived with this function. 
 *  
 * @param ctx The context to retreive the error message from.
 * @return a pointer to the error message
 */
OHMD_APIENTRY const char* ohmd_ctx_get_error(ohmd_context* ctx);

/**
 * Update a context.
 *
 * Performs tasks like pumping events from the device. The exact details are up to the driver
 * but try to call it quite frequently.
 * Once per frame in a "game loop" should be sufficient.
 * If OpenHMD is handled in a background thread, calling ohmd_ctx_update and then sleeping for 10-20 ms
 * is recommended.
 * 
 * @param ctx The context that needs updating.
 */
OHMD_APIENTRY void ohmd_ctx_update(ohmd_context* ctx);

/**
 * Probe for devices.
 *
 * Probes for and enumerates supported devices attached to the system.
 * 
 * @param ctx A context with no currently open devices.
 * @return number of devices found on the system
 */
OHMD_APIENTRY int ohmd_ctx_probe(ohmd_context* ctx);

/**
 * Get device description from enumeration list index.
 *
 * Gets a human readable device description string from a zero indexed enumeration index 
 * between 0 and max, where max is the number ohmd_ctx_probe returned. 
 * (I.e. if ohmd_ctx_probe returns 3,valid indices are 0, 1 and 2).
 * The function can return three types of data. The vendor name, the product name and
 * a driver specific path where the device is attached.
 * ohmd_ctx_probe must be called before calling ohmd_list_gets.
 * 
 * @param ctx A (probed) context.
 * @param index An index, between 0 and the value returned from ohmd_ctx_probe.
 * @param type The type of data to fetch. One of OHMD_VENDOR, OHMD_PRODUCT and OHMD_PATH.
 * @return a string with a human readable device name
 */
OHMD_APIENTRY const char* ohmd_list_gets(ohmd_context* ctx, int index, ohmd_string_value type);

/**
 * Open a device.
 *
 * Opens a device from a zero indexed enumeration index between 0 and max, 
 * where max is the number ohmd_ctx_probe returned. (I.e. if ohmd_ctx_probe returns 3,
 * valid indices are 0, 1 and 2).
 * ohmd_ctx_probe must be called before calling ohmd_list_open_device.
 * 
 * @param ctx A (probed) context.
 * @param index An index, between 0 and the value returned from ohmd_ctx_probe.
 * @return a pointer to an ohmd_device, which represents a hardware device, such as an HMD.
 */
OHMD_APIENTRY ohmd_device* ohmd_list_open_device(ohmd_context* ctx, int index);

/**
 * Get a floating point value from a device.
 *
 * 
 * @param device An open device to retreive the value from.
 * @param type What type of value to retreive, see ohmd_float_value section for more information.
 * @param[out] out A pointer to a float, or float array where the retreived value should be written.
 * @return 0 on success, <0 on failure.
 */
OHMD_APIENTRY int ohmd_device_getf(ohmd_device* device, ohmd_float_value type, float* out);

/**
 * Set a floating point value for a device.
 *
 * @param device An open device to set the value in.
 * @param type What type of value to set, see ohmd_float_value section for more information.
 * @param in A pointer to a float, or float array where the new value is stored.
 * @return 0 on success, <0 on failure.
 */
OHMD_APIENTRY int ohmd_device_setf(ohmd_device* device, ohmd_float_value type, float* in);

/**
 * Get an integer value from a device. 
 *
 * @param device An open device to retreive the value from.
 * @param type What type of value to retreive, ohmd_int_value section for more information.
 * @param[out] out A pointer to an integer, or integer array where the retreived value should be written.
 * @return 0 on success, <0 on failure.
 */
OHMD_APIENTRY int ohmd_device_geti(ohmd_device* device, ohmd_int_value type, int* out);

#ifdef __cplusplus
}
#endif

#endif
