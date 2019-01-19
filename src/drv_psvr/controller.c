// Copyright 2018, Joey Ferwerda.
// SPDX-License-Identifier: BSL-1.0
/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 */

/* Sony Playstation Dual Shock 4 Driver */

#define FEATURE_BUFFER_SIZE	45

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
	vec3f raw_accel, raw_gyro;
	uint32_t last_ticks;
	uint8_t last_seq;
	ds4_controller_packet sensor;
} controller_priv;

void accel_from_ds4controller_vec(const int16_t* smp, vec3f* out_vec)
{
	out_vec->x = (float)smp[0] / 8192.0f;
	out_vec->y = (float)smp[1] / 8192.0f;
	out_vec->z = (float)smp[2] / 8192.0f;
}

void gyro_from_ds4controller_vec(const int16_t* smp, vec3f* out_vec)
{
	out_vec->x = (float)smp[0] * 2.0f * 0.0105f;
	out_vec->y = (float)smp[1] * 2.0f * 0.0105f;
	out_vec->z = (float)smp[2] * 2.0f * 0.0105f;
}

static void handle_controller_sensor_msg(controller_priv* priv, unsigned char* buffer, int size)
{
	uint32_t last_sample_tick = priv->sensor.timestamp;

	if(!ds4_controller_decode_packet(&priv->sensor, buffer, size)){
		LOGE("couldn't decode tracker sensor message");
	}

	ds4_controller_packet* s = &priv->sensor;


	vec3f mag = {{0.0f, 0.0f, 0.0f}};

	uint64_t tick_delta = 1000;
	if(last_sample_tick > 0) //startup correction
		tick_delta = s->timestamp - last_sample_tick;

	float dt = tick_delta * TICK_LEN;

	//printf("Accel: x %f, y %f, z %f\n", (float)s->accel[0],(float)s->accel[1], (float)s->accel[2]);
	//printf("Gyro: x %f, y %f, z %f\n", (float)s->gyro[0], (float)s->gyro[1], (float)s->gyro[2]);
	gyro_from_ds4controller_vec(s->gyro, &priv->raw_gyro);
	accel_from_ds4controller_vec(s->accel, &priv->raw_accel);

	ofusion_update(&priv->sensor_fusion, dt, &priv->raw_gyro, &priv->raw_accel, &mag);

	last_sample_tick = s->timestamp;
}

static void update_device(ohmd_device* device)
{
	controller_priv* priv = (controller_priv*)device;

	int size = 0;
	unsigned char buffer[FEATURE_BUFFER_SIZE];

	while(true){
		int size = hid_read(priv->controller_imu, buffer, FEATURE_BUFFER_SIZE);
		if(size < 0){
			LOGE("error reading from device");
			return;
		} else if(size == 0) {
			return; // No more messages, return.
		}

		// currently the only message type the hardware supports (I think)
		if(buffer[0] == 17) { //HID packet as reported by hidraw on Linux 
			handle_controller_sensor_msg(priv, buffer, size);
		}else {
			LOGE("unknown message type: %u", buffer[0]);
		}
	}

	if(size < 0){
		LOGE("error reading from device");
	}
}

static int getf(ohmd_device* device, ohmd_float_value type, float* out)
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

	case OHMD_DISTORTION_K:
		// TODO this should be set to the equivalent of no distortion
		memset(out, 0, sizeof(float) * 6);
		break;

	case OHMD_CONTROLS_STATE:
	/*
		out[0] = !!(priv->sensor.buttons & MOTION_CONTROLLER_BUTTON_STICK);
		out[1] = !!(priv->sensor.buttons & MOTION_CONTROLLER_BUTTON_WINDOWS);
		out[2] = !!(priv->sensor.buttons & MOTION_CONTROLLER_BUTTON_MENU);
		out[3] = !!(priv->sensor.buttons & MOTION_CONTROLLER_BUTTON_GRIP);
		out[4] = !!(priv->sensor.buttons & MOTION_CONTROLLER_BUTTON_PAD_PRESS);
		out[5] = !!(priv->sensor.buttons & MOTION_CONTROLLER_BUTTON_PAD_TOUCH);
		out[6] = priv->sensor.stick[0] * (2.0 / 4095.0) - 1.0;
		out[7] = priv->sensor.stick[1] * (-2.0 / 4095.0) + 1.0;
		out[8] = (float)priv->sensor.trigger / 255.0;
		if (priv->sensor.touchpad[0] == 255)
			out[9] = 0.0;
		else
			out[9] = priv->sensor.touchpad[0] * (2.0 / 100.0) - 1.0;
		if (priv->sensor.touchpad[1] == 255)
			out[10] = 0.0;
		else
			out[10] = priv->sensor.touchpad[1] * (-2.0 / 100.0) + 1.0;
		*/break;

	default:
		ohmd_set_error(priv->base.ctx, "invalid type given to controller getf (%ud)", type);
		return -1;
		break;
	}

	return 0;
}

static void close_device(ohmd_device* device)
{
	controller_priv* priv = (controller_priv*)device;

	LOGD("closing Playstation Dual Shock 4 Controller device");

	hid_close(priv->controller_imu);

	free(device);
}

