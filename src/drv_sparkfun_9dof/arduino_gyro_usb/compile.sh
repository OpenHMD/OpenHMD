#!/bin/sh
# 
# OpenHMD - Free and Open Source API and drivers for immersive technology.
# Copyright (C) 2018 Bernd Lehmann.
# Distributed under the Boost 1.0 licence, see LICENSE for full text.

ARDUINO="arduino"

$ARDUINO --pref compiler.warning_level=none --board "SparkFun:samd:samd21_dev" --verify gyro.ino
