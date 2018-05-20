#include "vive.h"

#include "../ext_deps/miniz.h"
#include "../ext_deps/nxjson.h"

#ifdef _MSC_VER
#define inline __inline
#endif

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

bool vive_decode_sensor_packet(vive_headset_imu_packet* pkt, const unsigned char* buffer, int size)
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

//Trim function for removing tabs and spaces from string buffers
void trim(const char* src, char* buff, const unsigned int sizeBuff)
{
    if(sizeBuff < 1)
    return;

    const char* current = src;
    unsigned int i = 0;
    while(*current != '\0' && i < sizeBuff-1)
    {
        if(*current != ' ' && *current != '\t' && *current != '\n')
            buff[i++] = *current;
        ++current;
    }
    buff[i] = '\0';
}

void get_vec3f_from_json(const nx_json* json, const char* name, vec3f* result)
{
	const nx_json* acc_bias_arr = nx_json_get(json, name);

	for (int i = 0; i < acc_bias_arr->length; i++) {
		const nx_json* item = nx_json_item(acc_bias_arr, i);
		result->arr[i] = (float) item->dbl_value;
	}
}

bool vive_decode_config_packet(vive_config_packet* pkt,
                               const unsigned char* buffer,
                               uint16_t size)
{
	/*
	if(size != 4069){
		LOGE("invalid vive sensor packet size (expected 4069 but got %d)", size);
		return false;
	}*/

	pkt->report_id = VIVE_CONFIG_READ_PACKET_ID;
	pkt->length = size;

	unsigned char output[32768];
	mz_ulong output_size = 32768;

	//int cmp_status = uncompress(pUncomp, &uncomp_len, pCmp, cmp_len);
	int cmp_status = uncompress(output, &output_size,
	                            buffer, (mz_ulong)pkt->length);
	if (cmp_status != Z_OK){
		LOGE("invalid vive config, could not uncompress");
		return false;
	}

	LOGE("Decompressed from %u to %u bytes\n",
	     (mz_uint32)pkt->length, (mz_uint32)output_size);

	//LOGD("Debug print all the RAW JSON things!\n%s", output);
	//pUncomp should now be the uncompressed data, lets get the json from it
	/** DEBUG JSON PARSER CODE **/
	trim((char*)output, (char*)output, (unsigned int)output_size);
	//LOGD("%s\n",output);
	/*
	FILE* dfp;
	dfp = fopen("jsondebug.json","w");
	json_enable_debug(3, dfp);*/
	const nx_json* json = nx_json_parse((char*)output, 0);

	vec3f acc_bias;
	vec3f acc_scale;
	vec3f gyro_bias;
	vec3f gyro_scale;

	if (json) {
		get_vec3f_from_json(json, "acc_bias", &acc_bias);
		get_vec3f_from_json(json, "acc_scale", &acc_scale);
		get_vec3f_from_json(json, "gyro_bias", &gyro_bias);
		get_vec3f_from_json(json, "gyro_scale", &gyro_scale);

		nx_json_free(json);

		LOGI("\n--- Converted Vive JSON Data ---\n\n");
		LOGI("acc_bias = %f %f %f\n", acc_bias.x, acc_bias.y, acc_bias.z);
		LOGI("acc_scale = %f %f %f\n", acc_scale.x, acc_scale.y, acc_scale.z);
		LOGI("gyro_bias = %f %f %f\n", gyro_bias.x, gyro_bias.y, gyro_bias.z);
		LOGI("gyro_scale = %f %f %f\n", gyro_scale.x, gyro_scale.y, gyro_scale.z);
		LOGI("\n--- End of Vive JSON Data ---\n\n");
	} else {
		LOGE("Could not parse JSON data.\n");
		return false;
	}

	/** END OF DEBUG JSON PARSER CODE **/

//	free(pCmp);
//	free(pUncomp);

	return true;
}
