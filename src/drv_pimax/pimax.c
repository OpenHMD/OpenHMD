/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

#include <stdlib.h>
#include <hidapi.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>

#include "pimax.h"
#include "../hid.h"

#define OHMD_GRAVITY_EARTH 9.80665 // m/s²

#define TICK_LEN (1.0f / 1000000.0f) // 1000 Hz ticks
#define KEEP_ALIVE_VALUE (10 * 1000)
#define SETFLAG(_s, _flag, _val) (_s) = ((_s) & ~(_flag)) | ((_val) ? (_flag) : 0)

#define DEVICE_ID					0x0483
#define DEVICE_HMD					0x0021

typedef struct {
    ohmd_device base;

    hid_device* handle;
    pkt_sensor_range sensor_range;
    pkt_sensor_display_info display_info;
    pkt_sensor_config sensor_config;
    pkt_tracker_sensor sensor;
    double last_keep_alive;
    uint32_t last_imu_timestamp;
    fusion sensor_fusion;
    vec3f raw_mag, raw_accel, raw_gyro;
} drv_priv;

static drv_priv* drv_priv_get(ohmd_device* device)
{
    return (drv_priv*)device;
}

static int get_feature_report(drv_priv* priv, drv_sensor_feature_cmd cmd, unsigned char* buf)
{
    memset(buf, 0, FEATURE_BUFFER_SIZE);
    buf[0] = (unsigned char)cmd;
    return hid_get_feature_report(priv->handle, buf, FEATURE_BUFFER_SIZE);
}

static int send_feature_report(drv_priv* priv, const unsigned char *data, size_t length)
{
    return hid_send_feature_report(priv->handle, data, length);
}

static void handle_tracker_sensor_msg(drv_priv* priv, unsigned char* buffer, int size)
{
    uint32_t last_sample_tick = priv->sensor.tick;

    if(!pm_decode_tracker_sensor_msg(&priv->sensor, buffer, size)){
        LOGE("couldn't decode tracker sensor message");
    }

    pkt_tracker_sensor* s = &priv->sensor;

    pm_dump_packet_tracker_sensor(s);

    uint32_t tick_delta = 1000;
    if(last_sample_tick > 0) //startup correction
        tick_delta = s->tick - last_sample_tick;

    // TODO: handle overflows in a nicer way
    float dt = TICK_LEN;	// TODO: query the Rift for the sample rate
    if (s->timestamp > priv->last_imu_timestamp) {
        dt = (s->timestamp - priv->last_imu_timestamp) / 1000000.0f;
        dt -= (s->num_samples - 1) * TICK_LEN;	// TODO: query the Pimax for the sample rate
    }

    // TODO: find a use for these values. got some of
    //       these values whilst reverse engineering
    //       but currently don't have a use for them.
    const float accel_scale = OHMD_GRAVITY_EARTH / 4;
    const float gyro_scale = 1.0 / 2000;
    const float mag_scale = 1.0 / 49120;

    int32_t mag32[] = { s->mag[0], s->mag[1], s->mag[2] };
    vec3f_from_mag(mag32, &priv->raw_mag);

    // TODO: put this somewhere nice and maybe configurable
    //       these are the magic numbers from one manual calibration run
    //       with the python scripts from OpenHmdPimaxTools
    const float gyro_offset[] = { 0.0029455, -0.0009648000000000001, -0.002771 };

    for(int i = 0; i < s->num_samples; i++){ // just use 1 sample since we don't have sample order for this frame
        vec3f_from_accel(s->samples[i].accel, &priv->raw_accel);
        vec3f_from_gryo(s->samples[i].gyro, &priv->raw_gyro, gyro_offset);

        ofusion_update(&priv->sensor_fusion, dt, &priv->raw_gyro, &priv->raw_accel, &priv->raw_mag);

        // reset dt to tick_len for the last samples if there were more than one sample
        dt = TICK_LEN;	// TODO: query the Pimax for the sample rate
    }

    priv->last_imu_timestamp = s->timestamp;

}

#define WRITE8(_val) *(buffer++) = (_val);
static int encode_pimax_cmd_17(unsigned char *buffer)
{
    WRITE8(0x11);
    WRITE8(0x00);
    WRITE8(0x00);
    WRITE8(0x0b);
    WRITE8(0x10);
    WRITE8(0x27);
    return 6;
}


static void update_device(ohmd_device* device)
{
    int size = 0;

    drv_priv* priv = drv_priv_get(device);
    unsigned char buffer[FEATURE_BUFFER_SIZE];

    double t = ohmd_get_tick();
    // Keep alive interval hardcoded to 3 sec
    if (t - priv->last_keep_alive >= 3) {
        int size = encode_pimax_cmd_17(buffer);
        if (send_feature_report(priv, buffer, size) == -1) {
            LOGE("error setting up cmd17");
        } else {
            LOGI("OK1");
        }
        priv->last_keep_alive = t;
    }

    while((size = hid_read(priv->handle, buffer, FEATURE_BUFFER_SIZE)) > 0) {
        // currently the only message type the hardware supports (I think)
        if(buffer[0] == 11){
            handle_tracker_sensor_msg(priv, buffer, size);
        }else{
            LOGE("unknown message type: %u", buffer[0]);
        }
    }
}

