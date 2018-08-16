#include"APItestCode.h"

static unsigned int len =  pow(2, 10) - 1;

int TYResolutionTest(const char* deviceID,TY_DEV_HANDLE hDevice, TY_COMPONENT_ID componentID, TY_FEATURE_ID featureID, int32_t value)
{
	bool captfalg = false;
	TY_FEATURE_INFO m_info;
	m_info.isValid = false;
	m_info.accessMode = 0;
	int width = 0, height = 0;
	TY_FRAME_DATA frame;
	TY_STATUS ty_status;
	switch (value)
	{
	case TY_IMAGE_MODE_320x240:
		width = 320;
		height = 240;
		break;
	case TY_IMAGE_MODE_640x480:
		width = 640;
		height = 480;
		break;
	case TY_IMAGE_MODE_2592x1944:
	case TY_IMAGE_MODE_1280x960:
		width = 1280;
		height = 960;
		break;
	default:
		break;
	}
	std::string  devicename;
	switch (componentID)
	{
	case TY_COMPONENT_DEPTH_CAM:
		devicename = "TY_COMPONENT_DEPTH_CAM";
		break;
	case TY_COMPONENT_IR_CAM_LEFT:
		devicename = "TY_COMPONENT_IR_CAM_LEFT";
		break;
	case TY_COMPONENT_IR_CAM_RIGHT:
		devicename = "TY_COMPONENT_IR_CAM_RIGHT";
		break;
	case TY_COMPONENT_RGB_CAM_LEFT:
		devicename = "TY_COMPONENT_RGB_CAM_LEFT";
		break;
	case TY_COMPONENT_POINT3D_CAM:
		devicename = "TY_COMPONENT_POINT3D_CAM";
		break;
	default:
		break;
	}
	if (componentID == TY_COMPONENT_RGB_CAM_LEFT)
	{
		int32_t componentIDs;
		ASSERT_OK(TYGetComponentIDs(hDevice, &componentIDs));
		if (!(componentIDs&TY_COMPONENT_RGB_CAM_LEFT))
		{
			std::cout << "[!!!WARNING!!!]: No TY_COMPONENT_RGB_CAM_LEFT Component " << std::endl;
			return TY_SUCCESS_TEST;
		}
	}
	ASSERT_OK(TYIsCapturing(hDevice, &captfalg));
	if (captfalg)
	{
		ASSERT_OK(TYStopCapture(hDevice));
	}
	int32_t componentIDs;
	ASSERT_OK(TYGetComponentIDs(hDevice, &componentIDs));
	for (int32_t i = 0; i < sizeof(PercipioDeviceComponent) / sizeof(int32_t); i++)
	{
		if (componentIDs&PercipioDeviceComponent[i] && PercipioDeviceComponent[i] != TY_COMPONENT_BRIGHT_HISTO)
		{
			ASSERT_OK(TYEnableComponents(hDevice, PercipioDeviceComponent[i]));
		}
	}
	ASSERT_OK(TYGetFeatureInfo(hDevice, componentID, featureID, &m_info));
	
	if(componentID == TY_COMPONENT_IR_CAM_LEFT || componentID == TY_COMPONENT_IR_CAM_RIGHT || componentID == TY_COMPONENT_POINT3D_CAM)
	{
		if (m_info.isValid)
		{
			std::cout << "[###FAIL###]: Device:"<< devicename <<" The feature: [TY_ENUM_IMAGE_MODE] is valid"<< std::endl;
			return TY_FAULT_APPEARS;
		}
		if (m_info.accessMode & TY_ACCESS_WRITABLE)
		{
			std::cout << "[###FAIL###]: Device:" << devicename << "  The feature: [TY_ENUM_IMAGE_MODE]  is writeable" << std::endl;
			return TY_FAULT_APPEARS;
		}
	}
	else
	{
		if (!m_info.isValid)
		{
			std::cout << "[###FAIL###]: Device:"<< devicename <<" The feature: [TY_ENUM_IMAGE_MODE] is invalid"<< std::endl;
			return TY_FEATURE_INVALID;
		}
		if (!(m_info.accessMode & TY_ACCESS_WRITABLE))
		{
			std::cout << "[###FAIL###]: Device:" << devicename << "  The feature: [TY_ENUM_IMAGE_MODE]  is unwriteable" << std::endl;
			return TY_NOT_WRITEABLE;
		}
		ty_status = TYSetEnum(hDevice, componentID, featureID, value);
		if (ty_status != TY_STATUS_OK)
		{
			std::cout << "[###FAIL###]: Device:" << devicename << "  The feature: [TY_ENUM_IMAGE_MODE] normal value write failed, error code:" << ty_status << std::endl;
			return TY_NORMALVALUE_SET_FAIL;
		}
		else{
			int32_t frameSize;
			char* frameBuffer[2];
			TYGetFrameBufferSize(hDevice, &frameSize);
			frameBuffer[0] = new char[frameSize];
    		frameBuffer[1] = new char[frameSize];
			ASSERT_OK( TYEnqueueBuffer(hDevice, frameBuffer[0], frameSize) );
			ASSERT_OK( TYEnqueueBuffer(hDevice, frameBuffer[1], frameSize) );
		}
	}
	ASSERT_OK(TYStartCapture(hDevice));
	ty_status = TYFetchFrame(hDevice, &frame, 10000);
	if(ty_status != TY_STATUS_OK)
	{
		std::cout << "[###FAIL###]: Device:" << devicename << "  The resolution test is Fail, Fetchframe no response" << std::endl;
		return TY_FAULT_APPEARS;
	}
	cv::Mat depth, irl, irr, color, pt3d;
	parseFrame(frame, &depth, &irl, &irr, &color, &pt3d);
	ty_status = TYEnqueueBuffer(hDevice, frame.userBuffer, frame.bufferSize);
	ty_status = TYStopCapture(hDevice);
	if (componentID == TY_COMPONENT_IR_CAM_LEFT)
	{
		if (irl.cols != width || irl.rows != height)
		{
			std::cout << "[###FAIL###]: Device:" << devicename << " The resolution of TY_COMPONENT_IR_CAM_LEFT is Wrong";
			std::cout << ", Designed value is " << width << "*" << height << ", Frame got value is " << irl.cols << "*" << irl.rows << std::endl;
			return TY_WRONG_VALUE;
		}
	}
	if (componentID == TY_COMPONENT_IR_CAM_RIGHT)
	{
		if (irr.cols != width || irr.rows != height)
		{
			std::cout << "[###FAIL###]: Device:" << devicename << "  The resolution of TY_COMPONENT_IR_CAM_RIGHT is Wrong";
			std::cout << ", Designed value is " << width << "*" << height << ", Frame got value is " << irr.cols << "*" << irr.rows << std::endl;
			return TY_WRONG_VALUE;
		}
	}
	if (componentID == TY_COMPONENT_RGB_CAM_LEFT)
	{
		if (color.cols != width || color.rows != height)
		{
			std::cout << "[###FAIL###]: Device:" << devicename << "  The resolution of TY_COMPONENT_RGB_CAM_LEFT is Wrong";
			std::cout << ", Seted value is " << width << "*" << height << ", Frame got value is " << color.cols << "*" << color.rows << std::endl;
			return TY_WRONG_VALUE;
		}
	}
	if (componentID == TY_COMPONENT_DEPTH_CAM)
	{
		if (depth.cols != width || depth.rows != height)
		{
			std::cout << "[###FAIL###]: Device: TY_COMPONENT_DEPTH_CAM , The resolution of TY_COMPONENT_DEPTH_CAM is Wrong";
			std::cout << ", Seted value is " << width << "*" << height << ", Frame got value is " << depth.cols << "*" << depth.rows << std::endl;
			return TY_WRONG_VALUE;
		}
		if (pt3d.cols != width || pt3d.rows != height)
		{
			std::cout << "[###FAIL###]: Device: TY_COMPONENT_POINT3D_CAM  The resolution of TY_COMPONENT_POINT3D_CAM is Wrong";
			std::cout << ", Seted value is " << width << "*" << height << ", Frame got value is " << pt3d.cols << "*" << pt3d.rows << std::endl;
			return TY_WRONG_VALUE;
		}
	}
	std::cout << "[***PASS***]: Device:" << devicename << "  The resolution test is SUCCESS" << std::endl;
	return TY_SUCCESS_TEST;
}

