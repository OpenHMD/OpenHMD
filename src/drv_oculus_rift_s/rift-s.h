/*
 * Copyright 2013, Fredrik Hultin.
 * Copyright 2013, Jakob Bornecrantz.
 * Copyright 2016 Philipp Zabel
 * Copyright 2019-2020 Jan Schmidt
 * SPDX-License-Identifier: BSL-1.0
 *
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 */

/* Oculus Rift S Driver Internal Interface */
#ifndef RIFT_S_H
#define RIFT_S_H

#include "../openhmdi.h"

typedef struct rift_s_device_priv_s rift_s_device_priv;
typedef struct rift_s_hmd_s rift_s_hmd_t;
typedef struct rift_s_controller_device_s rift_s_controller_device;

#define OHMD_GRAVITY_EARTH 9.80665 // m/s²

struct rift_s_device_priv_s {
	ohmd_device base;
	int id;
	bool opened;

	rift_s_hmd_t *hmd;
};

struct rift_s_controller_device_s {
	rift_s_device_priv base;

	/* Index into the controllers state array in the hmd_t struct, or -1 not found yet */
	int device_num;
};

#endif
