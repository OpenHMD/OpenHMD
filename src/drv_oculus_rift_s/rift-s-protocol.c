/*
 * Copyright 2013, Fredrik Hultin.
 * Copyright 2013, Jakob Bornecrantz.
 * Copyright 2016 Philipp Zabel
 * Copyright 2019 Lucas Teske <lucas@teske.com.br>
 * Copyright 2019-2020 Jan Schmidt
 * SPDX-License-Identifier: BSL-1.0
 *
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 */

/* Oculus Rift S Driver - HID/USB Driver Implementation */

#include <assert.h>
#include <hidapi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "rift-s.h"
#include "rift-s-protocol.h"
#include "../hid.h"

/* FIXME: The code in this file is not portable to big-endian as-is - it needs endian swaps */
bool
rift_s_parse_hmd_report (rift_s_hmd_report_t *report, const unsigned char *buf, int size)
{
	if (buf[0] != 0x65)
		return false;

	if (size != 64 || size != sizeof (rift_s_hmd_report_t))
		return false;

	*report = *(rift_s_hmd_report_t *)(buf);

	return true;
}

bool
rift_s_parse_controller_report (rift_s_controller_report_t *report, const unsigned char *buf, int size)
{
	uint8_t avail;

	if (buf[0] != 0x67)
		return false;

	if (size < 62) {
		LOGW("Controller report with size %d - please report it", size);
		return false;
	}

	report->id = buf[0];
	report->device_id = *(uint64_t *)(buf + 1);
	report->data_len = buf[9];
	report->num_info = 0;
	report->extra_bytes_len = 0;
	report->flags = 0;
	memset (report->log, 0, sizeof (report->log));

	if (report->data_len < 4) {
		if (report->data_len != 0)
			LOGW("Controller report with data len %u - please report it", report->data_len);
		return true; // No more to read
	}

	/* Advance the buffer pointer to the end of the common header.
	 * We now have data_len bytes left to read
	 */
	buf += 10;
	size -= 10;

	if (report->data_len > size) {
		LOGW("Controller report with data len %u > packet size 62 - please report it", report->data_len);
		report->data_len = size;
	}

	avail = report->data_len;

	report->flags = buf[0];
	report->log[0] = buf[1];
	report->log[1] = buf[2];
	report->log[2] = buf[3];
	buf += 4;
	avail -= 4;

	/* While we have at least 2 bytes (type + at least 1 byte data), read a block */
	while (avail > 1 && report->num_info < sizeof(report->info) / sizeof(report->info[0])) {
		rift_s_controller_info_block_t *info = report->info + report->num_info;
		size_t block_size = 0;
		info->block_id = buf[0];

		switch (info->block_id) {
			case RIFT_S_CTRL_MASK08:
			case RIFT_S_CTRL_BUTTONS:
			case RIFT_S_CTRL_FINGERS:
			case RIFT_S_CTRL_MASK0e:
				block_size = sizeof (rift_s_controller_maskbyte_block_t);
				break;
			case RIFT_S_CTRL_TRIGGRIP:
				block_size = sizeof (rift_s_controller_triggrip_block_t);
				break;
			case RIFT_S_CTRL_JOYSTICK:
				block_size = sizeof (rift_s_controller_joystick_block_t);
				break;
			case RIFT_S_CTRL_CAPSENSE:
				block_size = sizeof (rift_s_controller_capsense_block_t);
				break;
			case RIFT_S_CTRL_IMU:
				block_size = sizeof (rift_s_controller_imu_block_t);
				break;
			default:
				break;
		}

		if (block_size == 0 || avail < block_size)
			break; /* Invalid block, or not enough data */

		memcpy (info->raw.data, buf, block_size);
		buf += block_size;
		avail -= block_size;
		report->num_info++;
	}

	if (avail > 0) {
		assert (avail < sizeof (report->extra_bytes));
		report->extra_bytes_len = avail;
		memcpy (report->extra_bytes, buf, avail);
	}

	return true;
}

