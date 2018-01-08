/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2018 Philipp Zabel.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* Microsoft HoloLens Sensors Driver */

#define FEATURE_BUFFER_SIZE 497

#define TICK_LEN (1.0f / 10000000.0f) // 1000 Hz ticks

#define MICROSOFT_VID        0x045e
#define HOLOLENS_SENSORS_PID 0x0659

#include <string.h>
#include <wchar.h>
#include <hidapi.h>
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdbool.h>

#include "hololens.h"

typedef struct {
	ohmd_device base;

	hid_device* hmd_imu;
	fusion sensor_fusion;
	vec3f raw_accel, raw_gyro;
	uint32_t last_ticks;
	uint8_t last_seq;
	hololens_sensor_packet sensor;

} hololens_priv;

static void vec3f_from_hololens_gyro(const int16_t smp[3][32], int i, vec3f* out_vec)
{
	out_vec->x = (float)(smp[1][8*i+0] +
			     smp[1][8*i+1] +
			     smp[1][8*i+2] +
			     smp[1][8*i+3] +
			     smp[1][8*i+4] +
			     smp[1][8*i+5] +
			     smp[1][8*i+6] +
			     smp[1][8*i+7]) * 0.001f * -0.125f;
	out_vec->y = (float)(smp[0][8*i+0] +
			     smp[0][8*i+1] +
			     smp[0][8*i+2] +
			     smp[0][8*i+3] +
			     smp[0][8*i+4] +
			     smp[0][8*i+5] +
			     smp[0][8*i+6] +
			     smp[0][8*i+7]) * 0.001f * -0.125f;
	out_vec->z = (float)(smp[2][8*i+0] +
			     smp[2][8*i+1] +
			     smp[2][8*i+2] +
			     smp[2][8*i+3] +
			     smp[2][8*i+4] +
			     smp[2][8*i+5] +
			     smp[2][8*i+6] +
			     smp[2][8*i+7]) * 0.001f * -0.125f;
}

static void vec3f_from_hololens_accel(const int32_t smp[3][4], int i, vec3f* out_vec)
{
	out_vec->x = (float)smp[1][i] * 0.001f * -1.0f;
	out_vec->y = (float)smp[0][i] * 0.001f * -1.0f;
	out_vec->z = (float)smp[2][i] * 0.001f * -1.0f;
}

static void handle_tracker_sensor_msg(hololens_priv* priv, unsigned char* buffer, int size)
{
	uint64_t last_sample_tick = priv->sensor.gyro_timestamp[3];

	if(!hololens_decode_sensor_packet(&priv->sensor, buffer, size)){
		LOGE("couldn't decode tracker sensor message");
	}

	hololens_sensor_packet* s = &priv->sensor;


	vec3f mag = {{0.0f, 0.0f, 0.0f}};

	for(int i = 0; i < 4; i++){
		uint64_t tick_delta = 1000;
		if(last_sample_tick > 0) //startup correction
			tick_delta = s->gyro_timestamp[i] - last_sample_tick;

		float dt = tick_delta * TICK_LEN;

		vec3f_from_hololens_gyro(s->gyro, i, &priv->raw_gyro);
		vec3f_from_hololens_accel(s->accel, i, &priv->raw_accel);

		ofusion_update(&priv->sensor_fusion, dt, &priv->raw_gyro, &priv->raw_accel, &mag);

		last_sample_tick = s->gyro_timestamp[i];
	}
}

static void update_device(ohmd_device* device)
{
	hololens_priv* priv = (hololens_priv*)device;

	int size = 0;
	unsigned char buffer[FEATURE_BUFFER_SIZE];

	while(true){
		int size = hid_read(priv->hmd_imu, buffer, FEATURE_BUFFER_SIZE);
		if(size < 0){
			LOGE("error reading from device");
			return;
		} else if(size == 0) {
			return; // No more messages, return.
		}

		// currently the only message type the hardware supports (I think)
		if(buffer[0] == HOLOLENS_IRQ_SENSORS){
			handle_tracker_sensor_msg(priv, buffer, size);
		}else{
			LOGE("unknown message type: %u", buffer[0]);
		}
	}

	if(size < 0){
		LOGE("error reading from device");
	}
}

