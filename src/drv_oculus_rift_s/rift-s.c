/*
 * Copyright 2013, Fredrik Hultin.
 * Copyright 2013, Jakob Bornecrantz.
 * Copyright 2016 Philipp Zabel
 * Copyright 2019-2020 Jan Schmidt
 * SPDX-License-Identifier: BSL-1.0
 *
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 */

/* Oculus Rift S Driver - HID/USB Driver Implementation */

#include <stdlib.h>
#include <hidapi.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>

#include "rift-s.h"
#include "rift-s-hmd.h"

#define UDEV_WIKI_URL "https://github.com/OpenHMD/OpenHMD/wiki/Udev-rules-list"
#define OCULUS_VR_INC_ID 0x2833
#define RIFT_S_PID 0x0051

/* Interfaces for the various reports / HID controls */
#define RIFT_S_INTF_HMD 6
#define RIFT_S_INTF_STATUS 7
#define RIFT_S_INTF_CONTROLLERS 8

typedef struct device_list_s device_list_t;
struct device_list_s {
	char path[OHMD_STR_SIZE];
	rift_s_hmd_t *hmd;

	device_list_t* next;
};

typedef struct {
	const char* name;
	int company;
	int id;
	int iface;
} rift_devices;

/* Global list of (probably 1) active HMD devices */
static device_list_t* rift_hmds;

static hid_device* open_hid_dev (ohmd_context* ctx, int vid, int pid, int iface_num);
static void close_hmd (rift_s_hmd_t *hmd);

static rift_s_hmd_t *find_hmd(char *hid_path)
{
	device_list_t* current = rift_hmds;

	while (current != NULL) {
		if (strcmp(current->path, hid_path)==0) {
			current->hmd->use_count++;
			return current->hmd;
		}
		current = current->next;
	}
	return NULL;
}

static void push_hmd(rift_s_hmd_t *hmd, char *hid_path)
{
	device_list_t* d = calloc(1, sizeof(device_list_t));
	d->hmd = hmd;
	strcpy (d->path, hid_path);

	d->next = rift_hmds;
	rift_hmds = d;
}

static void release_hmd(rift_s_hmd_t *hmd)
{
	device_list_t* current, *prev;

	if (hmd->use_count > 1) {
		hmd->use_count--;
		return;
	}

	/* Use count on the HMD device hit 0, release it
	 * and remove from the list */
	current = rift_hmds;
	prev = NULL;
	while (current != NULL) {
		if (current->hmd == hmd) {
			close_hmd (current->hmd);

			if (prev == NULL)
				rift_hmds = current->next;
			else
				prev->next = current->next;
			free (current);
			return;
		}
		prev = current;
		current = current->next;
	}

	LOGE("Failed to find HMD in the active device list");
}

static rift_s_device_priv* rift_s_device_priv_get(ohmd_device* device)
{
	return (rift_s_device_priv*)device;
}

static void
vec3f_rotate_3x3(vec3f *vec, float rot[3][3])
{
	vec3f in = *vec;
	for (int i = 0; i < 3; i++)
		vec->arr[i] = rot[i][0] * in.arr[0] + rot[i][1] * in.arr[1] + rot[i][2] * in.arr[2];
}

