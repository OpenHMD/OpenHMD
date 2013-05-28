/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* Unit Tests - Quaternion Tests */

#include "tests.h"

static const float t = 0.001;

bool quatf_eq(quatf q1, quatf q2, float t)
{
	for(int i = 0; i < 4; i++)
		if(!float_eq(q1.arr[i], q2.arr[i], t))
			return false;

	return true;
}

typedef struct {
	vec3f v;
	float f;
	quatf q;
} vec_float_quat;

void test_oquatf_init_axis()
{
	//void oquatf_init_axis(quatf* me, const vec3f* vec, float angle);
	
	vec_float_quat list[] = {
		{ {{0, 0, 0}}, 0, {{0, 0, 0, 1}}},
		{ {{5, 12, 3}}, 1, {{0.1796723168488794, 0.43121356043731063, 0.10780339010932766, 0.8775825618903728}}},
		{ {{-2, -3, 3}}, 1, {{-0.2044277365391809, -0.30664160480877134, 0.30664160480877134, 0.8775825618903728}} },
		{ {{100, -3, 3}}, -300, {{0.7142339081165469, -0.021427017243496407, 0.021427017243496407, 0.6992508064783751}} },
	};

	int sz = sizeof(vec_float_quat);

	for(int i = 0; i < sizeof(list) / sz; i++){
		quatf q;
		oquatf_init_axis(&q, &list[i].v, list[i].f);
		//printf("%f %f %f %f\n", q.x, q.y, q.z, q.w);
		TAssert(quatf_eq(q, list[i].q, t));
	}
}

typedef struct {
	quatf q;
	vec3f v1, v2;
} quat_vec2;

void test_oquatf_get_rotated()
{
	// void oquatf_get_rotated(const quatf* me, const vec3f* vec, vec3f* out_vec);
	
	quat_vec2 list[] = {
		{ {{0, 0, 0, 0}}, {{0, 0, 0}}, {{0, 0, 0}} }, 
		{ {{0, 0, 0, 0}}, {{1, 2, 3}}, {{0, 0, 0}} }, 
		{ {{0, 0, 0, 1}}, {{1, 2, 3}}, {{1, 2, 3}} }, 
		{ {{.4, .2, .1, 1}}, {{2, 1, 0}}, {{2.18, 1.59, 0.2}} }, 
		{ {{.4, .2, .1, -1}}, {{2, 1, 0}}, {{2.58, 0.79, 0.2}} }, 
	};

	int sz = sizeof(quat_vec2);

	for(int i = 0; i < sizeof(list) / sz; i++){
		vec3f vec;
		oquatf_get_rotated(&list[i].q, &list[i].v1, &vec);
		TAssert(vec3f_eq(vec, list[i].v2, t));
	}
}

void test_oquatf_mult()
{
}

void test_oquatf_normalize()
{
}

void test_oquatf_get_length()
{
}

void test_oquatf_get_mat4x4()
{
}

void test_oquatf_get_mat3x3()
{
}

void test_omat3x3_get_scales()
{
}

void test_omat3x3_get_euler_angles()
{
}
