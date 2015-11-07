MY_LOCAL_PATH := $(abspath $(call my-dir))

OHMD_PATH := $(MY_LOCAL_PATH)/../..
include $(OHMD_PATH)/Android.mk

include $(CLEAR_VARS)

TARGET_PLATFORM := 19
LOCAL_MODULE    := OpenHMD-Example
LOCAL_CFLAGS    := -std=c99 -Wall
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv2
LOCAL_STATIC_LIBRARIES := android_native_app_glue

LOCAL_SRC_FILES := $(MY_LOCAL_PATH)/app.c

LOCAL_SHARED_LIBRARIES += openhmd
LOCAL_C_INCLUDES += $(OHMD_PATH)/include

include $(BUILD_SHARED_LIBRARY) 
$(call import-module,android/native_app_glue) 
