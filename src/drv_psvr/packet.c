#include "psvr.h"

inline static uint8_t read8(const unsigned char** buffer)
{
	uint8_t ret = **buffer;
	*buffer += 1;
	return ret;
}

inline static int16_t read16(const unsigned char** buffer)
{
	int16_t ret = **buffer | (*(*buffer + 1) << 8);
	*buffer += 2;
	return ret;
}

inline static uint32_t read32(const unsigned char** buffer)
{
	uint32_t ret = **buffer | (*(*buffer + 1) << 8) | (*(*buffer + 2) << 16) | (*(*buffer + 3) << 24);
	*buffer += 4;
	return ret;
}

bool psvr_decode_sensor_packet(psvr_sensor_packet* pkt, const unsigned char* buffer, int size)
{
	if(size != 64){
		LOGE("invalid psvr sensor packet size (expected 64 but got %d)", size);
		return false;
	}
	int16_t accel[3];
	int16_t gyro[3];

	buffer += 2;
	uint16_t volume = read16(&buffer); //volume
	buffer += 12; //uknown, skip 12
	uint32_t tick = read32(&buffer); //TICK
	// acceleration
	for(int i = 0; i < 3; i++){
		pkt->samples[0].gyro[i] = read16(&buffer);
	}

	// rotation
	for(int i = 0; i < 3; i++){
		pkt->samples[0].accel[i] = read16(&buffer);
	}
/*
	printf("JOEDEBUG - Tick = %u\n", tick);
	printf("JOEDEBUG - Accel = %d %d %d\n", accel[0], accel[1], accel[2]);
	printf("JOEDEBUG - Gyro = %d %d %d\n", gyro[0], gyro[1], gyro[2]);*/
	return true;

/*
	for(int j = 0; j < 3; j++){
		// acceleration
		for(int i = 0; i < 3; i++){
			pkt->samples[j].acc[i] = read16(&buffer);
		}

		// rotation
		for(int i = 0; i < 3; i++){
			pkt->samples[j].rot[i] = read16(&buffer);
		}

		pkt->samples[j].time_ticks = read32(&buffer);
		pkt->samples[j].seq = read8(&buffer);
	}

	return true;*/
}