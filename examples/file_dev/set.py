#!/usr/bin/env python3

# Sets up/reconfigures an OpenHMD filedev file.

import sys
import os
import struct
import math

data_header = struct.pack("IIffffff", 1280, 800, 0.149760, 0.093600, 0.063500, 0.046800, math.radians(125.5144), (1280.0 / 800.0) / 2.0)
data_dynamic = struct.pack("ffffff", math.radians(float(sys.argv[1])), math.radians(float(sys.argv[2])), math.radians(float(sys.argv[3])), float(sys.argv[4]), float(sys.argv[5]), float(sys.argv[6]))

fn = os.getenv("OPENHMD_FILEDEV_HMD")

try:
	f = open(fn, "x+b")
except:
	f = open(fn, "r+b")

f.seek(0)
f.write(data_header + data_dynamic)
f.close()

