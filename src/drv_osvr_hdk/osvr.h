/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* udev rules for HDK2
 * SUBSYSTEM=="usb", ATTR{idVendor}=="1532", ATTR{idProduct}=="0b00", MODE="0666", GROUP="plugdev" # osvr sensors
 * SUBSYSTEM=="usb", ATTR{idVendor}=="0572", ATTR{idProduct}=="1806", MODE="0666", GROUP="plugdev" # osvr audio
 * SUBSYSTEM=="usb", ATTR{idVendor}=="0bda", ATTR{idProduct}=="57e8", MODE="0666", GROUP="plugdev" # tracker camera for uvc-camera
*/

/* OSVR HDK template Internal Interface */

#ifndef OSVR_HDK_H
#define OSVR_HDK_H

#include "../openhmdi.h"

#define FEATURE_BUFFER_SIZE 256

typedef enum {
	DRV_CMD_SENSOR_CONFIG = 2,
	DRV_CMD_RANGE = 4,
	DRV_CMD_KEEP_ALIVE = 8,
	DRV_CMD_DISPLAY_INFO = 9
} drv_sensor_feature_cmd;

typedef struct {
	uint16_t command_id;
	uint16_t accel_scale;
	uint16_t gyro_scale;
	uint16_t mag_scale;
} pkt_sensor_range;

typedef struct {
	int32_t accel[3];
	int32_t gyro[3];
} pkt_tracker_sample;

typedef struct {
	uint8_t report_id;
	uint8_t sample_delta;
	uint16_t sample_number;
	uint32_t tick;
	pkt_tracker_sample samples[2];
	int16_t mag[3];
} pkt_tracker_sensor;

typedef struct {
    uint16_t command_id;
    uint8_t flags;
    uint16_t packet_interval;
    uint16_t keep_alive_interval; // in ms
} pkt_sensor_config;

typedef struct {
	uint16_t command_id;
	uint8_t distortion_type;
	uint8_t distortion_type_opts;
	uint16_t h_resolution, v_resolution;
	float h_screen_size, v_screen_size;
	float v_center;
	float lens_separation;
	float eye_to_screen_distance[2];
	float distortion_k[6];
} pkt_sensor_display_info;

typedef struct {
	uint16_t command_id;
	uint16_t keep_alive_interval;
} pkt_keep_alive;


bool osvr_decode_sensor_range(pkt_sensor_range* range, const unsigned char* buffer, int size);
bool osvr_decode_sensor_display_info(pkt_sensor_display_info* info, const unsigned char* buffer, int size);
bool osvr_decode_sensor_config(pkt_sensor_config* config, const unsigned char* buffer, int size);
bool osvr_decode_tracker_sensor_msg(pkt_tracker_sensor* msg, const unsigned char* buffer, int size);

void vec3f_from_vec(const int32_t* smp, vec3f* out_vec);

int osvr_encode_sensor_config(unsigned char* buffer, const pkt_sensor_config* config);
int osvr_encode_keep_alive(unsigned char* buffer, const pkt_keep_alive* keep_alive);

void osvr_dump_packet_sensor_range(const pkt_sensor_range* range);
void osvr_dump_packet_sensor_config(const pkt_sensor_config* config);
void osvr_dump_packet_sensor_display_info(const pkt_sensor_display_info* info);
void osvr_dump_packet_tracker_sensor(const pkt_tracker_sensor* sensor);

#endif