static int getf(ohmd_device* device, ohmd_float_value type, float* out)
{
    drv_priv* priv = drv_priv_get(device);

    switch(type){
        case OHMD_DISTORTION_K: {
                                    for (int i = 0; i < 6; i++) {
                                        out[i] = priv->display_info.distortion_k[i];
                                    }
                                    break;
                                }

        case OHMD_ROTATION_QUAT: {
                                     *(quatf*)out = priv->sensor_fusion.orient;
                                     break;
                                 }

        case OHMD_POSITION_VECTOR:
                                 out[0] = out[1] = out[2] = 0;
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
    LOGD("closing device");
    drv_priv* priv = drv_priv_get(device);
    hid_close(priv->handle);
    free(priv);
}

static ohmd_device* open_device(ohmd_driver* driver, ohmd_device_desc* desc)
{
    drv_priv* priv = ohmd_alloc(driver->ctx, sizeof(drv_priv));
    if(!priv)
        goto cleanup;

    priv->base.ctx = driver->ctx;

    // Open the HID device
    priv->handle = hid_open_path(desc->path);

    if(!priv->handle) {
        char* path = _hid_to_unix_path(desc->path);
        ohmd_set_error(driver->ctx, "Could not open %s. "
                "Check your rights.", path);
        free(path);
        goto cleanup;
    }

    if(hid_set_nonblocking(priv->handle, 1) == -1){
        ohmd_set_error(driver->ctx, "failed to set non-blocking on device");
        goto cleanup;
    }

    unsigned char buf[FEATURE_BUFFER_SIZE];

    int size;

    // Update the time of the last keep alive we have sent.
    priv->last_keep_alive = ohmd_get_tick();

    // Set default device properties
    ohmd_set_default_device_properties(&priv->base.properties);

    // Set device properties
    //NOTE: These values are estimations, no one has taken one appart to check
    priv->base.properties.hsize = 0.1698f;
    priv->base.properties.vsize = 0.0936f;
    priv->base.properties.hres = 1920;
    priv->base.properties.vres = 1080;
    priv->base.properties.lens_sep = 0.0849f;
    priv->base.properties.lens_vpos = 0.0468f;;
    priv->base.properties.fov = DEG_TO_RAD(110.0); // TODO calculate.
    priv->base.properties.ratio = ((float)1920 / (float)1080) / 2.0f;

    // taken from the 3glasses, almost certainly not accurate
    ohmd_set_universal_distortion_k(&(priv->base.properties), 0.75239515, -0.84751135, 0.42455423, 0.66200626);

    // manually dialed in, may not be perfect
    ohmd_set_universal_aberration_k(&(priv->base.properties), 0.995, 1.000, 1.005);

    // calculate projection eye projection matrices from the device properties
    ohmd_calc_default_proj_matrices(&priv->base.properties);

    // set up device callbacks
    priv->base.update = update_device;
    priv->base.close = close_device;
    priv->base.getf = getf;

    // initialize sensor fusion
    ofusion_init(&priv->sensor_fusion);

    return &priv->base;

cleanup:
    if(priv)
        free(priv);

    return NULL;
}

static void get_device_list(ohmd_driver* driver, ohmd_device_list* list)
{
    struct hid_device_info* devs = hid_enumerate(DEVICE_ID, DEVICE_HMD);
    struct hid_device_info* cur_dev = devs;

    while (cur_dev) {
        ohmd_device_desc* desc = &list->devices[list->num_devices++];

        strcpy(desc->driver, "OpenHMD PIMAX Driver");
        strcpy(desc->vendor, "PIMAX");
        strcpy(desc->product, "PIMAX 4K");

        desc->revision = 0;
        desc->device_class = OHMD_DEVICE_CLASS_HMD;
        desc->device_flags = OHMD_DEVICE_FLAGS_ROTATIONAL_TRACKING;

        strcpy(desc->path, cur_dev->path);

        desc->driver_ptr = driver;

        cur_dev = cur_dev->next;
    }

    hid_free_enumeration(devs);
}

static void destroy_driver(ohmd_driver* drv)
{
    LOGD("shutting down driver");
    hid_exit();
    free(drv);
}

ohmd_driver* ohmd_create_pimax_drv(ohmd_context* ctx)
{
    ohmd_driver* drv = ohmd_alloc(ctx, sizeof(ohmd_driver));
    if(drv == NULL)
        return NULL;

    drv->get_device_list = get_device_list;
    drv->open_device = open_device;
    drv->ctx = ctx;
    drv->get_device_list = get_device_list;
    drv->open_device = open_device;
    drv->destroy = destroy_driver;

    return drv;
}

