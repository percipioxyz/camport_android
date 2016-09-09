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
#include "depth_render.h"

#define LOG_TAG "percipio_jni"
#include <android/log.h>
#include <string.h>
#define LOGD(fmt, args...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##args)


static cv::Mat left, right, depth, point_cloud;
static DepthRender render;
static int fps_counter = 0;
static clock_t fps_tm = 0;

cv::Mat *t;
percipio::DepthCameraDevice port(percipio::MODEL_DPB04GN);


int get_fps();
void CopyBuffer(percipio::ImageBuffer *pbuf, cv::Mat &img);



void frame_arrived_callback(void *user_data) {
  // call port.FramePackageGet to update internal buffer
  // call port.FrameGet to get frame data here
  // To avoid performance problem ,time consuming task in callback function is not recommended.
}

void process_frames(percipio::DepthCameraDevice &port, jlong mat) {
  LOGD("jni process_frames");
  percipio::ImageBuffer pimage;
  int ret = port.FrameGet(percipio::CAMDATA_LEFT, &pimage);
  if (percipio::CAMSTATUS_SUCCESS == ret) {
    CopyBuffer(&pimage, left);
  }
  ret = port.FrameGet(percipio::CAMDATA_RIGHT, &pimage);
  if (percipio::CAMSTATUS_SUCCESS == ret) {
    CopyBuffer(&pimage, right);
  }
  ret = port.FrameGet(percipio::CAMDATA_DEPTH, &pimage);
  if (percipio::CAMSTATUS_SUCCESS == ret) {
	CopyBuffer(&pimage, depth);
    int fps = get_fps();

    t = (cv::Mat*)mat;
    render.Compute(depth, *t);

    if (fps > 0) {
      unsigned short v = depth.ptr<unsigned short>(depth.rows / 2)[depth.cols / 2];
      printf("fps:%d distance: %d\n", (int)fps, v);
    }
  }

  ret = port.FrameGet(percipio::CAMDATA_POINT3D, &pimage);
  if (percipio::CAMSTATUS_SUCCESS == ret) {
    CopyBuffer(&pimage, point_cloud);
  }
}

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

void CopyBuffer(percipio::ImageBuffer *pbuf, cv::Mat &img) {
  switch (pbuf->type) {
  case percipio::ImageBuffer::PIX_8C1:
    img.create(pbuf->height, pbuf->width, CV_8UC1);
    break;
  case percipio::ImageBuffer::PIX_16C1:
    img.create(pbuf->height, pbuf->width, CV_16UC1);
    break;
  case percipio::ImageBuffer::PIX_8C3:
    img.create(pbuf->height, pbuf->width, CV_8UC3);
    break;
  case percipio::ImageBuffer::PIX_32FC3:
    img.create(pbuf->height, pbuf->width, CV_32FC3);
    break;
  default:
    img.release();

    return;
  }
   memcpy(img.data, pbuf->data, pbuf->width * pbuf->height * pbuf->get_pixel_size());
}

int OpenDevice() {
	render.range_mode = DepthRender::COLOR_RANGE_DYNAMIC;
	render.color_type = DepthRender::COLORTYPE_BLUERED;
	render.invalid_label = 0;
	render.Init();
	percipio::SetLogLevel(percipio::LOG_LEVEL_INFO);
	port.SetCallbackUserData(NULL);
	port.SetFrameReadyCallback(frame_arrived_callback);


	int ret = port.OpenDevice();
	LOGD("InitDevice, ret : %d", ret);
	if (percipio::CAMSTATUS_SUCCESS != ret) {
		printf("open device failed\n");
		return -1;
	}

	int reti = port.SetProperty_Int(percipio::PROP_WORKMODE, percipio::WORKMODE_DEPTH);
	if (reti < 0) {
		printf("set mode failed,error code:%d\n", reti);
		return -1;
	}
	LOGD("open device successful");
	return 0;
}

int CloseDevice() {
    port.CloseDevice();
    left.release();
    right.release();
    depth.release();
    point_cloud.release();
    render.Uninit();
	return 0;
}

extern "C" {
JNIEXPORT void JNICALL Java_org_opencv_samples_tutorial2_Tutorial2Activity_OpenDevice(JNIEnv* env, jobject thiz);
JNIEXPORT void JNICALL Java_org_opencv_samples_tutorial2_Tutorial2Activity_CloseDevice(JNIEnv* env, jobject thiz);
JNIEXPORT void JNICALL Java_org_opencv_samples_tutorial2_Tutorial2Activity_FetchData(JNIEnv* env, jobject thiz,jlong matAddr);
}

JNIEXPORT void JNICALL Java_org_opencv_samples_tutorial2_Tutorial2Activity_OpenDevice(JNIEnv* env, jobject thiz)
{
    OpenDevice();
}

JNIEXPORT void JNICALL Java_org_opencv_samples_tutorial2_Tutorial2Activity_CloseDevice(JNIEnv* env, jobject thiz)
{
    CloseDevice();
}

JNIEXPORT void JNICALL Java_org_opencv_samples_tutorial2_Tutorial2Activity_FetchData(JNIEnv* env, jobject thiz, jlong matAddr)
{
	LOGD("Fetch Data");
    fps_tm = clock();
    fps_counter = 0;
    if (port.FramePackageGet() == percipio::CAMSTATUS_SUCCESS) {
           process_frames(port, matAddr);
    }
}


