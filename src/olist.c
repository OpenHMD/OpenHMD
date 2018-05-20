#include <string.h>
#include <assert.h>

#include "openhmdi.h"

#define CLNEXT(_elem, _size) ((void**)(((char*)(_elem)) + _size))

struct olist
{
	ohmd_context* ctx;
	size_t elem_size;
	void* first;
	void* last;
};

olist* olist_create(ohmd_context* ctx, size_t elem_size)
{
	olist* me = ohmd_alloc(ctx, sizeof(olist));

	me->ctx = ctx;

	if(!me)
		return NULL;

	me->elem_size = elem_size;

	return me;
}

void* olist_insert(olist* me, void* elem, void* after)
{
	void* link = ohmd_alloc(me->ctx, me->elem_size + sizeof(void*));
	memcpy(link, elem, me->elem_size);

	if(!me->first){
		assert(after == NULL);
		me->first = link;
		me->last = link;
	}else{
		assert(after != NULL);

		*CLNEXT(link, me->elem_size) = *CLNEXT(after, me->elem_size);
		*CLNEXT(after, me->elem_size) = link;
		
		if(*CLNEXT(link, me->elem_size) == NULL)
			me->last = link;
	}

	return link;
}

void* olist_get_first(olist* me)
{
	return me->first;
}

void* olist_get_next(olist* me, void* elem)
{
	return *CLNEXT(elem, me->elem_size);
}

void* olist_append(olist* me, void* elem)
{
	void* link = ohmd_alloc(me->ctx, me->elem_size + sizeof(void*));
	memcpy(link, elem, me->elem_size);

	if(!me->first){
		me->first = link;
		me->last = link;
	}else{
		*CLNEXT(me->last, me->elem_size) = link;
		me->last = link;
	}

	return elem;
}