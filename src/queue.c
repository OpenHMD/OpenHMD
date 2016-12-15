/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2016 Fredrik Hultin.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* Naive Thread Safe Circular Queue Implementation */

#include <stdlib.h>
#include <string.h>

#include "queue.h"
#include "openhmdi.h"

struct ohmdq {
	unsigned read_pos;
	unsigned write_pos;
	unsigned size;
	unsigned max;
	unsigned elem_size;
	char* elems;
	ohmd_mutex* mutex;
};

ohmdq* ohmdq_create(ohmd_context* ctx, unsigned elem_size, unsigned max)
{
	ohmdq* me = ohmd_alloc(ctx, sizeof(ohmdq));

	me->elems = ohmd_alloc(ctx, elem_size * max);
	me->max = max;
	me->elem_size = elem_size;
	me->read_pos = 0;
	me->write_pos = 0;
	me->size = 0;
	me->mutex = ohmd_create_mutex(ctx);

	return me;
}

bool ohmdq_push(ohmdq* me, const void* elem)
{
	bool ret = false;

	ohmd_lock_mutex(me->mutex);	

	if(me->size < me->max){
		memcpy(me->elems + me->write_pos, elem, me->elem_size);
		me->write_pos = (me->write_pos + me->elem_size) % (me->max * me->elem_size);
		me->size++;
		ret = true;
	}

	ohmd_unlock_mutex(me->mutex);	

	return ret;
}

bool ohmdq_pop(ohmdq* me, void* out_elem)
{
	bool ret = false;
	
	ohmd_lock_mutex(me->mutex);	

	if(me->size > 0){
		memcpy(out_elem, me->elems + me->read_pos, me->elem_size);
		me->read_pos = (me->read_pos + me->elem_size) % (me->max * me->elem_size);
		me->size--;
		ret = true;
	}
	
	ohmd_unlock_mutex(me->mutex);	

	return ret;
}

unsigned ohmdq_get_size(ohmdq* me)
{
	unsigned ret;

	ohmd_lock_mutex(me->mutex);
	ret = me->size;
	ohmd_unlock_mutex(me->mutex);

	return ret;
}

unsigned ohmdq_get_max(ohmdq* me)
{
	return me->max;
}

void ohmdq_destroy(ohmdq* me)
{
	free(me->elems);
	ohmd_destroy_mutex(me->mutex);
	free(me);
}
