/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* Unit Tests - Vector3f Tests */

#include "tests.h"

bool vec3f_eq(vec3f v1, vec3f v2, float t)
{
	for(int i = 0; i < 3; i++)
		if(!float_eq(v1.arr[i], v2.arr[i], t)){
			printf("\nvec.arr[%d] == %f, expected %f\n", i, v1.arr[i], v2.arr[i]);
			return false;
		}

	return true;
}

void test_ovec3f_normalize_me()
{
	vec3f v[][2] = {
		{ {{1, 0, 0}}, {{1, 0, 0}} },
		{ {{1, 2, 3}}, {{0.267261241912424, 0.534522483824849, 0.801783725737273}} },
		{ {{-7, 13, 22}}, {{-0.264197974633739, 0.490653381462658, 0.830336491706037}} },
		{ {{.1, .1, .1}}, {{0.577350269189626, 0.577350269189626, 0.577350269189626}} },
		{ {{0, 0, 0}}, {{0, 0, 0}} },
	};

	int sz = sizeof(vec3f) * 2;
	float t = 0.001;

	for(int i = 0; i < sizeof(v) / sz; i++){
		vec3f norm = v[i][0];
		ovec3f_normalize_me(&norm);
		TAssert(vec3f_eq(norm, v[i][1], t));
	}
}

typedef struct {
	vec3f vec;
	float f;
} vec_float;

void test_ovec3f_get_length()
{
	vec_float vf[] = {
		{ {{0, 0, 0}}, 0},
		{ {{1, 0, 0}}, 1},
		{ {{1, 2, 0}}, 2.23606797749979},
		{ {{1, -2, 0}}, 2.23606797749979},
		{ {{1, 2, 3}}, 3.7416573867739413},
		{ {{-1, -2, -3}}, 3.7416573867739413},
	};

	int sz = sizeof(vec_float);
	float t = 0.001;

	for(int i = 0; i < sizeof(vf) / sz; i++){
		TAssert(float_eq(ovec3f_get_length(&vf[i].vec), vf[i].f, t));
	}
}

typedef struct {
	vec3f v1, v2;
	float f;
} vec2_float;
	
void test_ovec3f_get_angle()
{
	vec2_float vf[] = {
		{ {{0, 0, 0}}, {{0, 0, 0}}, 0},
		{ {{1, 0, 0}}, {{0, 0, 0}}, 0},
		{ {{2, 4, 3}}, {{1, 2, 3}}, 0.33940126397005316},
		{ {{2, 4, 3}}, {{-1, 2, 3}}, 0.7311043352203973},
	};

	int sz = sizeof(vec2_float);
	float t = 0.001;

	for(int i = 0; i < sizeof(vf) / sz; i++){
		TAssert(float_eq(ovec3f_get_angle(&vf[i].v1, &vf[i].v2), vf[i].f, t));
	}
}

//float ovec3f_get_dot(const vec3f* me, const vec3f* vec);

void test_ovec3f_get_dot()
{
	vec2_float vf[] = {
		{ {{0, 0, 0}}, {{0, 0, 0}}, 0},
		{ {{1, 2, 3}}, {{-.30, 2, 25}}, 78.7},
		{ {{-1, -10000000, 3}}, {{-.30, 2, 25}}, -19999924.7},
	};

	int sz = sizeof(vec2_float);
	float t = 0.001;

	for(int i = 0; i < sizeof(vf) / sz; i++){
		TAssert(float_eq(ovec3f_get_dot(&vf[i].v1, &vf[i].v2), vf[i].f, t));
	}
}