int TYCalibrationParameterTest(TY_DEV_HANDLE hDevice, TY_COMPONENT_ID componentID, TY_FEATURE_ID featureID)
{
	std::string  featurename;
	switch (featureID)
	{
	case TY_STRUCT_CAM_INTRINSIC:
		featurename = "TY_STRUCT_CAM_INTRINSIC";
		break;
	case TY_STRUCT_CAM_DISTORTION:
		featurename = "TY_STRUCT_CAM_DISTORTION";
		break;
	case TY_STRUCT_EXTRINSIC_TO_LEFT_IR:
		featurename = "TY_STRUCT_EXTRINSIC_TO_LEFT_IR";
		break;
	default:
		break;
	}
	std::string  devicename;
	switch (componentID)
	{
	case TY_COMPONENT_DEPTH_CAM:
		devicename = "TY_COMPONENT_DEPTH_CAM";
		break;
	case TY_COMPONENT_IR_CAM_LEFT:
		devicename = "TY_COMPONENT_IR_CAM_LEFT";
		break;
	case TY_COMPONENT_IR_CAM_RIGHT:
		devicename = "TY_COMPONENT_IR_CAM_RIGHT";
		break;
	case TY_COMPONENT_RGB_CAM_LEFT:
		devicename = "TY_COMPONENT_RGB_CAM_LEFT";
		break;
	default:
		break;
	}
	if (componentID == TY_COMPONENT_RGB_CAM_LEFT)
	{
		int32_t componentIDs;
		ASSERT_OK(TYGetComponentIDs(hDevice, &componentIDs));
		if (!(componentIDs&TY_COMPONENT_RGB_CAM_LEFT))
		{
			std::cout << "[!!!WARNING!!!]: No TY_COMPONENT_RGB_CAM_LEFT Component " << std::endl;
			return TY_SUCCESS_TEST;
		}
	}
	
	TY_CAMERA_INTRINSIC  mIRIntrinsic;
	TY_CAMERA_DISTORTION mIRDistortion;
	TY_CAMERA_EXTRINSIC  mCameraExtrinsic;
	TY_FEATURE_INFO m_info;
	ASSERT_OK(TYGetFeatureInfo(hDevice, componentID, featureID, &m_info));
	if (!m_info.isValid)
	{
		std::cout << "[###FAIL###]: Device:"<< devicename <<" The feature of ["<< featurename <<"] is invalid" << std::endl;
		return TY_FEATURE_INVALID;
	}
	if (!(m_info.accessMode&TY_ACCESS_READABLE))
	{
		std::cout << "[###FAIL###]: The feature of [" << featurename << "] is unreadable" << std::endl;
		return TY_NOT_READABLE;
	}
	if (featureID == TY_STRUCT_CAM_INTRINSIC)
	{
		ASSERT_OK(TYGetStruct(hDevice, componentID, featureID, (void*)&mIRIntrinsic,sizeof(mIRIntrinsic)));
		if (!(mIRIntrinsic.data[0] != 0 && mIRIntrinsic.data[2] != 0 && mIRIntrinsic.data[4] != 0 && mIRIntrinsic.data[5] != 0 && mIRIntrinsic.data[8] == 1))
		{
			std::cout << "[###FAIL###]: The value of TY_STRUCT_CAM_INTRINSIC is Wrong" << std::endl;
			return TY_WRONG_VALUE;
		}
	}
	if (featureID == TY_STRUCT_CAM_DISTORTION)
	{
		ASSERT_OK(TYGetStruct(hDevice, componentID, featureID, (void*)&mIRDistortion, sizeof(mIRDistortion)));
		if (!(mIRDistortion.data[0] != 0 && mIRDistortion.data[1] != 0))
		{
			std::cout << "[###FAIL###]: The value of TY_STRUCT_CAM_DISTORTION is Wrong" << std::endl;
			return TY_WRONG_VALUE;
		}
	}
	if (featureID == TY_STRUCT_EXTRINSIC_TO_LEFT_IR)
	{
		ASSERT_OK(TYGetStruct(hDevice, componentID, featureID, (void*)&mCameraExtrinsic, sizeof(mCameraExtrinsic)));
		if (!(mCameraExtrinsic.data[15] == 1))
		{
			std::cout << "[###FAIL###]: The value of TY_STRUCT_EXTRINSIC_TO_LEFT_IR is Wrong" << std::endl;
			return TY_WRONG_VALUE;
		}
	}
	std::cout << "[***PASS***]: Device:" << devicename << " The feature of [" << featurename << "] is SUCCSEE" << std::endl;
	return TY_SUCCESS_TEST;
}

