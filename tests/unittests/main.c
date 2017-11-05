/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* Unit Tests - Main */

#include <string.h>
#include "tests.h"

bool float_eq(float a, float b, float t)
{
	return fabsf(a - b) < t;
}

#define Test(_t) printf("   "#_t); _t(); printf("%*sok\n", 50 - (int)strlen(#_t), "");

int main()
{
	printf("vec3f tests\n");
	Test(test_ovec3f_normalize_me);
	Test(test_ovec3f_get_length);
	Test(test_ovec3f_get_angle);
	Test(test_ovec3f_get_dot);
	printf("\n");
	
	printf("quatf tests\n");
	Test(test_oquatf_init_axis);
	Test(test_oquatf_get_rotated);
	Test(test_oquatf_get_dot);
	Test(test_oquatf_inverse);
	Test(test_oquatf_diff);
	printf("\n");

	printf("high level tests\n");
	Test(test_highlevel_open_close_device);
	Test(test_highlevel_open_close_many_devices);
	printf("\n");

	printf("all a-ok\n");
	return 0;
}
