/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* New-Driver template Internal Interface */

#ifndef PIMAX_H
#define PIMAX_H

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
    uint8_t num_samples;
    uint32_t timestamp;
    uint16_t last_command_id;
    int16_t temperature;
    uint32_t tick;
    pkt_tracker_sample samples[3];
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
    short distortion_type;
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

//bool decode_sensor_range(pkt_sensor_range* range, const unsigned char* buffer, int size);
//bool decode_sensor_display_info(pkt_sensor_display_info* info, const unsigned char* buffer, int size);
//bool decode_sensor_config(pkt_sensor_config* config, const unsigned char* buffer, int size);
bool pm_decode_tracker_sensor_msg(pkt_tracker_sensor* msg, const unsigned char* buffer, int size);

void vec3f_from_gryo(const int32_t* smp, vec3f* out_vec, const float* gyro_offset);
void vec3f_from_accel(const int32_t* smp, vec3f* out_vec);
void vec3f_from_mag(const int32_t* smp, vec3f* out_vec);

//int encode_sensor_config(unsigned char* buffer, const pkt_sensor_config* config);
//int encode_keep_alive(unsigned char* buffer, const pkt_keep_alive* keep_alive);

//void dump_packet_sensor_range(const pkt_sensor_range* range);
//void dump_packet_sensor_config(const pkt_sensor_config* config);
//void dump_packet_sensor_display_info(const pkt_sensor_display_info* info);
void pm_dump_packet_tracker_sensor(const pkt_tracker_sensor* sensor);

#endif

