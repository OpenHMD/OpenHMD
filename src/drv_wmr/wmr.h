// Copyright 2018, Philipp Zabel.
// SPDX-License-Identifier: BSL-1.0
/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 */

/* Windows Mixed Reality Driver */


#ifndef WMR_H
#define WMR_H

#include <stdint.h>
#include <stdbool.h>
#include <libusb.h>

#include "../openhmdi.h"

#define MICROSOFT_VID                   0x045e
#define HOLOLENS_SENSORS_PID            0x0659
#define MOTION_CONTROLLER_PID           0x065b
#define MOTION_CONTROLLER_PID_SAMSUNG   0x065d

typedef enum
{
	HOLOLENS_IRQ_SENSORS = 1,
	HOLOLENS_IRQ_CONTROL = 2,
	HOLOLENS_IRQ_DEBUG = 3,
} hololens_sensors_irq_cmd;

typedef enum
{
	CONTROLLER_IRQ_SENSORS = 1,
} motion_controller_irq_cmd;

typedef struct
{
        uint8_t id;
        uint16_t temperature[4];
        uint64_t gyro_timestamp[4];
        int16_t gyro[3][32];
        uint64_t accel_timestamp[4];
        int32_t accel[3][4];
        uint64_t video_timestamp[4];
} hololens_sensors_packet;

#define MOTION_CONTROLLER_BUTTON_STICK     0x01
#define MOTION_CONTROLLER_BUTTON_WINDOWS   0x02
#define MOTION_CONTROLLER_BUTTON_MENU      0x04
#define MOTION_CONTROLLER_BUTTON_GRIP      0x08
#define MOTION_CONTROLLER_BUTTON_PAD_PRESS 0x10
#define MOTION_CONTROLLER_BUTTON_PAD_TOUCH 0x40

typedef struct
{
	uint8_t id;
	uint8_t buttons;
	uint16_t stick[2];
	uint8_t trigger;
	uint8_t touchpad[2];
	uint8_t battery;
	int32_t accel[3];
	int32_t gyro[3];
	uint32_t timestamp;
} motion_controller_packet;

static const unsigned char hololens_sensors_imu_on[64] = {
	0x02, 0x07
};

static const unsigned char hololens_camera_start[] = {
	0x44, 0x6c, 0x6f, 0x2b, 0x0c, 0x00, 0x00, 0x00, 0x81, 0x00, 0x00, 0x00
};

static const unsigned char hololens_camera_stop[] = {
	0x44, 0x6c, 0x6f, 0x2b, 0x0c, 0x00, 0x00, 0x00, 0x82, 0x00, 0x00, 0x00
};

typedef struct {
        uint32_t json_start;
        uint32_t json_size;
        char manufacturer[0x40];
        char device[0x40];
        char serial[0x40];
        char uid[0x26];
        char unk[0xd5];
        char name[0x40];
        char revision[0x20];
        char revision_date[0x20];
} wmr_config_header;

static const unsigned char motion_controller_imu_on[64] = {
	0x06, 0x03, 0x01, 0x00, 0x02
};

static const unsigned char motion_controller_leds_bright[12] = {
	0x03, 0x01, 0x21, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x2c,
};

typedef struct {
	libusb_context* ctx;
	libusb_device_handle* dev;
	struct libusb_transfer* img_xfer;
	uint8_t img_buffer[616538];
	uint8_t img_frame[1280*481];
} wmr_usb;

bool wmr_usb_init(wmr_usb* usb, int idx);
void wmr_usb_destroy(wmr_usb* usb);
void wmr_usb_update(wmr_usb* usb);

bool hololens_sensors_decode_packet(hololens_sensors_packet* pkt, const unsigned char* buffer, int size);
bool motion_controller_decode_packet(motion_controller_packet* pkt, const unsigned char* buffer, int size);

ohmd_device* open_motion_controller_device(ohmd_driver* driver, ohmd_device_desc* desc);

#endif