int TYCameraParameterTest(TY_DEV_HANDLE hDevice, TY_COMPONENT_ID componentID, TY_FEATURE_ID featureID, int value)
{
	std::string  featurename;
	switch (featureID)
	{
	case TY_INT_GAIN:
		featurename = "TY_INT_GAIN";
		break;
	case TY_INT_EXPOSURE_TIME:
		featurename = "TY_INT_EXPOSURE_TIME";
		break;
	case TY_BOOL_AUTO_AWB:
		featurename = "TY_BOOL_AUTO_AWB";
		break;
	default:
		break;
	}
	std::string  devicename;
	switch (componentID)
	{
	case TY_COMPONENT_IR_CAM_LEFT:
		devicename = "TY_COMPONENT_IR_CAM_LEFT";
		break;
	case TY_COMPONENT_IR_CAM_RIGHT:
		devicename = "TY_COMPONENT_IR_CAM_RIGHT";
		break;
	case TY_COMPONENT_RGB_CAM_LEFT:
		devicename = "TY_COMPONENT_RGB_CAM_LEFT";
		break;
	default:
		break;
	}
	if (componentID == TY_COMPONENT_RGB_CAM_LEFT)
	{
		int32_t componentIDs;
		ASSERT_OK(TYGetComponentIDs(hDevice, &componentIDs));
		if (!(componentIDs&TY_COMPONENT_RGB_CAM_LEFT))
		{
			std::cout << "[!!!WARNING!!!]: No TY_COMPONENT_RGB_CAM_LEFT Component " << std::endl;
			return TY_SUCCESS_TEST;
		}
	}
	bool captflag;
	ASSERT_OK(TYIsCapturing(hDevice, &captflag));
	if (captflag)
	{
		ASSERT_OK(TYStopCapture(hDevice));
	}
	int32_t _val;
	TY_STATUS ty_status;
	TY_FEATURE_INFO m_info;
	m_info.isValid = false;
	m_info.accessMode = 0;
	ASSERT_OK(TYGetFeatureInfo(hDevice, componentID, featureID, &m_info));
	if (!m_info.isValid)
	{
		std::cout << "[###FAIL###]: Device:"<< devicename <<" The feature of ["<<featurename<<"] is invalid" << std::endl;
		return TY_FEATURE_INVALID;
	}
	if (!(m_info.accessMode & TY_ACCESS_WRITABLE))
	{
		std::cout << "[###FAIL###]: Device:" << devicename << "  The feature of [" << featurename << "] is unwriteable" << std::endl;
		return TY_NOT_WRITEABLE;
	}
	if (featureID == TY_BOOL_AUTO_AWB)
	{
		value > 0 ? true : false;
		ty_status = TYSetBool(hDevice, componentID, featureID, value);
	}
	else
	{
		ty_status = TYSetInt(hDevice, componentID, featureID, value);
	}
	if (ty_status != TY_STATUS_OK)
	{
		std::cout << "[###FAIL###]: Device:" << devicename << "  The feature of [" << featurename << "] fail to set, error code:" << ty_status << std::endl;
		return TY_NORMALVALUE_SET_FAIL;
	}
	if (!(m_info.accessMode & TY_ACCESS_READABLE))
	{
		std::cout << "[###FAIL###]: Device:" << devicename << "  The feature of [" << featurename << "] is unreadable" << std::endl;
		return TY_NOT_READABLE;
	}
	ASSERT_OK(TYGetInt(hDevice, componentID, featureID, &_val));
	if (_val != value)
	{
		std::cout << "[###FAIL###]: Device:" << devicename << "  The value [" << featurename << "] getted is different with setted " << std::endl;
		return TY_WRONG_VALUE;
	}
	std::cout << "[***PASS***]: Device:" << devicename << "  Set camera parameters ["<<featurename<<"] test is SUCCESS" << std::endl;
	return TY_SUCCESS_TEST;
}

int TYBrightHistoTest(TY_DEV_HANDLE hDevice)
{
	int32_t allComps;
	int32_t BrightHisto;
	TY_STATUS ty_status;
	bool captflag;
	ty_status = TYIsCapturing(hDevice, &captflag);
	if (captflag)
	{
		ASSERT_OK(TYStopCapture(hDevice));
	}
	ASSERT_OK(TYGetComponentIDs(hDevice, &allComps));
	if (!(allComps & TY_COMPONENT_BRIGHT_HISTO))
	{
		std::cout << "[###FAIL###]: Can not find TY_COMPONENT_BRIGHT_HISTO component" << std::endl;
		return TY_WRONG_VALUE;
	}
	ASSERT_OK(TYGetEnabledComponentIDs(hDevice,&BrightHisto));
	if (!(BrightHisto&TY_COMPONENT_BRIGHT_HISTO))
	{
		ASSERT_OK(TYEnableComponents(hDevice, TY_COMPONENT_BRIGHT_HISTO));
		ASSERT_OK(TYGetEnabledComponentIDs(hDevice, &BrightHisto));
	}
	if (!(BrightHisto&TY_COMPONENT_BRIGHT_HISTO))
	{
		std::cout << "[###FAIL###]: Can not find TY_COMPONENT_BRIGHT_HISTO component" << std::endl;
		return TY_WRONG_VALUE;
	}
	else
	{
		ASSERT_OK(TYStartCapture(hDevice));
		TY_FRAME_DATA frame;
		TY_STATUS ty_status = TYFetchFrame(hDevice, &frame, 10000);
		if(ty_status != TY_STATUS_OK)
		{
			std::cout << "[###FAIL###]: TY_COMPONENT_BRIGHT_HISTO component is existed BUT Device can not capture " << std::endl;
			return TY_FAULT_APPEARS;
		}
		
		std::cout << "[***PASS***]: TY_COMPONENT_BRIGHT_HISTO component is existed and Device work correctly " << std::endl;
		return TY_SUCCESS_TEST;
	}
}

int TYLongTimeInitDeinitTest()
{
	ASSERT_OK(TYDeinitLib());
	TY_STATUS ty_status, ty_status0 = -1, ty_status1 = -1;
	int failcnt = 0;
	for (unsigned int i = 0; i < len; i++)
	{
		ty_status = TYInitLib();
		if (ty_status != TY_STATUS_OK)
		{
			failcnt++;
			std::cout << "[###FAIL###]: Fail to run TYInitLib, error code:" << ty_status << std::endl;
			return TY_FAULT_APPEARS;
		}
		ty_status = TYDeinitLib();
		if (ty_status != TY_STATUS_OK)
		{
			failcnt++;
			std::cout << "[###FAIL###]: Fail to run TYDeinitLib, error code:" << ty_status << std::endl;
			return TY_FAULT_APPEARS;
		}
	}
	if(failcnt > 0)
	{
		do{
			ty_status0 = TYInitLib();
			ty_status1 = TYDeinitLib();
		}while(ty_status0 != TY_STATUS_OK || ty_status1 != TY_STATUS_OK);
		std::cout << "[###FAIL###]: Long Time Init&Deinit LIB Test is FAIL"<< std::endl;
		return TY_FAULT_APPEARS;
	}
	else{
		std::cout << "[***PASS***]: Long Time Init&Deinit LIB Test is SUCCESS " << std::endl;
		ASSERT_OK(TYInitLib());
		return TY_SUCCESS_TEST;
	}
	
}

