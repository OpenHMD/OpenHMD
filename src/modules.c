#include <openhmd-dev.h>
#include <string.h>
#include "openhmdi.h"

void oinput_init(oinput* me, omodule* module, oconnection_type type, 
	oconnection_callback callback, void* user_data)
{
	me->callback = callback;
	me->user_data = user_data;
	me->type = type;
	me->module = module;
}

void omodule_init(omodule* me, ohmd_context* ctx, omodule_type type, 
	const char* name, uint64_t id)
{
	me->ctx = ctx;
	me->type = type;
	me->id = id;
	strncpy(me->name, name, sizeof(me->name) - 1);
}

ooutput* ooutput_create(omodule* module, oconnection_type type)
{
	ooutput* ret = ohmd_alloc(module->ctx, sizeof(ooutput));

	ret->module = module;
	ret->type = type;
	ret->output_list = olist_create(module->ctx, sizeof(oconnection));

	return ret;
}

void omodule_imu_init(oimu_module* me, ohmd_context* ctx, const char* name, 
	uint64_t id, oimu_module_flags flags)
{
	memset(me, 0, sizeof(oimu_module));
	omodule_init((omodule*)me, ctx, omt_imu, name, id);

	if(flags | oimf_has_accelerator)
		me->accelerator = ooutput_create((omodule*)me, oct_vec3f);

	if(flags | oimf_has_gyro)
		me->gyro = ooutput_create((omodule*)me, oct_vec3f);

	if(flags | oimf_has_rotation)
		me->rotation = ooutput_create((omodule*)me, oct_quatf);
}

void oimu_filter_module_init(oimu_filter_module* me, ohmd_context* ctx, 
	const char* name, uint64_t id, oconnection_callback rotation_callback, void* rotation_user_data)
{
	memset(me, 0, sizeof(oimu_filter_module));
	omodule_init((omodule*)me, ctx, omt_imu_filter, name, id);
	oinput_init(&me->gyro, (omodule*)me, oct_quatf, rotation_callback, rotation_user_data);
}

ohmd_status omodule_connect(ooutput* output, oinput* input)
{
	if(output == NULL || input->callback == NULL)
		return OHMD_S_INVALID_OPERATION;
	
	if(input->type != output->type)
		return OHMD_S_INVALID_OPERATION;
	
	oconnection conn;
	
	conn.callback = input->callback;
	conn.from = output->module;
	conn.to = input->module;
	conn.user_data = input->user_data;

	olist_append(output->output_list, &conn);

	return OHMD_S_OK;
}

void ooutput_send(ooutput* output, ooutput_data* data)
{
	olist* list = output->output_list;

	for(oconnection* curr = olist_get_first(list); curr != NULL; curr = olist_get_next(list, curr)){
		curr->callback(output->module, data, curr->user_data);
	}
}