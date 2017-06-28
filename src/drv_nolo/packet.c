/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* NOLO VR - Packet Decoding and Utilities */

#include <stdio.h>
#include "nolo.h"

#define SKIP_CMD (buffer++)
#define READ8 *(buffer++);
#define READ16 *buffer | (*(buffer + 1) << 8); buffer += 2;
#define READ32 *buffer | (*(buffer + 1) << 8) | (*(buffer + 2) << 16) | (*(buffer + 3) << 24); buffer += 4;
#define READFLOAT ((float)(*buffer)); buffer += 4;
#define READFIXED (float)(*buffer | (*(buffer + 1) << 8) | (*(buffer + 2) << 16) | (*(buffer + 3) << 24)) / 1000000.0f; buffer += 4;

#define WRITE8(_val) *(buffer++) = (_val);
#define WRITE16(_val) WRITE8((_val) & 0xff); WRITE8(((_val) >> 8) & 0xff);
#define WRITE32(_val) WRITE16((_val) & 0xffff) *buffer; WRITE16(((_val) >> 16) & 0xffff);

#define DELTA 0x9e3779b9
#define MX (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (key[(p&3)^e] ^ z)))

void btea_decrypt(uint32_t *v, int n, int base_rounds, uint32_t const key[4]) 
{
	uint32_t y, z, sum;
	unsigned p, rounds, e;

	/* Decoding Part */
	rounds = base_rounds + 52/n;
	sum = rounds*DELTA;
	y = v[0];

	do {
		e = (sum >> 2) & 3;
		for (p=n-1; p>0; p--) {
			z = v[p-1];
			y = v[p] -= MX;
		}
		z = v[n-1];
		y = v[0] -= MX;
		sum -= DELTA;
	} while (--rounds);
}

void nolo_decrypt_data(unsigned char* buf)
{
	const int cryptwords = (64-4)/4, cryptoffset = 1;
	static const uint32_t key[4] = {0x875bcc51, 0xa7637a66, 0x50960967, 0xf8536c51};
	uint32_t cryptpart[cryptwords];

	// Decrypt encrypted portion
	for (int i = 0; i < cryptwords; i++) {
	cryptpart[i] =
		((uint32_t)buf[cryptoffset+4*i  ]) << 0  |
		((uint32_t)buf[cryptoffset+4*i+1]) << 8  |
		((uint32_t)buf[cryptoffset+4*i+2]) << 16 |
		((uint32_t)buf[cryptoffset+4*i+3]) << 24;
	}

	btea_decrypt(cryptpart, cryptwords, 1, key);

	for (int i = 0; i < cryptwords; i++) {
		buf[cryptoffset+4*i  ] = cryptpart[i] >> 0;
		buf[cryptoffset+4*i+1] = cryptpart[i] >> 8;
		buf[cryptoffset+4*i+2] = cryptpart[i] >> 16;
		buf[cryptoffset+4*i+3] = cryptpart[i] >> 24;
	}
}

void nolo_decode_position(const unsigned char* data, vec3f* pos)
{
	const double scale = 0.0001f;

	pos->x = (scale * (int16_t)(data[0]<<8 | data[1]));
	pos->y = (scale * (int16_t)(data[2]<<8 | data[3]));
	pos->z = (scale *          (data[4]<<8 | data[5]));
}

void nolo_decode_orientation(const unsigned char* data, quatf* quat)
{
	double w,i,j,k, scale;
	// CV1 order
	w = (int16_t)(data[0]<<8 | data[1]);
	i = (int16_t)(data[2]<<8 | data[3]);
	j = (int16_t)(data[4]<<8 | data[5]);
	k = (int16_t)(data[6]<<8 | data[7]);
	// Normalize (unknown if scale is constant)
	//scale = 1.0/sqrt(i*i+j*j+k*k+w*w);
	// Turns out it is fixed point. But the android driver author
	// either didn't know, or didn't trust it.
	// Unknown if normalizing it helps
	scale = 1.0 / 16384;
	//std::cout << "Scale: " << scale << std::endl;
	w *= scale;
	i *= scale;
	j *= scale;
	k *= scale;

	// Reorder
	quat->w = w;
	quat->x = i;
	quat->y = k;
	quat->z = -j;
}

void nolo_decode_controller(int idx, unsigned char* data)
{
	uint8_t buttons, bit;

	if (data[0] != 2 || data[1] != 1) {
	// Unknown version
	/* Happens when controllers aren't on.
	std::cout << "Nolo: Unknown controller " << idx
	  << " version " << (int)data[0] << " " << (int)data[1]
	  << std::endl;
	*/
		return;
	}

	vec3f position;
	quatf orientation;

	nolo_decode_position(data+3, &position);
	nolo_decode_orientation(data+3+3*2, &orientation);

	buttons = data[3+3*2+4*2];
}

void nolo_decode_hmd_marker(drv_priv* priv, unsigned char* data)
{
	if (data[0] != 2 || data[1] != 1) {
		/* Happens with corrupt packages (mixed with controller data)
		std::cout << "Nolo: Unknown headset marker"
		  << " version " << (int)data[0] << " " << (int)data[1]
		  << std::endl;
		*/
		// Unknown version
		return;
	}

	vec3f homepos;
	vec3f position;
	quatf orientation;

	nolo_decode_position(data+3, &position);
	nolo_decode_position(data+3+3*2, &homepos);
	nolo_decode_orientation(data+3+2*3*2+1, &orientation);

	// Tracker viewer kept using the home for head.
	// Something wrong with how they handle the descriptors.
	priv->base.position = position;
	priv->base.rotation = orientation;
}

void nolo_decode_base_station(drv_priv* priv, unsigned char* data)
{
	// Unknown version
	if (data[0] != 2 || data[1] != 1)
		return;
}