/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* Simple Test */

#include <openhmd.h>

#include <stdio.h>

void ohmd_sleep(double);
double ohmd_get_tick();

int main(int argc, char** argv)
{
	(void)argc; (void)argv;
	ohmd_context* ctx = ohmd_ctx_create();

	int num_devices = ohmd_ctx_probe(ctx);
	if(num_devices < 0){
		printf("failed to probe devices: %s\n", ohmd_ctx_get_error(ctx));
		return -1;
	}

	printf("num devices: %d\n", num_devices);

	for(int i = 0; i < num_devices; i++){
		printf("vendor: %s\n", ohmd_list_gets(ctx, i, OHMD_VENDOR));
		printf("product: %s\n", ohmd_list_gets(ctx, i, OHMD_PRODUCT));
		printf("path: %s\n", ohmd_list_gets(ctx, i, OHMD_PATH));
	}

	ohmd_device* hmd = ohmd_list_open_device(ctx, 0);

	if(!hmd){
		printf("failed to open device: %s\n", ohmd_ctx_get_error(ctx));
		return -1;
	}

	for(int i = 0; i < 10000; i++){
		float q[4];

		ohmd_ctx_update(ctx);

		ohmd_device_getf(hmd, OHMD_ROTATION_QUAT, q);
		printf("quat: % 4.4f, % 4.4f, % 4.4f, % 4.4f\n", q[0], q[1], q[2], q[3]);

		ohmd_sleep(.01);
	}

	ohmd_ctx_destroy(ctx);
	
	return 0;
}