static void
handle_hmd_report (rift_s_hmd_t *priv, const unsigned char *buf, int size)
{
	rift_s_hmd_report_t report;

	if (!rift_s_parse_hmd_report (&report, buf, size)) {
		return;
	}

	const int32_t TICK_LEN_US = 1000000 / priv->imu_config.imu_hz;
	int32_t dt = TICK_LEN_US;
	uint32_t end_ts = report.timestamp;

	if (priv->last_imu_timestamp != 0) {
		dt = report.timestamp - priv->last_imu_timestamp;
		end_ts -= dt;
	}

	const float gyro_scale = 1.0 / priv->imu_config.gyro_scale;
	const float accel_scale = OHMD_GRAVITY_EARTH / priv->imu_config.accel_scale;
	const float temperature_scale = 1.0 / priv->imu_config.temperature_scale;
	const float temperature_offset = priv->imu_config.temperature_offset;

	for(int i = 0; i < 3; i++) {
		rift_s_hmd_imu_sample_t *s = report.samples + i;

		if (s->marker & 0x80)
				break; /* Sample (and remaining ones) are invalid */

		vec3f gyro, accel;
		float dt_sec = dt / 1000000.0;

		gyro.x = DEG_TO_RAD(gyro_scale * s->gyro[0]);
		gyro.y = DEG_TO_RAD(gyro_scale * s->gyro[1]);
		gyro.z = DEG_TO_RAD(gyro_scale * s->gyro[2]);

		accel.x = accel_scale * s->accel[0];
		accel.y = accel_scale * s->accel[1];
		accel.z = accel_scale * s->accel[2];

		/* Apply correction offsets first, then rectify */
		for (int j = 0; j < 3; j++) {
			accel.arr[j] -= priv->imu_calibration.accel.offset_at_0C.arr[j];
			gyro.arr[j] -= priv->imu_calibration.gyro.offset.arr[j];
		}

		vec3f_rotate_3x3(&accel, priv->imu_calibration.accel.rectification);
		vec3f_rotate_3x3(&gyro, priv->imu_calibration.gyro.rectification);

		priv->raw_accel = accel;
		priv->raw_gyro = gyro;
		/* FIXME: This doesn't seem to produce the right numbers, but it's OK - we don't use it anyway */
		priv->temperature = temperature_scale * (s->temperature - temperature_offset) + 25;

#if 0
		printf ("Sample %d dt %f accel %f %f %f gyro %f %f %f\n",
			i, dt_sec, priv->raw_accel.x, priv->raw_accel.y, priv->raw_accel.z,
			priv->raw_gyro.x, priv->raw_gyro.y, priv->raw_gyro.z);
#endif

		ofusion_update(&priv->sensor_fusion, dt_sec, &priv->raw_gyro, &priv->raw_accel, &priv->raw_mag);
		end_ts += dt;
		dt = TICK_LEN_US;
	}

	priv->last_imu_timestamp = end_ts;
}

static void update_hmd(rift_s_hmd_t *priv)
{
	unsigned char buf[FEATURE_BUFFER_SIZE];

	// Handle keep alive messages
	double t = ohmd_get_tick();
	if(t - priv->last_keep_alive >= ((double)(KEEPALIVE_INTERVAL_MS) / 1000.0)) {
		// send keep alive message
		rift_s_send_keepalive (priv->handles[0]);
		// Update the time of the last keep alive we have sent.
		priv->last_keep_alive = t;
	}

	/* Poll each of the 3 devices for messages and process them */
	for (int i = 0; i < 3; i++) {
		if (priv->handles[i] == NULL)
				continue;

		while(true){
			int size = hid_read(priv->handles[i], buf, FEATURE_BUFFER_SIZE);
			if(size < 0){
				LOGE("error reading from HMD device");
				break;
			} else if(size == 0) {
				break; // No more messages, return.
			}

			if (buf[0] == 0x65)
				handle_hmd_report (priv, buf, size);
			else if (buf[0] == 0x67)
				rift_s_handle_controller_report (priv, priv->handles[0], buf, size);
			else if (buf[0] == 0x66) {
				// System state packet. Enable the screen if the prox sensor is
				// triggered
				bool prox_sensor = (buf[1] == 0) ? false : true;
				if (prox_sensor != priv->display_on) {
					rift_s_set_screen_enable (priv->handles[0], prox_sensor);
					priv->display_on = prox_sensor;
				}
			}
			else
			 LOGW("Unknown Rift S report 0x%02x!", buf[0]);
		}
	}

	rift_s_radio_update (&priv->radio_state, priv->handles[0]);
}

static void update_device(ohmd_device* device)
{
	rift_s_device_priv* dev_priv = rift_s_device_priv_get(device);
	rift_s_hmd_t *hmd = dev_priv->hmd;

	/* Update on whichever is the lowest open id device */
	if (dev_priv->id == 2) {
		if (hmd->hmd_dev.opened)
			return;
		if (hmd->touch_dev[0].base.opened)
			return;
	}
	else if (dev_priv->id == 1) {
		if (hmd->hmd_dev.opened)
			return;
	}

	update_hmd (dev_priv->hmd);
}

