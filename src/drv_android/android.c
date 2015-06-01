/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Copyright (C) 2015 Joey Ferwerda
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* Android Driver */

#include <string.h>
#include "../openhmdi.h"

/* Android specific includes, should be available in the API */
#include <jni.h>
//#include <errno.h>
//#include <math.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>

#define ALOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define ALOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

struct engine {
    struct android_app* app;

    ASensorManager* sensorManager;
    const ASensor* accelerometerSensor;
    ASensorEventQueue* sensorEventQueue;
	ALooper* looper;
};
/* end of android thingy's */

typedef struct {
	ohmd_device base;
	vec3f raw_mag, raw_accel, raw_gyr;
	struct engine droidengine;
} android_priv;

static android_priv* adpriv = 0;

static void update_device(ohmd_device* device)
{
}

static int getf(ohmd_device* device, ohmd_float_value type, float* out)
{
	android_priv* priv = (android_priv*)device;

	switch(type){
	case OHMD_ROTATION_QUAT: 
		out[0] = priv->raw_accel.x;
		out[1] = priv->raw_accel.y;
		out[2] = priv->raw_accel.z;
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
		ohmd_set_error(priv->base.ctx, "invalid type given to getf (%d)", type);
		return -1;
		break;
	}

	return 0;
}

static void close_device(ohmd_device* device)
{
	LOGD("closing Android device");
	free(device);
}

static ohmd_device* open_device(ohmd_driver* driver, ohmd_device_desc* desc)
{
	android_priv* priv = ohmd_alloc(driver->ctx, sizeof(android_priv));
	if(!priv)
		return NULL;
	
	//Set android specific variables
	//struct engine engine;
	    
    //memset(&engine, 0, sizeof(engine));
    memset(&priv->droidengine, 0, sizeof(priv->droidengine));

	// Prepare to monitor accelerometer
    priv->droidengine.sensorManager = ASensorManager_getInstance();
    priv->droidengine.accelerometerSensor = ASensorManager_getDefaultSensor(priv->droidengine.sensorManager,
            ASENSOR_TYPE_ACCELEROMETER);
    priv->droidengine.sensorEventQueue = ASensorManager_createEventQueue(priv->droidengine.sensorManager,
            priv->droidengine.looper, LOOPER_ID_USER, NULL, NULL);

	// Set default device properties
	ohmd_set_default_device_properties(&priv->base.properties);

	// Set device properties
	//TODO: Get information from android about device
	//TODO: Use profile string to set default for a particular device (Durovis, VR One etc)
	priv->base.properties.hsize = 0.149760f;
	priv->base.properties.vsize = 0.093600f;
	priv->base.properties.hres = 1280;
	priv->base.properties.vres = 800;
	priv->base.properties.lens_sep = 0.063500;
	priv->base.properties.lens_vpos = 0.046800;
	priv->base.properties.fov = DEG_TO_RAD(125.5144f);
	priv->base.properties.ratio = (1280.0f / 800.0f) / 2.0f;

	// calculate projection eye projection matrices from the device properties
	ohmd_calc_default_proj_matrices(&priv->base.properties);

	// set up device callbacks
	priv->base.update = update_device;
	priv->base.close = close_device;
	priv->base.getf = getf;
	
	//set global priv for sharing with android code
	adpriv = priv;
	
	return (ohmd_device*)priv;
}

static void get_device_list(ohmd_driver* driver, ohmd_device_list* list)
{
	ohmd_device_desc* desc = &list->devices[list->num_devices++];

	strcpy(desc->driver, "OpenHMD Generic Android Driver");
	strcpy(desc->vendor, "OpenHMD");
	strcpy(desc->product, "Android Device");

	strcpy(desc->path, "(none)");

	desc->driver_ptr = driver;
}

static void destroy_driver(ohmd_driver* drv)
{
	LOGD("shutting down Android driver");
	free(drv);
}

ohmd_driver* ohmd_create_android_drv(ohmd_context* ctx)
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

void android_main(struct android_app* state) {		
		// Make sure glue isn't stripped.
	    app_dummy();

		android_priv* priv = adpriv;//(android_priv*)device;
		
		//local vars
        int ident;
        int events;
        struct android_poll_source* source;
		
		//Poll android device for sensors
		while ((ident=ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0) {

            // Process this event.
            if (source != NULL) {
                source->process(state, source);
            }

            // If a sensor has data, process it now.
            if (ident == LOOPER_ID_USER) {
                if (priv->droidengine.accelerometerSensor != NULL) {
                    ASensorEvent event;
                    
                    while (ASensorEventQueue_getEvents(priv->droidengine.sensorEventQueue, &event, 1) > 0) {
								priv->raw_accel.x = event.acceleration.x;                    			
								priv->raw_accel.y = event.acceleration.y;
                        		priv->raw_accel.z = event.acceleration.z;
                        		ALOGI("RAW accelerometer: x=%f y=%f z=%f", event.acceleration.x, event.acceleration.y, event.acceleration.z);
                    }
                }
            }
}
}
