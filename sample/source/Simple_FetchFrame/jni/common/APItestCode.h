#include<TY_API.h>
#include "../common/common.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>

#define TY_SUCCESS_TEST 0
#define TY_FEATURE_INVALID 1
#define TY_NOT_WRITEABLE 2
#define TY_NOT_READABLE 3
#define TY_WRONG_VALUE 4
#define TY_ABNORMALVALUE_SET_SUCCESS 5
#define TY_NORMALVALUE_SET_FAIL 6
#define TY_FAULT_APPEARS 7

static int PercipioDeviceComponent[] = {
	TY_COMPONENT_DEPTH_CAM,
	TY_COMPONENT_POINT3D_CAM,
	TY_COMPONENT_IR_CAM_LEFT,
	TY_COMPONENT_IR_CAM_RIGHT,
	TY_COMPONENT_RGB_CAM_LEFT,
	TY_COMPONENT_RGB_CAM_RIGHT,
	TY_COMPONENT_LASER,
	TY_COMPONENT_IMU,
	TY_COMPONENT_BRIGHT_HISTO
};

int TYResolutionTest(const char* deviceID,TY_DEV_HANDLE hDevice, TY_COMPONENT_ID componentID, TY_FEATURE_ID featureID, int32_t value);

int TYCalibrationParameterTest(TY_DEV_HANDLE hDevice, TY_COMPONENT_ID componentID, TY_FEATURE_ID featureID);

int TYCameraParameterTest(TY_DEV_HANDLE hDevice, TY_COMPONENT_ID componentID, TY_FEATURE_ID featureID, int value);

int TYBrightHistoTest(TY_DEV_HANDLE hDevice);

int TYLongTimeInitDeinitTest();

int TYLongTimeOpenCloseDeviceTest(const char* deviceID,TY_DEV_HANDLE hDevice);

int TYLongTimeStartStopCaptureTest(TY_DEV_HANDLE hDevice);

int TYLongTimeRepeatedlyTest(const char* deviceID, TY_DEV_HANDLE hDevice);

int TYLongTimeGetEnableComponentsTest(TY_DEV_HANDLE hDevice);

int TYLongTimeEnableDisableComponentsTest(TY_DEV_HANDLE hDevice);

int TYWorldDepthTransferTest(TY_DEV_HANDLE hDevice);

int TYLibversionTest(TY_VERSION_INFO* _pVer, TY_VERSION_INFO std);

int TYDeviceInfoTest(TY_DEV_HANDLE hDevice, TY_DEVICE_BASE_INFO _info);

int TYTriggerModeTest(TY_DEV_HANDLE hDevice, bool flag);

int TYCaptureOutputTest(const char* deviceID, TY_DEV_HANDLE hDevice);

int TYLongTimeDeviceNumTest( int num);

int TYExposureTimeTest(TY_DEV_HANDLE hDevice,TY_COMPONENT_ID componentID);

int TYSingleSensorFrameRateTest(TY_DEV_HANDLE hDevice, TY_COMPONENT_ID componentID,int stdFrameRate);
