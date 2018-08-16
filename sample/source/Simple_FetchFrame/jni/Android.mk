LOCAL_PATH := $(call my-dir)
LOCAL_PATH_SAVE := $(call my-dir)

OPENCV_MK_PATH := /home/vincent/Downloads/android-ndk-r10e/samples/opencv-3.0-android-sdk/OpenCV-android-sdk/sdk/native/jni/OpenCV.mk

ifeq ($(TARGET_ARCH_ABI), arm64-v8a)
	THIRD_LIB_PATH := $(LOCAL_PATH)/lib/arm64-v8a
endif

ifeq ($(TARGET_ARCH_ABI), armeabi)
	THIRD_LIB_PATH := $(LOCAL_PATH)/lib/armeabi
endif

ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
	THIRD_LIB_PATH := $(LOCAL_PATH)/lib/armeabi-v7a
endif

#$(warning $(TARGET_ARCH_ABI))
#$(warning $(THIRD_LIB_PATH))
include $(CLEAR_VARS)
LOCAL_MODULE := usb
LOCAL_SRC_FILES := $(THIRD_LIB_PATH)/libusb.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := tycam
LOCAL_SRC_FILES := $(THIRD_LIB_PATH)/libtycam.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
OpenCV_INSTALL_MODULES := on
OpenCV_CAMERA_MODULES := off
OPENCV_LIB_TYPE :=STATIC
include $(OPENCV_MK_PATH)

LOCAL_CPPFLAG := -fvisibility=hidden -fvisibility-inlines-hidden
LOCAL_SHARED_LIBRARIES := usb tycam
LOCAL_CFLAGS += -pie -fPIE
LOCAL_LDFLAGS += -pie -fPIE
LOCAL_MODULE := SimpleView_FetchFrame 
LOCAL_SRC_FILES := $(LOCAL_PATH)/main.cpp
include $(BUILD_EXECUTABLE)
