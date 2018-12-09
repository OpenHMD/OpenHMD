// Copyright 2018, Joey Ferwerda.
// SPDX-License-Identifier: BSL-1.0
/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 */

/* LG R100 (360VR) Driver */

#include "lgr100.h"

#define READFLOAT (*(float*)buffer); buffer += 4;

bool decode_lgr100_imu_msg(lgr100_sensor_sample* smp, const unsigned char* buffer, int size)
{
	if(size != 32){
		LOGE("invalid packet size (expected 32 but got %d)", size);
		return false;
	}

	buffer += 1;
	
	for(int i = 0; i < 3; i++){
		smp->gyro[i] = READFLOAT;
	}

	for(int i = 0; i < 3; i++){
		smp->accel[i] = READFLOAT;
	}

	//Unused
	//buffer += 1 //unknown
	//buffer += 4 //some counter
	//buffer += 1 //unknown or 0?
	return true;
}