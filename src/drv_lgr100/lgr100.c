// Copyright 2018, Joey Ferwerda.
// SPDX-License-Identifier: BSL-1.0
/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 */

/* LG R100 (360VR) Driver */

#include <string.h>
#include <hidapi.h>
#include "../openhmdi.h"

#include "lgr100.h"

typedef struct {
	ohmd_device base;
	hid_device* handle;
	int id;

	int controller_values[2];
	lgr100_sensor_sample sample;
	fusion sensor_fusion;
	vec3f raw_accel, raw_gyro;
} lgr100_priv;

#define TICK_LEN (1.0f / 200000.0f) // 200 Hz ticks

static lgr100_priv* lgr100_priv_get(ohmd_device* device)
{
	return (lgr100_priv*)device;
}

void accel_from_lgr100_vec(const float* smp, vec3f* out_vec)
{
	out_vec->x = (float)smp[0];
	out_vec->y = (float)smp[1];
	out_vec->z = (float)smp[2];
	//printf("accel = %f, %f, %f\n", out_vec->x, out_vec->y, out_vec->z);
}

void gyro_from_lgr100_vec(const float* smp, vec3f* out_vec)
{
	out_vec->x = (float)smp[0] * 4.0f;
	out_vec->y = (float)smp[1] * 4.0f;
	out_vec->z = (float)smp[2] * 4.0f;
	//printf("gyro = %f, %f, %f\n", out_vec->x, out_vec->y, out_vec->z);
}

static void handle_tracker_sensor_msg(lgr100_priv* priv, unsigned char* buffer, int size)
{
	uint32_t last_sample_tick = priv->sample.tick;
	uint32_t tick_delta = 200;
	if(last_sample_tick > 0) //startup correction
		tick_delta = priv->sample.tick - last_sample_tick;

	float dt = tick_delta * TICK_LEN;

	decode_lgr100_imu_msg(&priv->sample, buffer, size);
	accel_from_lgr100_vec(priv->sample.accel, &priv->raw_accel);
	gyro_from_lgr100_vec(priv->sample.gyro, &priv->raw_gyro);
	
	vec3f mag = {{0.0f, 0.0f, 0.0f}};
	
	//printf("x %f, y %f, z %f\n", (float)priv->sample.accel[0], (float)priv->sample.accel[1], (float)priv->sample.accel[2]);
	//printf("x %f, y %f, z %f\n", (float)priv->sample.gyro[0], (float)priv->sample.gyro[1], (float)priv->sample.gyro[2]);
	//printf("x %f, y %f, z %f\n", priv->raw_accel.x, priv->raw_accel.y, priv->raw_accel.z);

	ofusion_update(&priv->sensor_fusion, dt, &priv->raw_gyro, &priv->raw_accel, &mag);
}

static void update_device(ohmd_device* device)
{
	lgr100_priv* priv = lgr100_priv_get(device);
	unsigned char buffer[FEATURE_BUFFER_SIZE];

	// Read all the messages from the device.
	while(true){
		int size = hid_read_timeout(priv->handle, buffer, FEATURE_BUFFER_SIZE, 20);
		if(size < 0){
			LOGE("error reading from device");
			return;
		} else if(size == 0) {
			return; // No more messages, return.
		}

		//NULL package
		if(buffer[0] == LGR100_IRQ_NULL) {
			return;
		}
		else if(buffer[0] == LGR100_IRQ_BUTTONS){
			//button 'OK' is buffer[1] state 01 and 04
			//button '<-' is buffer[1] state 02 and 03
			if (buffer[1] == LGR100_BUTTON_OK_ON)
				priv->controller_values[0] = 1;
			else if (buffer[1] == LGR100_BUTTON_BACK_ON)
				priv->controller_values[1] = 1;
			else if (buffer[1] == LGR100_BUTTON_BACK_OFF)
				priv->controller_values[1] = 0;
			else if (buffer[1] == LGR100_BUTTON_OK_OFF)
				priv->controller_values[0] = 0;
		}
		//Print all the verbose debug information
		else if(buffer[0] == LGR100_IRQ_DEBUG1 ||
				buffer[0] == LGR100_IRQ_DEBUG2 ||
				buffer[0] == LGR100_IRQ_DEBUG_SEQ1 ||
				buffer[0] == LGR100_IRQ_DEBUG_SEQ2)
		{
			*buffer += 1;
			printf("%s", buffer);
		}
		else if(buffer[0] == LGR100_IRQ_SENSORS) {
			handle_tracker_sensor_msg(priv, buffer, size);
		}
		else if (buffer[0] == LGR100_IRQ_UNKNOWN1) {

		}
		else if (buffer[0] == LGR100_IRQ_UNKNOWN2) {
			/*
			for (int i = 0; i < size; i++)
			{
				if (i+1 < size)
					printf("%02X:", buffer[i]);
				else
					printf("%02X", buffer[i]);
			}
			printf("\n");
			printf("%s", buffer);
			*/
		}
		else if (buffer[0] == LGR100_IRQ_UNKNOWN3) {

		}else{
			LOGE("unknown message type: %u", buffer[0]);
		}
	}
}

