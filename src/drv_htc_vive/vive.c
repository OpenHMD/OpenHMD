/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* HTC Vive Driver */

#define FEATURE_BUFFER_SIZE 256

#define HTC_ID                   0x0bb4
#define VIVE_HMD                 0x2c87
#define VALVE_ID                 0x28de
#define VIVE_WATCHMAN_DONGLE     0x2101
#define VIVE_LIGHTHOUSE_FPGA_RX  0x2000


#include <string.h>
#include <wchar.h>
#include <hidapi.h>
#include <assert.h>

#include "vive.h"

typedef struct {
	ohmd_device base;
	
	hid_device* handle;
} vive_priv;

static void update_device(ohmd_device* device)
{
	vive_priv* priv = (vive_priv*)device;

	int size = 0;
	unsigned char buffer[FEATURE_BUFFER_SIZE];
	
	while((size = hid_read(priv->handle, buffer, FEATURE_BUFFER_SIZE)) > 0){
		if(buffer[0] == VIVE_IRQ_SENSORS){
			vive_sensor_packet pkt;
			vive_decode_sensor_packet(&pkt, buffer, size);

			printf("vive sensor sample:\n");
			printf("  report_id: %u\n", pkt.report_id);
			for(int i = 0; i < 3; i++){
				printf("    sample[%d]:\n", i);

				for(int j = 0; j < 3; j++){
					printf("      acc[%d]: %d\n", j, pkt.samples[i].acc[j]);
				}
				
				for(int j = 0; j < 3; j++){
					printf("      rot[%d]: %d\n", j, pkt.samples[i].rot[j]);
				}

				printf("time_ticks: %d\n", pkt.samples[i].time_ticks);
				printf("seq: %u\n", pkt.samples[i].seq);
				printf("\n");
			}

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
	vive_priv* priv = (vive_priv*)device;

	switch(type){
	case OHMD_ROTATION_QUAT: 
		out[0] = out[1] = out[2] = 0;
		out[3] = 1.0f;
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
	int hret = 0;
	vive_priv* priv = (vive_priv*)device;

	LOGD("closing HTC Vive device");

	// turn the display off
	hret = hid_send_feature_report(priv->handle, vive_magic_power_off1, sizeof(vive_magic_power_off1));
	printf("power off magic 1: %d\n", hret);
	
	hret = hid_send_feature_report(priv->handle, vive_magic_power_off2, sizeof(vive_magic_power_off2));
	printf("power off magic 2: %d\n", hret);

	free(device);
}

static void dump_indexed_string(hid_device* device, int index)
{
	wchar_t wbuffer[512] = {0};
	char buffer[1024] = {0};

	int hret = hid_get_indexed_string(device, index, wbuffer, 511);	

	if(hret == 0){
		wcstombs(buffer, wbuffer, sizeof(buffer));
		printf("indexed string 0x%02x: '%s'\n", index, buffer);
	}
}

static void dump_info_string(int (*fun)(hid_device*, wchar_t*, size_t), const char* what, hid_device* device)
{
	wchar_t wbuffer[512] = {0};
	char buffer[1024] = {0};

	int hret = fun(device, wbuffer, 511);	

	if(hret == 0){
		wcstombs(buffer, wbuffer, sizeof(buffer));
		printf("%s: '%s'\n", what, buffer);
	}
}

static void dumpbin(const char* label, const unsigned char* data, int length)
{
	printf("%s:\n", label);
	for(int i = 0; i < length; i++){
		printf("%02x ", data[i]);
		if((i % 16) == 15)
			printf("\n");
	}
	printf("\n");
}

static ohmd_device* open_device(ohmd_driver* driver, ohmd_device_desc* desc)
{
	vive_priv* priv = ohmd_alloc(driver->ctx, sizeof(vive_priv));

	if(!priv)
		return NULL;

	int hret = 0;
	
	priv->base.ctx = driver->ctx;

	// Open the HID device
	priv->handle = hid_open_path(desc->path);

	if(!priv->handle)
		goto cleanup;
	
	if(hid_set_nonblocking(priv->handle, 1) == -1){
		ohmd_set_error(driver->ctx, "failed to set non-blocking on device");
		goto cleanup;
	}

	dump_info_string(hid_get_manufacturer_string, "manufacturer", priv->handle);
	dump_info_string(hid_get_product_string , "product", priv->handle);
	dump_info_string(hid_get_serial_number_string, "serial number", priv->handle);

	// turn the display on
	hret = hid_send_feature_report(priv->handle, vive_magic_power_on, sizeof(vive_magic_power_on));
	printf("power on magic: %d\n", hret);
	
	// Set default device properties
	ohmd_set_default_device_properties(&priv->base.properties);

	// Set device properties
	priv->base.properties.hsize = 0.149760f;
	priv->base.properties.vsize = 0.093600f;
	priv->base.properties.hres = 2160;
	priv->base.properties.vres = 1200;
	priv->base.properties.lens_sep = 0.063500;
	priv->base.properties.lens_vpos = 0.046800;
	priv->base.properties.fov = DEG_TO_RAD(125.5144f);
	priv->base.properties.ratio = (2160.0f / 1200.0f) / 2.0f;

	// calculate projection eye projection matrices from the device properties
	ohmd_calc_default_proj_matrices(&priv->base.properties);

	// set up device callbacks
	priv->base.update = update_device;
	priv->base.close = close_device;
	priv->base.getf = getf;
	
	return (ohmd_device*)priv;

cleanup:
	if(priv)
		free(priv);

	return NULL;
}

static void get_device_list(ohmd_driver* driver, ohmd_device_list* list)
{
	struct hid_device_info* devs = hid_enumerate(HTC_ID, VIVE_HMD);
	struct hid_device_info* cur_dev = devs;

	while (cur_dev) {
		ohmd_device_desc* desc = &list->devices[list->num_devices++];

		strcpy(desc->driver, "OpenHMD HTC Vive Driver");
		strcpy(desc->vendor, "HTC/Valve");
		strcpy(desc->product, "HTC Vive");

		desc->revision = 0;

		strcpy(desc->path, cur_dev->path);

		desc->driver_ptr = driver;

		cur_dev = cur_dev->next;
	}

	hid_free_enumeration(devs);
}

static void destroy_driver(ohmd_driver* drv)
{
	LOGD("shutting down HTC Vive driver");
	free(drv);
}

ohmd_driver* ohmd_create_htc_vive_drv(ohmd_context* ctx)
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
