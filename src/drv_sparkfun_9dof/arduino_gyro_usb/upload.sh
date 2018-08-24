#!/bin/sh

if [ -z "$1" ]; then 
        echo "Usage: $0 [path to the arduino uart]"
        exit 1
fi

ARDUINO="/home/bernd/working/arduino/arduino-1.8.5/arduino"
PORT=$1
$ARDUINO --pref programmer=SparkFun:sam_ice --pref compiler.warning_level=none --upload --port $PORT --board "SparkFun:samd:samd21_dev" gyro.ino
