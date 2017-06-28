/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* NOLO VR - Internal Interface */

#ifndef TEMPLATEDRIVER_H
#define TEMPLATEDRIVER_H

#include "../openhmdi.h"
#include <hidapi.h>

#define FEATURE_BUFFER_SIZE 64

typedef struct {
	ohmd_device base;

	hid_device* handle;
} drv_priv;

typedef enum {
	DRV_CMD_SENSOR_CONFIG = 2,
	DRV_CMD_RANGE = 4,
	DRV_CMD_KEEP_ALIVE = 8,
	DRV_CMD_DISPLAY_INFO = 9
} drv_sensor_feature_cmd;


void btea_decrypt(uint32_t *v, int n, int base_rounds, uint32_t const key[4]);
void nolo_decrypt_data(unsigned char* buf);

void nolo_decode_base_station(drv_priv* priv, unsigned char* data);
void nolo_decode_hmd_marker(drv_priv* priv, unsigned char* data);
void nolo_decode_controller(int idx, unsigned char* data);
void nolo_decode_orientation(const unsigned char* data, quatf* quat);
void nolo_decode_position(const unsigned char* data, vec3f* pos);

#endif

