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
		if(!float_eq(q1.arr[i], q2.arr[i], t)){
			printf("\nquat.arr[%d] == %f, expected %f\n", i, q1.arr[i], q2.arr[i]);
			return false;
		}

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

// TODO test_oquatf_mult
void test_oquatf_mult()
{
}

// TODO test_oquatf_normalize
void test_oquatf_normalize()
{
}

// TODO test_oquatf_get_length
void test_oquatf_get_length()
{
}

// TODO test_oquatf_get_mat4x4
void test_oquatf_get_mat4x4()
{
}

typedef struct {
	quatf q1, q2;
	float f;
} quat2_float;

void test_oquatf_get_dot()
{
	// TODO add more test cases
	quat2_float list[] = {
		{ {{1, 2, 3, 1}}, {{4, 3, 2, .5}}, 16.5 },
	};

	int sz = sizeof(quat2_float);

	for(int i = 0; i < sizeof(list) / sz; i++){
		TAssert(float_eq(oquatf_get_dot(&list[i].q1, &list[i].q2), list[i].f, t));
	}
}

typedef struct {
	quatf q1, q2;
} quat2;

void test_oquatf_inverse()
{
	// TODO add more test cases
	quat2 list[] = {
		{ {{1, 2, 3, 1}}, {{-0.06666666666666667, -0.13333333333333333, -0.2, 0.06666666666666667}} },
	};

	int sz = sizeof(quat2);

	for(int i = 0; i < sizeof(list) / sz; i++){
		oquatf_inverse(&list[i].q1);
		TAssert(quatf_eq(list[i].q1, list[i].q2, t));
	}
}

typedef struct {
	quatf q1, q2, q3;
} quat3;

void test_oquatf_diff()
{
	// TODO add more test cases
	quat3 list[] = {
		{ {{1, 2, 3, 1}}, {{5, 3, 2, .1}}, {{0.660000, -0.680000, 0.580000, 1.140000}} },
	};

	int sz = sizeof(quat3);

	for(int i = 0; i < sizeof(list) / sz; i++){
		quatf q;
		oquatf_diff(&list[i].q1, &list[i].q2, &q);
		TAssert(quatf_eq(q, list[i].q3, t));
	}
}