void rift_s_hexdump_buffer (const char *label, const unsigned char *buf, int length) {
	int indent = 0;
	char ascii[17];

	if (label)
		indent = strlen (label) + 2;
	printf("%s: ", label);

	ascii[16] = '\0';
	for(int i = 0; i < length; i++){
		printf("%02x ", buf[i]);

		if (buf[i] >= ' ' && buf[i] <= '~')
			ascii[i % 16] = buf[i];
		else
			ascii[i % 16] = '.';

		if((i % 16) == 15 || (i+1) == length) {
			 if ((i % 16) < 15) {
				int remain = 15 - (i%16);
				 ascii[(i+1) % 16] = '\0';
				 /* Pad the hex dump out to 48 chars */
				 printf("%*s", 3*remain, " ");
			 }
			 printf("| %s", ascii);

			 if ((i+1) != length)
				 printf("\n%*s", indent, " ");
		}
	}
	printf("\n");
}

static int get_feature_report(hid_device *hid, uint8_t cmd, uint8_t *buf, int len)
{
	memset(buf, 0, len);
	buf[0] = cmd;
	return hid_get_feature_report(hid, (unsigned char *) buf, len);
}

static int
read_one_fw_block (hid_device *dev, uint8_t block_id, uint32_t pos, uint8_t read_len, uint8_t *buf)
{
	unsigned char req[64] = { 0x4a, 0x00, };
	int ret, loops = 0;
	bool send_req = true;

	req[2] = block_id;

	do {
		if (send_req) {
			/* FIXME: Little-endian code: */
			* (uint32_t *)(req + 3) = pos;
			req[7] = read_len;
			ret = hid_send_feature_report(dev, req, 64);
			if (ret < 0) {
				LOGE("Report 74 SET failed");
				return ret;
			}
		}

		ret = get_feature_report(dev, 0x4A, buf, 64);
		if (ret < 0) {
			LOGE("Report 74 GET failed");
			return ret;
		}
		/* Loop until the result matches the address we asked for and
		 * the 2nd byte == 0x00 (0x1 = busy or req ignored?), or 20 attempts have passed */
		if (memcmp (req, buf, 7) == 0)
			break;

		/* Or if the 2nd byte of the return result is 0x1, the read is being processed,
		 * don't send the req again. If it's 0x00, we seem to need to re-send the request	*/
		send_req = (buf[1] == 0x00);

		/* FIXME: Avoid the sleep and just try again later? */
		ohmd_sleep (0.002);
	} while (loops++ < 20);

	if (loops > 20)
		return -1;

	return ret;
}

int rift_s_read_firmware_block (hid_device *dev, uint8_t block_id,
		char **data_out, int *len_out)
{
	uint32_t pos = 0x00, block_len;
	unsigned char buf[64] = { 0x4a, 0x00, };
	unsigned char *outbuf;
	size_t total_read = 0;
	int ret;

	ret = read_one_fw_block (dev, block_id, 0, 0xC, buf);
	if (ret < 0) {
		LOGE ("Failed to read fw block %02x header", block_id);
		return ret;
	}

	/* The block header is 12 bytes. 8 byte checksum, 4 byte size? */
	block_len = *(uint32_t *)(buf + 16);

	if (block_len < 0xC || block_len == 0xFFFFFFFF)
		return 0; /* Invalid block */

#if 0
	uint64_t checksum = *(uint64_t *)(buf + 8);
	printf ("FW Block %02x Header. Checksum(?) %08lx len %d\n", block_id, checksum, block_len);
#endif

	/* Copy the contents of the fw block, minus the header */
	outbuf = malloc (block_len + 1);
	outbuf[block_len] = 0;
	total_read = 0x0;

	for (pos = 0x0; pos < block_len; pos += 56) {
		uint8_t read_len = 56;
		if (pos + read_len > block_len)
			read_len = block_len - pos;

		ret = read_one_fw_block (dev, block_id, pos + 0xC, read_len, buf);
		if (ret < 0) {
			LOGE("Failed to read fw block %02x at pos 0x%08x len %d", block_id, pos, read_len);
			free(outbuf);
			return ret;
		}
		memcpy (outbuf + total_read, buf + 8, read_len);
		total_read += read_len;
	}

	if (total_read > 0) {
		if (total_read < block_len) {
			LOGE ("Short FW read - only read %u bytes of %u",
				 (unsigned int) total_read, block_len);
			free(outbuf);
			return -1;
		}

#if 0
		char label[64];
		sprintf (label, "FW Block %02x", block_id);
		if (outbuf[0] == '{' && outbuf[total_read-2] == '}' && outbuf[total_read-1] == 0)
			printf ("%s\n", outbuf); // Dump JSON string
		else
			rift_s_hexdump_buffer (label, outbuf, total_read);
#endif
	}

	*data_out = (char *)(outbuf);
	*len_out = block_len;

	return ret;
}