int TYLongTimeOpenCloseDeviceTest(const char* deviceID,TY_DEV_HANDLE hDevice)
{
	TY_STATUS ty_status,ty_status0 = -1,ty_status1 = -1;;
	ty_status = TYCloseDevice(hDevice);
	int failcnt = 0;
	for (unsigned int i = 0; i < len; i++)
	{
		ty_status = TYOpenDevice(deviceID, &hDevice);
		if (TY_STATUS_OK != ty_status)
		{
			failcnt++;
		}
		ty_status = TYCloseDevice(hDevice);
		if (TY_STATUS_OK != ty_status)
		{
			failcnt++;
		}
	}
	if(failcnt > 0)
	{
		do{
			ty_status0 = TYOpenDevice(deviceID, &hDevice);
			ty_status1 = TYCloseDevice(hDevice);
			
		}while(ty_status0 != TY_STATUS_OK || ty_status1 != TY_STATUS_OK);
		std::cout << "[###FAIL###]: Long Time Open&Close Device Test is FAIL" << std::endl;
		return TY_FAULT_APPEARS;
	}
	else{
		std::cout << "[***PASS***]: Long Time Open&Close Device Test is SUCCESS" << std::endl;
		return TY_SUCCESS_TEST;
	}
	
	
}

int TYLongTimeStartStopCaptureTest(TY_DEV_HANDLE hDevice)
{
	TY_STATUS ty_status,ty_status0 = -1,ty_status1 = -1;
	ty_status = TYStopCapture(hDevice);
	int failcnt = 0;
	for (unsigned int i = 0; i < len; i++)
	{
		ty_status = TYStartCapture(hDevice);
		if (ty_status != TY_STATUS_OK)
		{
			failcnt++;
		}
		ty_status = TYStopCapture(hDevice);
		if (ty_status != TY_STATUS_OK)
		{
			failcnt++;
		}
	}
	if(failcnt > 0)
	{
		do{
			ty_status0 = TYStartCapture(hDevice);
			ty_status1 = TYStopCapture(hDevice);
		}while(ty_status0 != TY_STATUS_OK || ty_status1 != TY_STATUS_OK);
		std::cout << "[###FAIL###]: Long Time Start&Stop Capture Test is FAIL"<< std::endl;
		return TY_FAULT_APPEARS;
	}
	else
	{
		std::cout << "[***PASS***]: Long Time Start&Stop Capture Test is SUCCESS" << std::endl;
		return TY_SUCCESS_TEST;
	}
	
}

int TYLongTimeRepeatedlyTest(const char* deviceID, TY_DEV_HANDLE hDevice)
{
	int32_t componentIDs;
	TY_STATUS ty_status;
	ty_status = TYStopCapture(hDevice);
	ty_status = TYCloseDevice(hDevice);
	ty_status = TYDeinitLib();

	for (unsigned int i = 0; i < len; i++)
	{
		ty_status = TYInitLib();
		if (ty_status != TY_STATUS_OK)
		{
			std::cout << "[###FAIL###]: Failed to run TYInitLib, error code:" << ty_status << std::endl;
			return TY_FAULT_APPEARS;
		}
		ty_status = TYOpenDevice(deviceID, &hDevice);
		if (ty_status != TY_STATUS_OK)
		{
			std::cout << "[###FAIL###]: Failed to run TYOpenDevice, error code:" << ty_status << std::endl;
			return TY_FAULT_APPEARS;
		}
		ASSERT_OK(TYGetComponentIDs(hDevice,&componentIDs));
		for (int32_t i = 0; i < sizeof(PercipioDeviceComponent)/sizeof(int32_t); i++)
		{
			if (componentIDs&PercipioDeviceComponent[i] && PercipioDeviceComponent[i] != TY_COMPONENT_BRIGHT_HISTO)
			{
				ASSERT_OK(TYEnableComponents(hDevice, PercipioDeviceComponent[i]));
			}
		}
		
		ty_status = TYStartCapture(hDevice);
		if (ty_status != TY_STATUS_OK)
		{
			std::cout << "[###FAIL###]: Failed to run TYStartCapture, error code:" << ty_status << std::endl;
			return TY_FAULT_APPEARS;
		}
		ty_status = TYStopCapture(hDevice);
		if (ty_status != TY_STATUS_OK)
		{
			std::cout << "[###FAIL###]: Failed to run TYStopCapture, error code:" << ty_status << std::endl;
			return TY_FAULT_APPEARS;
		}
		ty_status = TYCloseDevice(hDevice);
		if (ty_status != TY_STATUS_OK)
		{
			std::cout << "[###FAIL###]: Failed to run TYCloseDevice, error code:" << ty_status << std::endl;
			return TY_FAULT_APPEARS;
		}
		ty_status = TYDeinitLib();
		if (ty_status != TY_STATUS_OK)
		{
			std::cout << "[###FAIL###]: Failed to run TYDeinitLib, error code:" << ty_status << std::endl;
			return TY_FAULT_APPEARS;
		}
	}

	ty_status = TYInitLib();
	ty_status = TYOpenDevice(deviceID, &hDevice);
	ASSERT_OK(TYGetComponentIDs(hDevice, &componentIDs));
	for (int32_t i = 0; i < sizeof(PercipioDeviceComponent) / sizeof(int32_t); i++)
	{
		if (componentIDs&PercipioDeviceComponent[i] && PercipioDeviceComponent[i] != TY_COMPONENT_BRIGHT_HISTO)
		{
			ASSERT_OK(TYEnableComponents(hDevice, PercipioDeviceComponent[i]));
		}
	}
	ty_status = TYStartCapture(hDevice);

	std::cout << "[***PASS***]: Long Time Repeatedly Test is SUCCESS" << std::endl;
	return TY_SUCCESS_TEST;
}

int TYLongTimeGetEnableComponentsTest(TY_DEV_HANDLE hDevice)
{
	int32_t comp;
	int32_t componentIDs;
	TY_STATUS ty_status;
	bool captflag;
	ty_status = TYIsCapturing(hDevice, &captflag);
	if (captflag)
	{
		ASSERT_OK(TYStopCapture(hDevice));
	}
	ASSERT_OK(TYGetComponentIDs(hDevice, &componentIDs));
	for (unsigned int i = 0; i < len; i++)
	{
		for (int32_t j = 0; j < sizeof(PercipioDeviceComponent) / sizeof(int32_t); j++)
		{
			if (componentIDs&PercipioDeviceComponent[j] && PercipioDeviceComponent[i] != TY_COMPONENT_BRIGHT_HISTO)
			{
				ASSERT_OK(TYEnableComponents(hDevice, PercipioDeviceComponent[j]));
				ASSERT_OK(TYGetEnabledComponentIDs(hDevice, &comp));
				if (!(comp&PercipioDeviceComponent[j]))
				{
					std::cout << "[###FAIL###]: Enable component can not be Getted" << std::endl;
					return TY_WRONG_VALUE;
				}
			}
		}
	}

	ASSERT_OK(TYStartCapture(hDevice));
	std::cout << "[***PASS***]: Long Time Get Enable Components Test is SUCCESS" << std::endl;
	return TY_SUCCESS_TEST;
}

