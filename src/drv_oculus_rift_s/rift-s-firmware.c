/*
 * Copyright 2020 Jan Schmidt
 * SPDX-License-Identifier: BSL-1.0
 *
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 */

/* Oculus Rift S Driver - firmware JSON parsing functions */
#include <string.h>

#include "rift-s-firmware.h"
#include "../ext_deps/nxjson.h"

static bool json_read_vec3(const nx_json *nxj, const char *key, vec3f *out)
{
	const nx_json *member = nx_json_get (nxj, key);

	if (member->type != NX_JSON_ARRAY)
		return false;

	for (int i = 0; i < 3; i++) {
		const nx_json *item = nx_json_item (member, i);

		if (item->type != NX_JSON_DOUBLE)
			return false;

		out->arr[i] = item->dbl_value;
	}

	return true;
}

static bool json_read_mat4x4(const nx_json *nxj, const char *key, mat4x4f *out)
{
	const nx_json *member = nx_json_get (nxj, key);

	if (member->type != NX_JSON_ARRAY)
		return false;

	for (int i = 0; i < 12; i++) {
		const nx_json *item = nx_json_item (member, i);

		if (item->type != NX_JSON_DOUBLE)
			return false;

		out->arr[i] = item->dbl_value;
	}

	return true;
}

static bool json_read_mat3x3(const nx_json *nxj, const char *key, float out[3][3])
{
	const nx_json *member = nx_json_get (nxj, key);
	int i = 0;

	if (member->type != NX_JSON_ARRAY)
		return false;

	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++) {
			const nx_json *item = nx_json_item (member, i);

			if (item->type != NX_JSON_DOUBLE)
				return false;

			out[y][x] = item->dbl_value;
			i++;
		}
	}

	return true;
}

int rift_s_parse_imu_calibration(char *json,
		rift_s_imu_calibration *c)
{
	const nx_json* nxj, *obj, *version, *imu;
	float version_number = -1;

	nxj = nx_json_parse (json, 0);
	if (nxj == NULL)
		return -1;

	obj = nx_json_get(nxj, "FileFormat");
	if (obj->type == NX_JSON_NULL)
		goto fail;

	version = nx_json_get (obj, "Version");
	if (version->type != NX_JSON_STRING) {
		goto fail;
	}
	version_number = strtof(version->text_value, NULL);
	if (version_number != 1.0)
		goto fail;

	imu = nx_json_get(nxj, "ImuCalibration");
	if (obj->type == NX_JSON_NULL)
		goto fail;

	if (!json_read_mat4x4 (imu, "DeviceFromImu", &c->imu_to_device_transform))
		goto fail;

	obj = nx_json_get (imu, "Gyroscope");
	if (obj->type != NX_JSON_OBJECT)
		goto fail;

	if (!json_read_mat3x3(obj, "RectificationMatrix", c->gyro.rectification))
		goto fail;

	obj = nx_json_get (obj, "Offset");
	if (obj->type != NX_JSON_OBJECT)
		goto fail;

	if (!json_read_vec3(obj, "ConstantOffset", &c->gyro.offset))
		goto fail;

	obj = nx_json_get (imu, "Accelerometer");
	if (obj->type != NX_JSON_OBJECT)
		goto fail;

	if (!json_read_mat3x3(obj, "RectificationMatrix", c->accel.rectification))
		goto fail;

	obj = nx_json_get (obj, "Offset");
	if (obj->type != NX_JSON_OBJECT)
		goto fail;

	if (!json_read_vec3(obj, "OffsetAtZeroDegC", &c->accel.offset_at_0C))
		goto fail;
	if (!json_read_vec3(obj, "OffsetTemperatureCoefficient", &c->accel.temp_coeff))
		goto fail;

	nx_json_free (nxj);
	return 0;

fail:
	LOGW ("Unrecognised Rift S IMU Calibration JSON data. Version %f\n%s\n", version_number, json);
	nx_json_free (nxj);
	return -1;
}

