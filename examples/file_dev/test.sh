#!/bin/sh

hmd_restore() {
echo Restore
./set.py 0 0 0 0 0 0
sleep 1
}

hmd_normalized_shift() {
./set.py $1 -1 $2
sleep 0.5
./set.py $1 -0.5 $2
sleep 0.5
./set.py $1 0 $2
sleep 0.5
./set.py $1 0.5 $2
sleep 0.5
./set.py $1 1 $2
sleep 1
}

hmd_angle_shift() {
./set.py $1 -45 $2
sleep 0.5
./set.py $1 -22.5 $2
sleep 0.5
./set.py $1 0 $2
sleep 0.5
./set.py $1 22.5 $2
sleep 0.5
./set.py $1 45 $2
sleep 1
}

hmd_restore

echo X shift
hmd_normalized_shift "0 0 0" "0 0"

echo Y shift
hmd_normalized_shift "0 0 0 0" "0"

echo Z shift
hmd_normalized_shift "0 0 0 0 0" ""

hmd_restore

echo Pitch
hmd_angle_shift "" "0 0 0 0 0"

echo Yaw
hmd_angle_shift "0" "0 0 0 0"

echo Roll
hmd_angle_shift "0 0" "0 0 0"

hmd_restore

echo Roll with constant yaw and pitch
hmd_angle_shift "22.5 -22.5" "0 0 0"

echo Yaw with constant pitch
hmd_angle_shift "22.5" "0 0 0 0"

echo Pitch with constant yaw
hmd_angle_shift "" "22.5 0 0 0 0"

hmd_restore