static int getf_hmd(ohmd_device* device, ohmd_float_value type, float* out)
{
	rift_s_device_priv* dev_priv = rift_s_device_priv_get(device);
	rift_s_hmd_t *hmd = dev_priv->hmd;

	switch(type){
	case OHMD_DISTORTION_K: {
			for (int i = 0; i < 6; i++) {
				out[i] = 0.0; // hmd->display_info.distortion_k[i];
			}
			break;
		}

	case OHMD_ROTATION_QUAT: {
			*(quatf*)out = hmd->sensor_fusion.orient;
			break;
		}

	case OHMD_POSITION_VECTOR:
		out[0] = out[1] = out[2] = 0;
		break;

	case OHMD_CONTROLS_STATE:
		break;

	default:
		ohmd_set_error(hmd->ctx, "invalid type given to getf (%ud)", type);
		return -1;
		break;
	}

	return 0;
}

static int getf_touch_controller(ohmd_device *device, ohmd_float_value type, float* out)
{
	rift_s_device_priv* dev_priv = rift_s_device_priv_get(device);
	rift_s_hmd_t *hmd = dev_priv->hmd;
	rift_s_controller_device *touch = (rift_s_controller_device *)(dev_priv);

	if (touch->device_num < 0)
		return -1; /* Device not online yet */

	rift_s_controller_state *ctrl = hmd->controllers + touch->device_num;

	switch(type){
	case OHMD_ROTATION_QUAT: {
			*(quatf*)out = ctrl->imu_fusion.orient;
			break;
		}
	case OHMD_POSITION_VECTOR:
		out[0] = out[1] = out[2] = 0;
		break;
	case OHMD_DISTORTION_K:
		return -1;
	case OHMD_CONTROLS_STATE:
		out[0] = (ctrl->buttons & RIFT_S_BUTTON_A) != 0 ? 1.0 : 0.0;
		out[1] = (ctrl->buttons & RIFT_S_BUTTON_B) != 0 ? 1.0 : 0.0;
		out[2] = (ctrl->buttons & RIFT_S_BUTTON_OCULUS) != 0 ? 1.0 : 0.0;
		out[3] = (ctrl->buttons & RIFT_S_BUTTON_STICK) != 0 ? 1.0 : 0.0;

		out[4] = 1.0 - (float)(ctrl->trigger) / 4096.0;
		out[5] = 1.0 - (float)(ctrl->grip) / 4096.0;
		out[6] = (float)(ctrl->joystick_x) / 32768.0; /* FIXME: Scale this properly */
		out[7] = (float)(ctrl->joystick_y) / 32768.0; /* FIXME: Scale this properly */
		break;
	default:
		ohmd_set_error(hmd->ctx, "invalid type given to getf (%u)", type);
		return -1;
	}

	return 0;
}

static void close_device(ohmd_device* device)
{
	LOGD("closing device");
	rift_s_device_priv* dev_priv = rift_s_device_priv_get(device);
	dev_priv->opened = false;
	release_hmd (dev_priv->hmd);
}

#if 0
static int
dump_fw_block(hid_device *handle, uint8_t block_id) {
	int res;
	char *data = NULL;
	int len;

	res = rift_s_read_firmware_block (handle, block_id, &data, &len);
	if (res	< 0)
			return res;

	free (data);
	return 0;
}
#endif

static int read_calibration (rift_s_hmd_t *hmd, hid_device *hid) {

	char *json = NULL;
	int json_len = 0;

	int ret = rift_s_read_firmware_block (hid, RIFT_S_FIRMWARE_BLOCK_IMU_CALIB, &json, &json_len);
	if (ret < 0)
		return ret;

	ret = rift_s_parse_imu_calibration(json, &hmd->imu_calibration);
	free(json);

	return ret;
}

