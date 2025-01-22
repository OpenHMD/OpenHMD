/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* New-Driver template Driver - Packet Decoding and Utilities */

#include <stdio.h>
#include "pimax.h"

#define SKIP_CMD (buffer++)
#define READ8 *(buffer++);
#define READ16 *buffer | (*(buffer + 1) << 8); buffer += 2;
#define READ32 *buffer | (*(buffer + 1) << 8) | (*(buffer + 2) << 16) | (*(buffer + 3) << 24); buffer += 4;
#define READFLOAT ((float)(*buffer)); buffer += 4;
#define READFIXED (float)(*buffer | (*(buffer + 1) << 8) | (*(buffer + 2) << 16) | (*(buffer + 3) << 24)) / 1000000.0f; buffer += 4;

#define WRITE8(_val) *(buffer++) = (_val);
#define WRITE16(_val) WRITE8((_val) & 0xff); WRITE8(((_val) >> 8) & 0xff);
#define WRITE32(_val) WRITE16((_val) & 0xffff) *buffer; WRITE16(((_val) >> 16) & 0xffff);

bool decodesensor_range(pkt_sensor_range* range, const unsigned char* buffer, int size)
{
    if(!(size == 8 || size == 9)){
        LOGE("invalid packet size (expected 8 or 9 but got %d)", size);
        return false;
    }

    SKIP_CMD;
    range->command_id = READ16;
    range->accel_scale = READ8;
    range->gyro_scale = READ16;
    range->mag_scale = READ16;

    return true;
}

bool decodesensor_display_info(pkt_sensor_display_info* info, const unsigned char* buffer, int size)
{
    if(!(size == 56 || size == 57)){
        LOGE("invalid packet size (expected 56 or 57 but got %d)", size);
        //return false;
    }

    SKIP_CMD;
    info->command_id = READ16;
    info->distortion_type = READ8;
    info->h_resolution = READ16;
    info->v_resolution = READ16;
    info->h_screen_size = READFIXED;
    info->v_screen_size = READFIXED;
    info->v_center = READFIXED;
    info->lens_separation = READFIXED;
    info->eye_to_screen_distance[0] = READFIXED;
    info->eye_to_screen_distance[1] = READFIXED;

    info->distortion_type_opts = 0;

    for(int i = 0; i < 6; i++)
        info->distortion_k[i] = READFLOAT;

    return true;
}

//For use with packing
static void decodesample(const unsigned char* buffer, int32_t* smp)
{
    /*
     * Decode 3 tightly packed 21 bit values from 4 bytes.
     * We unpack them in the higher 21 bit values first and then shift
     * them down to the lower in order to get the sign bits correct.
     */

    int x = (buffer[0] << 24)          | (buffer[1] << 16) | ((buffer[2] & 0xF8) << 8);
    int y = ((buffer[2] & 0x07) << 29) | (buffer[3] << 21) | (buffer[4] << 13) | ((buffer[5] & 0xC0) << 5);
    int z = ((buffer[5] & 0x3F) << 26) | (buffer[6] << 18) | (buffer[7] << 10);

    smp[0] = x >> 11;
    smp[1] = y >> 11;
    smp[2] = z >> 11;
}

bool pm_decode_tracker_sensor_msg(pkt_tracker_sensor* msg, const unsigned char* buffer, int size)
{
    if(!(size == 62 || size == 64)){
        LOGE("invalid packet size (expected 62 or 64 but got %d)", size);
        return false;
    }

    SKIP_CMD;
    msg->last_command_id = READ16;
    msg->num_samples = READ8;
    /* Next is the number of samples since start, excluding the samples
       contained in this packet */
    buffer += 2;		// unused: nb_samples_since_start
    msg->temperature = READ16;
    msg->timestamp = READ32;
    LOGI("timestamp %08X", msg->timestamp);
    /* Second sample value is junk (outdated/uninitialized) value if
       num_samples < 2. */
    LOGI("samples %02X", msg->num_samples);
    msg->num_samples = OHMD_MIN(msg->num_samples, 2);
    for (int i = 0; i < msg->num_samples; i++) {
        decodesample(buffer, msg->samples[i].accel);
        buffer += 8;

        decodesample(buffer, msg->samples[i].gyro);
        buffer += 8;
    }

    // Skip empty samples
    buffer += (2 - msg->num_samples) * 16;

    for (int i = 0; i < 3; i++) {
        msg->mag[i] = READ16;
    }

    return true;
}

//const static float SCALEFACTOR = 0.000061035156f;
const static float SCALEFACTOR = 0.0001f;

void vec3f_from_gryo(const int32_t* smp, vec3f* out_vec, const float* gyro_offset)
{
    out_vec->x = ((float)smp[0] * SCALEFACTOR) - gyro_offset[0];
    out_vec->y = ((float)smp[1] * SCALEFACTOR) - gyro_offset[1];
    out_vec->z = ((float)smp[2] * SCALEFACTOR) - gyro_offset[2];
}

void vec3f_from_accel(const int32_t* smp, vec3f* out_vec)
{
    out_vec->x = (float)smp[0] * SCALEFACTOR;
    out_vec->y = (float)smp[1] * SCALEFACTOR;
    out_vec->z = (float)smp[2] * SCALEFACTOR;
}

void vec3f_from_mag(const int32_t* smp, vec3f* out_vec)
{
    out_vec->x = (float)smp[0] * SCALEFACTOR;
    out_vec->y = (float)smp[1] * SCALEFACTOR;
    out_vec->z = (float)smp[2] * SCALEFACTOR;
}

void pm_dump_packet_tracker_sensor(const pkt_tracker_sensor* sensor)
{
    (void)sensor;
    LOGI("  tick: 		%u", sensor->tick);
    for(int i = 0; i < 2; i++){
        LOGI("    accel: %d %d %d", sensor->samples[i].accel[0], sensor->samples[i].accel[1], sensor->samples[i].accel[2]);
        LOGI("    gyro:  %d %d %d", sensor->samples[i].gyro[0], sensor->samples[i].gyro[1], sensor->samples[i].gyro[2]);
        LOGI("    mag:  %d %d %d", sensor->mag[0], sensor->mag[1], sensor->mag[2]);
    }
}

