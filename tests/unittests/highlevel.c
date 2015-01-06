/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* Unit Tests - High-level functions */

#include "tests.h"
#include "openhmd.h"

void test_highlevel_open_close_device()
{
	ohmd_context* ctx = ohmd_ctx_create();
	TAssert(ctx);

	// Probe for devices
	int num_devices = ohmd_ctx_probe(ctx);
	TAssert(num_devices > 0);

	// Open dummy device (num_devices - 1)
	ohmd_device* hmd = ohmd_list_open_device(ctx, num_devices - 1);
	TAssert(hmd);

	// Close the device
	int ret = ohmd_close_device(hmd);
	TAssert(ret == 0);
	
	ohmd_ctx_destroy(ctx);	
}

void test_highlevel_open_close_many_devices()
{
	ohmd_context* ctx = ohmd_ctx_create();
	TAssert(ctx);

	// Probe for devices
	int num_devices = ohmd_ctx_probe(ctx);
	TAssert(num_devices > 0);

	ohmd_device* hmds[16];

	for(int i = 0; i < 8; i++){
		// Open dummy device (num_devices - 1)
		hmds[i] = ohmd_list_open_device(ctx, num_devices - 1);
		TAssert(hmds[i]);
	}

	for(int i = 4; i < 8; i++){
		// Close the device
		int ret = ohmd_close_device(hmds[i]);
		TAssert(ret == 0);
	}
	
	for(int i = 4; i < 16; i++){
		// Open dummy device (num_devices - 1)
		hmds[i] = ohmd_list_open_device(ctx, num_devices - 1);
		TAssert(hmds[i]);
	}

	for(int i = 0; i < 16; i++){
		// Close the device
		int ret = ohmd_close_device(hmds[i]);
		TAssert(ret == 0);
	}
	
	ohmd_ctx_destroy(ctx);	
}
