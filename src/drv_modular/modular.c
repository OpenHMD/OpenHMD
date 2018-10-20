/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* Modular Driver */

#include <string.h>
#include "../openhmdi.h"

typedef struct {
	ohmd_device base;
	float position[3];
	float rotation[4];
	omodule* module;
} modular_priv;

static void update_device(ohmd_device* device)
{
}

static int getf(ohmd_device* device, ohmd_float_value type, float* out)
{
	modular_priv* priv = (modular_priv*)device;

	switch(type){
	case OHMD_ROTATION_QUAT:
		memcpy(out, priv->rotation, 4 * sizeof(float));
		break;

	case OHMD_POSITION_VECTOR:
		memcpy(out, priv->position, 3 * sizeof(float));
		break;

	case OHMD_DISTORTION_K:
		// TODO this should be set to the equivalent of no distortion
		memset(out, 0, sizeof(float) * 6);
		break;
	
	case OHMD_CONTROLS_STATE:
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
	LOGD("closing modular device");
	free(device);
}

static ohmd_device* open_device(ohmd_driver* driver, ohmd_device_desc* desc)
{
	modular_priv* priv = ohmd_alloc(driver->ctx, sizeof(modular_priv));

	if(!priv)
		return NULL;
	
	priv->rotation[3] = 1.0f;
	
	// Set default device properties
	ohmd_set_default_device_properties(&priv->base.properties);

	// Set device properties (imitates the rift values)
	priv->base.properties.hsize = 0.149760f;
	priv->base.properties.vsize = 0.093600f;
	priv->base.properties.hres = 1280;
	priv->base.properties.vres = 800;
	priv->base.properties.lens_sep = 0.063500f;
	priv->base.properties.lens_vpos = 0.046800f;
	priv->base.properties.fov = DEG_TO_RAD(125.5144f);
	priv->base.properties.ratio = (1280.0f / 800.0f) / 2.0f;
	
	// Some buttons and axes
	priv->base.properties.control_count = 2;
	priv->base.properties.controls_hints[0] = OHMD_BUTTON_A;
	priv->base.properties.controls_hints[1] = OHMD_MENU;
	priv->base.properties.controls_types[0] = OHMD_ANALOG;
	priv->base.properties.controls_types[1] = OHMD_DIGITAL;

	// calculate projection eye projection matrices from the device properties
	ohmd_calc_default_proj_matrices(&priv->base.properties);

	// set up device callbacks
	priv->base.update = update_device;
	priv->base.close = close_device;
	priv->base.getf = getf;
	
	return (ohmd_device*)priv;
}

static void get_device_list(ohmd_driver* driver, ohmd_device_list* list)
{
	ohmd_device_desc* desc;

	desc = &list->devices[list->num_devices++];

	strcpy(desc->driver, "OpenHMD Null Driver");
	strcpy(desc->vendor, "OpenHMD");
	strcpy(desc->product, "HMD Null Device");

	strcpy(desc->path, "(none)");

	desc->driver_ptr = driver;

	desc->device_flags = OHMD_DEVICE_FLAGS_NULL_DEVICE | OHMD_DEVICE_FLAGS_ROTATIONAL_TRACKING;
	desc->device_class = OHMD_DEVICE_CLASS_HMD;
}

static void destroy_driver(ohmd_driver* drv)
{
	LOGD("shutting down modular driver");
	free(drv);
}

ohmd_driver* ohmd_create_modular_drv(ohmd_context* ctx, omodule** out_module)
{
	ohmd_driver* drv = ohmd_alloc(ctx, sizeof(ohmd_driver));

	if(!drv)
		return NULL;

	drv->get_device_list = get_device_list;
	drv->open_device = open_device;
	drv->get_device_list = get_device_list;
	drv->open_device = open_device;
	drv->destroy = destroy_driver;

	omodule* m = omodule_create(ctx, "driver_module", 0x1111, drv);

	return drv;
}
