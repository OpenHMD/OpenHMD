// Copyright 2019, Joey Ferwerda.
// SPDX-License-Identifier: BSL-1.0
/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 */

/* DreamWorld DreamGlass Driver */

#ifndef DWDG_H
#define DWDG_H

#include <stdint.h>
#include <stdbool.h>

#include "../openhmdi.h"

#define FEATURE_BUFFER_SIZE	256

typedef struct
{
	int32_t accel[3];
	int32_t gyro[3];
	uint32_t tick;
} dwdg_sensor_sample;

bool decode_dwdg_imu_msg(dwdg_sensor_sample* smp, const unsigned char* buffer, int size);

#endif