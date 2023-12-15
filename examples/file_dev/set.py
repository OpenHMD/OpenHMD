#!/usr/bin/env python3

# Sets up/reconfigures an OpenHMD filedev file.

import sys
import os
import struct
import math

data_header = struct.pack("I", 0)
# HMD with positional and rotational tracking
data_devtc = struct.pack("II", 0, 2 | 4)
# called "set.py"
data_name =  b"set.py" + (b"\0" * 250)
data_rotation = struct.pack("Iffff", 0, math.radians(float(sys.argv[1])), math.radians(float(sys.argv[2])), math.radians(float(sys.argv[3])), 0)
data_position = struct.pack("fff", float(sys.argv[4]), float(sys.argv[5]), float(sys.argv[6]))
data_hmd_info = struct.pack("IIffffff", 1280, 800, 0.149760, 0.093600, 0.063500, 0.046800, math.radians(125.5144), (1280.0 / 800.0) / 2.0)
data_hmd_distortion = struct.pack("ffffff", 0, 0, 0, 0, 0, 0)
data_control_header = struct.pack("I", 0)
data_control_body = struct.pack("IIf", 0, 0, 0) * 64


fn = os.getenv("OPENHMD_FILEDEV_HMD")

try:
	f = open(fn, "x+b")
except:
	f = open(fn, "r+b")

f.seek(0)
f.write(data_header + data_devtc + data_name + data_rotation + data_position + data_hmd_info + data_hmd_distortion + data_control_header + data_control_body)
f.close()

