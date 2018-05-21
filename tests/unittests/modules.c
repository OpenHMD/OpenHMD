#include <stdbool.h>
#include <string.h>
#include "openhmdi.h"
#include "tests.h"

#include <openhmd-dev.h>

void imu_filter_msg_handler(omodule* source, omessage* msg, void* user_data)
{
	float* d = user_data;
	const float* gyro = omessage_get_float_data(msg, "gyro", NULL);
	const float* accel = omessage_get_float_data(msg, "accel", NULL);

	for(int i = 0; i < 3; i++)
	{
		d[i] = gyro[i] * 2.f;
		d[i+3] = accel[i] * 2.f;
	}
}

void test_module_connect()
{
	ohmd_context* ctx = ohmd_ctx_create();
	
	omodule* imu = omodule_create(ctx, "test imu", 0x123);
	omodule_add_output(imu, "accel+gyro");

	omodule* imu_filter = omodule_create(ctx, "test imu filter", 0x234);

	float output[3+3] = {};

	omodule_add_input(imu_filter, "accel+gyro", imu_filter_msg_handler, output);
	
	ohmd_status s = omodule_connect(imu, "accel+gyro", imu_filter, "accel+gyro");
	TAssert(s == OHMD_S_OK);

	vec3f gyro;
	gyro.x = 1.f;
	gyro.y = 2.f;
	gyro.z = 3.f;
	
	vec3f accel;
	accel.x = 4.f;
	accel.y = 5.f;
	accel.z = 6.f;

	omessage* msg = omessage_create(ctx, "imu data");
	omessage_add_float_data(msg, "gyro", gyro.arr, 3);
	omessage_add_float_data(msg, "accel", accel.arr, 3);

	s = omodule_send_message(imu, "accel+gyro", msg);
	TAssert(s == OHMD_S_OK);

	TAssert(output[0] == 2.f);
	TAssert(output[1] == 4.f);
	TAssert(output[2] == 6.f);
	TAssert(output[3] == 8.f);
	TAssert(output[4] == 10.f);
	TAssert(output[5] == 12.f);
}
