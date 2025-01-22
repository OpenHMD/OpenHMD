// Copyright 2013, Fredrik Hultin.
// Copyright 2013, Jakob Bornecrantz.
// Copyright 2015, Joey Ferwerda.
// Copyright 2023, 20kdc.
// SPDX-License-Identifier: BSL-1.0
/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 */

/* File Driver (based off of External Driver) */

// Environment variable OPENHMD_FILEDEV_(0,1,2...) specify binary files.
// This file is dynamically updated to control HMD rotation (as Euler angles) and position.
// It also contains the initial projection/setup information.
// Using this interface, it is possible to connect homebrew setups to OpenHMD and OpenXR.

#include "../openhmdi.h"
#include "string.h"
#include "drv_file_fmt.h"

// returns non-zero on error
int ohmd_file_drv_read_file(FILE * file, ohmd_file_data_latest* out);
quatf ohmd_file_drv_rotation2quat(ohmd_file_rotation rot);

typedef struct {
	ohmd_device base;
	FILE* fd;
	// latest version (reader module will port)
	ohmd_file_data_latest data;
	quatf rotation_quat;
} file_priv;

static void update_device(ohmd_device* device)
{
	file_priv* priv = (file_priv*)device;

	rewind(priv->fd);
	fflush(priv->fd);
	if (ohmd_file_drv_read_file(priv->fd, &priv->data) == 0)
		priv->rotation_quat = ohmd_file_drv_rotation2quat(priv->data.rotation);
}

static int getf(ohmd_device* device, ohmd_float_value type, float* out)
{
	file_priv* priv = (file_priv*)device;

	switch(type){
		case OHMD_ROTATION_QUAT:
			*(quatf*)out = priv->rotation_quat;
			break;

		case OHMD_POSITION_VECTOR:
			memcpy(out, priv->data.position, sizeof(float) * 3);
			break;

		case OHMD_CONTROLS_STATE:
			// always copy the initially advertised, fixed, control count
			for (int i = 0; i < priv->base.properties.control_count; i++)
				out[i] = priv->data.controls.control[i].state;
			break;

		case OHMD_DISTORTION_K:
			memcpy(out, priv->data.hmd_info.distortion_k, sizeof(float) * 6);
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

	int err = ohmd_file_drv_read_file(priv->fd, &priv->data);
	if (err != 0) {
		LOGE("file device %s unable to read initial data, code %i", desc->path, err);
		fclose(priv->fd);
		free(priv);
		return NULL;
	}
	priv->rotation_quat = ohmd_file_drv_rotation2quat(priv->data.rotation);

	// Set device HMD properties
	if (priv->data.hmd_info.hres != 0) {
		priv->base.properties.hsize = priv->data.hmd_info.hsize;
		priv->base.properties.vsize = priv->data.hmd_info.vsize;
		priv->base.properties.hres = priv->data.hmd_info.hres;
		priv->base.properties.vres = priv->data.hmd_info.vres;
		priv->base.properties.lens_sep = priv->data.hmd_info.lens_sep;
		priv->base.properties.lens_vpos = priv->data.hmd_info.lens_vpos;
		priv->base.properties.fov = priv->data.hmd_info.fov;
		priv->base.properties.ratio = priv->data.hmd_info.ratio;
	}

	// calculate projection eye projection matrices from the device properties
	ohmd_calc_default_proj_matrices(&priv->base.properties);

	// Set device controls
	priv->base.properties.control_count = priv->data.controls.count;
	for (int i = 0; i < priv->data.controls.count; i++) {
		priv->base.properties.controls_hints[i] = priv->data.controls.control[i].hint;
		priv->base.properties.controls_types[i] = priv->data.controls.control[i].type;
	}

	// set up device callbacks
	priv->base.update = update_device;
	priv->base.close = close_device;
	priv->base.getf = getf;

	return (ohmd_device*)priv;
}

static void get_device_list_attach(ohmd_driver* driver, ohmd_device_list* list, const char * path, const ohmd_file_data_latest* data)
{
	ohmd_device_desc* desc = &list->devices[list->num_devices++];

	strcpy(desc->driver, "OpenHMD File-Backed Port");
	strcpy(desc->vendor, "OpenHMD");
	// Null terminator is added by ohmd_file_drv_read_file
	strcpy(desc->product, data->name);

	strcpy(desc->path, path);

	desc->device_class = data->device_class;
	desc->device_flags = data->device_flags;

	desc->driver_ptr = driver;
}

static void get_device_list(ohmd_driver* driver, ohmd_device_list* list)
{
	char envname[20];
	for (int i = 0; i < 32; i++) {
		sprintf(envname, "OPENHMD_FILEDEV_%i", i);
		const char * devhmd = getenv(envname);
		if (devhmd) {
			// we don't want to list devices that won't work, so let's run a "pre-check"
			// still report missing files as errors so users know what's up
			FILE * f = fopen(devhmd, "rb");
			if(!f) {
				LOGE("file device %s cannot be opened", devhmd);
			} else {
				ohmd_file_data_latest data;
				int err = ohmd_file_drv_read_file(f, &data);
				fclose(f);
				if (!err) {
					get_device_list_attach(driver, list, devhmd, &data);
				} else {
					LOGE("file device %s is invalid, code %i", devhmd, err);
				}
			}
		}
	}
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
