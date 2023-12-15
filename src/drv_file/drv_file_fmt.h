// Copyright 2023, 20kdc.
// SPDX-License-Identifier: BSL-1.0
/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 */

/*
 * File Driver (based off of External Driver) Structures
 *
 * Effectively ABI:
 * These structs are effectively ABI!
 * It defines the binding between an external HMD daemon and OpenHMD.
 * In practice, this would be driven by a helper script written in, say, Python.
 */

#include "openhmd.h"
#include <stdint.h>

// -- Components --

// projection/setup info, version 0
typedef struct {
	// This struct is ignored if this value is 0.
	uint32_t hres;
	uint32_t vres;
	float hsize;
	float vsize;
	float lens_sep;
	float lens_vpos;
	float fov;
	float ratio;
	float distortion_k[6];
} ohmd_file_hmd_info0;

// control, version 0
typedef struct {
	ohmd_control_hint hint;
	ohmd_control_type type;
	float state;
} ohmd_file_control0;

// controls, version 0
typedef struct {
	uint32_t count; // 0-64
	ohmd_file_control0 control[64];
} ohmd_file_controls0;

// rotation that supports Euler Angles and Quaternions
typedef struct {
	uint32_t is_quaternion;
	// is_quaternion = 0: Pitch, Yaw, Roll (in radians), and an ignored value
	// is_quaternion = 1: OpenHMD quaternion
	float data[4];
} ohmd_file_rotation;

// -- Header --

typedef struct {
	// Structure version
	uint32_t version;
} ohmd_file_header;

// -- Versions --

// "Version 0": Euler
typedef struct {
	ohmd_device_class device_class;
	ohmd_device_flags device_flags;
	// name, null-padded
	// last character must be a null terminator or will be forced to be one
	// Size is important to security!
	char name[OHMD_STR_SIZE];
	ohmd_file_rotation rotation;
	float position[3];
	ohmd_file_hmd_info0 hmd_info;
	ohmd_file_controls0 controls;
} ohmd_file_data0;

#define OHMD_FILE_DATA_LATEST 0
typedef ohmd_file_data0 ohmd_file_data_latest;