static void init_touch_device(rift_s_controller_device *touch, int id)
{
	ohmd_device *ohmd_dev = &touch->base.base;

	touch->device_num = -1;

	ohmd_set_default_device_properties(&ohmd_dev->properties);

	ohmd_dev->properties.control_count = 8;

	if (id == 1) { // Right controller
		ohmd_dev->properties.controls_hints[0] = OHMD_BUTTON_A;
		ohmd_dev->properties.controls_hints[1] = OHMD_BUTTON_B;
		ohmd_dev->properties.controls_hints[2] = OHMD_HOME; // Oculus button
		ohmd_dev->properties.controls_hints[3] = OHMD_ANALOG_PRESS; // stick button
	} else {
		ohmd_dev->properties.controls_hints[0] = OHMD_BUTTON_X;
		ohmd_dev->properties.controls_hints[1] = OHMD_BUTTON_Y;
		ohmd_dev->properties.controls_hints[2] = OHMD_MENU;
		ohmd_dev->properties.controls_hints[3] = OHMD_ANALOG_PRESS; // stick button
	}
	ohmd_dev->properties.controls_hints[4] = OHMD_TRIGGER;
	ohmd_dev->properties.controls_hints[5] = OHMD_SQUEEZE;
	ohmd_dev->properties.controls_hints[6] = OHMD_ANALOG_X;
	ohmd_dev->properties.controls_hints[7] = OHMD_ANALOG_Y;

	ohmd_dev->properties.controls_types[0] = OHMD_DIGITAL;
	ohmd_dev->properties.controls_types[1] = OHMD_DIGITAL;
	ohmd_dev->properties.controls_types[2] = OHMD_DIGITAL;
	ohmd_dev->properties.controls_types[3] = OHMD_DIGITAL;

	ohmd_dev->properties.controls_types[4] = OHMD_ANALOG;
	ohmd_dev->properties.controls_types[5] = OHMD_ANALOG;
	ohmd_dev->properties.controls_types[6] = OHMD_ANALOG;
	ohmd_dev->properties.controls_types[7] = OHMD_ANALOG;
}

static rift_s_hmd_t *open_hmd(ohmd_driver* driver, ohmd_device_desc* desc)
{
	const int interfaces[3] = {
			RIFT_S_INTF_HMD,
			RIFT_S_INTF_STATUS,
			RIFT_S_INTF_CONTROLLERS,
	};
	hid_device *hid = NULL;
	rift_s_hmd_t* priv = ohmd_alloc(driver->ctx, sizeof(rift_s_hmd_t));
	rift_s_device_priv *hmd_dev;
	if(!priv)
		goto cleanup;

	hmd_dev = &priv->hmd_dev;

	priv->use_count = 1;
	priv->ctx = driver->ctx;

	priv->last_imu_timestamp = -1;

	// Open the HID devices
	for (int i = 0; i < 3; i++) {
		priv->handles[i] = open_hid_dev (driver->ctx, OCULUS_VR_INC_ID, RIFT_S_PID, interfaces[i]);
		if (priv->handles[i] == NULL)
			goto cleanup;
	}
	hid = priv->handles[0];

	rift_s_radio_state_init (&priv->radio_state, driver->ctx);

	if (rift_s_read_device_info (hid, &priv->device_info) < 0) {
			LOGE("Failed to read Rift S device info");
			goto cleanup;
	}

	if (rift_s_get_report1 (hid) < 0) {
			LOGE("Failed to read Rift S Report 1");
			goto cleanup;
	}

	if (rift_s_read_imu_config (hid, &priv->imu_config) < 0) {
			LOGE("Failed to read IMU configuration block");
			goto cleanup;
	}

	if (read_calibration (priv, hid) < 0)
			goto cleanup;

#if 0
	dump_fw_block(hid, 0xB);
	dump_fw_block(hid, 0xD);
	dump_fw_block(hid, 0xF);
	dump_fw_block(hid, 0x10);
	dump_fw_block(hid, 0x12);
#endif

	// Set default device properties
	ohmd_set_default_device_properties(&hmd_dev->base.properties);

	/* FIXME: These defaults should be replaced from device configuration */
	hmd_dev->base.properties.hsize = 0.149760f;
	hmd_dev->base.properties.vsize = 0.093600f;
	hmd_dev->base.properties.lens_sep = 0.074f;
	hmd_dev->base.properties.lens_vpos = 0.046800f;
	hmd_dev->base.properties.fov = DEG_TO_RAD(105);

	/* FIXME: Incorrection distortion taken from the Rift CV1 for now */
#if 1
  ohmd_set_universal_distortion_k(&(hmd_dev->base.properties), 0.098f, .324f, -0.241f, 0.819f);
	ohmd_set_universal_aberration_k(&(hmd_dev->base.properties), 0.9952420f, 1.0f, 1.0008074f);
#else
  /* First pass at manual calibration */
	double scale = 1.31;
	double a = 0.049582 * scale, b = 0.221123 * scale, c = -0.174273 * scale;
	double d = 1.0 - (a+b+c);
	ohmd_set_universal_distortion_k(&(hmd_dev->base.properties), a, b, c, d);
	ohmd_set_universal_aberration_k(&(hmd_dev->base.properties), 0.99043452, 1.0, 1.0073939);
#endif

	hmd_dev->base.properties.hres = priv->device_info.h_resolution;
	hmd_dev->base.properties.vres = priv->device_info.v_resolution;

#if 0
	hmd_dev->base.properties.hsize = priv->device_info.h_screen_size;
	hmd_dev->base.properties.vsize = priv->device_info.v_screen_size;
	hmd_dev->base.properties.lens_sep = priv->device_info.lens_separation;
	hmd_dev->base.properties.lens_vpos = priv->device_info.v_center;
#endif
	hmd_dev->base.properties.ratio = ((float)priv->device_info.h_resolution / (float)priv->device_info.v_resolution) / 2.0f;

	ohmd_calc_default_proj_matrices(&hmd_dev->base.properties);

	hmd_dev->id = 0;
	hmd_dev->hmd = priv;

	// initialize sensor fusion
	ofusion_init(&priv->sensor_fusion);

	// Init touch devices 
	for (int i = 0; i < MAX_CONTROLLERS; i++)
		init_touch_device (priv->touch_dev + i, i);

	if (rift_s_hmd_enable (hid, true) < 0) {
			LOGE("Failed to enable Rift S");
			goto cleanup;
	}

	return priv;

cleanup:
	if (priv)
		close_hmd (priv);
	return NULL;
}

