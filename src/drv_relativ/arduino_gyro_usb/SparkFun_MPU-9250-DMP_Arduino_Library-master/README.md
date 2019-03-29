SparkFun MPU-9250 Digital Motion Processor (DMP) Arduino Library
========================================

[![SparkFun MPU-9250](https://cdn.sparkfun.com//assets/parts/1/1/3/0/6/13762-00a.jpg)](https://www.sparkfun.com/products/13762)

[*SparkFun MPU-9250 (SEN-13762)*](https://www.sparkfun.com/products/13762)

Advanced Arduino library for the Invensense MPU-9250 inertial measurement unit (IMU), which enables the sensor's digital motion processing (DMP) features. Along with configuring and reading from the accelerometer, gyroscope, and magnetometer, this library also supports the chip's DMP features like:

* Quaternion calculation
* Pedometer
* Gyroscope calibration
* Tap detection
* Orientation dtection

For help getting started with this library, refer to the [Using the MPU-9250 DMP Arduino Library](https://learn.sparkfun.com/tutorials/9dof-razor-imu-m0-hookup-guide#using-the-mpu-9250-dmp-arduino-library) section of the 9DoF Razor IMU M0 Hookup Guide.

**Note**: This library currently only supports and is tested on **SAMD processors**. It's a major part of the [SparkFun 9DoF Razor IMU M0](https://www.sparkfun.com/products/14001) firmware.

If you're looking for an AVR-compatible (Arduino Uno, SparkFun RedBoard, etc.) library, check out the [SparkFun MPU-9250 Breakout Library](https://github.com/sparkfun/SparkFun_MPU-9250_Breakout_Arduino_Library), which provides access to all standard MPU-9250 sensor-readings.

Repository Contents
-------------------

* **/examples** - Example sketches for the library (.ino). Run these from the Arduino IDE. 
* **/src** - Source files for the library (.cpp, .h).
	* **/src/util** - Source and headers for the MPU-9250 driver and dmp configuration. These are available and adapted from [Invensene's downloads page](https://www.invensense.com/developers/software-downloads/#sla_content_45).
* **keywords.txt** - Keywords from this library that will be highlighted in the Arduino IDE. 
* **library.properties** - General library properties for the Arduino package manager. 

Documentation
--------------

* **[Installing an Arduino Library Guide](https://learn.sparkfun.com/tutorials/installing-an-arduino-library)** - Basic information on how to install an Arduino library.
* **[SparkFun 9DoF Razor IMU M0 Repository](https://github.com/sparkfun/9DOF_Razor_IMU)** - Main repositor (including hardware files) for the MPU-9250-based SparkFun 9DoF Razor IMU M0
* **[MPU-9250 Breakout Repository](https://github.com/sparkfun/MPU-9250_Breakout)** - Main repository (including hardware files) for the MPU-9250 Breakout.
* **[Hookup Guide](https://learn.sparkfun.com/tutorials/9dof-razor-imu-m0-hookup-guide)** - Basic hookup guide for the SparkFun 9DoF Razor IMU M0, including a [section on using this library](https://learn.sparkfun.com/tutorials/9dof-razor-imu-m0-hookup-guide#using-the-mpu-9250-dmp-arduino-library).

Products that use this Library 
---------------------------------

* [SparkFun 9DoF Razor IMU M0 (SEN-14001)](https://www.sparkfun.com/products/14001)- An MPU-9250 development board, which includes an Arduino-compatible SAMD21 processor, LiPo battery charger, and USB interface.
* [SparkFun MPU-9250 Breakout (SEN-13762)](https://www.sparkfun.com/products/13762)- Easily adaptible breakout board for the MPU-9250.

Version History
---------------


License Information
-------------------

This product is _**open source**_! 

Please review the LICENSE.md file for license information. 

If you have any questions or concerns on licensing, please contact techsupport@sparkfun.com.

Distributed as-is; no warranty is given.

- Your friends at SparkFun.
