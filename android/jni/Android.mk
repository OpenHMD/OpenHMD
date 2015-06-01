LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

OHMD_PATH := ../$(LOCAL_PATH)/../..

TARGET_PLATFORM := 19
LOCAL_MODULE    := AndroidTest
LOCAL_CFLAGS    := -std=c99 -Wall -I$(LOCAL_PATH)/../../src -I$(LOCAL_PATH)/../../include
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv2
LOCAL_STATIC_LIBRARIES := android_native_app_glue

LOCAL_SRC_FILES := $(OHMD_PATH)/src/fusion.c $(OHMD_PATH)/src/omath.c $(OHMD_PATH)/src/openhmd.c $(OHMD_PATH)/src/platform-posix.c $(OHMD_PATH)/src/platform-win32.c $(OHMD_PATH)/src/drv_dummy/dummy.c $(OHMD_PATH)/src/drv_android/android.c
include $(BUILD_SHARED_LIBRARY) 

$(call import-module,android/native_app_glue) 