int TYLongTimeEnableDisableComponentsTest(TY_DEV_HANDLE hDevice)
{
	int32_t componentIDs;
	int32_t comp;
	TY_STATUS ty_status;
	bool captflag;
	ty_status = TYIsCapturing(hDevice, &captflag);
	if (captflag)
	{
		ASSERT_OK(TYStopCapture(hDevice));
	}
	ASSERT_OK(TYGetComponentIDs(hDevice, &componentIDs));
	for (unsigned int i = 0; i < len; i++)
	{
		for (int32_t j = 0; j < sizeof(PercipioDeviceComponent) / sizeof(int32_t); j++)
		{
			if (componentIDs&PercipioDeviceComponent[j] && PercipioDeviceComponent[i] != TY_COMPONENT_BRIGHT_HISTO)
			{
				ty_status = TYEnableComponents(hDevice, PercipioDeviceComponent[j]);
				if(ty_status != TY_STATUS_OK){
					std::cout<<"TYEnableComponents failed, Long Time EnableDisable Components Test is FAIL"<<std::endl;
					return TY_FAULT_APPEARS;
				}
				ty_status = TYGetEnabledComponentIDs(hDevice, &comp);
				if(ty_status != TY_STATUS_OK){
					std::cout<<"TYGetEnabledComponentIDs failed, Long Time EnableDisable Components Test is FAIL"<<std::endl;
					return TY_FAULT_APPEARS;
				}
				if (!(comp&PercipioDeviceComponent[j]))
				{
					std::cout << "[###FAIL###]: Enable component can not be Getted"<< std::endl;
					return TY_WRONG_VALUE;
				}
				ty_status = TYDisableComponents(hDevice, PercipioDeviceComponent[j]);
				if(ty_status != TY_STATUS_OK){
					std::cout<<"TYDisableComponents failed, Long Time EnableDisable Components Test is FAIL"<<std::endl;
					return TY_FAULT_APPEARS;
				}
				ty_status = TYGetEnabledComponentIDs(hDevice, &comp);
				if(ty_status != TY_STATUS_OK){
					std::cout<<"TYGetEnabledComponentIDs failed, Long Time EnableDisable Components Test is FAIL"<<std::endl;
					return TY_FAULT_APPEARS;
				}
				if (comp&PercipioDeviceComponent[j])
				{
					std::cout << "[###FAIL###]: Disable component be Getted"<< std::endl;
					return TY_WRONG_VALUE;
				}
			}
		}
	}
	for (int32_t i = 0; i < sizeof(PercipioDeviceComponent) / sizeof(int32_t); i++)
	{
		if (componentIDs&PercipioDeviceComponent[i] && PercipioDeviceComponent[i] != TY_COMPONENT_BRIGHT_HISTO)
		{
			TYEnableComponents(hDevice, PercipioDeviceComponent[i]);
		}
	}

	std::cout << "[***PASS***]: Long Time Enable&Disable Components Test is SUCCESS" << std::endl;
	return TY_SUCCESS_TEST;
}

int TYWorldDepthTransferTest(TY_DEV_HANDLE hDevice)
{
	bool captflag;
	TY_FRAME_DATA frame;
	int32_t componentIDs;
	TY_STATUS ty_status = TYIsCapturing(hDevice, &captflag);
	if (captflag)
	{
		ASSERT_OK(TYStopCapture(hDevice));
	}
	ASSERT_OK(TYGetComponentIDs(hDevice, &componentIDs));
	for (int32_t i = 0; i < sizeof(PercipioDeviceComponent) / sizeof(int32_t); i++)
	{
		if (componentIDs&PercipioDeviceComponent[i] && PercipioDeviceComponent[i] != TY_COMPONENT_BRIGHT_HISTO)
		{
			ASSERT_OK(TYEnableComponents(hDevice, PercipioDeviceComponent[i]));
		}
	}
	ASSERT_OK(TYStartCapture(hDevice));
	ty_status = TYFetchFrame(hDevice, &frame, 3000);
	if(ty_status != TY_STATUS_OK)
	{
		std::cout<<"[###FAIL###]: FetchFrame does not wrok"<<std::endl;
		ASSERT_OK(TYStopCapture(hDevice));
		return TY_FAULT_APPEARS;
	}

	cv::Mat depth, irl, irr, color, pt3d;
	parseFrame(frame, &depth, &irl, &irr, &color, &pt3d);
	ty_status = TYEnqueueBuffer(hDevice, frame.userBuffer, frame.bufferSize);
	ty_status = TYStopCapture(hDevice);
	static TY_VECT_3F depthbuf[1280 * 960];
	static TY_VECT_3F worldbuf[1280 * 960];
	static TY_VECT_3F newdepthbuf[1280 * 960];
	if (!depth.empty())
	{
		int k = 0;
		uint16_t* pdepth = (uint16_t*)depth.data;
		for (int r = 0; r < depth.rows; r++)
		{
			for (int c = 0; c < depth.cols; c++)
			{
				depthbuf[k].x = c;
				depthbuf[k].y = r;
				depthbuf[k].z = pdepth[k];
				k++;
			}
		}
		ty_status = TYDepthToWorld(hDevice, depthbuf, worldbuf, 0, depth.rows*depth.cols);
		if (ty_status != TY_STATUS_OK)
		{
			std::cout << "[###FAIL###]: Fail to run depth to world, error code:" << ty_status << std::endl;
			return TY_FAULT_APPEARS;
		}
	}
	int sum = 0, cnt = 0;
	for (int i = 0; i < depth.rows*depth.cols; i = i + 800)
	{
		sum += worldbuf[i].z;
		cnt++;
	}
	sum = sum / cnt;
	if (sum <= 0)
	{
		std::cout << "[###FAIL###]: The value of depth to world is wrong"<< std::endl;
		return TY_WRONG_VALUE;
	}
	else
	{
		ty_status = TYWorldToDepth(hDevice, worldbuf, newdepthbuf, 0, depth.rows*depth.cols);
		if (ty_status != TY_STATUS_OK)
		{
			std::cout << "[###FAIL###]: Fail to run world to depth, error code:" << ty_status << std::endl;
			return TY_FAULT_APPEARS;
		}
		else
		{
			std::cout << "[***PASS***]: Depth to world test is SUCCESS" << std::endl;
			int diffvalue = 0, sum_diffvalue = 0;
			for (int i = 0; i < depth.rows; i++)
			{
				for (int j = 0; j < depth.cols; j++)
				{
					diffvalue = abs(newdepthbuf[i*depth.cols + j].z - depthbuf[i*depth.cols + j].z);
					sum_diffvalue += diffvalue;
				}
			}
			if (sum_diffvalue > 100)
			{
				std::cout << "[###FAIL###]: The value of world to depth is wrong"<< std::endl;
				return TY_WRONG_VALUE;
			}
		}
		std::cout << "[***PASS***]: World to depth test is SUCCESS" << std::endl;
		return TY_SUCCESS_TEST;
	}
}

