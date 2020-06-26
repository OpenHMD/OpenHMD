/*
 * Copyright 2020 Jan Schmidt
 * SPDX-License-Identifier: BSL-1.0
 *
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 */
#include <stdlib.h>
#include <hidapi.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>

#include "rift-s-radio.h"
#include "rift-s-protocol.h"

/* Struct that forms a double linked queue of pending commands,
 * with the head being the currently active command */
struct rift_s_radio_command {
		rift_s_radio_command *prev;
		rift_s_radio_command *next;

		/* Request packet data */
    rift_s_hmd_radio_command_t read_command;

		/* Completion callback */
		rift_s_radio_completion_fn cb;
		void *cb_data;
};

static int get_radio_response_report (hid_device *hid, rift_s_hmd_radio_response_t *radio_response)
{
	int ret;

	radio_response->cmd = 0xb;
	ret = hid_get_feature_report(hid, (unsigned char *)(radio_response), sizeof(rift_s_hmd_radio_response_t));

	return ret;
}

void
rift_s_radio_update (rift_s_radio_state *state, hid_device *hid)
{
	bool read_another = false;
	
	do {
		/* Send a radio command if there is none active and some pending */
		if (state->command_result_pending == false && state->pending_commands) {
			rift_s_radio_command *cmd = state->pending_commands;
 			rift_s_hmd_radio_command_t *pkt = &cmd->read_command;

 			pkt->cmd = 0x12;
			hid_send_feature_report(hid, (unsigned char *)(pkt), sizeof(*pkt));
 			// rift_s_hexdump_buffer ("ControllerFWSend", (unsigned char *)(pkt), sizeof(*pkt));
			state->command_result_pending = true;
		}

		if (!state->command_result_pending)
 			break; /* Nothing to do right now */

		/* There's a command result pending, poll the radio response until it's complete */
		rift_s_hmd_radio_response_t radio_response;

		/* The radio response is ready when the busy flag has cleared, and the seqnum
		 * has incremented */
		int ret = get_radio_response_report (hid, &radio_response);

		if (ret < 2) {
 			break; /* HID read failed - bail */
 		}

		if (radio_response.busy_flag != 0x00 || radio_response.seqnum == state->last_radio_seqnum) {
			if (radio_response.busy_flag)
				state->last_radio_seqnum = radio_response.seqnum;
			return;
		}
		state->last_radio_seqnum = radio_response.seqnum;

		/* We have the controller response! */
		assert (ret > 3 && ret <= sizeof(radio_response));

		state->command_result_pending = false;
		//rift_s_hexdump_buffer ("ControllerFWReply", (unsigned char *)(&radio_response), ret);

 		assert (state->pending_commands != NULL);

 		rift_s_radio_command *cmd = state->pending_commands;

 		/* Pop the head off the cmds queue, because it's complete now */
 		state->pending_commands = cmd->next;
 		if (state->pending_commands == NULL)
 			state->pending_commands_tail = NULL;
 		else
 			state->pending_commands->prev = NULL;

 		/* Call the completion callback */
 		if (cmd->cb)
 			cmd->cb (true, radio_response.response_bytes, ret - 3, cmd->cb_data);
 		free(cmd);
 		read_another = true;

	} while (read_another);
}

void rift_s_radio_state_init (rift_s_radio_state *state, ohmd_context *ctx)
{
	state->ctx = ctx;
	state->command_result_pending = false;
	state->pending_commands = NULL;
	state->pending_commands_tail = NULL;
	state->last_radio_seqnum = -1;
}

void rift_s_radio_state_clear (rift_s_radio_state *state)
{
	/* Clear and free any pending commands. */
	rift_s_radio_command *head = state->pending_commands;
	while (head != NULL) {
		rift_s_radio_command *prev = head;
		head = prev->next;

		if (prev->cb)
				prev->cb (false, NULL, 0, prev->cb_data);
		free(prev);
	}

	state->pending_commands = state->pending_commands_tail = NULL;
}

void rift_s_radio_queue_command (rift_s_radio_state *state, const uint64_t device_id,
	const uint8_t *cmd_bytes, const int cmd_bytes_len,
	rift_s_radio_completion_fn cb, void *cb_data)
{
	rift_s_radio_command *cmd = ohmd_alloc(state->ctx, sizeof(rift_s_radio_command));

	assert (cmd_bytes_len <= sizeof (cmd->read_command.cmd_bytes));

	cmd->read_command.device_id = device_id;
	memcpy (cmd->read_command.cmd_bytes, cmd_bytes, cmd_bytes_len);
	cmd->cb = cb;
	cmd->cb_data = cb_data;

	/* Append to the pending commands queue. The command itself will be sent by the update() function
	 * when possible */
	if (state->pending_commands_tail == NULL) {
		assert (state->pending_commands == NULL);
		state->pending_commands = state->pending_commands_tail = cmd;
	}
	else {
		state->pending_commands_tail->next = cmd;
		cmd->prev = state->pending_commands_tail;
		state->pending_commands_tail = cmd;
	}
}