static void close_hmd(rift_s_hmd_t *hmd)
{
	rift_s_radio_state_clear (&hmd->radio_state);

	if (hmd->handles[0]) {
		if (rift_s_hmd_enable (hmd->handles[0], true) < 0) {
				LOGW("Failed to disable Rift S");
		}
	}

	for (int i = 0; i < 3; i++) {
		if (hmd->handles[i])
			hid_close(hmd->handles[i]);
	}
	free(hmd);
}

/* FIXME: This opens the first device that matches the
 * requested VID/PID/interface, which works fine if there's
 * 1 rift attached. To support multiple rift, we need to
 * match parent USB devices like ouvrt does */
static hid_device* open_hid_dev(ohmd_context* ctx,
		int vid, int pid, int iface_num)
{
	struct hid_device_info* devs = hid_enumerate(vid, pid);
	struct hid_device_info* cur_dev = devs;
	hid_device *handle = NULL;

	if(devs == NULL)
		return NULL;

	while (cur_dev) {
		if (cur_dev->interface_number == iface_num) {
			handle = hid_open_path(cur_dev->path);
			if (handle)
				break;
			else {
				char* path = _hid_to_unix_path(cur_dev->path);
				ohmd_set_error(ctx, "Could not open %s.\n"
											 "Check your permissions: "
											 UDEV_WIKI_URL, path);
				free(path);
			}
		}
		cur_dev = cur_dev->next;
	}
	hid_free_enumeration(devs);

	if (handle) {
		if(hid_set_nonblocking(handle, 1) == -1){
			ohmd_set_error(ctx, "Failed to set non-blocking mode on USB device");
			goto cleanup;
		}
	}

	return handle;
cleanup:
	hid_close(handle);
	return NULL;
}

static ohmd_device* open_device(ohmd_driver* driver, ohmd_device_desc* desc)
{
	rift_s_device_priv *dev = NULL;
	rift_s_hmd_t *hmd = find_hmd(desc->path);

	if (hmd == NULL) {
		hmd = open_hmd (driver, desc);
		if (hmd == NULL)
			return NULL;
		push_hmd (hmd, desc->path);
	}

	if (desc->id == 0)
		dev = &hmd->hmd_dev;
	else if (desc->id <= MAX_CONTROLLERS) {
		dev = &hmd->touch_dev[desc->id-1].base;
	}
	else {
		LOGE ("Invalid device description passed to open_device()");
		release_hmd (hmd);
		return NULL;
	}

	// set up device callbacks
	dev->hmd = hmd;
	dev->id = desc->id;
	dev->opened = true;

	dev->base.update = update_device;
	dev->base.close = close_device;
	if (desc->id == 0)
		dev->base.getf = getf_hmd;
	else
		dev->base.getf = getf_touch_controller;

	return &dev->base;
}

