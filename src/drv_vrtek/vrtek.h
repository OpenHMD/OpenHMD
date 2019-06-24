// Copyright 2019, Mark Nelson.
// SPDX-License-Identifier: BSL-1.0
/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 */

/* VR-Tek Driver */

#ifndef VRTEK_H
#define VRTEK_H

#include <stdint.h>
#include "../openhmdi.h"

typedef struct {
    uint8_t  message_num;
    float    quaternion[4];
    double   euler[3];
    uint16_t acceleration[3];
    uint16_t gyroscope[3];
    uint16_t magnetometer[3];
} vrtek_hmd_data_t;

typedef enum {
    VRTEK_REPORT_CONTROL_OUTPUT = 0x01,    /* control to HMD */
    VRTEK_REPORT_CONTROL_INPUT  = 0x02,    /* control from HMD */
    VRTEK_REPORT_SENSOR         = 0x13     /* sensor report from HMD */
} vrtek_report_num;


int vrtek_encode_command_packet(uint8_t command_num, uint8_t command_arg,
                                uint8_t* data_buffer, uint8_t data_length,
                                uint8_t* comm_packet);
int vrtek_decode_command_packet(const uint8_t* comm_packet,
                                uint8_t* command_num, uint8_t* command_arg,
                                uint8_t* data_buffer, uint8_t* data_length);

int vrtek_decode_hmd_data_packet(const uint8_t* buffer, int size,
                                 vrtek_hmd_data_t* data);

#endif