static bool json_read_led_point (const nx_json *led_model, rift_s_led *led, int n) {
	const nx_json* array;
	const nx_json* point[9];
	char name[32];
	int j;

	snprintf(name, 32, "Point%d", n);
	array = nx_json_get (led_model, name);
	if (array->type != NX_JSON_ARRAY || array->length != 9) {
		return false;
	}

	for (j = 0; j < 9; j++)
		point[j] = nx_json_item (array, j);

	led->pos.x = point[0]->dbl_value;
	led->pos.y = point[1]->dbl_value;
	led->pos.z = point[2]->dbl_value;
	led->dir.x = point[3]->dbl_value;
	led->dir.y = point[4]->dbl_value;
	led->dir.z = point[5]->dbl_value;
	led->angles.x = point[6]->dbl_value;
	led->angles.y = point[7]->dbl_value;
	led->angles.z = point[8]->dbl_value;

	return true;
}

static bool json_read_lensing_model (const nx_json *lensing_model, rift_s_lensing_model *model, int n) {
	const nx_json* array;
	char name[32];
	int j;

	snprintf(name, 32, "Model%d", n);
	array = nx_json_get (lensing_model, name);
	if (array->type != NX_JSON_ARRAY || array->length != 5) {
		return false;
	}

	model->num_points = nx_json_item (array, 0)->int_value;

	for (j = 0; j < 4; j++)
		model->points[j] = nx_json_item (array, j + 1)->dbl_value;

	return true;
}

int rift_s_controller_parse_imu_calibration(char *json,
		rift_s_controller_imu_calibration *c)
{
	const nx_json* nxj, *obj, *version, *leds;
	int i;

	nxj = nx_json_parse (json, 0);
	if (nxj == NULL)
		return -1;

	obj = nx_json_get(nxj, "TrackedObject");
	if (obj->type == NX_JSON_NULL)
		goto fail;

	version = nx_json_get (obj, "FlsVersion");
	if (version->type != NX_JSON_STRING) {
		goto fail;
	}
	if (strcmp(version->text_value, "1.0.10")) {
		LOGE("Controller calibration version number has changed - got %s", version->text_value);
		goto fail;
	}

	if (!json_read_vec3(obj, "ImuPosition", &c->imu_position))
		goto fail;

	if (!json_read_mat4x4 (obj, "AccCalibration", &c->accel_calibration))
		goto fail;

	if (!json_read_mat4x4 (obj, "GyroCalibration", &c->gyro_calibration))
		goto fail;

	/* LED positions */
	leds = nx_json_get (obj, "ModelPoints");
	if (leds->type != NX_JSON_OBJECT)
		goto fail;

	c->num_leds = leds->length;
	c->leds = calloc (c->num_leds, sizeof(rift_s_led));
	for (i = 0; i < c->num_leds; i++) {
		if (!json_read_led_point (leds, c->leds + i, i))
			goto fail;
	}

	/* LED lensing models */
	leds = nx_json_get (obj, "Lensing");
	if (leds->type != NX_JSON_OBJECT)
		goto fail;

	c->num_lensing_models = leds->length;
	c->lensing_models = calloc (c->num_lensing_models, sizeof(rift_s_lensing_model));
	for (i = 0; i < c->num_lensing_models; i++) {
		if (!json_read_lensing_model (leds, c->lensing_models + i, i))
			goto fail;
	}

	if (!json_read_mat3x3(nxj, "gyro_m", c->gyro.rectification))
		goto fail;
	if (!json_read_vec3(nxj, "gyro_b", &c->gyro.offset))
		goto fail;

	if (!json_read_mat3x3(nxj, "acc_m", c->accel.rectification))
		goto fail;
	if (!json_read_vec3(nxj, "acc_b", &c->accel.offset))
		goto fail;

	nx_json_free (nxj);
	return 0;

fail:
	LOGW ("Unrecognised Rift S Controller Calibration JSON data.\n%s\n", json);
	rift_s_controller_free_imu_calibration(c);
	nx_json_free (nxj);
	return -1;
}

void rift_s_controller_free_imu_calibration(rift_s_controller_imu_calibration *c)
{
	if (c->lensing_models) {
		free (c->lensing_models);
		c->lensing_models = NULL;
	}

	if (c->leds) {
		free (c->leds);
		c->leds = NULL;
	}
}
