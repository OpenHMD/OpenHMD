/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2019 Anorak.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* Relativ HMD Driver */

//Note: Code only supported on the STM32 tracker
//Using as code base: https://github.com/der-b/OpenHMD/tree/master/src/drv_sparkfun_9dof

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <hidapi/hidapi.h>
#include <openhmdi.h>
#define VENDOR_ID 0x0483                // STMicroelectronics VID
#define PRODUCT_ID 0x5740 // STMicroelectronics Virtual COM Port PID
//Taked from Justin's code
#define DEVICE_NAME "Relativ HMD"

struct quater
{
    float w;
    float x;
    float y;
    float z;
};

typedef struct
{
    ohmd_device base;
    hid_device *handle;
} relativ_priv;

static void update_device(ohmd_device *device)
{
//Defining the previous var as the Relativ HMD
    relativ_priv *priv = (relativ_priv *)device;
    int size;
    struct quater data;

    if (!priv->handle)
    {
        return;
    }

    while (0 < (size = hid_read(priv->handle, (void *)&data, sizeof(data))))
    {
        priv->base.rotation.x = data.x;
        priv->base.rotation.y = data.y;
        priv->base.rotation.z = data.z;
        priv->base.rotation.w = data.w;
    }
    printf(" %f %f %f %f\n", data.x, data.y, data.z, data.w);

}
static int getf(ohmd_device *device, ohmd_float_value type, float *out)
{
    relativ_priv *priv = (relativ_priv *)device;

    switch (type)
    {
    case OHMD_ROTATION_QUAT:
        *(quatf *)out = priv->base.rotation;
        break;
    case OHMD_POSITION_VECTOR:
        out[0] = out[1] = out[2] = 0;
        break;
    case  OHMD_DISTORTION_K:
        memset(out, 0, sizeof(float) * 6);
        break;
    case OHMD_CONTROLS_STATE:
        out[0] = .1f;
        out[1] = 1.0f;
        break;
    default:
        ohmd_set_error(priv->base.ctx, "invalid type given to getf (%ud)", type);
        return OHMD_S_INVALID_PARAMETER;
        break;
    }
    return OHMD_S_OK;
}

static void close_device(ohmd_device *device)
{
    relativ_priv *priv = (relativ_priv *)device;
    LOGD("closing relativ device");
    if (!priv->handle)
    {
        hid_close(priv->handle);
        priv->handle = NULL;
    }

    free(device);
}

static ohmd_device* open_device(ohmd_driver* driver, ohmd_device_desc* desc)
{
	relativ_priv* priv = ohmd_alloc(driver->ctx, sizeof(relativ_priv));

	if(!priv)
		return NULL;

	priv->handle = hid_open_path(desc->path);
	if (!priv->handle) {
		LOGD("Clould not open directory '" DEVICE_NAME "'.");
		goto err;
	}

	if(hid_set_nonblocking(priv->handle, 1) == -1){
		LOGD("Clould not set nonblocking on '" DEVICE_NAME "'.");
		goto err_handle;
	}

	// Set default device properties
	ohmd_set_default_device_properties(&priv->base.properties);

	// Set device properties (imitates the rift values)
	priv->base.properties.hsize = 0.128490f;
	priv->base.properties.vsize = 0.070940f;
	priv->base.properties.hres = 1440;
	priv->base.properties.vres = 2560;
	priv->base.properties.lens_sep = 0.063500f;
	priv->base.properties.lens_vpos = 0.070940f / 2;
	priv->base.properties.fov = DEG_TO_RAD(103.0f);
	priv->base.properties.ratio = (2560.0f / 1440.0f) / 2.0f;
	
	// calculate projection eye projection matrices from the device properties
	ohmd_calc_default_proj_matrices(&priv->base.properties);

	// set up device callbacks
	priv->base.update = update_device;
	priv->base.close = close_device;
	priv->base.getf = getf;
	
	return (ohmd_device*)priv;
err_handle:
        hid_close(priv->handle);
        priv->handle = NULL;
err:
        free(priv);
        return NULL;
}

static void get_device_list(ohmd_driver* driver, ohmd_device_list* list)
{
	int id = 0;
	struct hid_device_info* devs = hid_enumerate(VENDOR_ID,PRODUCT_ID);
	struct hid_device_info* cur_dev = devs;
	ohmd_device_desc* desc;

	// HMD
        while(cur_dev) {
                printf("id: %d\n", id);
                desc = &list->devices[list->num_devices++];

                strcpy(desc->driver, "Relativty Driver");
                strcpy(desc->vendor, "Relativty");
                strcpy(desc->product, "Relativty Device");

                strncpy(desc->path, cur_dev->path, OHMD_STR_SIZE);

                desc->driver_ptr = driver;

                desc->device_flags = OHMD_DEVICE_FLAGS_ROTATIONAL_TRACKING;
                desc->device_class = OHMD_DEVICE_CLASS_HMD;

                desc->id = id++;
		cur_dev = cur_dev->next;
        }

	hid_free_enumeration(devs);
}

static void destroy_driver(ohmd_driver* drv)
{
	LOGD("shutting down " DEVICE_NAME " driver");
	free(drv);
}

ohmd_driver* ohmd_create_relativty6dof_drv(ohmd_context* ctx)
{
	ohmd_driver* drv = ohmd_alloc(ctx, sizeof(ohmd_driver));
	if(!drv)
		return NULL;

	drv->get_device_list = get_device_list;
	drv->open_device = open_device;
	drv->destroy = destroy_driver;

	return drv;
}








