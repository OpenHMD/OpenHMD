#!/bin/sh
# 
# OpenHMD - Free and Open Source API and drivers for immersive technology.
# Copyright (C) 2018 Bernd Lehmann.
# Distributed under the Boost 1.0 licence, see LICENSE for full text.

if [ -z "$1" ]; then 
        echo "Usage: $0 [path to the arduino uart]"
        exit 1
fi

ARDUINO="/home/bernd/working/arduino/arduino-1.8.5/arduino"
PORT=$1
$ARDUINO --pref programmer=SparkFun:sam_ice --pref compiler.warning_level=none --upload --port $PORT --board "SparkFun:samd:samd21_dev" gyro.ino
