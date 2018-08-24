#include "RawHID.h"
#include <SparkFunMPU9250-DMP.h>

struct test {
        float w;
        float x;
        float y;
        float z;
};

RawHID raw;
MPU9250_DMP imu;

void setup()
{
        raw.begin();
        imu.begin();
        imu.dmpBegin(DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_GYRO_CAL);
}

void loop()
{
        unsigned char buf[SIZE];
        struct test *_data = (struct test *)buf;

        if (0 < imu.fifoAvailable()) {
                if (INV_SUCCESS == imu.dmpUpdateFifo()) {
                        _data->w = imu.calcQuat(imu.qw);
                        _data->x = imu.calcQuat(imu.qx);
                        _data->y = imu.calcQuat(imu.qy);
                        _data->z = imu.calcQuat(imu.qz);
                        raw.SendReport(buf, sizeof(buf));
                }
        }

}
