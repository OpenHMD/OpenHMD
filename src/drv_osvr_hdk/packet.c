/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* OSVR HDK Driver - Packet Decoding and Utilities */

#include <stdio.h>
#include "osvr.h"

#define SKIP_CMD (buffer++)
#define READ8 *(buffer++);
#define READ16 *buffer | (*(buffer + 1) << 8); buffer += 2;
#define READ32 *buffer | (*(buffer + 1) << 8) | (*(buffer + 2) << 16) | (*(buffer + 3) << 24); buffer += 4;
#define READFLOAT ((float)(*buffer)); buffer += 4;
#define READFIXED (float)(*buffer | (*(buffer + 1) << 8) | (*(buffer + 2) << 16) | (*(buffer + 3) << 24)) / 1000000.0f; buffer += 4;

#define WRITE8(_val) *(buffer++) = (_val);
#define WRITE16(_val) WRITE8((_val) & 0xff); WRITE8(((_val) >> 8) & 0xff);
#define WRITE32(_val) WRITE16((_val) & 0xffff) *buffer; WRITE16(((_val) >> 16) & 0xffff);

bool osvr_decodesensor_display_info(pkt_sensor_display_info* info, const unsigned char* buffer, int size)
{
	if(!(size == 56 || size == 57)){
		LOGE("invalid packet size (expected 56 or 57 but got %d)", size);
		//return false;
	}

	SKIP_CMD;
	info->command_id = READ16;
	info->distortion_type = READ8;
	info->h_resolution = READ16;
	info->v_resolution = READ16;
	info->h_screen_size = READFIXED;
	info->v_screen_size = READFIXED;
	info->v_center = READFIXED;
	info->lens_separation = READFIXED;
	info->eye_to_screen_distance[0] = READFIXED;
	info->eye_to_screen_distance[1] = READFIXED;

	info->distortion_type_opts = 0;

	for(int i = 0; i < 6; i++)
		info->distortion_k[i] = READFLOAT;

	return true;
}

bool osvr_decodesensor_config(pkt_sensor_config* config, const unsigned char* buffer, int size)
{
	if(!(size == 7 || size == 8)){
		LOGE("invalid packet size (expected 7 or 8 but got %d)", size);
		return false;
	}

	SKIP_CMD;
	config->command_id = READ16;
	config->flags = READ8;
	config->packet_interval = READ8;
	config->keep_alive_interval = READ16;

	return true;
}


bool osvr_decode_tracker_sensor_msg(pkt_tracker_sensor* msg, const unsigned char* buffer, int size)
{
	if(!(size == 16)){
		LOGE("invalid packet size (expected 16 but got %d)", size);
		return false;
	}

	READ8; //version number
	msg->sequence_number = READ8;
	for(int i = 0; i < 4; i++){
		msg->device_quat[i] = READ16;
	}

	for(int i = 0; i < 3; i++){
		msg->accel[i] = READ16;
	}

	return true;
}

void osvr_dump_packet_tracker_sensor(const pkt_tracker_sensor* sensor)
{
	(void)sensor;
	LOGD("  sequence numer: 	%d", sensor->sequence_number);
	LOGD("  device quat: %d %d %d %d", sensor->device_quat[0], sensor->device_quat[1], sensor->device_quat[2], sensor->device_quat[3]);
	LOGD("  accelerometer:  %d %d %d", sensor->accel[0], sensor->accel[1], sensor->accel[2]);
}
