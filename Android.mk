OHMD_PATH := $(abspath $(call my-dir))

include $(CLEAR_VARS)

LOCAL_MODULE := openhmd

#ABS=$(abspath $(OHMD_PATH))
#$(error $(ABS))

LOCAL_C_INCLUDES := $(OHMD_PATH)/include # $(NDK_ROOT)/sources/android/native_app_glue
LOCAL_CFLAGS := -std=c99 -Wall -DDRIVER_ANDROID

LOCAL_SRC_FILES := $(OHMD_PATH)/src/fusion.c $(OHMD_PATH)/src/omath.c $(OHMD_PATH)/src/openhmd.c $(OHMD_PATH)/src/platform-posix.c $(OHMD_PATH)/src/platform-win32.c $(OHMD_PATH)/src/drv_dummy/dummy.c $(OHMD_PATH)/src/drv_android/android.c

LOCAL_LDLIBS := -landroid -llog
LOCAL_STATIC_LIBRARIES := cpufeatures android_native_app_glue ndk_helper
LOCAL_SHARED_LIBRARIES :=

LOCAL_EXPORT_C_INCLUDES += $(LOCAL_C_INCLUDES)

#$(error $(NDK_ROOT))

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue) 
