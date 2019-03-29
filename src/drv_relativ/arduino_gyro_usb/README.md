# Introduction

This arduino sketch for the SparkFun 9DoF Razor IMU M0 reads the quaternation values
from the motion processor integrated in the MPU-9250 and provides this data over USB
via a custum hid device interface.

# Prerequisite

The provided code is intendet [SparkFun 9DoF Razor IMU M0](https://www.sparkfun.com/products/14001).
Folowing software packages are required:

- Arduino IDE (>=1.8.5)
- Arduino SAMD Boards
- SparkFun SAMD Boards
- [SparkFun MPU-9250-DMP Arduino Library](https://github.com/sparkfun/SparkFun_MPU-9250-DMP_Arduino_Library)

For more information on the prerequisites for the SparkFun 9DoF Razor IMU M0 refere
to the [Hookup Guide](https://learn.sparkfun.com/tutorials/9dof-razor-imu-m0-hookup-guide).

# Install on Linux

Assuming that the Arduino IDE is installed and can be accessed via the global search 
path, then you can the install script.

    ./install_libs.sh

# Compiling on Linux

    ./compile.sh

# Uploading to the board on Linux

    ./upload.sh [path/to/SparkFunMPU9250/uart]