int TYLibversionTest(TY_VERSION_INFO* _pVer, TY_VERSION_INFO std)
{	
	TY_STATUS ty_status = TYLibVersion(_pVer);
	if (ty_status != TY_STATUS_OK)
	{
		std::cout << "[###FAIL###]: Get LIB Version Failed, error code:" << ty_status << std::endl;
		return TY_FAULT_APPEARS;
	}
	if (_pVer->major!= std.major || _pVer->minor!= std.minor || _pVer->patch != std.patch)
	{
		std::cout << "[###FAIL###]: The LIB Version is Wrong ";
		std::cout << "major:" << _pVer->major << " minor:" << _pVer->minor << " patch:" << _pVer->patch << std::endl;
	}
	std::cout << "[***PASS***]: LIB Version is OK" << std::endl;
	return TY_SUCCESS_TEST;
}

int TYDeviceInfoTest(TY_DEV_HANDLE hDevice, TY_DEVICE_BASE_INFO _info)
{
	int failcnt = 0;
	TY_DEVICE_BASE_INFO info;
	TY_STATUS ty_status = TYGetDeviceInfo(hDevice, &info);
	if (ty_status != TY_STATUS_OK)
	{
		std::cout << "[###FAIL###]: Get Device Information Failed, error code:" << ty_status << std::endl;
		return TY_FAULT_APPEARS;
	}
	if(info.devInterface != _info.devInterface)
	{
		std::cout << "[###FAIL###]: The Device Information devInterface is Wrong" << std::endl;
		std::cout<< " Actual devInterface: "<<_info.devInterface << std::endl;
		std::cout<< " Test devInterface: "<<info.devInterface << std::endl;
		failcnt++;
	}
	if(info.hardwareVersion.major != _info.hardwareVersion.major || 
	   info.hardwareVersion.minor != _info.hardwareVersion.minor || info.hardwareVersion.patch != _info.hardwareVersion.patch)
	{
		std::cout << "[###FAIL###]: The Device Information hardwareVersion is Wrong" << std::endl;
		std::cout << " Actual hardwareVersion: " << _info.hardwareVersion.major << " " << _info.hardwareVersion.minor << " " << _info.hardwareVersion.patch << std::endl;
		std::cout << " Test hardwareVersion: " << info.hardwareVersion.major << " " << info.hardwareVersion.minor << " " << info.hardwareVersion.patch << std::endl;
		failcnt++;
	}
	if(info.firmwareVersion.major != _info.firmwareVersion.major || 
	   info.firmwareVersion.minor != _info.firmwareVersion.minor || info.firmwareVersion.patch != _info.firmwareVersion.patch)
	{
		std::cout << "[###FAIL###]: The Device Information firmwareVersion is Wrong" << std::endl;
		std::cout << " Actual firmwareVersion: " << info.firmwareVersion.major << " " << info.firmwareVersion.minor << " " << info.firmwareVersion.patch << std::endl;
		std::cout << " Test firmwareVersion: " << _info.firmwareVersion.major << " " << _info.firmwareVersion.minor << " " << _info.firmwareVersion.patch << std::endl;
		failcnt++;
	}
	
	#if 0
	std::cout << "--Percipio Device infomation: " << std::endl;
	std::cout << "  devInterface: " << info.devInterface << std::endl;
	std::cout << "  id: " << info.id << std::endl;
	std::cout << "  vendorName: " << info.vendorName << std::endl;
	std::cout << "  modelName: " << info.modelName << std::endl;
	std::cout << "  hardwareVersion: " << info.hardwareVersion.major << " " << info.hardwareVersion.minor << " " << info.hardwareVersion.patch << std::endl;
	std::cout << "  firmwareVersion: " << info.firmwareVersion.major << " " << info.firmwareVersion.minor << " " << info.firmwareVersion.patch << std::endl;
	std::cout << "  netInfo: " << std::endl;
	std::cout << "    netmask: " << info.netInfo.netmask << std::endl;
	std::cout << "    gateway: " << info.netInfo.gateway << std::endl;
	std::cout << "    mac: " << info.netInfo.mac << std::endl;
	std::cout << "    ip: " << info.netInfo.ip << std::endl;
	#endif
	if(failcnt > 0)
	{
		std::cout << "[###FAIL###]: Device information test is FAIL" << std::endl;
	}
	else
	{
		std::cout << "[***PASS***]: Device information test is SUCCESS" << std::endl;
	}
	
	return TY_SUCCESS_TEST;
}

