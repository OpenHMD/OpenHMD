/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* OSVR HDK - HID/USB Driver Implementation */

#include <stdlib.h>
#include <hidapi.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>

#include "osvr.h"

#define TICK_LEN (1.0f / 1000000.0f) // 1000 Hz ticks
#define KEEP_ALIVE_VALUE (10 * 1000)
#define SETFLAG(_s, _flag, _val) (_s) = ((_s) & ~(_flag)) | ((_val) ? (_flag) : 0)

#define OSVR_HDK2_ID					0x1532
#define OSVR_HDK2_HMD					0x0b00

typedef struct {
	ohmd_device base;

	hid_device* handle;
	pkt_sensor_display_info display_info;
	pkt_sensor_config sensor_config;
	pkt_tracker_sensor sensor;
	double last_keep_alive;
	fusion sensor_fusion;
	vec3f raw_accel;
	quatf processed_quat;
} drv_priv;

static drv_priv* drv_priv_get(ohmd_device* device)
{
	return (drv_priv*)device;
}

static int get_feature_report(drv_priv* priv, drv_sensor_feature_cmd cmd, unsigned char* buf)
{
	memset(buf, 0, FEATURE_BUFFER_SIZE);
	buf[0] = (unsigned char)cmd;
	return hid_get_feature_report(priv->handle, buf, FEATURE_BUFFER_SIZE);
}

static int send_feature_report(drv_priv* priv, const unsigned char *data, size_t length)
{
	return hid_send_feature_report(priv->handle, data, length);
}

void quatf_from_device_quat(const int16_t* smp, quatf* out_quat)
{
	out_quat->x = ((float)smp[0] / (1 << 14)) * -1;
	out_quat->y = ((float)smp[2] / (1 << 14)) * -1;
	out_quat->z = ((float)smp[1] / (1 << 14)) * 1;
	out_quat->w = ((float)smp[3] / (1 << 14)) * -1;

	quatf abs_rotate_offset = { sqrt(0.5), 0, 0 ,sqrt(0.5) };
	oquatf_mult_me(out_quat, &abs_rotate_offset);
}

void vec3f_from_device_accel(const int16_t* accel, vec3f* out_vec)
{
	out_vec->x = (float)accel[0] / (1 << 9);
	out_vec->y = (float)accel[1] / (1 << 9);
	out_vec->z = (float)accel[2] / (1 << 9);
}

static void handle_tracker_sensor_msg(drv_priv* priv, unsigned char* buffer, int size)
{
	if(!osvr_decode_tracker_sensor_msg(&priv->sensor, buffer, size)){
		LOGE("couldn't decode tracker sensor message");
	}

	pkt_tracker_sensor* s = &priv->sensor;

	osvr_dump_packet_tracker_sensor(s);
	quatf_from_device_quat(s->device_quat, &priv->processed_quat);
	vec3f_from_device_accel(s->accel, &priv->raw_accel);
	priv->sensor_fusion.orient = priv->processed_quat;
}

static void update_device(ohmd_device* device)
{
	drv_priv* priv = drv_priv_get(device);
	unsigned char buffer[FEATURE_BUFFER_SIZE];


	// Read all the messages from the device.
	while(true){
		int size = hid_read(priv->handle, buffer, FEATURE_BUFFER_SIZE);
		if(size < 0){
			LOGE("error reading from device");
			return;
		} else if(size == 0) {
			return; // No more messages, return.
		}

		// currently the only message type the hardware supports (I think)
		if(buffer[0] == 3 || buffer[0] == 19) {
			handle_tracker_sensor_msg(priv, buffer, size);
		}else{
			LOGE("unknown message type: %u", buffer[0]);
		}
	}
}

static int getf(ohmd_device* device, ohmd_float_value type, float* out)
{
	drv_priv* priv = drv_priv_get(device);

	switch(type){
	case OHMD_DISTORTION_K: {
			for (int i = 0; i < 6; i++) {
				out[i] = priv->display_info.distortion_k[i];
			}
			break;
		}

	case OHMD_ROTATION_QUAT: {
			*(quatf*)out = priv->sensor_fusion.orient;
			break;
		}

	case OHMD_POSITION_VECTOR:
		out[0] = out[1] = out[2] = 0;
		break;

	default:
		ohmd_set_error(priv->base.ctx, "invalid type given to getf (%ud)", type);
		return -1;
		break;
	}

	return 0;
}

