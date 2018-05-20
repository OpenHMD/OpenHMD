#include <stdbool.h>
#include <string.h>
#include <openhmd-dev.h>
#include "openhmdi.h"
#include "tests.h"

typedef struct
{
	oimu_module base;
} imu_test;

typedef struct
{
	oimu_filter_module base;
	vec3f result;
} imu_filter_test;

void imu_filter_module_on_input(omodule* source, ooutput_data* value, void* user_data)
{
	imu_filter_test* me = user_data;
	ovec3f_data* q = (ovec3f_data*)value;

	printf("%f %f %f\n", q->value.x, q->value.y, q->value.z);

	vec3f filtered = q->value;
	filtered.x *= -1.f;
	me->result = filtered;
}

void test_module_connect()
{
	ohmd_context* ctx = ohmd_ctx_create();
	imu_test imu;

	omodule_imu_init((oimu_module*)&imu, ctx, "test imu", 0x1231, oimf_has_gyro);
	
	imu_filter_test filter;
	memset(&filter, 0, sizeof(imu_filter_test));
	oimu_filter_module_init((oimu_filter_module*)&filter, ctx, "test imu filter", 0x13131, imu_filter_module_on_input, &filter);

	ohmd_status s = omodule_connect(imu.base.gyro, &filter.base.gyro);
	TAssert(s == OHMD_S_OK);

	ovec3f_data v;

	v.base.source = (omodule*)&imu;
	v.base.ts = 1;

	v.value.x = 1.0f;
	v.value.y = 1.0f;
	v.value.z = 1.0f;
	
	ooutput_send(imu.base.gyro, (ooutput_data*)&v);

	printf("%f %f %f\n", filter.result.x, filter.result.y, filter.result.z);
}