#include "vive.h"

/* Suppress the warnings for this include, since we don't care about them for external dependencies
/* Requires at least GCC 4.6 or higher
*/
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#include "../ext_deps/miniz.c"
#include "../ext_deps/jsmn.h"
#pragma GCC diagnostic pop

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

bool vive_decode_config_packet(vive_config_packet* pkt, const unsigned char* buffer, int size)
{
	if(size != 64){
		LOGE("invalid vive sensor packet size (expected 64 but got %d)", size);
		return false;
	}

	pkt->report_id = read8(&buffer);
	pkt->length = read8(&buffer);
	pkt->config_data = read8(&buffer);

	uLong uncomp_len = pkt->length;
	uLong cmp_len = compressBound(pkt->length);

	uint8_t* pCmp = (mz_uint8 *)malloc((size_t)cmp_len);
	uint8_t* pUncomp = (mz_uint8 *)malloc((size_t)pkt->length);

	int cmp_status = uncompress(pUncomp, &uncomp_len, pCmp, cmp_len);
	if (cmp_status != Z_OK){
		LOGE("invalid vive config, could not uncompress");
		free(pCmp);
		free(pUncomp);
		return false;
	}

	LOGE("Decompressed from %u to %u bytes\n", (mz_uint32)cmp_len, (mz_uint32)uncomp_len);

	// Ensure uncompress() returned the expected data.
	if ((uncomp_len != pkt->length) || (memcmp(pUncomp, &pkt->config_data, (size_t)pkt->length))){
		LOGE("Decompression of vive config failed!\n");
		free(pCmp);
		free(pUncomp);
		return false;
	}

	//pUncomp should now be the uncompressed data, lets get the json from it
	/** DEBUG JSON PARSER CODE **/
	const char* uncompressed_vive_data = (char*)pUncomp;
	int resultCode;
	jsmn_parser p;
	jsmntok_t tokens[128]; // a number >= total number of tokens

	jsmn_init(&p);
	resultCode = jsmn_parse(&p, uncompressed_vive_data, pkt->length, tokens, 256);
	jsmntok_t key = tokens[1];
	unsigned int length = key.end - key.start;
	char keyString[length + 1];
	memcpy(keyString, &uncompressed_vive_data[key.start], length);
	keyString[length] = '\0';
	printf("Key: %s\n", keyString);
	/** END OF DEBUG JSON PARSER CODE **/

	free(pCmp);
	free(pUncomp);

	return true;
}