static void close_device(ohmd_device* device)
{
	LOGD("closing device");
	drv_priv* priv = drv_priv_get(device);
	hid_close(priv->handle);
	free(priv);
}

static char* _hid_to_unix_path(char* path)
{
	const int len = 4;
	char bus [len];
	char dev [len];
	char *result = malloc( sizeof(char) * ( 20 + 1 ) );

	sprintf (bus, "%.*s\n", len, path);
	sprintf (dev, "%.*s\n", len, path + 5);

	sprintf (result, "/dev/bus/usb/%03d/%03d",
		(int)strtol(bus, NULL, 16),
		(int)strtol(dev, NULL, 16));
	return result;
}

static ohmd_device* open_device(ohmd_driver* driver, ohmd_device_desc* desc)
{
	drv_priv* priv = ohmd_alloc(driver->ctx, sizeof(drv_priv));
	if(!priv)
		goto cleanup;

	priv->base.ctx = driver->ctx;

	// Open the HID device
	priv->handle = hid_open_path(desc->path);

	if(!priv->handle) {
		char* path = _hid_to_unix_path(desc->path);
		ohmd_set_error(driver->ctx, "Could not open %s. "
		                            "Check your rights.", path);
		free(path);
		goto cleanup;
	}

	if(hid_set_nonblocking(priv->handle, 1) == -1){
		ohmd_set_error(driver->ctx, "failed to set non-blocking on device");
		goto cleanup;
	}

	unsigned char buf[FEATURE_BUFFER_SIZE];

	int size;

	// Update the time of the last keep alive we have sent.
	priv->last_keep_alive = ohmd_get_tick();

	// Set default device properties
	ohmd_set_default_device_properties(&priv->base.properties);

	// Set device properties
	//NOTE: display's each is 3.81" diagonal
	//67.60 × 78.95 mm outline dimension, and 64.8 × 72.0 mm active area.
	priv->base.properties.hsize = 0.1296f; //2 times sceens + ipd as estimation
	priv->base.properties.vsize = 0.0720f;
	priv->base.properties.hres = 2160;
	priv->base.properties.vres = 1200;
	priv->base.properties.lens_sep = 0.0725f;
	priv->base.properties.lens_vpos = 0.03125;
	priv->base.properties.fov = DEG_TO_RAD(110.0); // TODO calculate.
	priv->base.properties.ratio = ((float)2160 / (float)1200) / 2.0f;

	// calculate projection eye projection matrices from the device properties
	ohmd_calc_default_proj_matrices(&priv->base.properties);

	// set up device callbacks
	priv->base.update = update_device;
	priv->base.close = close_device;
	priv->base.getf = getf;

	// initialize sensor fusion
	ofusion_init(&priv->sensor_fusion);

	return &priv->base;

cleanup:
	if(priv)
		free(priv);

	return NULL;
}

static void get_device_list(ohmd_driver* driver, ohmd_device_list* list)
{
	struct hid_device_info* devs = hid_enumerate(OSVR_HDK2_ID, OSVR_HDK2_HMD);
	struct hid_device_info* cur_dev = devs;

	while (cur_dev) {
		ohmd_device_desc* desc = &list->devices[list->num_devices++];

		strcpy(desc->driver, "OSVR HDK2");
		strcpy(desc->vendor, "Razer / OSVR");
		strcpy(desc->product, "HDK2");

		desc->revision = 0;

		strcpy(desc->path, cur_dev->path);

		desc->driver_ptr = driver;

		cur_dev = cur_dev->next;
	}

	hid_free_enumeration(devs);
}

static void destroy_driver(ohmd_driver* drv)
{
	LOGD("shutting down driver");
	hid_exit();
	free(drv);
}

ohmd_driver* ohmd_create_osvr_hdk_drv(ohmd_context* ctx)
{
	ohmd_driver* drv = ohmd_alloc(ctx, sizeof(ohmd_driver));
	if(drv == NULL)
		return NULL;

	drv->get_device_list = get_device_list;
	drv->open_device = open_device;
	drv->ctx = ctx;
	drv->get_device_list = get_device_list;
	drv->open_device = open_device;
	drv->destroy = destroy_driver;

	return drv;
}
