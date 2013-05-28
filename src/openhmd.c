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
	return device->getf(device, type, out);
}

void* ohmd_allocfn(ohmd_context* ctx, char* e_msg, size_t size)
{
	void* ret = calloc(1, size);
	if(!ret)
		ohmd_set_error(ctx, "%s", e_msg);
	return ret;
}
