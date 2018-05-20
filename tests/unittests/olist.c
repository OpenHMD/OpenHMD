/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* Unit Tests - Vector3f Tests */

#include "tests.h"

void test_olist_insert()
{
	ohmd_context* ctx = ohmd_ctx_create();
  olist* list = olist_create(ctx, sizeof(int));

	int ints[] = { 0, 1, 2, 3, 4, 666, 5, 6, 7, 8, 9, 777, 888};

  int* last = NULL;
  int* mid = NULL;

  for(int i = 0; i < 10; i++){
    last = olist_insert(list, &i, last);

    if(i == 4)
      mid = last;
  }

  int i = 666;
  int j = 777;
  int k = 888;

  last = olist_insert(list, &i, mid);
  
  olist_append(list, &j);
  olist_append(list, &k);
  
	int* correct = ints;

  for(int* curr = olist_get_first(list); curr != NULL; curr = olist_get_next(list, curr)){
		TAssert(*curr == *(correct++));
  }
}
