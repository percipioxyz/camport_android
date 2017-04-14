LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := usb-1.0
LOCAL_SRC_FILES := lib/libusb1.0.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := tycamm
LOCAL_SRC_FILES := lib/libtycamm.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

OpenCV_INSTALL_MODULES := on
OpenCV_CAMERA_MODULES := off
OPENCV_LIB_TYPE :=STATIC
include ../../sdk/native/jni/OpenCV.mk

LOCAL_SHARED_LIBRARIES := tycamm usb-1.0

LOCAL_MODULE    := mixed_sample
LOCAL_SRC_FILES := jni_part.cpp
LOCAL_LDLIBS +=  -llog

include $(BUILD_SHARED_LIBRARY)