static hid_device* open_device_idx(int manufacturer, int product, int iface, int iface_tot, int device_index)
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

ohmd_device* open_ds4_controller_device(ohmd_driver* driver, ohmd_device_desc* desc)
{
	controller_priv* priv = ohmd_alloc(driver->ctx, sizeof(controller_priv));

	if(!priv)
		return NULL;

	priv->base.ctx = driver->ctx;
	priv->device_flags = desc->device_flags;

	int idx = atoi(desc->path);

	// Open the controller device
	priv->controller_imu = open_device_idx(SONY_ID, DUALSHOCK_4, 0, 1, idx);

	// TODO read_config

	if(hid_set_nonblocking(priv->controller_imu, 1) == -1){
		ohmd_set_error(driver->ctx, "failed to set non-blocking on device");
		goto cleanup;
	}

	/*
	// turn the IMU on
	hid_write(priv->controller_imu, motion_controller_imu_on, sizeof(motion_controller_imu_on));
	*/
	// Set default device properties
	ohmd_set_default_device_properties(&priv->base.properties);


	// Set device properties
	priv->base.properties.control_count = 22;
	priv->base.properties.controls_hints[0] = OHMD_ANALOG_X; // Left Stick
	priv->base.properties.controls_hints[1] = OHMD_ANALOG_Y; // Left Stick
	priv->base.properties.controls_hints[2] = OHMD_ANALOG_X; // Right Stick
	priv->base.properties.controls_hints[3] = OHMD_ANALOG_Y; // Right Stick
	priv->base.properties.controls_hints[4] = OHMD_BUTTON_Y; // Triangle
	priv->base.properties.controls_hints[5] = OHMD_BUTTON_B; // Circle
	priv->base.properties.controls_hints[6] = OHMD_BUTTON_A; // X
	priv->base.properties.controls_hints[7] = OHMD_BUTTON_X; // Square

	priv->base.properties.controls_hints[8] = OHMD_ANALOG_X; // DPAD X
	priv->base.properties.controls_hints[9] = OHMD_ANALOG_Y; // DPAD Y

	priv->base.properties.controls_hints[10] = OHMD_ANALOG_PRESS; // R3
	priv->base.properties.controls_hints[11] = OHMD_ANALOG_PRESS; // L3
	priv->base.properties.controls_hints[12] = OHMD_MENU; // Options
	priv->base.properties.controls_hints[13] = OHMD_HOME; // Share
	priv->base.properties.controls_hints[14] = OHMD_TRIGGER_CLICK; // R2
	priv->base.properties.controls_hints[15] = OHMD_TRIGGER_CLICK; // L2
	priv->base.properties.controls_hints[16] = OHMD_TRIGGER_CLICK; // R1
	priv->base.properties.controls_hints[17] = OHMD_TRIGGER_CLICK; // L1

	priv->base.properties.controls_hints[18] = OHMD_TRIGGER_CLICK; // Trackpad Click
	priv->base.properties.controls_hints[19] = OHMD_TRIGGER_CLICK; // PS Button

	priv->base.properties.controls_hints[20] = OHMD_TRIGGER; //L2
	priv->base.properties.controls_hints[21] = OHMD_TRIGGER; //R2

	priv->base.properties.controls_types[0] = OHMD_ANALOG;
	priv->base.properties.controls_types[1] = OHMD_ANALOG;
	priv->base.properties.controls_types[2] = OHMD_ANALOG;
	priv->base.properties.controls_types[3] = OHMD_ANALOG;
	priv->base.properties.controls_types[4] = OHMD_DIGITAL;
	priv->base.properties.controls_types[5] = OHMD_DIGITAL;
	priv->base.properties.controls_types[6] = OHMD_DIGITAL;
	priv->base.properties.controls_types[7] = OHMD_DIGITAL;
	priv->base.properties.controls_types[8] = OHMD_ANALOG;
	priv->base.properties.controls_types[9] = OHMD_ANALOG;
	priv->base.properties.controls_types[10] = OHMD_DIGITAL;
	priv->base.properties.controls_types[11] = OHMD_DIGITAL;
	priv->base.properties.controls_types[12] = OHMD_DIGITAL;
	priv->base.properties.controls_types[13] = OHMD_DIGITAL;
	priv->base.properties.controls_types[14] = OHMD_DIGITAL;
	priv->base.properties.controls_types[15] = OHMD_DIGITAL;
	priv->base.properties.controls_types[16] = OHMD_DIGITAL;
	priv->base.properties.controls_types[17] = OHMD_DIGITAL;
	priv->base.properties.controls_types[18] = OHMD_DIGITAL;
	priv->base.properties.controls_types[19] = OHMD_DIGITAL;
	priv->base.properties.controls_types[20] = OHMD_ANALOG;
	priv->base.properties.controls_types[21] = OHMD_ANALOG;

	// set up device callbacks
	priv->base.update = update_device;
	priv->base.close = close_device;
	priv->base.getf = getf;

	ofusion_init(&priv->sensor_fusion);

	return (ohmd_device*)priv;

cleanup:
	if(priv)
		free(priv);

	return NULL;
}