static int getf(ohmd_device* device, ohmd_float_value type, float* out)
{
	hololens_priv* priv = (hololens_priv*)device;

	switch(type){
	case OHMD_ROTATION_QUAT:
		*(quatf*)out = priv->sensor_fusion.orient;
		break;

	case OHMD_POSITION_VECTOR:
		out[0] = out[1] = out[2] = 0;
		break;

	case OHMD_DISTORTION_K:
		// TODO this should be set to the equivalent of no distortion
		memset(out, 0, sizeof(float) * 6);
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
	hololens_priv* priv = (hololens_priv*)device;

	LOGD("closing Microsoft HoloLens Sensors device");

	hid_close(priv->hmd_imu);

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
		printf("%04x:%04x %s\n", manufacturer, product, cur_dev->path);

		if(findEndPoint(cur_dev->path, device_index) > 0 && iface == iface_cur){
			ret = hid_open_path(cur_dev->path);
			printf("opening\n");
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

static ohmd_device* open_device(ohmd_driver* driver, ohmd_device_desc* desc)
{
	hololens_priv* priv = ohmd_alloc(driver->ctx, sizeof(hololens_priv));

	if(!priv)
		return NULL;

	priv->base.ctx = driver->ctx;

	// Open the HMD device
	priv->hmd_imu = open_device_idx(MICROSOFT_VID, HOLOLENS_SENSORS_PID, 0, 0, 2);

	if(!priv->hmd_imu)
		goto cleanup;

	if(hid_set_nonblocking(priv->hmd_imu, 1) == -1){
		ohmd_set_error(driver->ctx, "failed to set non-blocking on device");
		goto cleanup;
	}

	// turn the IMU on
	hid_write(priv->hmd_imu, hololens_imu_on, sizeof(hololens_imu_on));

	// Set default device properties
	ohmd_set_default_device_properties(&priv->base.properties);

//FIXME FIXME
	// Set device properties TODO: Get from device
	priv->base.properties.hsize = 0.126; //from calculated specs
	priv->base.properties.vsize = 0.071; //from calculated specs
	priv->base.properties.hres = 1920;
	priv->base.properties.vres = 1080;
	priv->base.properties.lens_sep = 0.063500;
	priv->base.properties.lens_vpos = 0.049694;
	priv->base.properties.fov = DEG_TO_RAD(105.0f); //TODO: Measure
	priv->base.properties.ratio = (1920.0f / 1080.0f) / 2.0f;

	// calculate projection eye projection matrices from the device properties
	ohmd_calc_default_proj_matrices(&priv->base.properties);

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

static void get_device_list(ohmd_driver* driver, ohmd_device_list* list)
{
	struct hid_device_info* devs = hid_enumerate(MICROSOFT_VID, HOLOLENS_SENSORS_PID);
	struct hid_device_info* cur_dev = devs;

	int idx = 0;
	while (cur_dev) {
		ohmd_device_desc* desc = &list->devices[list->num_devices++];

		strcpy(desc->driver, "OpenHMD Microsoft HoloLens Sensors Driver");
		strcpy(desc->vendor, "Microsoft");
		strcpy(desc->product, "HoloLens");

		desc->revision = 0;

		snprintf(desc->path, OHMD_STR_SIZE, "%d", idx);

		desc->driver_ptr = driver;

		desc->device_class = OHMD_DEVICE_CLASS_HMD;
		desc->device_flags = OHMD_DEVICE_FLAGS_ROTATIONAL_TRACKING;

		cur_dev = cur_dev->next;
		idx++;
	}

	hid_free_enumeration(devs);
}

static void destroy_driver(ohmd_driver* drv)
{
	LOGD("shutting down Microsoft HoloLens Sensors driver");
	free(drv);
}

ohmd_driver* ohmd_create_hololens_drv(ohmd_context* ctx)
{
	ohmd_driver* drv = ohmd_alloc(ctx, sizeof(ohmd_driver));

	if(!drv)
		return NULL;

	drv->get_device_list = get_device_list;
	drv->open_device = open_device;
	drv->destroy = destroy_driver;
	drv->ctx = ctx;

	return drv;
}
