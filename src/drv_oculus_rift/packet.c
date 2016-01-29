/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* Oculus Rift Driver - Packet Decoding and Utilities */

#include <stdio.h>
#include "rift.h"

#define SKIP_CMD (buffer++)
#define READ8 *(buffer++);
#define READ16 *buffer | (*(buffer + 1) << 8); buffer += 2;
#define READ32 *buffer | (*(buffer + 1) << 8) | (*(buffer + 2) << 16) | (*(buffer + 3) << 24); buffer += 4;
#define READFLOAT ((float)(*buffer)); buffer += 4;
#define READFIXED (float)(*buffer | (*(buffer + 1) << 8) | (*(buffer + 2) << 16) | (*(buffer + 3) << 24)) / 1000000.0f; buffer += 4;

#define WRITE8(_val) *(buffer++) = (_val);
#define WRITE16(_val) WRITE8((_val) & 0xff); WRITE8(((_val) >> 8) & 0xff);
#define WRITE32(_val) WRITE16((_val) & 0xffff) *buffer; WRITE16(((_val) >> 16) & 0xffff);

bool decode_sensor_range(pkt_sensor_range* range, const unsigned char* buffer, int size)
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

bool decode_sensor_display_info(pkt_sensor_display_info* info, const unsigned char* buffer, int size)
{
	if(!(size == 56 || size == 57)){
		LOGE("invalid packet size (expected 56 or 57 but got %d)", size);
		return false;
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

bool decode_sensor_config(pkt_sensor_config* config, const unsigned char* buffer, int size)
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

static void decode_sample(const unsigned char* buffer, int32_t* smp)
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

bool decode_tracker_sensor_msg(pkt_tracker_sensor* msg, const unsigned char* buffer, int size)
{
	if(!(size == 62 || size == 64)){
		LOGE("invalid packet size (expected 62 or 64 but got %d)", size);
		return false;
	}

	SKIP_CMD;
	msg->num_samples = READ8;
	msg->timestamp = READ16;
	msg->last_command_id = READ16;
	msg->temperature = READ16;

	int actual = OHMD_MIN(msg->num_samples, 3);
	for(int i = 0; i < actual; i++){
		decode_sample(buffer, msg->samples[i].accel);
		buffer += 8;

		decode_sample(buffer, msg->samples[i].gyro);
		buffer += 8;
	}

	// Skip empty samples
	buffer += (3 - actual) * 16;
	for(int i = 0; i < 3; i++){
		msg->mag[i] = READ16;
	}

	return true;
}

// TODO do we need to consider HMD vs sensor "centric" values
void vec3f_from_rift_vec(const int32_t* smp, vec3f* out_vec)
{
	out_vec->x = (float)smp[0] * 0.0001f;
	out_vec->y = (float)smp[1] * 0.0001f;
	out_vec->z = (float)smp[2] * 0.0001f;
}

int encode_sensor_config(unsigned char* buffer, const pkt_sensor_config* config)
{
	WRITE8(RIFT_CMD_SENSOR_CONFIG);
	WRITE16(config->command_id);
	WRITE8(config->flags);
	WRITE8(config->packet_interval);
	WRITE16(config->keep_alive_interval);
	return 7; // sensor config packet size
}

int encode_keep_alive(unsigned char* buffer, const pkt_keep_alive* keep_alive)
{
	WRITE8(RIFT_CMD_KEEP_ALIVE);
	WRITE16(keep_alive->command_id);
	WRITE16(keep_alive->keep_alive_interval);
	return 5; // keep alive packet size
}

void dump_packet_sensor_range(const pkt_sensor_range* range)
{
	(void)range;

	LOGD("sensor range\n");
	LOGD("  command id:  %d", range->command_id);
	LOGD("  accel scale: %d", range->accel_scale);
	LOGD("  gyro scale:  %d", range->gyro_scale);
	LOGD("  mag scale:   %d", range->mag_scale);
}

void dump_packet_sensor_display_info(const pkt_sensor_display_info* info)
{
	(void)info;

	LOGD("display info");
	LOGD("  command id:             %d", info->command_id);
	LOGD("  distortion_type:        %d", info->distortion_type);
	LOGD("  resolution:             %d x %d", info->h_resolution, info->v_resolution);
	LOGD("  screen size:            %f x %f", info->h_screen_size, info->v_screen_size);
	LOGD("  vertical center:        %f", info->v_center);
	LOGD("  lens_separation:        %f", info->lens_separation);
	LOGD("  eye_to_screen_distance: %f, %f", info->eye_to_screen_distance[0], info->eye_to_screen_distance[1]);
	LOGD("  distortion_k:           %f, %f, %f, %f, %f, %f",
		info->distortion_k[0], info->distortion_k[1], info->distortion_k[2],
		info->distortion_k[3], info->distortion_k[4], info->distortion_k[5]);
}

void dump_packet_sensor_config(const pkt_sensor_config* config)
{
	(void)config;

	LOGD("sensor config");
	LOGD("  command id:          %u", config->command_id);
	LOGD("  flags:               %02x", config->flags);
	LOGD("    raw mode:                  %d", !!(config->flags & RIFT_SCF_RAW_MODE));
	LOGD("    calibration test:          %d", !!(config->flags & RIFT_SCF_CALIBRATION_TEST));
	LOGD("    use calibration:           %d", !!(config->flags & RIFT_SCF_USE_CALIBRATION));
	LOGD("    auto calibration:          %d", !!(config->flags & RIFT_SCF_AUTO_CALIBRATION));
	LOGD("    motion keep alive:         %d", !!(config->flags & RIFT_SCF_MOTION_KEEP_ALIVE));
	LOGD("    motion command keep alive: %d", !!(config->flags & RIFT_SCF_COMMAND_KEEP_ALIVE));
	LOGD("    sensor coordinates:        %d", !!(config->flags & RIFT_SCF_SENSOR_COORDINATES));
	LOGD("  packet interval:     %u", config->packet_interval);
	LOGD("  keep alive interval: %u", config->keep_alive_interval);
}

void dump_packet_tracker_sensor(const pkt_tracker_sensor* sensor)
{
	(void)sensor;

	LOGD("tracker sensor:");
	LOGD("  last command id: %u", sensor->last_command_id);
	LOGD("  timestamp:       %u", sensor->timestamp);
	LOGD("  temperature:     %d", sensor->temperature);
	LOGD("  num samples:     %u", sensor->num_samples);
	LOGD("  magnetic field:  %i %i %i", sensor->mag[0], sensor->mag[1], sensor->mag[2]);

	for(int i = 0; i < OHMD_MIN(sensor->num_samples, 3); i++){
		LOGD("    accel: %d %d %d", sensor->samples[i].accel[0], sensor->samples[i].accel[1], sensor->samples[i].accel[2]);
		LOGD("    gyro:  %d %d %d", sensor->samples[i].gyro[0], sensor->samples[i].gyro[1], sensor->samples[i].gyro[2]);
	}
}
