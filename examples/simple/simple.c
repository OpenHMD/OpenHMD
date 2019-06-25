/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* Simple Test */

#include <openhmd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void ohmd_sleep(double);

// gets float values from the device and prints them
void print_infof(ohmd_device* hmd, const char* name, int len, ohmd_float_value val)
{
	float f[16];
	assert(len <= 16);
	ohmd_device_getf(hmd, val, f);
	printf("%-25s", name);
	for(int i = 0; i < len; i++)
		printf("%f ", f[i]);
	printf("\n");
}

// gets int values from the device and prints them
void print_infoi(ohmd_device* hmd, const char* name, int len, ohmd_int_value val)
{
	int iv[16];
	assert(len <= 16);
	ohmd_device_geti(hmd, val, iv);
	printf("%-25s", name);
	for(int i = 0; i < len; i++)
		printf("%d ", iv[i]);
	printf("\n");
}

int main(int argc, char** argv)
{
	int device_idx = 0;

	if(argc > 1)
		device_idx = atoi(argv[1]);

	ohmd_require_version(0, 3, 0);

	int major, minor, patch;
	ohmd_get_version(&major, &minor, &patch);

	printf("OpenHMD version: %d.%d.%d\n", major, minor, patch);

	ohmd_context* ctx = ohmd_ctx_create();

	// Probe for devices
	int num_devices = ohmd_ctx_probe(ctx);
	if(num_devices < 0){
		printf("failed to probe devices: %s\n", ohmd_ctx_get_error(ctx));
		return -1;
	}

	printf("num devices: %d\n\n", num_devices);

	// Print device information
	for(int i = 0; i < num_devices; i++){
		int device_class = 0, device_flags = 0;
		const char* device_class_s[] = {"HMD", "Controller", "Generic Tracker", "Unknown"};

		ohmd_list_geti(ctx, i, OHMD_DEVICE_CLASS, &device_class);
		ohmd_list_geti(ctx, i, OHMD_DEVICE_FLAGS, &device_flags);

		printf("device %d\n", i);
		printf("  vendor:  %s\n", ohmd_list_gets(ctx, i, OHMD_VENDOR));
		printf("  product: %s\n", ohmd_list_gets(ctx, i, OHMD_PRODUCT));
		printf("  path:    %s\n", ohmd_list_gets(ctx, i, OHMD_PATH));
		printf("  class:   %s\n", device_class_s[device_class > OHMD_DEVICE_CLASS_GENERIC_TRACKER ? 4 : device_class]);
		printf("  flags:   %02x\n",  device_flags);
		printf("    null device:         %s\n", device_flags & OHMD_DEVICE_FLAGS_NULL_DEVICE ? "yes" : "no");
		printf("    rotational tracking: %s\n", device_flags & OHMD_DEVICE_FLAGS_ROTATIONAL_TRACKING ? "yes" : "no");
		printf("    positional tracking: %s\n", device_flags & OHMD_DEVICE_FLAGS_POSITIONAL_TRACKING ? "yes" : "no");
		printf("    left controller:     %s\n", device_flags & OHMD_DEVICE_FLAGS_LEFT_CONTROLLER ? "yes" : "no");
		printf("    right controller:    %s\n\n", device_flags & OHMD_DEVICE_FLAGS_RIGHT_CONTROLLER ? "yes" : "no");
	}

	// Open specified device idx or 0 (default) if nothing specified
	printf("opening device: %d\n", device_idx);
	ohmd_device* hmd = ohmd_list_open_device(ctx, device_idx);
	
	if(!hmd){
		printf("failed to open device: %s\n", ohmd_ctx_get_error(ctx));
		return -1;
	}

	// Print hardware information for the opened device
	int ivals[2];
	ohmd_device_geti(hmd, OHMD_SCREEN_HORIZONTAL_RESOLUTION, ivals);
	ohmd_device_geti(hmd, OHMD_SCREEN_VERTICAL_RESOLUTION, ivals + 1);
	printf("resolution:              %i x %i\n", ivals[0], ivals[1]);

	print_infof(hmd, "hsize:",            1, OHMD_SCREEN_HORIZONTAL_SIZE);
	print_infof(hmd, "vsize:",            1, OHMD_SCREEN_VERTICAL_SIZE);
	print_infof(hmd, "lens separation:",  1, OHMD_LENS_HORIZONTAL_SEPARATION);
	print_infof(hmd, "lens vcenter:",     1, OHMD_LENS_VERTICAL_POSITION);
	print_infof(hmd, "left eye fov:",     1, OHMD_LEFT_EYE_FOV);
	print_infof(hmd, "right eye fov:",    1, OHMD_RIGHT_EYE_FOV);
	print_infof(hmd, "left eye aspect:",  1, OHMD_LEFT_EYE_ASPECT_RATIO);
	print_infof(hmd, "right eye aspect:", 1, OHMD_RIGHT_EYE_ASPECT_RATIO);
	print_infof(hmd, "distortion k:",     6, OHMD_DISTORTION_K);
	
	print_infoi(hmd, "control count:   ", 1, OHMD_CONTROL_COUNT);

	int control_count;
	ohmd_device_geti(hmd, OHMD_CONTROL_COUNT, &control_count);

	const char* controls_fn_str[] = { "generic", "trigger", "trigger_click", "squeeze", "menu", "home",
		"analog-x", "analog-y", "anlog_press", "button-a", "button-b", "button-x", "button-y",
		"volume-up", "volume-down", "mic-mute"};

	const char* controls_type_str[] = {"digital", "analog"};

	int controls_fn[64];
	int controls_types[64];

	ohmd_device_geti(hmd, OHMD_CONTROLS_HINTS, controls_fn);
	ohmd_device_geti(hmd, OHMD_CONTROLS_TYPES, controls_types);
	
	printf("%-25s", "controls:");
	for(int i = 0; i < control_count; i++){
		printf("%s (%s)%s", controls_fn_str[controls_fn[i]], controls_type_str[controls_types[i]], i == control_count - 1 ? "" : ", ");
	}

	printf("\n\n");

	// Ask for n rotation quaternions and position vectors
	for(int i = 0; i < 10000; i++){
		ohmd_ctx_update(ctx);

		// this can be used to set a different zero point
		// for rotation and position, but is not required.
		//float zero[] = {.0, .0, .0, 1};
		//ohmd_device_setf(hmd, OHMD_ROTATION_QUAT, zero);
		//ohmd_device_setf(hmd, OHMD_POSITION_VECTOR, zero);

		// get rotation and postition
		print_infof(hmd, "rotation quat:", 4, OHMD_ROTATION_QUAT);
		print_infof(hmd, "position vec: ", 3, OHMD_POSITION_VECTOR);

		// read controls
		if (control_count) {
			float control_state[256];
			ohmd_device_getf(hmd, OHMD_CONTROLS_STATE, control_state);

			printf("%-25s", "controls state:");
			for(int i = 0; i < control_count; i++)
			{
				printf("%f ", control_state[i]);
			}
		}
		puts("");
			
		ohmd_sleep(.01);
	}

	ohmd_ctx_destroy(ctx);
	
	return 0;
}
