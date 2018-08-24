#!/bin/sh
ARDUINO="arduino"
BOARDMNG="https://raw.githubusercontent.com/sparkfun/Arduino_Boards/master/IDE_Board_Manager/package_sparkfun_index.json"
MPU_9250_DMP="https://github.com/sparkfun/SparkFun_MPU-9250-DMP_Arduino_Library/archive"

$ARDUINO --install-boards "arduino:samd"
$ARDUINO --pref boardsmanager.additional.urls=$BOARDMNG --install-boards "SparkFun:samd"

# bossac is installed whitout execution rights
BOSSAC=`$ARDUINO --get-pref runtime.tools.SparkFun-bossac-1.4.0.path`
BOSSAC="$BOSSAC/bossac"
chmod u+x $BOSSAC

SKETCHBOOK_PATH=`$ARDUINO --get-pref sketchbook.path`
SKETCHBOOK_PATH="$SKETCHBOOK_PATH/libraries/"

sleep 1
pushd $SKETCHBOOK_PATH
wget "$MPU_9250_DMP/master.zip"
unzip master.zip
rm master.zip
popd
