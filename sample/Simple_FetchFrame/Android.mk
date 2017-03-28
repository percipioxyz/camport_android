LOCAL_PATH := $(call my-dir)
LOCAL_PATH_SAVE := $(call my-dir)

OPENCV_MK_PATH := /home/vincent/Downloads/android-ndk-r10e/samples/OpenCV-2.4.9-android-sdk/sdk/native/jni/OpenCV.mk

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

OpenCV_INSTALL_MODULES := on

OpenCV_CAMERA_MODULES := off

OPENCV_LIB_TYPE :=STATIC

include $(OPENCV_MK_PATH)

LOCAL_CPPFLAG := -fvisibility=hidden -fvisibility-inlines-hidden

LOCAL_SHARED_LIBRARIES := libusb-1.0 tycamm

LOCAL_CFLAGS += -pie -fPIE
LOCAL_LDFLAGS += -pie -fPIE

LOCAL_MODULE := SimpleView_FetchFrame 

LOCAL_SRC_FILES := $(LOCAL_PATH)/main.cpp

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)   
LOCAL_MODULE := tycamm
LOCAL_SRC_FILES := libtycamm.so  
include $(PREBUILT_SHARED_LIBRARY)
