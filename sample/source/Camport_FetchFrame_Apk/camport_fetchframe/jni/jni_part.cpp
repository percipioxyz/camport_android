#include <jni.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <vector>

#define _CRT_SECURE_NO_WARNINGS

#ifdef _WIN32
#include <time.h>
#else
#include <sys/time.h>
#endif

#include "percipio_camport.h"
#include <opencv2/opencv.hpp>
#include <stdlib.h>
#include "common.hpp"


#define LOG_TAG "percipio_jni"
#include <android/log.h>

struct CallbackData {
    int             index;
    TY_DEV_HANDLE   hDevice;
    DepthRender*    render;
};

static int n;
static char buffer[1024*1024];
static TY_DEV_HANDLE hDevice;
static DepthRender render;
static CallbackData cb_data;
static char* frameBuffer[2];
static cv::Mat *t, *t_color;
static bool hasColor = false;

static int fps_counter = 0;
static clock_t fps_tm = 0;

int get_fps() {
  const int kMaxCounter = 20;
  struct timeval start;
  fps_counter++;
  if (fps_counter < kMaxCounter) {
    return -1;
  }

  gettimeofday(&start, NULL);
  int elapse = start.tv_sec * 1000 + start.tv_usec / 1000 - fps_tm;
  int v = (int)(((float)fps_counter) / elapse * 1000);
  gettimeofday(&start, NULL);
  fps_tm = start.tv_sec * 1000 + start.tv_usec / 1000;

  fps_counter = 0;
  return v;
}

void frameHandler(TY_FRAME_DATA* frame, void* userdata, jlong mat, jlong mat_color)
{
    CallbackData* pData = (CallbackData*) userdata;
    LOGD("=== Get frame %d", ++pData->index);

    for( int i = 0; i < frame->validCount; i++ ){
        // get & show depth image
        if(frame->image[i].componentID == TY_COMPONENT_DEPTH_CAM){
        	//LOGD("222222, %d, %d", frame->image[i].height, frame->image[i].width);
            cv::Mat depth(frame->image[i].height, frame->image[i].width
                    , CV_16U, frame->image[i].buffer);
            cv::Mat colorDepth = pData->render->Compute(depth);
            t = (cv::Mat*)mat;
            //LOGD("rows: %d, cols: %d, size: %d", t->rows, t->cols, t->size);
            //LOGD("rows: %d, cols: %d, size: %d", colorDepth.rows, colorDepth.cols, colorDepth.size);
            //cv::imwrite("/data/test.jpg", colorDepth);
            colorDepth.copyTo(*t);
        }
        // get & show RGB
        if(frame->image[i].componentID == TY_COMPONENT_RGB_CAM){
            cv::Mat pColor;
            LOGD("format: %x", frame->image[i].pixelFormat);
            if (frame->image[i].pixelFormat == TY_PIXEL_FORMAT_JPEG){
            	LOGD("======== jpeg");
			} else if (frame->image[i].pixelFormat == TY_PIXEL_FORMAT_YVYU){
				LOGD("======== yvyu");
                cv::Mat yuv(frame->image[i].height, frame->image[i].width
                            , CV_8UC2, frame->image[i].buffer);
                cv::cvtColor(yuv, pColor, cv::COLOR_YUV2RGB_YVYU);
            } else if (frame->image[i].pixelFormat == TY_PIXEL_FORMAT_YUYV){
            	LOGD("======== yuyv");
                cv::Mat yuv(frame->image[i].height, frame->image[i].width
                            , CV_8UC2, frame->image[i].buffer);
                cv::cvtColor(yuv, pColor, cv::COLOR_YUV2RGB_YUYV);
            } else if(frame->image[i].pixelFormat == TY_PIXEL_FORMAT_RGB){
            	LOGD("======== rgb");
                cv::Mat rgb(frame->image[i].height, frame->image[i].width
                        , CV_8UC3, frame->image[i].buffer);
                rgb.copyTo(pColor);
            } else if(frame->image[i].pixelFormat == TY_PIXEL_FORMAT_MONO){
            	LOGD("======== mono");
                cv::Mat gray(frame->image[i].height, frame->image[i].width
                        , CV_8U, frame->image[i].buffer);
                cv::cvtColor(gray, pColor, cv::COLOR_GRAY2RGB);
            } else if(frame->image[i].pixelFormat == TY_PIXEL_FORMAT_BAYER8GB){
            	LOGD("======== bayergb");
                cv::Mat raw(frame->image[i].height, frame->image[i].width
                        , CV_8U, frame->image[i].buffer);
                cv::cvtColor(raw, pColor, cv::COLOR_BayerGB2RGB);
            }
            cv::imwrite("/data/test_color.jpg", pColor);
            t_color = (cv::Mat*)mat_color;
            //LOGD("rows: %d, cols: %d, size: %d", t_color->rows, t_color->cols, t_color->size);
            //LOGD("rows: %d, cols: %d, size: %d", pColor.rows, pColor.cols, pColor.size);
            pColor.copyTo(*t_color);
        }
    }
    LOGD("=== Callback: Re-enqueue buffer(%p, %d)", frame->userBuffer, frame->bufferSize);
    ASSERT_OK( TYEnqueueBuffer(pData->hDevice, frame->userBuffer, frame->bufferSize) );
}