int TYTriggerModeTest(TY_DEV_HANDLE hDevice, bool flag)
{
	TY_FEATURE_INFO m_info;
	m_info.isValid = false;
	m_info.accessMode = 0;
	TY_STATUS ty_status;
	bool captflag;
	ty_status = TYIsCapturing(hDevice, &captflag);
	if (captflag)
	{
		ASSERT_OK(TYStopCapture(hDevice));
	}
	ty_status = TYGetFeatureInfo(hDevice, TY_COMPONENT_DEVICE, TY_BOOL_TRIGGER_MODE, &m_info);
	if (ty_status != TY_STATUS_OK)
	{
		std::cout << "[###FAIL###]: Get Trigger mode feature Failed, error code:" << ty_status << std::endl;
		return TY_FAULT_APPEARS;
	}
	if (!(m_info.accessMode & TY_ACCESS_WRITABLE))
	{
		std::cout << "[###FAIL###]: The feature of [TY_BOOL_TRIGGER_MODE] is unwriteable" << std::endl;
		return TY_NOT_WRITEABLE;
	}
	ty_status = TYSetBool(hDevice, TY_COMPONENT_DEVICE, TY_BOOL_TRIGGER_MODE, flag);
	if (ty_status != TY_STATUS_OK)
	{
		std::cout << "[###FAIL###]: Set Trigger mode feature Failed, error code:" << ty_status << std::endl;
		return TY_NORMALVALUE_SET_FAIL;
	}

	int32_t componentIDs;
	ASSERT_OK(TYGetComponentIDs(hDevice, &componentIDs));
	for (int32_t j = 0; j < sizeof(PercipioDeviceComponent) / sizeof(int32_t); j++)
	{
		if (componentIDs&PercipioDeviceComponent[j] && PercipioDeviceComponent[j] != TY_COMPONENT_BRIGHT_HISTO)
		{
			ASSERT_OK(TYEnableComponents(hDevice, PercipioDeviceComponent[j]));
		}
	}
	ASSERT_OK(TYSetEnum(hDevice, TY_COMPONENT_DEPTH_CAM, TY_ENUM_IMAGE_MODE, TY_IMAGE_MODE_640x480));
	int32_t frameSize;
	char* frameBuffer[2];
	TYGetFrameBufferSize(hDevice, &frameSize);
	frameBuffer[0] = new char[frameSize];
    frameBuffer[1] = new char[frameSize];
	ASSERT_OK( TYEnqueueBuffer(hDevice, frameBuffer[0], frameSize) );
	ASSERT_OK( TYEnqueueBuffer(hDevice, frameBuffer[1], frameSize) );

	ASSERT_OK(TYStartCapture(hDevice));
	unsigned int framecnt = len;
	while (framecnt > 0)
	{
		TY_FRAME_DATA frame;
		ty_status = TYSendSoftTrigger(hDevice);
		if (ty_status != TY_STATUS_OK)
		{
			std::cout << "[###FAIL###]: Trigger mode test, Send trigger signal failed,error code: "<<ty_status << std::endl;
			ASSERT_OK(TYStopCapture(hDevice));
			return TY_FAULT_APPEARS;
		}
		int err = TYFetchFrame(hDevice, &frame, 3000);
		if (err == TY_STATUS_OK) 
		{
			cv::Mat depth, irl, irr, color;
			parseFrame(frame, &depth, &irl, &irr, &color, 0);
			ty_status = TYEnqueueBuffer(hDevice, frame.userBuffer, frame.bufferSize);
			if (irl.empty()) 
			{
				std::cout << "[###FAIL###]: IR left is empty" << std::endl;
				ASSERT_OK(TYStopCapture(hDevice));
				return TY_WRONG_VALUE;
			}
			if (irr.empty()) 
			{
				std::cout << "[###FAIL###]: IR right is empty" << std::endl;
				ASSERT_OK(TYStopCapture(hDevice));
				return TY_WRONG_VALUE;
			}
			if (componentIDs&TY_COMPONENT_RGB_CAM_LEFT)
			{
				if (color.empty())
				{
					std::cout << "[###FAIL###]: Color is empty" << std::endl;
					ASSERT_OK(TYStopCapture(hDevice));
					return TY_WRONG_VALUE;
				}
			}
		}
		else
		{
			std::cout<<"Trigger mode test, fetchframe can not work"<<std::endl;
			ASSERT_OK(TYStopCapture(hDevice));
			return TY_FAULT_APPEARS;
		}
		framecnt--;
	}
	ASSERT_OK(TYStopCapture(hDevice));
	std::cout << "[***PASS***]: Device trigger mode test is SUCCESS" << std::endl;
	return TY_SUCCESS_TEST;
}

int TYCaptureOutputTest(const char* deviceID, TY_DEV_HANDLE hDevice)
{
	bool captfalg = false;
	TY_FEATURE_INFO m_info;
	m_info.isValid = false;
	m_info.accessMode = 0;
	int width = 0, height = 0;
	TY_FRAME_DATA frame;
	int failcnt = 0;
	int32_t componentIDs;
	ASSERT_OK(TYGetComponentIDs(hDevice, &componentIDs));
	ASSERT_OK(TYIsCapturing(hDevice, &captfalg));
	if (captfalg)
	{
		ASSERT_OK(TYStopCapture(hDevice));
	}

	ASSERT_OK(TYGetFeatureInfo(hDevice, TY_COMPONENT_DEPTH_CAM, TY_ENUM_IMAGE_MODE, &m_info));
	if (!m_info.isValid)
	{
		std::cout << "[###FAIL###]: The resolutio is invalid" << std::endl;
		return TY_FEATURE_INVALID;
	}
	if (!(m_info.accessMode & TY_ACCESS_WRITABLE))
	{
		std::cout << "[###FAIL###]: The resolutio  is unwriteable" << std::endl;
		return TY_NOT_WRITEABLE;
	}
	for (int32_t j = 0; j < sizeof(PercipioDeviceComponent) / sizeof(int32_t); j++)
	{
		if (componentIDs&PercipioDeviceComponent[j] && PercipioDeviceComponent[j] != TY_COMPONENT_BRIGHT_HISTO)
		{
			ASSERT_OK(TYEnableComponents(hDevice, PercipioDeviceComponent[j]));
		}
	}
	ASSERT_OK(TYSetEnum(hDevice, TY_COMPONENT_DEPTH_CAM, TY_ENUM_IMAGE_MODE, TY_IMAGE_MODE_640x480));
	int32_t frameSize;
	char* frameBuffer[2];
	TYGetFrameBufferSize(hDevice, &frameSize);
	frameBuffer[0] = new char[frameSize];
    frameBuffer[1] = new char[frameSize];
	ASSERT_OK( TYEnqueueBuffer(hDevice, frameBuffer[0], frameSize) );
	ASSERT_OK( TYEnqueueBuffer(hDevice, frameBuffer[1], frameSize) );
	int loopcnt = len;
	while (loopcnt > 0)
	{
		ASSERT_OK(TYStartCapture(hDevice));
		TY_STATUS ty_status = TYFetchFrame(hDevice, &frame, 3000);
		if(ty_status != TY_STATUS_OK)
		{
			std::cout<<"Fetchframe can not work, CaptureOut test is FAIL"<<std::endl;
			ASSERT_OK(TYStopCapture(hDevice));
			return TY_FAULT_APPEARS;
		}
		cv::Mat depth, irl, irr, color, pt3d;
		parseFrame(frame, &depth, &irl, &irr, &color, &pt3d);
		ty_status = TYEnqueueBuffer(hDevice, frame.userBuffer, frame.bufferSize);
		ASSERT_OK(TYStopCapture(hDevice));
		if (depth.empty())
		{
			std::cout << "[###FAIL###]: depth is empty" << std::endl;
			failcnt++;
		}
		if (irl.empty())
		{
			std::cout << "[###FAIL###]: irl is empty" << std::endl;
			failcnt++;
		}
		if (irr.empty())
		{
			std::cout << "[###FAIL###]: irr is empty" << std::endl;
			failcnt++;
		}
		if (componentIDs&TY_COMPONENT_RGB_CAM_LEFT)
		{
			if (color.empty())
			{
				std::cout << "[###FAIL###]: Color is empty" << std::endl;
				failcnt++;
			}
		}
		if (pt3d.empty())
		{
			std::cout << "[###FAIL###]: pt3d is empty" << std::endl;
			failcnt++;
		}
		loopcnt--;
	}
	if (failcnt > 0)
	{
		std::cout << "[###FAIL###]: Device capture output test is FAIL" << std::endl;
		return TY_WRONG_VALUE;
	}
	else
	{
		std::cout << "[***PASS***]: Device capture output test is SUCCESS" << std::endl;
		return TY_SUCCESS_TEST;
	}
}

