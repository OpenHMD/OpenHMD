// Copyright 2018, Philipp Zabel.
// SPDX-License-Identifier: BSL-1.0
/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 */

/* Sony PlayStation Move controller driver */

#define FEATURE_BUFFER_SIZE	49

#define TICK_LEN		1e-7 // 0.1 µs ticks

#include <string.h>
#include <wchar.h>
#include <hidapi.h>
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdbool.h>

#include "psvr.h"

typedef struct {
	ohmd_device base;
	ohmd_device_flags device_flags;

	hid_device* controller_imu;
	fusion sensor_fusion;
	vec3f raw_accel;
	vec3f raw_gyro;
	double last_set_led;
	psmove_packet sensor;
} controller_priv;

static void psmove_accel_from_vec(const int16_t* smp, vec3f* out_vec)
{
	out_vec->x = (float)smp[0] * 9.80665f * 8.0f / 32767.0f;
	out_vec->y = (float)smp[1] * 9.80665f * 8.0f / 32767.0f;
	out_vec->z = (float)smp[2] * 9.80665f * 8.0f / 32767.0f;
}

static void psmove_gyro_from_vec(const int16_t* smp, vec3f* out_vec)
{
	out_vec->x = (float)smp[0] * 14e-4;
	out_vec->y = (float)smp[1] * 14e-4;
	out_vec->z = (float)smp[2] * 14e-4;
}

static void psmove_handle_sensor_msg(controller_priv* priv,
				     unsigned char* buffer, int size)
{
	uint32_t last_sample_tick = priv->sensor.timestamp;

	if(!psmove_decode_packet(&priv->sensor, buffer, size)){
		LOGE("couldn't decode tracker sensor message");
	}

	psmove_packet* s = &priv->sensor;

	vec3f mag = {{0.0f, 0.0f, 0.0f}};

	uint64_t tick_delta = 1000;
	if(last_sample_tick > 0) //startup correction
		tick_delta = s->timestamp - last_sample_tick;

	float dt = tick_delta * TICK_LEN;

	psmove_gyro_from_vec(s->gyro[0], &priv->raw_gyro);
	psmove_accel_from_vec(s->accel[0], &priv->raw_accel);

	ofusion_update(&priv->sensor_fusion, dt, &priv->raw_gyro,
		       &priv->raw_accel, &mag);

	last_sample_tick = s->timestamp;
}

static void psmove_set_led(hid_device* dev, uint8_t red, uint8_t green,
			   uint8_t blue)
{
	uint8_t set_led[9] = {
		0x06, 0x00, red, green, blue, 0x00, 0x00, 0x00, 0x00,
	};

	hid_write(dev, set_led, sizeof(set_led));
}

static void psmove_update_device(ohmd_device* device)
{
	controller_priv* priv = (controller_priv*)device;
	unsigned char buffer[FEATURE_BUFFER_SIZE];
	int size = 0;

	// Keep tracking sphere lit up
	double t = ohmd_get_tick();
	if(t - priv->last_set_led >= 5.0){
		if (priv->device_flags & OHMD_DEVICE_FLAGS_LEFT_CONTROLLER)
			psmove_set_led(priv->controller_imu, 0xff, 0x00, 0x00);
		else
			psmove_set_led(priv->controller_imu, 0xff, 0x00, 0xff);

		priv->last_set_led = t;
	}

	while(true){
		int size = hid_read(priv->controller_imu, buffer,
				    FEATURE_BUFFER_SIZE);
		if(size < 0){
			LOGE("error reading from device");
			return;
		} else if(size == 0) {
			return; // No more messages, return.
		}

		if(buffer[0] == 1) {
			psmove_handle_sensor_msg(priv, buffer, size);
		}else {
			LOGE("unknown message type: %u", buffer[0]);
		}
	}

	if(size < 0){
		LOGE("error reading from device");
	}
}

static int psmove_getf(ohmd_device* device, ohmd_float_value type, float* out)
{
	controller_priv* priv = (controller_priv*)device;

	switch(type){
	case OHMD_ROTATION_QUAT:
		*(quatf*)out = priv->sensor_fusion.orient;
		break;

	case OHMD_POSITION_VECTOR:
		if (priv->device_flags & OHMD_DEVICE_FLAGS_LEFT_CONTROLLER)
			out[0] = -0.5f;
		else
			out[0] = 0.5f;
		out[1] = out[2] = 0;
		break;

#if 0
	case OHMD_DISTORTION_K:
		// TODO this should be set to the equivalent of no distortion
		memset(out, 0, sizeof(float) * 6);
		break;
#endif

	case OHMD_CONTROLS_STATE:
		out[0] = !!(priv->sensor.buttons & PSMOVE_BUTTON_SELECT);
		out[1] = !!(priv->sensor.buttons & PSMOVE_BUTTON_START);
		out[2] = !!(priv->sensor.buttons & PSMOVE_BUTTON_TRIANGLE);
		out[3] = !!(priv->sensor.buttons & PSMOVE_BUTTON_CIRCLE);
		out[4] = !!(priv->sensor.buttons & PSMOVE_BUTTON_CROSS);
		out[5] = !!(priv->sensor.buttons & PSMOVE_BUTTON_SQUARE);
		out[6] = !!(priv->sensor.buttons & PSMOVE_BUTTON_PLAYSTATION);
		out[7] = !!(priv->sensor.buttons & PSMOVE_BUTTON_MOVE);
		out[8] = priv->sensor.trigger[0] / 255.0;
		break;

	default:
		ohmd_set_error(priv->base.ctx, "invalid type given to controller getf (%ud)", type);
		return -1;
	}

	return 0;
}