void
rift_s_send_keepalive (hid_device *hid)
{
	/* HID report 147 (0x93) 0xbb8 = 3000ms timeout, sent every 1000ms */
	unsigned char buf[6] = { 0x93, 0x01, 0xb8, 0x0b, 0x00, 0x00 };
	hid_send_feature_report(hid, buf, 6);
}

int
rift_s_send_camera_report (hid_device *hid, bool enable, bool radio_sync_bit)
{
/*
 *	 05 O1 O2 P1 P1 P2 P2 P3 P3 P4 P4 P5 P5 E1 E1 E3
 *	 E4 E5 U1 U2 U3 A1 A1 A1 A1 A2 A2 A2 A2 A3 A3 A3
 *	 A3 A4 A4 A4 A4 A5 A5 A5 A5
 *
 *	 O1 = Camera stream on (0x00 = off, 0x1 = on)
 *	 O2 = Radio Sync maybe?
 *	 Px = Vertical offset / position of camera x passthrough view
 *	 Ex = Exposure of camera x passthrough view
 *	 Ax = ? of camera x. 4 byte LE, Always seems to take values 0x3f0-0x4ff
 *        but I can't see the effect on the image
 *	 U1U2U3 = 26 00 40 always?
 */
	unsigned char buf[41] = {
#if 0
		0x05, 0x01, 0x01, 0xb3, 0x36, 0xb3, 0x36, 0xb3, 0x36, 0xb3, 0x36, 0xb3, 0x36, 0xf0, 0xf0, 0xf0,
		0xf0, 0xf0, 0x26, 0x00, 0x40, 0x7a, 0x04, 0x00, 0x00, 0xa7, 0x04, 0x00, 0x00, 0xa7, 0x04, 0x00,
		0x00, 0xa5, 0x04, 0x00, 0x00, 0xa8, 0x04, 0x00, 0x00
#else
		0x05, 0x01, 0x01, 0xb3, 0x36, 0xb3, 0x36, 0xb3, 0x36, 0xb3, 0x36, 0xb3, 0x36, 0xf0, 0xf0, 0xf0,
		0xf0, 0xf0, 0x26, 0x00, 0x40, 0x7a, 0x04, 0x00, 0x00, 0xa7, 0x04, 0x00, 0x00, 0xa7, 0x04, 0x00,
		0x00, 0xa5, 0x04, 0x00, 0x00, 0xa8, 0x04, 0x00, 0x00
#endif
	};

	buf[1] = enable ? 0x1 : 0x0;
	buf[2] = radio_sync_bit ? 0x1 : 0x0;

	return hid_send_feature_report(hid, buf, 41);
}

int
rift_s_set_screen_enable (hid_device *hid, bool enable)
{
	uint8_t buf[2];

	// Enable/disable LCD screen
	buf[0] = 0x08;
	buf[1] = enable ? 0x01 : 0;
	return hid_send_feature_report(hid, buf, 2);
}

