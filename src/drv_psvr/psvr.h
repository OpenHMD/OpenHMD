#ifndef PSVR_H
#define PSVR_H

#include <stdint.h>
#include <stdbool.h>

#include "../openhmdi.h"

typedef enum
{
	PSVR_IRQ_SENSORS = 0
} psvr_irq_cmd;

typedef struct
{
	int16_t accel[3];
	int16_t gyro[3];
	uint32_t tick;
	uint8_t seq;
} psvr_sensor_sample;

typedef struct
{
	uint8_t report_id;
	uint32_t tick;
	psvr_sensor_sample samples[1];
} psvr_sensor_packet;

void vec3f_from_psvr_vec(const int16_t* smp, vec3f* out_vec);
bool psvr_decode_sensor_packet(psvr_sensor_packet* pkt, const unsigned char* buffer, int size);

#endif