static void psmove_close_device(ohmd_device* device)
{
	controller_priv* priv = (controller_priv*)device;

	LOGD("closing Playstation Dual Shock 4 Controller device");

	hid_close(priv->controller_imu);

	free(device);
}

static hid_device* open_device_idx(int manufacturer, int product, int iface,
				   int iface_tot, int device_index)
{
	struct hid_device_info* devs = hid_enumerate(manufacturer, product);
	struct hid_device_info* cur_dev = devs;

	int idx = 0;
	int iface_cur = 0;
	hid_device* ret = NULL;

	while (cur_dev) {
		LOGI("%04x:%04x %s\n", manufacturer, product, cur_dev->path);

		if(idx == device_index && iface == iface_cur){
			ret = hid_open_path(cur_dev->path);
			LOGI("opening\n");
		}

		cur_dev = cur_dev->next;

		iface_cur++;

		if(iface_cur >= iface_tot){
			idx++;
			iface_cur = 0;
		}
	}

	hid_free_enumeration(devs);

	return ret;
}

ohmd_device* open_psmove_device(ohmd_driver* driver, ohmd_device_desc* desc)
{
	controller_priv* priv = ohmd_alloc(driver->ctx, sizeof(controller_priv));

	if(!priv)
		return NULL;

	priv->base.ctx = driver->ctx;
	priv->device_flags = desc->device_flags;

	int idx = atoi(desc->path);

	// Open the controller device
	priv->controller_imu = open_device_idx(SONY_ID, PSMOVE_ZCM2, 0, 1, idx);
	if(!priv->controller_imu) {
		priv->controller_imu = open_device_idx(SONY_ID, PSMOVE_ZCM1, 0, 1, idx);
		if(!priv->controller_imu)
			goto cleanup;
	}

	// TODO read stored calibration data

	if(hid_set_nonblocking(priv->controller_imu, 1) == -1){
		ohmd_set_error(driver->ctx, "failed to set non-blocking on device");
		goto cleanup;
	}

	// Light up the tracking sphere
	if (priv->device_flags & OHMD_DEVICE_FLAGS_LEFT_CONTROLLER)
		psmove_set_led(priv->controller_imu, 0xff, 0x00, 0x00);
	else
		psmove_set_led(priv->controller_imu, 0xff, 0x00, 0xff);
	priv->last_set_led = ohmd_get_tick();

	// Set default device properties
	ohmd_set_default_device_properties(&priv->base.properties);

	// Set device properties
	priv->base.properties.control_count = 9;
	priv->base.properties.controls_hints[0] = OHMD_GENERIC; // Select
	priv->base.properties.controls_hints[1] = OHMD_GENERIC; // Start
	priv->base.properties.controls_hints[2] = OHMD_BUTTON_Y; // Triangle
	priv->base.properties.controls_hints[3] = OHMD_BUTTON_B; // Circle
	priv->base.properties.controls_hints[4] = OHMD_BUTTON_A; // Cross
	priv->base.properties.controls_hints[5] = OHMD_BUTTON_X; // Square
	priv->base.properties.controls_hints[6] = OHMD_GENERIC; // PlayStation
	priv->base.properties.controls_hints[7] = OHMD_GENERIC; // Move
	priv->base.properties.controls_hints[8] = OHMD_TRIGGER; // Trigger

	priv->base.properties.controls_types[0] = OHMD_DIGITAL;
	priv->base.properties.controls_types[1] = OHMD_DIGITAL;
	priv->base.properties.controls_types[2] = OHMD_DIGITAL;
	priv->base.properties.controls_types[3] = OHMD_DIGITAL;
	priv->base.properties.controls_types[4] = OHMD_DIGITAL;
	priv->base.properties.controls_types[5] = OHMD_DIGITAL;
	priv->base.properties.controls_types[6] = OHMD_DIGITAL;
	priv->base.properties.controls_types[7] = OHMD_DIGITAL;
	priv->base.properties.controls_types[8] = OHMD_ANALOG;

	// set up device callbacks
	priv->base.update = psmove_update_device;
	priv->base.close = psmove_close_device;
	priv->base.getf = psmove_getf;

	ofusion_init(&priv->sensor_fusion);

	return (ohmd_device*)priv;

cleanup:
	if(priv)
		free(priv);

	return NULL;
}