static void get_device_list(ohmd_driver* driver, ohmd_device_list* list)
{
	// enumerate HID devices and add any Rift S devices found to the device list
	rift_devices rd[] = {
		{ "Rift S", OCULUS_VR_INC_ID, RIFT_S_PID,	RIFT_S_INTF_HMD },
	};
	const int RIFT_ID_COUNT = sizeof(rd) / sizeof(rd[0]);

	for(int i = 0; i < RIFT_ID_COUNT; i++){
		struct hid_device_info* devs = hid_enumerate(rd[i].company, rd[i].id);
		struct hid_device_info* cur_dev = devs;

		if(devs == NULL)
			continue;

		while (cur_dev) {
			if(rd[i].iface == -1 || cur_dev->interface_number == rd[i].iface) {
				int id = 0;
				ohmd_device_desc* desc = &list->devices[list->num_devices++];

				strcpy(desc->driver, "OpenHMD Rift Driver");
				strcpy(desc->vendor, "Oculus VR, Inc.");
				strcpy(desc->product, rd[i].name);

				desc->revision = 0;
		
				desc->device_class = OHMD_DEVICE_CLASS_HMD;
				desc->device_flags = OHMD_DEVICE_FLAGS_ROTATIONAL_TRACKING;

				strcpy(desc->path, cur_dev->path);

				desc->driver_ptr = driver;
				desc->id = id++;

				//Controller 0 (left)
				desc = &list->devices[list->num_devices++];
				desc->revision = 0;

				strcpy(desc->driver, "OpenHMD Rift Driver");
				strcpy(desc->vendor, "Oculus VR, Inc.");
				sprintf(desc->product, "%s: Left Controller", rd[i].name);

				strcpy(desc->path, cur_dev->path);

				desc->device_flags =
					//OHMD_DEVICE_FLAGS_POSITIONAL_TRACKING |
					OHMD_DEVICE_FLAGS_ROTATIONAL_TRACKING |
					OHMD_DEVICE_FLAGS_LEFT_CONTROLLER;

				desc->device_class = OHMD_DEVICE_CLASS_CONTROLLER;
				desc->driver_ptr = driver;
				desc->id = id++;

				// Controller 1 (right)
				desc = &list->devices[list->num_devices++];
				desc->revision = 0;

				strcpy(desc->driver, "OpenHMD Rift Driver");
				strcpy(desc->vendor, "Oculus VR, Inc.");
				sprintf(desc->product, "%s: Right Controller", rd[i].name);

				strcpy(desc->path, cur_dev->path);

				desc->device_flags =
					//OHMD_DEVICE_FLAGS_POSITIONAL_TRACKING |
					OHMD_DEVICE_FLAGS_ROTATIONAL_TRACKING |
					OHMD_DEVICE_FLAGS_RIGHT_CONTROLLER;

				desc->device_class = OHMD_DEVICE_CLASS_CONTROLLER;
				desc->driver_ptr = driver;
				desc->id = id++;
			}
			cur_dev = cur_dev->next;
		}

		hid_free_enumeration(devs);
	}
}

static void destroy_driver(ohmd_driver* drv)
{
	LOGD("shutting down driver");
	hid_exit();
	free(drv);

	ohmd_toggle_ovr_service(1); //re-enable OVRService if previously running
}

ohmd_driver* ohmd_create_oculus_rift_s_drv(ohmd_context* ctx)
{
	ohmd_driver* drv = ohmd_alloc(ctx, sizeof(ohmd_driver));
	if(drv == NULL)
		return NULL;

	ohmd_toggle_ovr_service(0); //disable OVRService if running

	drv->get_device_list = get_device_list;
	drv->open_device = open_device;
	drv->destroy = destroy_driver;
	drv->ctx = ctx;

	return drv;
}
