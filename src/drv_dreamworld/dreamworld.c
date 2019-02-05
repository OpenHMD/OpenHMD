// Copyright 2019, Joey Ferwerda.
// SPDX-License-Identifier: BSL-1.0
/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 */

/* DreamWorld DreamGlass Driver */


#include <string.h>
#include <hidapi.h>
#include "../openhmdi.h"

#include "dreamworld.h"

//STMicroelectronics chip, same as Nolo or DeePoon E2
#define DREAMGLASS_ID	0x0483
#define DREAMGLASS_HMD	0x5750

typedef struct {
	ohmd_device base;
	hid_device* handle;
	int id;

	dwdg_sensor_sample sample;
	fusion sensor_fusion;
	vec3f raw_accel, raw_gyro;
} dwdg_priv;

#define TICK_LEN (1.0f / 1000000.0f) // 1000 Hz ticks

static dwdg_priv* dwdg_priv_get(ohmd_device* device)
{
	return (dwdg_priv*)device;
}

void accel_from_dwdg_vec(const float* smp, vec3f* out_vec)
{
	out_vec->x = (float)smp[0];
	out_vec->y = (float)smp[2];
	out_vec->z = (float)smp[1];
}

void gyro_from_dwdg_vec(const float* smp, vec3f* out_vec)
{
	out_vec->x = (float)smp[0];
	out_vec->y = (float)smp[2];
	out_vec->z = (float)smp[1];
}

static void update_device(ohmd_device* device)
{
	dwdg_priv* priv = dwdg_priv_get(device);
	unsigned char buffer[FEATURE_BUFFER_SIZE];

	// Read all the messages from the device.
	while(true){
		//Setting timeout will result in never breaking from the loop, so needs 0ms
		int size = hid_read_timeout(priv->handle, buffer, FEATURE_BUFFER_SIZE, 0);
		if(size < 0){
			LOGE("error reading from device");
			return;
		} else if(size == 0) {
			return; // No more messages, return.
		}

		//It seems that as a exception to regular usb hid design, we have no packet descriptor
		//In this case just treating all data as IMU data

		uint32_t last_sample_tick = priv->sample.tick;
		uint32_t tick_delta = 1000;
		if(last_sample_tick > 0) //startup correction
			tick_delta = priv->sample.tick - last_sample_tick;

		float dt = tick_delta * TICK_LEN;

		decode_dwdg_imu_msg(&priv->sample, buffer, size);
		accel_from_dwdg_vec(priv->sample.accel, &priv->raw_accel);
		gyro_from_dwdg_vec(priv->sample.gyro, &priv->raw_gyro);
		
		vec3f mag = {{0.0f, 0.0f, 0.0f}};

		ofusion_update(&priv->sensor_fusion, dt, &priv->raw_gyro, &priv->raw_accel, &mag);
	}
}

static int getf(ohmd_device* device, ohmd_float_value type, float* out)
{
	dwdg_priv* priv = (dwdg_priv*)device;

	switch(type){
	case OHMD_ROTATION_QUAT: {
			*(quatf*)out = priv->sensor_fusion.orient;
			break;
		}

	case OHMD_POSITION_VECTOR:
		out[0] = out[1] = out[2] = 0;
		break;

	case OHMD_DISTORTION_K:
		// TODO this should be set to the equivalent of no distortion
		memset(out, 0, sizeof(float) * 6);
		break;

	default:
		ohmd_set_error(priv->base.ctx, "invalid type given to getf (%ud)", type);
		return OHMD_S_INVALID_PARAMETER;
		break;
	}

	return OHMD_S_OK;
}

static void close_device(ohmd_device* device)
{
	LOGD("closing dwdg device");
	free(device);
}

static ohmd_device* open_device(ohmd_driver* driver, ohmd_device_desc* desc)
{
	dwdg_priv* priv = ohmd_alloc(driver->ctx, sizeof(dwdg_priv));
	if(!priv)
		return NULL;
	
	priv->id = desc->id;
	priv->base.ctx = driver->ctx;

	// Open the HID device
	priv->handle = hid_open_path(desc->path);

	if(!priv->handle) {
		goto cleanup;
	}

	// Set default device properties
	ohmd_set_default_device_properties(&priv->base.properties);

	// Set device properties
	priv->base.properties.hsize = 0.1356f;
	priv->base.properties.vsize = 0.0412f;
	priv->base.properties.hres = 1600;
	priv->base.properties.vres = 1280;
	priv->base.properties.lens_sep = 0.073500f;
	priv->base.properties.lens_vpos = 0.016800f;
	priv->base.properties.fov = DEG_TO_RAD(90.0f);
	priv->base.properties.ratio = (1600.0f / 1280.0f) / 2.0f;

	// calculate projection eye projection matrices from the device properties
	ohmd_calc_default_proj_matrices(&priv->base.properties);

	// set up device callbacks
	priv->base.update = update_device;
	priv->base.close = close_device;
	priv->base.getf = getf;

	// initialize sensor fusion
	ofusion_init(&priv->sensor_fusion);

	return (ohmd_device*)priv;

cleanup:
	if(priv)
		free(priv);

	return NULL;
}

static void get_device_list(ohmd_driver* driver, ohmd_device_list* list)
{
	struct hid_device_info* devs = hid_enumerate(DREAMGLASS_ID, DREAMGLASS_HMD);
	struct hid_device_info* cur_dev = devs;

	while (cur_dev) {
		// This is needed because the same chip is used in Nolo and DeePoon E2.
		if (wcscmp(cur_dev->product_string, L"DREAM GLASS HID")==0) {
			ohmd_device_desc* desc = &list->devices[list->num_devices++];

			strcpy(desc->driver, "OpenHMD DreamWorld DreamGlass driver");
			strcpy(desc->vendor, "DreamWorld");
			strcpy(desc->product, "DreamGlass Developer Edition");

			strcpy(desc->path, "(none)");

			desc->driver_ptr = driver;

			desc->device_flags = OHMD_DEVICE_FLAGS_ROTATIONAL_TRACKING;
			desc->device_class = OHMD_DEVICE_CLASS_HMD;
			desc->revision = 0;
			strcpy(desc->path, cur_dev->path);
			desc->driver_ptr = driver;
		}
		cur_dev = cur_dev->next;
	}
	hid_free_enumeration(devs);
}

static void destroy_driver(ohmd_driver* drv)
{
	LOGD("shutting down DreamWorld DreamGlass driver");
	free(drv);
}

ohmd_driver* ohmd_create_dwdg_drv(ohmd_context* ctx)
{
	ohmd_driver* drv = ohmd_alloc(ctx, sizeof(ohmd_driver));
	if(!drv)
		return NULL;

	drv->get_device_list = get_device_list;
	drv->open_device = open_device;
	drv->get_device_list = get_device_list;
	drv->open_device = open_device;
	drv->destroy = destroy_driver;

	return drv;
}
