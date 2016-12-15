/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2016 Fredrik Hultin.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* Unit Tests - Queue */

#include "tests.h"
#include "openhmdi.h"

void test_ohmdq_push_pop()
{
	ohmd_context* ctx = ohmd_ctx_create();
	ohmdq* q = ohmdq_create(ctx, sizeof(int), 10);
	
	TAssert(ohmdq_get_max(q) == 10);

	for(int i = 0; i < 10; i++){
		bool ret = ohmdq_push(q, &i);
		TAssert(ret);
	}

	TAssert(ohmdq_get_size(q) == 10);
	
	int val = 0;
	bool ret = ohmdq_push(q, &val);
	TAssert(!ret);

	for(int i = 0; i < 10; i++){
		int val = 0;
		bool ret = ohmdq_pop(q, &val);
		TAssert(ret);
		TAssert(val == i);
	}
	
	TAssert(ohmdq_get_size(q) == 0);
	
	for(int i = 0; i < 10; i++){
		bool ret = ohmdq_push(q, &i);
		TAssert(ret);
	}

	TAssert(ohmdq_get_size(q) == 10);
	
	val = 0;
	ret = ohmdq_push(q, &val);
	TAssert(!ret);

	for(int i = 0; i < 10; i++){
		int val = 0;
		bool ret = ohmdq_pop(q, &val);
		TAssert(ret);
		TAssert(val == i);
	}
	
	TAssert(ohmdq_get_size(q) == 0);

	ohmdq_destroy(q);
	ohmd_ctx_destroy(ctx);
}
