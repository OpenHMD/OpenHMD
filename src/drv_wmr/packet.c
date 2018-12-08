// Copyright 2018, Philipp Zabel.
// SPDX-License-Identifier: BSL-1.0
/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 */

/* Windows Mixed Reality Driver */


#include "wmr.h"

#ifdef _MSC_VER
#define inline __inline
#endif

inline static uint8_t read8(const unsigned char** buffer)
{
	uint8_t ret = **buffer;
	*buffer += 1;
	return ret;
}

inline static int16_t read16(const unsigned char** buffer)
{
	int16_t ret = **buffer | (*(*buffer + 1) << 8);
	*buffer += 2;
	return ret;
}

inline static int32_t read_i32(const unsigned char** buffer)
{
	int32_t ret = **buffer | (*(*buffer + 1) << 8) | (*(*buffer + 2) << 16) | (*(*buffer + 3) << 24);
	*buffer += 4;
	return ret;
}

inline static int32_t read24(const unsigned char** buffer)
{
	int32_t ret = (**buffer << 8) | (*(*buffer + 1) << 16) | (*(*buffer + 2) << 24);
	*buffer += 3;
	return ret >> 8;
}

inline static int32_t read_u32(const unsigned char** buffer)
{
	int32_t ret = **buffer | (*(*buffer + 1) << 8) | (*(*buffer + 2) << 16) | (*(*buffer + 3) << 24);
	*buffer += 4;
	return ret;
}

inline static uint64_t read64(const unsigned char** buffer)
{
	uint64_t ret = (uint64_t)**buffer |
		       ((uint64_t)*(*buffer + 1) << 8) |
		       ((uint64_t)*(*buffer + 2) << 16) |
		       ((uint64_t)*(*buffer + 3) << 24) |
		       ((uint64_t)*(*buffer + 4) << 32) |
		       ((uint64_t)*(*buffer + 5) << 40) |
		       ((uint64_t)*(*buffer + 6) << 48) |
		       ((uint64_t)*(*buffer + 7) << 56);
	*buffer += 8;
	return ret;
}

bool hololens_sensors_decode_packet(hololens_sensors_packet* pkt, const unsigned char* buffer, int size)
{
	if(size != 497 &&
	   size != 381){
		LOGE("invalid hololens sensor packet size (expected 497 but got %d)", size);
		return false;
	}

	pkt->id = read8(&buffer);
	for(int i = 0; i < 4; i++)
		pkt->temperature[i] = read16(&buffer);
	for(int i = 0; i < 4; i++)
		pkt->gyro_timestamp[i] = read64(&buffer);
	for(int i = 0; i < 3; i++){
		for (int j = 0; j < 32; j++)
			pkt->gyro[i][j] = read16(&buffer);
	}
	for(int i = 0; i < 4; i++)
		pkt->accel_timestamp[i] = read64(&buffer);
	for(int i = 0; i < 3; i++){
		for (int j = 0; j < 4; j++)
			pkt->accel[i][j] = read_i32(&buffer);
	}
	for(int i = 0; i < 4; i++)
		pkt->video_timestamp[i] = read64(&buffer);

	return true;
}

bool motion_controller_decode_packet(motion_controller_packet* pkt, const unsigned char* buffer, int size)
{
	uint8_t stick[3];

	if(size != 45){
		LOGE("invalid motion controller packet size (expected 45 but got %d)", size);
		return false;
	}

	pkt->id = read8(&buffer);
	pkt->buttons = read8(&buffer);
	stick[0] = read8(&buffer);
	stick[1] = read8(&buffer);
	stick[2] = read8(&buffer);
	pkt->stick[0] = stick[0] | ((stick[1] & 0xf) << 8);
	pkt->stick[1] = ((stick[1] & 0xf0) >> 4) | (stick[2] << 4);
	pkt->trigger = read8(&buffer);
	pkt->touchpad[0] = read8(&buffer);
	pkt->touchpad[1] = read8(&buffer);
	pkt->battery = read8(&buffer);
	pkt->accel[0] = read24(&buffer); /* 2 µg per LSB */
	pkt->accel[1] = read24(&buffer);
	pkt->accel[2] = read24(&buffer);
	buffer += 2;
	pkt->gyro[0] = read24(&buffer);
	pkt->gyro[1] = read24(&buffer);
	pkt->gyro[2] = read24(&buffer);
	pkt->timestamp = read_u32(&buffer); /* 100 µs per LSB */

	return true;
}
