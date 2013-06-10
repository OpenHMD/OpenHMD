/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* Main Lib Implemenation */

#include "openhmdi.h"
#include <stdlib.h>
#include <string.h>

OHMD_APIENTRY ohmd_context* ohmd_ctx_create()
{
	ohmd_context* ctx = calloc(1, sizeof(ohmd_context));
	if(!ctx){
		LOGE("could not allocate RAM for context");
		return NULL;
	}

	ctx->drivers[ctx->num_drivers++] = ohmd_create_oculus_rift_drv(ctx);

	return ctx;
}

OHMD_APIENTRY void ohmd_ctx_destroy(ohmd_context* ctx)
{
	for(int i = 0; i < ctx->num_active_devices; i++){
		ctx->active_devices[i]->close(ctx->active_devices[i]);
	}

	for(int i = 0; i < ctx->num_drivers; i++){
		ctx->drivers[i]->destroy(ctx->drivers[i]);
	}

	free(ctx);
}

OHMD_APIENTRY void ohmd_ctx_update(ohmd_context* ctx)
{
	for(int i = 0; i < ctx->num_active_devices; i++)
		ctx->active_devices[i]->update(ctx->active_devices[i]);
}

OHMD_APIENTRY const char* ohmd_ctx_get_error(ohmd_context* ctx)
{
	return ctx->error_msg;
}

OHMD_APIENTRY int ohmd_ctx_probe(ohmd_context* ctx)
{
	memset(&ctx->list, 0, sizeof(ohmd_device_list));
	for(int i = 0; i < ctx->num_drivers; i++){
		ctx->drivers[i]->get_device_list(ctx->drivers[i], &ctx->list);
	}

	return ctx->list.num_devices;
}

OHMD_APIENTRY const char* ohmd_list_gets(ohmd_context* ctx, int index, ohmd_string_value type)
{
	if(index >= ctx->list.num_devices)
		return NULL;

	switch(type){
	case OHMD_VENDOR:
		return ctx->list.devices[index].vendor;
	case OHMD_PRODUCT:
		return ctx->list.devices[index].product;
	case OHMD_PATH:
		return ctx->list.devices[index].path;
	default:
		return NULL;
	}
}

OHMD_APIENTRY ohmd_device* ohmd_list_open_device(ohmd_context* ctx, int index)
{
	if(index >= 0 && index < ctx->list.num_devices){

		ohmd_device_desc* desc = &ctx->list.devices[index];
		ohmd_driver* driver = (ohmd_driver*)desc->driver_ptr;
		ohmd_device* device = driver->open_device(driver, desc);

		if (device == NULL)
			return NULL;

		ctx->active_devices[ctx->num_active_devices++] = device;
		return device;
	}

	ohmd_set_error(ctx, "no device with index: %d", index);
	return NULL;
}

OHMD_APIENTRY int ohmd_device_getf(ohmd_device* device, ohmd_float_value type, float* out)
{
	switch(type){
	case OHMD_LEFT_EYE_GL_MODELVIEW_MATRIX: {
			vec3f point = {{0, 0, 0}};
			quatf rot;
			device->getf(device, OHMD_ROTATION_QUAT, (float*)&rot);
			mat4x4f orient, world_shift, result;
			omat4x4f_init_look_at(&orient, &rot, &point);
			omat4x4f_init_translate(&world_shift, +(device->properties.idp / 2.0f), 0, 0);
			omat4x4f_mult(&world_shift, &orient, &result);
			omat4x4f_transpose(&result, (mat4x4f*)out);
			return 0;
		}
	case OHMD_RIGHT_EYE_GL_MODELVIEW_MATRIX: {
			vec3f point = {{0, 0, 0}};
			quatf rot;
			device->getf(device, OHMD_ROTATION_QUAT, (float*)&rot);
			mat4x4f orient, world_shift, result;
			omat4x4f_init_look_at(&orient, &rot, &point);
			omat4x4f_init_translate(&world_shift, -(device->properties.idp / 2.0f), 0, 0);
			omat4x4f_mult(&world_shift, &orient, &result);
			omat4x4f_transpose(&result, (mat4x4f*)out);
			return 0;
		}
	case OHMD_LEFT_EYE_GL_PROJECTION_MATRIX:
		omat4x4f_transpose(&device->properties.proj_left, (mat4x4f*)out);
		return 0;
	case OHMD_RIGHT_EYE_GL_PROJECTION_MATRIX:
		omat4x4f_transpose(&device->properties.proj_right, (mat4x4f*)out);
		return 0;

	case OHMD_SCREEN_HORIZONTAL_SIZE:
		*out = device->properties.hsize;
		return 0;
	case OHMD_SCREEN_VERTICAL_SIZE:
		*out = device->properties.vsize;
		return 0;

	case OHMD_LENS_HORIZONTAL_SEPERATION:
		*out = device->properties.lens_sep;
		return 0;
	case OHMD_LENS_VERTICAL_POSITION:
		*out = device->properties.lens_vpos;
		return 0;

	case OHMD_RIGHT_EYE_FOV:
	case OHMD_LEFT_EYE_FOV:
		*out = device->properties.fov;
		return 0;
	case OHMD_RIGHT_EYE_ASPECT_RATIO:
	case OHMD_LEFT_EYE_ASPECT_RATIO:
		*out = device->properties.fov;
		return 0;

	case OHMD_EYE_IPD:
		*out = device->properties.idp;
		return 0;

	case OHMD_PROJECTION_ZFAR:
		*out = device->properties.zfar;
		return 0;
	case OHMD_PROJECTION_ZNEAR:
		*out = device->properties.znear;
		return 0;
	default:
		return device->getf(device, type, out);
	}
}

OHMD_APIENTRY int ohmd_device_setf(ohmd_device* device, ohmd_float_value type, float* in)
{
	switch(type){
	case OHMD_EYE_IPD:
		device->properties.idp = *in;
		return 0;
	case OHMD_PROJECTION_ZFAR:
		device->properties.zfar = *in;
		return 0;
	case OHMD_PROJECTION_ZNEAR:
		device->properties.znear = *in;
		return 0;
	default:
		return -1;
	}
}

OHMD_APIENTRY int ohmd_device_geti(ohmd_device* device, ohmd_int_value type, int* out)
{
	switch(type){
	case OHMD_SCREEN_HORIZONTAL_RESOLUTION:
		*out = device->properties.hres;
		return 0;
	case OHMD_SCREEN_VERTICAL_RESOLUTION:
		*out = device->properties.vres;
		return 0;
	default:
		return -1;
	}
}

void* ohmd_allocfn(ohmd_context* ctx, char* e_msg, size_t size)
{
	void* ret = calloc(1, size);
	if(!ret)
		ohmd_set_error(ctx, "%s", e_msg);
	return ret;
}

void ohmd_set_default_device_properties(ohmd_device* device)
{
	device->properties.idp = 0.061f;
	device->properties.znear = 0.1f;
	device->properties.zfar = 1000.0f;
}
