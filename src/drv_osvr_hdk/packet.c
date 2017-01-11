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

bool osvr_decodesensor_range(pkt_sensor_range* range, const unsigned char* buffer, int size)
{
	if(!(size == 8 || size == 9)){
		LOGE("invalid packet size (expected 8 or 9 but got %d)", size);
		return false;
	}

	SKIP_CMD;
	range->command_id = READ16;
	range->accel_scale = READ8;
	range->gyro_scale = READ16;
	range->mag_scale = READ16;

	return true;
}

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

//For use with packing
static void decodesample(const unsigned char* buffer, int32_t* smp)
{
	/*
	 * Decode 3 tightly packed 21 bit values from 4 bytes.
	 * We unpack them in the higher 21 bit values first and then shift
	 * them down to the lower in order to get the sign bits correct.
	 */

	int x = (buffer[0] << 24)          | (buffer[1] << 16) | ((buffer[2] & 0xF8) << 8);
	int y = ((buffer[2] & 0x07) << 29) | (buffer[3] << 21) | (buffer[4] << 13) | ((buffer[5] & 0xC0) << 5);
	int z = ((buffer[5] & 0x3F) << 26) | (buffer[6] << 18) | (buffer[7] << 10);

	smp[0] = x >> 11;
	smp[1] = y >> 11;
	smp[2] = z >> 11;
}

bool osvr_decode_tracker_sensor_msg(pkt_tracker_sensor* msg, const unsigned char* buffer, int size)
{
	if(!(size == 62 || size == 64)){
		LOGE("invalid packet size (expected 62 or 64 but got %d)", size);
		return false;
	}

	msg->report_id = READ8;
	buffer += 2;
	msg->sample_delta = READ8;
	msg->sample_number = READ16;
	buffer += 2;
	msg->tick = READ32;

	for(int i = 0; i < 2; i++){
		decodesample(buffer, msg->samples[i].accel);
		buffer += 8;

		decodesample(buffer, msg->samples[i].gyro);
		buffer += 8;
	}

	return true;
}

// TODO do we need to consider HMD vs sensor "centric" values
void vec3f_from_vec(const int32_t* smp, vec3f* out_vec)
{
	out_vec->x = (float)smp[0] * 0.0001f;
	out_vec->y = ((float)smp[2] * 0.0001f) * -1;
	out_vec->z = (float)smp[1] * 0.0001f;
}

int osvr_encode_sensor_config(unsigned char* buffer, const pkt_sensor_config* config)
{
	return 7; // sensor config packet size
}

int osvr_encode_keep_alive(unsigned char* buffer, const pkt_keep_alive* keep_alive)
{
	return 5; // keep alive packet size
}

void osvr_dump_packet_sensor_config(const pkt_sensor_config* config)
{
	(void)config;

	LOGD("sensor config");
}

void osvr_dump_packet_tracker_sensor(const pkt_tracker_sensor* sensor)
{
	(void)sensor;
	LOGD("  report id: 	%u", sensor->report_id);
	LOGD("  sample delta: 	%u", sensor->sample_delta);
	LOGD("  sample number: 	%d", sensor->sample_number);
	LOGD("  tick: 		%u", sensor->tick);
	for(int i = 0; i < 2; i++){
		LOGD("    accel: %d %d %d", sensor->samples[i].accel[0], sensor->samples[i].accel[1], sensor->samples[i].accel[2]);
		LOGD("    gyro:  %d %d %d", sensor->samples[i].gyro[0], sensor->samples[i].gyro[1], sensor->samples[i].gyro[2]);
	}
}
