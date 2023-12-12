// Copyright 2013, Fredrik Hultin.
// Copyright 2013, Jakob Bornecrantz.
// Copyright 2015, Joey Ferwerda.
// Copyright 2023, 20kdc.
// SPDX-License-Identifier: BSL-1.0
/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 */

/* File Driver (based off of External Driver) */

// Environment variable OPENHMD_FILEDEV_HMD specifies a binary file.
// This file is dynamically updated to control HMD rotation (as Euler angles) and position.
// It also contains the initial projection/setup information.
// Using this interface, it is possible to connect homebrew setups to OpenHMD and OpenXR.

#include "../openhmdi.h"
#include "string.h"

// This struct is effectively ABI!
// It defines the binding between an external HMD daemon and OpenHMD.
// In practice, this would be driven by a helper script written in, say, Python.
typedef struct {
	// projection/setup info
	uint32_t hres;
	uint32_t vres;
	float hsize;
	float vsize;
	float lens_sep;
	float lens_vpos;
	float fov;
	float ratio;
	// dynamic data
	float pitch, yaw, roll;
	vec3f position_vector;
} file_data;

typedef struct {
	ohmd_device base;
	FILE * fd;
	file_data data;
	quatf rotation_quat;
} file_priv;

static quatf do_euler2quat(float pitch, float yaw, float roll)
{
	vec3f pitcher_axis = {{1, 0, 0}};
	vec3f yawer_axis =   {{0, 1, 0}};
	vec3f roller_axis =  {{0, 0, 1}};

	quatf pitcher, yawer, roller;
	quatf interm1, interm2;

	oquatf_init_axis(&pitcher, &pitcher_axis, pitch);
	oquatf_init_axis(&yawer, &yawer_axis, yaw);
	oquatf_init_axis(&roller, &roller_axis, roll);

	oquatf_mult(&yawer, &pitcher, &interm1);
	oquatf_mult(&interm1, &roller, &interm2);
	return interm2;
}

static void update_device(ohmd_device* device)
{
	file_priv* priv = (file_priv*)device;

	rewind(priv->fd);
	fflush(priv->fd);
	fread(&priv->data, sizeof(file_data), 1, priv->fd);
	priv->rotation_quat = do_euler2quat(priv->data.pitch, priv->data.yaw, priv->data.roll);
}

static int getf(ohmd_device* device, ohmd_float_value type, float* out)
{
	file_priv* priv = (file_priv*)device;

	switch(type){
		case OHMD_ROTATION_QUAT:
			*(quatf*)out = priv->rotation_quat;
			break;

		case OHMD_POSITION_VECTOR:
			*(vec3f*)out = priv->data.position_vector;
			break;

		default:
			ohmd_set_error(priv->base.ctx, "invalid type given to getf (%d)", type);
			return -1;
			break;
	}

	return 0;
}

static void close_device(ohmd_device* device)
{
	file_priv* priv = (file_priv*)device;

	LOGD("closing file device");
	fclose(priv->fd);
	free(device);
}

static ohmd_device* open_device(ohmd_driver* driver, ohmd_device_desc* desc)
{
	file_priv* priv = ohmd_alloc(driver->ctx, sizeof(file_priv));
	if(!priv)
		return NULL;

	// Set default device properties
	ohmd_set_default_device_properties(&priv->base.properties);

	priv->fd = fopen(desc->path, "rb");
	if(!priv->fd) {
		LOGE("file device %s cannot be opened", desc->path);
		free(priv);
		return NULL;
	}

	if (fread(&priv->data, sizeof(file_data), 1, priv->fd) != 1) {
		LOGE("file device %s unable to read initial data", desc->path);
		fclose(priv->fd);
		free(priv);
		return NULL;
	}
	priv->rotation_quat = do_euler2quat(priv->data.pitch, priv->data.yaw, priv->data.roll);

	// Set device properties
	priv->base.properties.hsize = priv->data.hsize;
	priv->base.properties.vsize = priv->data.vsize;
	priv->base.properties.hres = priv->data.hres;
	priv->base.properties.vres = priv->data.vres;
	priv->base.properties.lens_sep = priv->data.lens_sep;
	priv->base.properties.lens_vpos = priv->data.lens_vpos;
	priv->base.properties.fov = priv->data.fov;
	priv->base.properties.ratio = priv->data.ratio;

	// calculate projection eye projection matrices from the device properties
	ohmd_calc_default_proj_matrices(&priv->base.properties);

	// set up device callbacks
	priv->base.update = update_device;
	priv->base.close = close_device;
	priv->base.getf = getf;

	return (ohmd_device*)priv;
}

static void get_device_list_attach(ohmd_driver* driver, ohmd_device_list* list, const char * path, ohmd_device_class dev_class, ohmd_device_flags dev_flags)
{
	ohmd_device_desc* desc = &list->devices[list->num_devices++];

	strcpy(desc->driver, "OpenHMD File-Backed Port");
	strcpy(desc->vendor, "OpenHMD");
	strcpy(desc->product, "File-Backed Port");

	strcpy(desc->path, path);

	desc->device_class = dev_class;
	desc->device_flags = dev_flags;

	desc->driver_ptr = driver;
}

static void get_device_list(ohmd_driver* driver, ohmd_device_list* list)
{
	const char * devhmd = getenv("OPENHMD_FILEDEV_HMD");
	if (devhmd)
		get_device_list_attach(driver, list, devhmd, OHMD_DEVICE_CLASS_HMD, OHMD_DEVICE_FLAGS_ROTATIONAL_TRACKING | OHMD_DEVICE_FLAGS_POSITIONAL_TRACKING);
}

static void destroy_driver(ohmd_driver* drv)
{
	LOGD("shutting down file driver");
	free(drv);
}

ohmd_driver* ohmd_create_file_drv(ohmd_context* ctx)
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