int OpenDevice() {
    LOGD("=== Init lib");
	ASSERT_OK( TYInitLib() );
	TY_VERSION_INFO* pVer = (TY_VERSION_INFO*)buffer;
	ASSERT_OK(TYLibVersion(pVer));
	LOGD("     - lib version: %d.%d.%d", pVer->major, pVer->minor, pVer->patch);

	/*
	LOGD("=== Get device info");
	ASSERT_OK(TYGetDeviceNumber(&n));
	LOGD("     - device number %d", n);
    if(n == 0){
	    LOGD("=== No device got");
	    return -1;
	}
	*/

	n = 1;
	TY_DEVICE_BASE_INFO* pBaseInfo = (TY_DEVICE_BASE_INFO*)buffer;
	ASSERT_OK(TYGetDeviceList(pBaseInfo, 100, &n));

	LOGD("=== Open device 0");
	ASSERT_OK(TYOpenDevice(pBaseInfo[0].id, &hDevice));

	int32_t allComps;
	ASSERT_OK( TYGetComponentIDs(hDevice, &allComps) );
	if(allComps & TY_COMPONENT_RGB_CAM){
	    LOGD("=== Has RGB camera, open RGB cam");
	    ASSERT_OK( TYEnableComponents(hDevice, TY_COMPONENT_RGB_CAM) );
	    hasColor = true;
	}

	LOGD("=== Configure components, open depth cam");
	int32_t componentIDs = TY_COMPONENT_DEPTH_CAM | TY_COMPONENT_IR_CAM_LEFT | TY_COMPONENT_IR_CAM_RIGHT;
	//int32_t componentIDs = TY_COMPONENT_DEPTH_CAM;
	ASSERT_OK( TYEnableComponents(hDevice, componentIDs) );

	LOGD("=== Configure feature, set resolution to 640x480.");
	LOGD("Note: DM460 resolution feature is in component TY_COMPONENT_DEVICE,");
	LOGD("      other device may lays in some other components.");
	int err = TYSetEnum(hDevice, TY_COMPONENT_DEPTH_CAM, TY_ENUM_IMAGE_MODE, TY_IMAGE_MODE_640x480);
	ASSERT(err == TY_STATUS_OK || err == TY_STATUS_NOT_PERMITTED);

	if (hasColor) {
		err = TYSetEnum(hDevice, TY_COMPONENT_RGB_CAM, TY_ENUM_IMAGE_MODE, TY_IMAGE_MODE_640x480);
		ASSERT(err == TY_STATUS_OK || err == TY_STATUS_NOT_PERMITTED);
	}

	LOGD("=== Prepare image buffer");
	int32_t frameSize;
	ASSERT_OK( TYGetFrameBufferSize(hDevice, &frameSize) );
	LOGD("     - Get size of framebuffer, %d", frameSize);
	ASSERT( frameSize >= 640*480*2 );

	LOGD("     - Allocate & enqueue buffers");

	frameBuffer[0] = new char[frameSize];
	frameBuffer[1] = new char[frameSize];
	LOGD("     - Enqueue buffer (%p, %d)", frameBuffer[0], frameSize);
	ASSERT_OK( TYEnqueueBuffer(hDevice, frameBuffer[0], frameSize) );
	LOGD("     - Enqueue buffer (%p, %d)", frameBuffer[1], frameSize);
	ASSERT_OK( TYEnqueueBuffer(hDevice, frameBuffer[1], frameSize) );

	LOGD("=== Register callback");
	LOGD("Note: Callback may block internal data receiving,");
	LOGD("      so that user should not do long time work in callback.");
	LOGD("      To avoid copying data, we pop the framebuffer from buffer queue and");
	LOGD("      give it back to user, user should call TYEnqueueBuffer to re-enqueue it.");

	cb_data.index = 0;
	cb_data.hDevice = hDevice;
	cb_data.render = &render;

	LOGD("=== Disable trigger mode");
	ASSERT_OK( TYSetBool(hDevice, TY_COMPONENT_DEVICE, TY_BOOL_TRIGGER_MODE, false) );

	LOGD("=== Start capture");
	ASSERT_OK( TYStartCapture(hDevice) );

	LOGD("open device successful");
	return 0;
}