/* The current blocks are ~2-3KB, so this should be enough to read
 * the JSON config: */
#define MAX_JSON_LEN 4096

typedef struct rift_s_radio_json_read_state {
	rift_s_radio_state *state;
	uint64_t device_id;
	rift_s_radio_completion_fn cb;
	void *cb_data;

	uint32_t cur_offset;
	uint16_t block_len; /* Expected length, from the header */

	uint8_t data[MAX_JSON_LEN+1];
	uint16_t data_len;

} rift_s_radio_json_read_state;

static void
read_json_cb (bool success, uint8_t *response_bytes, int response_bytes_len, rift_s_radio_json_read_state *json_read)
{
	if (!success) {
		/* Failed, report to the caller */
		goto fail;
	}

	if (response_bytes_len < 5) {
		LOGW("Not enough bytes in radio response - needed 5, got %d\n", response_bytes_len);
		goto fail;
	}

	uint8_t reply_len = response_bytes[4];
	response_bytes += 5;

	if (json_read->cur_offset == 0) {
		/* Read the header 0u32 0x20                    01 00 62 09 7b 22 67 79 | .{..... ..b.{"gy
                            72 6f 5f 6d 22 3a 5b 2d 30 2e 30 31 34 33 35 36 | ro_m":[-0.014356
                            38 38 36 36 2c 2d 30 2e                         | 8866,-0.
        = len 0x20, 0001 (file type, or sequence number?), 0x0962 = file length */
		if (reply_len < 4) {
			LOGW("Not enough bytes in remote configuration header - needed 4, got %d\n", reply_len);
			goto fail; /* Not enough bytes in header */
		}

		uint16_t file_type = (response_bytes[1] << 8) | response_bytes[0];
		uint16_t block_len = (response_bytes[3] << 8) | response_bytes[2];

		if (file_type != 1) {
			LOGW("Unknown file type in remote configuration header - expected 1, got %d\n", file_type);
			goto fail;
		}
		/* Assert if the MAX_JSON_LEN ever needs expanding */
		assert (block_len <= MAX_JSON_LEN);
		if (block_len > MAX_JSON_LEN) {
			LOGW("Remote configuration block too long. Please expand the read buffer (needed %u bytes)\n", block_len);
			goto fail;
		}

		json_read->block_len = block_len;
		json_read->cur_offset = 0x4;
	}
	else {
		uint16_t data_remain = json_read->block_len - json_read->data_len;

		if (reply_len > data_remain)
			reply_len = data_remain; /* Truncate any over-read */

		/* Append these bytes to the buffer */
		memcpy (json_read->data + json_read->data_len, response_bytes, reply_len);
		json_read->data_len += reply_len;
	}

	/* If there is no more to read, then report success to the caller and return */
	if (json_read->data_len >= json_read->block_len) {
		json_read->data[json_read->data_len] = 0;

		if (json_read->cb)
			json_read->cb (true, json_read->data, json_read->data_len, json_read->cb_data);
		free (json_read);
		return;
	}

	/* Otherwise request more data */
	uint8_t read_len = OHMD_MIN (0x20, json_read->block_len - json_read->data_len);
	uint8_t read_cmd[] = { 0x2b, 0x20, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00 };

	read_cmd[4] = json_read->cur_offset;
	read_cmd[5] = json_read->cur_offset >> 8;
	read_cmd[6] = json_read->cur_offset >> 16;
	read_cmd[7] = json_read->cur_offset >> 24;
	read_cmd[8] = read_len;

	rift_s_radio_queue_command (json_read->state, json_read->device_id, read_cmd, sizeof(read_cmd),
		(rift_s_radio_completion_fn) read_json_cb, json_read);

	json_read->cur_offset += read_len;
	return;

fail:
	if (json_read->cb)
		json_read->cb (success, json_read->data, json_read->data_len, json_read->cb_data);
	free (json_read);
	return;
}

void
rift_s_radio_get_json_block (rift_s_radio_state *state, const uint64_t device_id, 
		rift_s_radio_completion_fn cb, void *cb_data)
{
	/* Configuration JSON block reading */
	rift_s_radio_json_read_state *json_read = ohmd_alloc (state->ctx, sizeof(rift_s_radio_json_read_state));
	/* cmd  = 0x2b  reply_buffer_len = 0x20  timeout(?) = 0x3e8 (=1000) offset = 0u32   len = 0x20 */
	const uint8_t read_cmd[] = { 0x2b, 0x20, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00 };

	json_read->state = state;
	json_read->device_id = device_id;
	json_read->cb = cb;
	json_read->cb_data = cb_data;

	rift_s_radio_queue_command (state, device_id, read_cmd, sizeof(read_cmd),
		(rift_s_radio_completion_fn) read_json_cb, json_read);
}
