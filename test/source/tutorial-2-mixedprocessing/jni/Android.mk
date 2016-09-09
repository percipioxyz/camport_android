LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)

include ../../sdk/native/jni/OpenCV.mk

LOCAL_SHARED_LIBRARIES := libusb-1.0 libcamm

LOCAL_MODULE    := mixed_sample
LOCAL_SRC_FILES := jni_part.cpp depth_render.cpp
LOCAL_LDLIBS +=  -llog -ldl

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libcamm
LOCAL_SRC_FILES := libcamm.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libusb-1.0
LOCAL_SRC_FILES := libusb1.0.so
include $(PREBUILT_SHARED_LIBRARY)