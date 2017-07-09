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

// gets float values from the device and prints them
void print_infof(ohmd_device* hmd, const char* name, int len, ohmd_float_value val)
{
	float f[len];
	ohmd_device_getf(hmd, val, f);
	printf("%-25s", name);
	for(int i = 0; i < len; i++)
		printf("%f ", f[i]);
	printf("\n");
}

// gets int values from the device and prints them
void print_infoi(ohmd_device* hmd, const char* name, int len, ohmd_int_value val)
{
	int iv[len];
	ohmd_device_geti(hmd, val, iv);
	printf("%-25s", name);
	for(int i = 0; i < len; i++)
		printf("%d ", iv[i]);
	printf("\n");
}

int main(int argc, char** argv)
{
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
		char* device_class_s[] = {"HMD", "Controller", "Generic Tracker", "Unknown"};

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

	// Open default device (0)
	ohmd_device* hmd = ohmd_list_open_device(ctx, 0);
	
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
	
	print_infoi(hmd, "digital button count:", 1, OHMD_BUTTON_COUNT);
	print_infoi(hmd, "analog axis count:   ", 1, OHMD_ANALOG_AXIS_COUNT);

	printf("\n");

	int analog_axis_count;
	ohmd_device_geti(hmd, OHMD_ANALOG_AXIS_COUNT, &analog_axis_count);

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

		// read analog axes
		float axes[256];
		ohmd_device_getf(hmd, OHMD_ANALOG_AXES_STATE, axes);

		printf("%-25s", "axes:");
		for(int i = 0; i < analog_axis_count; i++)
		{
			printf("%f ", axes[i]);
		}
		puts("");

		// handle digital button events
		print_infoi(hmd, "button event count:", 1, OHMD_BUTTON_EVENT_COUNT);
		
		int event_count = 0;

		ohmd_device_geti(hmd, OHMD_BUTTON_EVENT_COUNT, &event_count);

		for(int i = 0; i < event_count; i++){
			int event[2] = {0, 0};
			ohmd_device_geti(hmd, OHMD_BUTTON_POP_EVENT, event);
			printf("button %d: %s", event[0], event[1] == OHMD_BUTTON_DOWN ? "down" : "up");
		}
			
		ohmd_sleep(.01);
	}

	ohmd_ctx_destroy(ctx);
	
	return 0;
}
