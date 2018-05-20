#include <openhmd-dev.h>
#include "openhmdi.h"

ohmd_status omodule_connect(oinput* input, ooutput* output, void* user_data)
{
	if(input->type != output->type)
		return OHMD_S_INVALID_OPERATION;
	
	oconnection* conn = ohmd_alloc(output->module->ctx, sizeof(oconnection));
	olist_append(input->input_list, conn);

	return OHMD_S_OK;
}

void ooutput_send(ooutput* output, ooutput_data* data)
{
	olist* list = output->output_list;
	for(oconnection* curr = olist_get_first(list); curr != NULL; curr = olist_get_next(list, curr)){
		curr->callback(output->module, data, curr->user_data);
	}
}