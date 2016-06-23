#include "vive.h"

inline static uint8_t read8(const unsigned char** buffer)
{
	return *(*buffer++);
}

inline static int16_t read16(const unsigned char** buffer)
{
	int16_t ret = **buffer | (*(*buffer + 1) << 8);
	*buffer += 2;
	return ret;
}

inline static int32_t read32(const unsigned char** buffer)
{
	int32_t ret = **buffer | (*(*buffer + 1) << 8) | (*(*buffer + 1) << 16) | (*(*buffer + 1) << 24);
	*buffer += 4;
	return ret;
}

bool vive_decode_sensor_packet(vive_sensor_packet* pkt, const unsigned char* buffer, int size)
{
	if(size != 52){
		LOGE("invalid vive sensor packet size (expected 52 but got %d)", size);
		return false;
	}

	pkt->report_id = read8(&buffer);

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

	return true;
}
