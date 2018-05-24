#include <openhmd-dev.h>
#include <string.h>
#include "openhmdi.h"

#define MAX_INPUTS 32
#define MAX_OUTPUTS 32
#define MAX_CONNECTIONS 32
#define MAX_NAME_LENGTH 32
#define MAX_MESSAGE_DATA 32

typedef struct 
{
	char name[MAX_NAME_LENGTH];
	omodule* module;
	omessage_callback callback;
	void* user_data;
} oinput;

typedef struct 
{
	char name[MAX_NAME_LENGTH];
	oinput* conn[MAX_CONNECTIONS];
	int conn_count;
} ooutput;

struct omodule
{
	omodule_type type;
	ohmd_context* ctx;
	char name[MAX_NAME_LENGTH];
	uint64_t id;

	ooutput outputs[MAX_OUTPUTS];
	oinput inputs[MAX_INPUTS];

	int input_count;
	int output_count;
};

typedef struct
{
	omessage_data_type type;
	const void* data;
	int count;
	const char* name;
	uint64_t timestamp;
} omessage_data;

struct omessage
{
	const char* type_name;
	omessage_data data[MAX_MESSAGE_DATA];
	int data_count;
};

omodule* omodule_create(ohmd_context* ctx, const char* name, uint64_t id)
{
	omodule* me = ohmd_alloc(ctx, sizeof(omodule));

	strncpy(me->name, name, sizeof(me->name) - 1);
	me->id = id;

	return me;
}

int omodule_get_input_count(omodule* me)
{
	return me->input_count;
}

int omodule_get_output_count(omodule* me)
{
	return me->output_count;
}

const char* omodule_get_input_name(omodule* me, int idx)
{
	if(idx >= me->input_count)
		return NULL;
	
	return me->inputs[idx].name;
}

const char* omodule_get_output_name(omodule* me, int idx)
{
	if(idx >= me->output_count)
		return NULL;
	
	return me->outputs[idx].name;
}

void omodule_add_output(omodule* me, const char* name)
{
	int idx = me->output_count;
	strncpy(me->outputs[idx].name, name, sizeof(me->outputs[idx].name) - 1);
	me->output_count++;
}

void omodule_add_input(omodule* me, const char* name,  omessage_callback callback, void* user_data)
{
	int idx = me->input_count;
	strncpy(me->inputs[idx].name, name, sizeof(me->inputs[idx].name) - 1);
	me->inputs[idx].callback = callback;
	me->inputs[idx].user_data = user_data;
	me->input_count++;
}

omessage* omessage_create(ohmd_context* ctx, const char* type_name)
{
	omessage* msg = ohmd_alloc(ctx, sizeof(omessage));
	msg->type_name = type_name;
	return msg;
}

ooutput* omodule_lookup_output(omodule* me, const char* name)
{
	for(int i = 0; i < me->output_count; i++)
	{
		if(strcmp(me->outputs[i].name, name) == 0){
			return me->outputs + i;
		}
	}

	return NULL;
}

oinput* omodule_lookup_input(omodule* me, const char* name)
{
	for(int i = 0; i < me->input_count; i++)
	{
		if(strcmp(me->inputs[i].name, name) == 0){
			return me->inputs + i;
		}
	}

	return NULL;
}

ohmd_status omodule_connect(omodule* from, const char* output_name, omodule* to, const char* input_name)
{
	oinput* input = omodule_lookup_input(to, input_name);
	ooutput* output = omodule_lookup_output(from, output_name);

	if(input == NULL || output == NULL)
		return OHMD_S_INVALID_OPERATION;
	
	output->conn[output->conn_count++] = input;

	return OHMD_S_OK;
}

ohmd_status omodule_send_message(omodule* me, const char* output_name, omessage* msg)
{
	ooutput* output = omodule_lookup_output(me, output_name);

	if(output == NULL)
		return OHMD_S_INVALID_OPERATION;

	for(int i = 0; i < output->conn_count; i++)
	{
		output->conn[i]->callback(me, msg, output->conn[i]->user_data);
	}

	return OHMD_S_OK; 
}

void omessage_add_data(omessage* me, const char* name, omessage_data_type type, int count, const void* data, uint64_t timestamp)
{
	int i = me->data_count;
	
	me->data[i].type = type;
	me->data[i].data = data;
	me->data[i].count = count;
	me->data[i].name = name;
	me->data[i].timestamp = timestamp;

	me->data_count++;
}

void omessage_add_float_data(omessage* me, const char* name, const float* data, int count, uint64_t timestamp)
{
	omessage_add_data(me, name, omd_float, count, data, timestamp);
}

void omessage_add_int_data(omessage* me, const char* name, const int* data, int count, uint64_t timestamp)
{
	omessage_add_data(me, name, omd_bin, count, data, timestamp);
}

void omessage_add_bin_data(omessage* me, const char* name, const uint8_t* data, int count, uint64_t timestamp)
{
	omessage_add_data(me, name, omd_bin, count, data, timestamp);
}

void omessage_add_string_data(omessage* me, const char* name, const char* data, int length, uint64_t timestamp)
{
	omessage_add_data(me, name, omd_bin, length, data, timestamp);
}

int omessage_get_field_count(omessage* me)
{
	return me->data_count;
}

const char* omessage_get_field_name(omessage* me, int idx)
{
	if(idx >= me->data_count)
		return NULL;

	return me->data[idx].name; 
}

omessage_data* omessage_lookup_field(omessage* me, const char* name)
{
	for(int i = 0; i < me->data_count; i++)
	{
		if(strcmp(me->data[i].name, name) == 0){
			return &me->data[i];
		}
	}

	return NULL;
}

omessage_data_type omessage_get_field_type(omessage* me, const char* name)
{
	omessage_data* field = omessage_lookup_field(me, name);

	if(field == NULL)
		return omd_error;
	
	return field->type;
}

const float* omessage_get_float_data(omessage* me, const char* name, int* out_count)
{
	return (const float*)omessage_get_bin_data(me, name, out_count);
}

const int* omessage_get_int_data(omessage* me, const char* name, int* out_count)
{
	return (const int*)omessage_get_bin_data(me, name, out_count);
}

const char* omessage_get_string_data(omessage* me, const char* name, int* out_length)
{
	return (const char*)omessage_get_bin_data(me, name, out_length);
}

const uint8_t* omessage_get_bin_data(omessage* me, const char* name, int* out_count)
{
	omessage_data* field = omessage_lookup_field(me, name);

	if(field == NULL)
		return NULL;
	
	if(out_count != NULL)
		*out_count = field->count;

	return field->data;
}

ohmd_status omessage_get_timestamp(omessage* me, const char* name, uint64_t* out_timestamp)
{
	omessage_data* field = omessage_lookup_field(me, name);

	if(field == NULL)
		return OHMD_S_INVALID_PARAMETER;

	*out_timestamp = field->timestamp;
	
	return OHMD_S_OK;	
}