int TYLongTimeDeviceNumTest(int num)
{
	TY_STATUS ty_status;
	ty_status = TYDeinitLib();
	ASSERT_OK(TYInitLib());
	for (unsigned int i = 0; i < len; i++)
	{
		int n = 0;
		ty_status = TYGetDeviceNumber(&n);
		if (ty_status != TY_STATUS_OK)
		{
			std::cout << "[###FAIL###]: Get device number failed" << std::endl;
			return TY_FAULT_APPEARS;
		}
		if (n != num)
		{
			std::cout << "[###FAIL###]: The device's number is wrong, find " << n << ", actually is " << num << std::endl;
			return TY_WRONG_VALUE;
		}
	}
	std::cout << "[***PASS***]: Device num test is SUCCESS" << std::endl;
	return TY_SUCCESS_TEST;
}

int TYExposureTimeTest(TY_DEV_HANDLE hDevice,TY_COMPONENT_ID componentID)
{
	std::string  devicename;
	switch (componentID)
	{
	case TY_COMPONENT_DEPTH_CAM:
		devicename = "TY_COMPONENT_DEPTH_CAM";
		break;
	case TY_COMPONENT_IR_CAM_LEFT:
		devicename = "TY_COMPONENT_IR_CAM_LEFT";
		break;
	case TY_COMPONENT_IR_CAM_RIGHT:
		devicename = "TY_COMPONENT_IR_CAM_RIGHT";
		break;
	case TY_COMPONENT_RGB_CAM_LEFT:
		devicename = "TY_COMPONENT_RGB_CAM_LEFT";
		break;
	default:
		break;
	}
	if (componentID == TY_COMPONENT_RGB_CAM_LEFT)
	{
		int32_t componentIDs;
		ASSERT_OK(TYGetComponentIDs(hDevice, &componentIDs));
		if (!(componentIDs&TY_COMPONENT_RGB_CAM_LEFT))
		{
			std::cout << "[!!!WARNING!!!]: No TY_COMPONENT_RGB_CAM_LEFT Component " << std::endl;
			return TY_SUCCESS_TEST;
		}
	}
	int32_t _val;
	TY_STATUS ty_status;
	TY_FEATURE_INFO m_info;
	m_info.isValid = false;
	m_info.accessMode = 0;
	ASSERT_OK(TYGetFeatureInfo(hDevice, componentID, TY_INT_EXPOSURE_TIME, &m_info));
	if (!m_info.isValid)
	{
		std::cout << "[###FAIL###]: Device:"<< devicename <<" The feature of [TY_INT_EXPOSURE_TIME] is invalid" << std::endl;
		return TY_FEATURE_INVALID;
	}
	if(!(m_info.accessMode & TY_ACCESS_READABLE))
	{
		std::cout << "[###FAIL###]: Device:"<< devicename <<" The feature of [TY_INT_EXPOSURE_TIME] is unreadable" << std::endl;
		return TY_NOT_READABLE;
	}
	ty_status = TYGetInt(hDevice, componentID, TY_INT_EXPOSURE_TIME, &_val);
	if(ty_status != TY_STATUS_OK)
	{
		std::cout << "[###FAIL###]: Device:"<< devicename <<" The feature of [TY_INT_EXPOSURE_TIME] Get failed" << std::endl;
		return TY_FAULT_APPEARS;
	}
	if(_val<10 || _val > 60)
	{
		std::cout << "[###FAIL###]: Device:"<< devicename <<" The feature of [TY_INT_EXPOSURE_TIME] value is wrong" << std::endl;
		return TY_WRONG_VALUE;
	}
	std::cout << "[***PASS***]: Device:"<< devicename <<" The feature of [TY_INT_EXPOSURE_TIME] value is SUCCESS" << std::endl;
	return TY_SUCCESS_TEST;
}

int TYSingleSensorFrameRateTest(TY_DEV_HANDLE hDevice, TY_COMPONENT_ID componentID, int stdFrameRate)
{

	std::string  devicename;
	switch (componentID)
	{
	case TY_COMPONENT_DEPTH_CAM:
		devicename = "TY_COMPONENT_DEPTH_CAM";
		break;
	case TY_COMPONENT_IR_CAM_LEFT:
		devicename = "TY_COMPONENT_IR_CAM_LEFT";
		break;
	case TY_COMPONENT_IR_CAM_RIGHT:
		devicename = "TY_COMPONENT_IR_CAM_RIGHT";
		break;
	case TY_COMPONENT_RGB_CAM_LEFT:
		devicename = "TY_COMPONENT_RGB_CAM_LEFT";
		break;
	default:
		break;
	}
	bool captfalg = false;
	TY_FEATURE_INFO m_info;
	m_info.isValid = false;
	m_info.accessMode = 0;
	TY_FRAME_DATA frame;
	int32_t componentIDs;
	int start, end, framerate;
	ASSERT_OK(TYGetComponentIDs(hDevice, &componentIDs));
	ASSERT_OK(TYIsCapturing(hDevice, &captfalg));
	if (captfalg)
	{
		ASSERT_OK(TYStopCapture(hDevice));
	}
	for (int32_t j = 0; j < sizeof(PercipioDeviceComponent) / sizeof(int32_t); j++)
	{
		if (componentIDs&PercipioDeviceComponent[j])
		{
			if (PercipioDeviceComponent[j] == componentID)
			{
				ASSERT_OK(TYEnableComponents(hDevice, PercipioDeviceComponent[j]));
			}
			else
			{
				ASSERT_OK(TYDisableComponents(hDevice, PercipioDeviceComponent[j]));
			}
		}
	}
	ASSERT_OK(TYStartCapture(hDevice));
	int loopcnt = 100;
	start = clock();
	while (loopcnt > 0)
	{
		TY_STATUS ty_status = TYFetchFrame(hDevice, &frame, 5000);
		if (ty_status != TY_STATUS_OK)
		{
			std::cout << "[###FAIL###]: Device:" << devicename << " Can not Fetch Frame " << std::endl;
			return TY_FAULT_APPEARS;
		}
		cv::Mat depth, irl, irr, color, pt3d;
		parseFrame(frame, &depth, &irl, &irr, &color, &pt3d);
		ty_status = TYEnqueueBuffer(hDevice, frame.userBuffer, frame.bufferSize);
		loopcnt--;
	}
	end = clock();
	ASSERT_OK(TYStopCapture(hDevice));
	framerate = 100*1000 / (end - start);
	if (framerate >= stdFrameRate)
	{
		std::cout << "[***PASS***]: Device:" << devicename << " The Frame rate test is SUCCESS, FrameRate: "<< framerate<< std::endl;
		return TY_SUCCESS_TEST;
	}
	else
	{
		std::cout << "[###FAIL###]: Device:" << devicename << " The Frame rate test is FAIL, FrameRate: " << framerate << std::endl;
		return TY_WRONG_VALUE;
	}		
}