int rift_s_read_device_info (hid_device *hid, rift_s_device_info_t *device_info)
{
	uint8_t buf[FEATURE_BUFFER_SIZE];

	int res = get_feature_report(hid, 0x06, buf, FEATURE_BUFFER_SIZE);
	if (res < sizeof(rift_s_device_info_t)) {
		LOGE("Failed to read %d bytes of device info", FEATURE_BUFFER_SIZE);
		return res;
	}
	rift_s_hexdump_buffer ("device info", buf, res);

	*device_info = *(rift_s_device_info_t *)buf;

	return 0;
}

int rift_s_get_report1 (hid_device *hid) {
	uint8_t buf[FEATURE_BUFFER_SIZE];
	int res;

	res = get_feature_report(hid, 0x01, buf, 43);
	if (res < 0) {
		LOGW("Failed to read report 1\n");
		return res;
	}

	rift_s_hexdump_buffer ("report 1", buf, res);
	return 0;
}

int rift_s_read_imu_config (hid_device *hid, rift_s_imu_config_t *imu_config)
{
	uint8_t buf[FEATURE_BUFFER_SIZE];
	int res;

	res = get_feature_report(hid, 0x09, buf, FEATURE_BUFFER_SIZE);
	if (res < 21)
		return -1;

	*imu_config = *(rift_s_imu_config_t *)buf;

	return 0;
}

int rift_s_hmd_enable (hid_device *hid, bool enable) {
	uint8_t buf[3];
	int res;

	if (enable) {
		/* Unknown report 0x07 */
		buf[0] = 0x07;
		buf[1] = 0xa3;
		buf[2] = 0x01;
		if ((res = hid_send_feature_report(hid, buf, 3)) < 0)
				return res;
	}

	/* Not sure what this is doing, everything seems to work anyway without it */
	buf[0] = 0x14;
	buf[1] = enable ? 0x01 : 0x00;
	if ((res = hid_send_feature_report(hid, buf, 2)) < 0)
			return res;

	/* Turn on radio to controllers */
	buf[0] = 0x0A;
	buf[1] = enable ? 0x02 : 0x00;
	if ((res = hid_send_feature_report(hid, buf, 2)) < 0)
			return res;

	if (!enable) {
		/* Shutting off - turn off the LCD */
		res = rift_s_set_screen_enable (hid, false);
		if (res < 0)
				return res;
	}

	/* Enables prox sensor + HMD IMU etc */
	buf[0] = 0x02;
	buf[1] = enable ? 0x01 : 0x00;
	if ((res = hid_send_feature_report(hid, buf, 2)) < 0)
			return res;

	/* Send camera report with enable=true enables the streaming. The
	 * 2nd byte seems something to do with sync, but doesn't always work,
	 * not sure why yet. */
	return rift_s_send_camera_report (hid, enable, false);
}

int rift_s_read_devices_list (hid_device *handle, rift_s_devices_list_t *dev_list)
{
	unsigned char buf[200];

	int res = get_feature_report(handle, 0x0c, buf, sizeof(buf));
	if (res < 3) {
		/* This happens when the Rift is just starting, we'll try again later */
		return -1;
	}

	int num_records = (res - 3) / 28;
	if (num_records > buf[2])
		num_records = buf[2];
	if (num_records > DEVICES_LIST_MAX_DEVICES)
		num_records = DEVICES_LIST_MAX_DEVICES;

	unsigned char *pos = buf + 3;

	for (int i = 0; i < num_records; i++) {
		dev_list->devices[i] = *(rift_s_device_type_record_t *)(pos);
		pos += sizeof(rift_s_device_type_record_t);
		assert (sizeof(rift_s_device_type_record_t) == 28);
	}
	dev_list->num_devices = num_records;

	return 0;
}
