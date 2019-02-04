// Copyright 2019, Joey Ferwerda.
// SPDX-License-Identifier: BSL-1.0
/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 */

/* DreamWorld DreamGlass Driver */

#include "dreamworld.h"

#define READ8 *(buffer++);
#define READ16 *buffer | (*(buffer + 1) << 8); buffer += 2;
#define READ32 *buffer | (*(buffer + 1) << 8) | (*(buffer + 2) << 16) | (*(buffer + 3) << 24); buffer += 4;
#define READFLOAT (*(float*)buffer); buffer += 4;

bool decode_dwdg_imu_msg(dwdg_sensor_sample* smp, const unsigned char* buffer, int size)
{
	if(!(size == 64)){
		LOGE("invalid packet size (expected 64 but got %d)", size);
		return false;
	}

	//buffer += 1;
	
	for(int i = 0; i < 3; i++){
		smp->gyro[i] = READ32;
	}

	for(int i = 0; i < 3; i++){
		smp->accel[i] = READ32;
	}
	return true;
}