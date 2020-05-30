/*
 * Copyright 2013, Fredrik Hultin.
 * Copyright 2013, Jakob Bornecrantz.
 * Copyright 2016 Philipp Zabel
 * Copyright 2019-2020 Jan Schmidt
 * SPDX-License-Identifier: BSL-1.0
 *
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 */

/* Oculus Rift S Driver Internal Interface */
#ifndef RIFT_S_HMD_H
#define RIFT_S_HMD_H

#include "../openhmdi.h"
#include "../hid.h"

#include "rift-s-protocol.h"
#include "rift-s-firmware.h"
#include "rift-s-controller.h"
#include "rift-s-radio.h"

#define MAX_CONTROLLERS 2

struct rift_s_hmd_s {
	ohmd_context* ctx;
	int use_count;

	hid_device* handles[3];

	uint32_t last_imu_timestamp;
	double last_keep_alive;
	fusion sensor_fusion;
	vec3f raw_mag, raw_accel, raw_gyro;
	float temperature;

	bool display_on;

	rift_s_device_info_t device_info;
	rift_s_imu_config_t imu_config;
	rift_s_imu_calibration imu_calibration;

	/* Controller state tracking */
	int num_active_controllers;
	rift_s_controller_state controllers[MAX_CONTROLLERS];

	/* Radio comms manager */
  rift_s_radio_state radio_state;

	/* OpenHMD output devices */
	rift_s_device_priv hmd_dev;
	rift_s_controller_device touch_dev[MAX_CONTROLLERS];
};

#endif
