// Copyright 2016, Joey Ferwerda.
// SPDX-License-Identifier: BSL-1.0
/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 */

/* Sony PSVR Driver */


#ifndef PSVR_H
#define PSVR_H

#include <stdint.h>
#include <stdbool.h>

#include "../openhmdi.h"

#define SONY_ID                  0x054c
#define PSVR_HMD                 0x09af

#define DUALSHOCK_4 			 0x05c4

typedef enum
{
	PSVR_BUTTON_VOLUME_PLUS = 2,
	PSVR_BUTTON_VOLUME_MINUS = 4,
	PSVR_BUTTON_MIC_MUTE = 8
} psvr_button;

typedef struct
{
	int16_t accel[3];
	int16_t gyro[3];
	uint32_t tick;
} psvr_sensor_sample;

typedef struct
{
	uint8_t buttons;
	uint8_t state;
	uint16_t volume;
	psvr_sensor_sample samples[2];
	uint16_t button_raw;
	uint16_t proximity;
	uint8_t seq;
} psvr_sensor_packet;

typedef struct
{
	uint8_t id;
	uint8_t buttons;
	uint16_t stick[2];
	uint8_t trigger;
	uint8_t touchpad[2];
	uint8_t battery;
	int16_t accel[3];
	int16_t gyro[3];
	uint32_t timestamp;
} ds4_controller_packet;

static const unsigned char psvr_vrmode_on[8]  = {
	0x23, 0x00, 0xaa, 0x04, 0x01, 0x00, 0x00, 0x00
};

static const unsigned char psvr_tracking_on[12]  = {
	0x11, 0x00, 0xaa, 0x08, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00
};


static const unsigned char psvr_power_on[8]  = {
	0x17, 0x76, 0xaa, 0x04, 0x01, 0x00, 0x00, 0x00
};


void vec3f_from_psvr_vec(const int16_t* smp, vec3f* out_vec);
bool psvr_decode_sensor_packet(psvr_sensor_packet* pkt, const unsigned char* buffer, int size);
bool ds4_controller_decode_packet(ds4_controller_packet* pkt, const unsigned char* buffer, int size);

ohmd_device* open_ds4_controller_device(ohmd_driver* driver, ohmd_device_desc* desc);

#endif