static int getf(ohmd_device* device, ohmd_float_value type, float* out)
{
	lgr100_priv* priv = (lgr100_priv*)device;

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
	
	case OHMD_CONTROLS_STATE:
		for (int i = 0; i < 2; i++){
			out[i] = priv->controller_values[i];
		}
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
	LOGD("closing lgr100 device");
	free(device);
}

static ohmd_device* open_device(ohmd_driver* driver, ohmd_device_desc* desc)
{
	lgr100_priv* priv = ohmd_alloc(driver->ctx, sizeof(lgr100_priv));
	if(!priv)
		return NULL;
	
	priv->id = desc->id;
	priv->base.ctx = driver->ctx;

	// Open the HID device
	priv->handle = hid_open_path(desc->path);

	if(!priv->handle) {
		goto cleanup;
	}

	//Start the headset with the 'vr start app' command
	hid_write(priv->handle, start_device, sizeof(start_device));

	hid_write(priv->handle, start_accel, sizeof(start_accel));
	hid_write(priv->handle, start_gyro, sizeof(start_gyro));

	// Set default device properties
	ohmd_set_default_device_properties(&priv->base.properties);

	// Set device properties (currently just aproximations)
	// If you have one, please open it and measure!
	priv->base.properties.hsize = 0.110000f;
	priv->base.properties.vsize = 0.038000f;
	priv->base.properties.hres = 1440;
	priv->base.properties.vres = 960;
	priv->base.properties.lens_sep = 0.063500f;
	priv->base.properties.lens_vpos = 0.020000f;
	priv->base.properties.fov = DEG_TO_RAD(80.0f); //based on website information, probably not perfect
	priv->base.properties.ratio = (1920.0f / 720.0f) / 2.0f;
	
	// Some buttons and axes
	priv->base.properties.control_count = 2;
	priv->base.properties.controls_hints[0] = OHMD_BUTTON_A;
	priv->base.properties.controls_hints[1] = OHMD_BUTTON_B;
	priv->base.properties.controls_types[0] = OHMD_DIGITAL;
	priv->base.properties.controls_types[1] = OHMD_DIGITAL;

	// calculate projection eye projection matrices from the device properties
	ohmd_calc_default_proj_matrices(&priv->base.properties);

	// Set up rotation matrix
	mat4x4f rotation;
	omat4x4f_init_ident(&rotation);

	rotation.m[0][0] = 0.0f;
	rotation.m[0][1] = -1.0f;
	rotation.m[1][0] = 1.0f;
	rotation.m[1][1] = 0.0f;

	omat4x4f_mult(&priv->base.properties.proj_right, &rotation, &priv->base.properties.proj_right);

	rotation.m[0][0] = 0.0f;
	rotation.m[0][1] = 1.0f;
	rotation.m[1][0] = -1.0f;
	rotation.m[1][1] = 0.0f;

	omat4x4f_mult(&priv->base.properties.proj_left, &rotation, &priv->base.properties.proj_left);

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
	int id = 0;
	struct hid_device_info* cur_dev = hid_enumerate(0x1004, 0x6374);
	if(cur_dev == NULL)
		return;
	ohmd_device_desc* desc;

	// HMD
	desc = &list->devices[list->num_devices++];

	strcpy(desc->driver, "OpenHMD LG R100 (360VR) Driver");
	strcpy(desc->vendor, "LG");
	strcpy(desc->product, "LGR100");

	strcpy(desc->path, cur_dev->path);

	desc->driver_ptr = driver;

	desc->device_flags = OHMD_DEVICE_FLAGS_ROTATIONAL_TRACKING;
	desc->device_class = OHMD_DEVICE_CLASS_HMD | OHMD_DEVICE_CLASS_CONTROLLER;

	desc->id = id++;
	hid_free_enumeration(cur_dev);
}

static void destroy_driver(ohmd_driver* drv)
{
	LOGD("shutting down lgr100 driver");
	free(drv);
}

ohmd_driver* ohmd_create_lgr100_drv(ohmd_context* ctx)
{
	ohmd_driver* drv = ohmd_alloc(ctx, sizeof(ohmd_driver));
	if(!drv)
		return NULL;

	drv->get_device_list = get_device_list;
	drv->open_device = open_device;
	drv->destroy = destroy_driver;

	return drv;
}
