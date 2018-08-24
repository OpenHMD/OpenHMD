#!/bin/sh
ARDUINO="arduino"

$ARDUINO --pref compiler.warning_level=none --board "SparkFun:samd:samd21_dev" --verify gyro.ino
