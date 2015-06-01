LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

OHMD_PATH := ../$(LOCAL_PATH)/../..
#$(error $(LOCAL_PATH))

TARGET_PLATFORM := 19
LOCAL_MODULE    := AndroidTest
LOCAL_CFLAGS    := -std=c99 -Wall -I$(LOCAL_PATH)/../../src -I$(LOCAL_PATH)/../../include
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv2
LOCAL_STATIC_LIBRARIES := android_native_app_glue

#LOCAL_SRC_FILES += $(subst $(OHMD_PATH)/,,$(wildcard $(OHMD_PATH)/src/*.c))
#LOCAL_SRC_FILES += $(subst $(OHMD_PATH)/,,$(wildcard $(OHMD_PATH)/src/drv_dummy/*.c))
#LOCAL_SRC_FILES += $(subst $(OHMD_PATH)/,,$(wildcard $(OHMD_PATH)/src/drv_android/*.c))
#$(error $(OHMD_PATH))
#$(error $(LOCAL_SRC_FILES))

LOCAL_SRC_FILES := $(OHMD_PATH)/src/fusion.c $(OHMD_PATH)/src/omath.c $(OHMD_PATH)/src/openhmd.c $(OHMD_PATH)/src/platform-posix.c $(OHMD_PATH)/src/platform-win32.c $(OHMD_PATH)/src/drv_dummy/dummy.c $(OHMD_PATH)/src/drv_android/android.c

#$(error $(LOCAL_SRC_FILES))
#$(error $(OHMD_PATH))

include $(BUILD_SHARED_LIBRARY) 

$(call import-module,android/native_app_glue) 