int CloseDevice() {
    ASSERT_OK( TYStopCapture(hDevice) );
    ASSERT_OK( TYCloseDevice(hDevice) );
    ASSERT_OK( TYDeinitLib() );
    // MSLEEP(10); // sleep to ensure buffer is not used any more
    delete frameBuffer[0];
    delete frameBuffer[1];
	return 0;
}

extern "C" {
JNIEXPORT void JNICALL Java_com_percipio_sample_CamportActivity_OpenDevice(JNIEnv* env, jobject thiz);
JNIEXPORT void JNICALL Java_com_percipio_sample_CamportActivity_CloseDevice(JNIEnv* env, jobject thiz);
JNIEXPORT void JNICALL Java_com_percipio_sample_CamportActivity_FetchData(JNIEnv* env, jobject thiz,jlong matAddr, jlong matAddr_color);
}

JNIEXPORT void JNICALL Java_com_percipio_sample_CamportActivity_OpenDevice(JNIEnv* env, jobject thiz)
{
    OpenDevice();
}

JNIEXPORT void JNICALL Java_com_percipio_sample_CamportActivity_CloseDevice(JNIEnv* env, jobject thiz)
{
    CloseDevice();
}

JNIEXPORT void JNICALL Java_com_percipio_sample_CamportActivity_FetchData(JNIEnv* env, jobject thiz, jlong matAddr, jlong matAddr_color)
{
	LOGD("Fetch Data");
    LOGD("=== While loop to fetch frame");

    TY_FRAME_DATA frame;

    /*
    struct timeval xTime;
    int xRet = gettimeofday(&xTime, NULL);
    long long xFactor = 1;
    long long now = (long long)(( xFactor * xTime.tv_sec * 1000) + (xTime.tv_usec / 1000));
    LOGD("sec_d = %d, sec_ld = %ld, sec_lld = %lld\n", xTime.tv_sec, xTime.tv_sec, xTime.tv_sec);
    LOGD("usec_d = %d, usec_ld = %ld, usec_lld = %lld\n", xTime.tv_usec, xTime.tv_usec, xTime.tv_usec);
    LOGD("now_d = %d, now_ld = %ld, now_ld = %lld\n", now, now, now);
    */

    int err = TYFetchFrame(hDevice, &frame, -1);

    /*
    LOGD("-----------------");
    xRet = gettimeofday(&xTime, NULL);
    xFactor = 1;
    now = (long long)(( xFactor * xTime.tv_sec * 1000) + (xTime.tv_usec / 1000));
    LOGD("sec_d = %d, sec_ld = %ld, sec_lld = %lld\n", xTime.tv_sec, xTime.tv_sec, xTime.tv_sec);
    LOGD("usec_d = %d, usec_ld = %ld, usec_lld = %lld\n", xTime.tv_usec, xTime.tv_usec, xTime.tv_usec);
    LOGD("now_d = %d, now_ld = %ld, now_ld = %lld\n", now, now, now);
    */
    LOGD("err = %d", err);
    if( err != TY_STATUS_OK ){
        //LOGD("... Drop one frame");
    }

    frameHandler(&frame, &cb_data, matAddr, matAddr_color);
}


