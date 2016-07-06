#include "vive.h"

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

void vec3f_from_vive_vec_accel(const int16_t* smp, vec3f* out_vec)
{
	float gravity = 9.81;
	out_vec->x = (float)smp[0] * (4.0*gravity/32768.0);
	out_vec->y = (float)smp[1] * (4.0*gravity/32768.0) * -1;
	out_vec->z = (float)smp[2] * (4.0*gravity/32768.0);
}

void vec3f_from_vive_vec_gyro(const int16_t* smp, vec3f* out_vec)
{
	float scaler = 8.0 / 32768.0;
	out_vec->x = (float)smp[0] * scaler;
	out_vec->y = (float)smp[1] * scaler * -1;
	out_vec->z = (float)smp[2] * scaler;
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
