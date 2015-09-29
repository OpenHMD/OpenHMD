/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* Unit Tests - Internal Interface */

#ifndef TESTS_H
#define TESTS_H

#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "openhmdi.h"

#define TAssert(_v) if(!(_v)){ printf("\ntest failed: %s @ %s:%d\n", __func__, __FILE__, __LINE__); exit(1); }

bool float_eq(float a, float b, float t);
bool vec3f_eq(vec3f v1, vec3f v2, float t);

// vec3f tests
void test_ovec3f_normalize_me();
void test_ovec3f_get_length();
void test_ovec3f_get_angle();
void test_ovec3f_get_dot();

// quatf tests
void test_oquatf_init_axis();
void test_oquatf_get_rotated();
void test_oquatf_mult();
void test_oquatf_mult_me();
void test_oquatf_normalize();
void test_oquatf_get_length();
void test_oquatf_get_dot();
void test_oquatf_inverse();
void test_oquatf_diff();

void test_oquatf_get_mat4x4();

// high-level tests
void test_highlevel_open_close_device();
void test_highlevel_open_close_many_devices();

#endif
