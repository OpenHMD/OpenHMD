// Copyright 2023, 20kdc.
// SPDX-License-Identifier: BSL-1.0
/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 */

/*
 * File Driver (based off of External Driver) Version Remapper
 */

#include "drv_file_fmt.h"
#include "../openhmdi.h"
#include "string.h"

// For easy lookup
#define OHMD_FILE_DRV_ERROR_NOHEAD 1
#define OHMD_FILE_DRV_ERROR_NOBODY 2
#define OHMD_FILE_DRV_ERROR_BADVER 3
#define OHMD_FILE_DRV_ERROR_TOOMANYCONTROLS 4

// returns non-zero on error
int ohmd_file_drv_read_file(FILE * file, ohmd_file_data_latest* out)
{
	ohmd_file_header header = {};
	memset(out, 0, sizeof(*out));
	if (fread(&header, sizeof(header), 1, file) != 1)
		return OHMD_FILE_DRV_ERROR_NOHEAD;
	if (header.version == OHMD_FILE_DATA_LATEST) {
		if (fread(out, sizeof(*out), 1, file) != 1)
			return OHMD_FILE_DRV_ERROR_NOBODY;
	} else {
		return OHMD_FILE_DRV_ERROR_BADVER;
	}
	// Validation
	if (out->controls.count > 64)
		return OHMD_FILE_DRV_ERROR_TOOMANYCONTROLS;
	out->name[OHMD_STR_SIZE - 1] = 0;
	return 0;
}

static quatf do_euler2quat(float pitch, float yaw, float roll)
{
	vec3f pitcher_axis = {{1, 0, 0}};
	vec3f yawer_axis =   {{0, 1, 0}};
	vec3f roller_axis =  {{0, 0, 1}};

	quatf pitcher, yawer, roller;
	quatf interm1, interm2;

	oquatf_init_axis(&pitcher, &pitcher_axis, pitch);
	oquatf_init_axis(&yawer, &yawer_axis, yaw);
	oquatf_init_axis(&roller, &roller_axis, roll);

	oquatf_mult(&yawer, &pitcher, &interm1);
	oquatf_mult(&interm1, &roller, &interm2);
	return interm2;
}

quatf ohmd_file_drv_rotation2quat(ohmd_file_rotation rotation)
{
	if (rotation.is_quaternion) {
		quatf res = {{rotation.data[0], rotation.data[1], rotation.data[2], rotation.data[3]}};
		return res;
	} else {
		return do_euler2quat(rotation.data[0], rotation.data[1], rotation.data[2]);
	}
}

