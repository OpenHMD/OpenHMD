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
	uint8_t sequence_number;
	int16_t device_quat[4];
	int16_t accel[3];
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

void quatf_from_device_quat(const int16_t* smp, quatf* out_quat);

bool osvr_decode_sensor_display_info(pkt_sensor_display_info* info, const unsigned char* buffer, int size);
bool osvr_decode_sensor_config(pkt_sensor_config* config, const unsigned char* buffer, int size);
bool osvr_decode_tracker_sensor_msg(pkt_tracker_sensor* msg, const unsigned char* buffer, int size);

void osvr_dump_packet_tracker_sensor(const pkt_tracker_sensor* sensor);

#endif
