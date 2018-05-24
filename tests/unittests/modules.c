#include <stdbool.h>
#include <string.h>
#include "openhmdi.h"
#include "tests.h"

#include <openhmd-dev.h>

typedef struct
{
	float gyro[3];
	float accel[3];
	uint64_t gyro_ts;
	uint64_t accel_ts;
} test_data;

void imu_filter_msg_handler(omodule* source, omessage* msg, void* user_data)
{
	test_data* td = user_data;
	const float* gyro = omessage_get_float_data(msg, "gyro", NULL);
	const float* accel = omessage_get_float_data(msg, "accel", NULL);

	ohmd_status s;
	
	s = omessage_get_timestamp(msg, "gyro", &td->gyro_ts);
	TAssert(s == OHMD_S_OK);

	s = omessage_get_timestamp(msg, "accel", &td->accel_ts);
	TAssert(s == OHMD_S_OK);

	for(int i = 0; i < 3; i++)
	{
		td->gyro[i] = gyro[i] * 2.f;
		td->accel[i] = accel[i] * 2.f;
	}
}

void test_module_connect()
{
	ohmd_context* ctx = ohmd_ctx_create();
	
	omodule* imu = omodule_create(ctx, "test imu", 0x123);
	omodule_add_output(imu, "accel+gyro");

	omodule* imu_filter = omodule_create(ctx, "test imu filter", 0x234);

	test_data td;
	memset(&td, 0, sizeof(test_data));

	omodule_add_input(imu_filter, "accel+gyro", imu_filter_msg_handler, &td);
	
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
	omessage_add_float_data(msg, "gyro", gyro.arr, 3, 0x123);
	omessage_add_float_data(msg, "accel", accel.arr, 3, 0x124);

	s = omodule_send_message(imu, "accel+gyro", msg);
	TAssert(s == OHMD_S_OK);

	TAssert(td.gyro[0] == 2.f);
	TAssert(td.gyro[1] == 4.f);
	TAssert(td.gyro[2] == 6.f);

	TAssert(td.accel[0] == 8.f);
	TAssert(td.accel[1] == 10.f);
	TAssert(td.accel[2] == 12.f);

	TAssert(td.gyro_ts == 0x123);
	TAssert(td.accel_ts == 0x124);
}
