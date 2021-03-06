#pragma once

#include <dhnetsdk.h>
#include <avglobal.h>
#include <dhconfigsdk.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <thread>
#include <queue>
#include <windows.h>
#include "PictrueGetter.h"
#include "CameraController.h"

using namespace std;
using namespace cv;

const unsigned char MAX_CHCAHE_LEN = 2;

class CameraPictureGetter: public PictureGetter
{
public:
	// 将图片存入队列
	void chachePicture();
	~CameraPictureGetter();
	bool CameraPictureGetterInit(char device_ip[32], char username[64], char password[64]);
	PictureGetter& operator >> (cv::Mat image[2]);
	bool stop_thread = false;
	bool netSKD_inited();
private:
	CameraController controller;
	// SDK初始化是否成功
	bool is_netSKD_inited = false;
	WORD port = 37777; // tcp 连接端口，需与期望登录设备页面 tcp 端口配置一致
	const int interval = 1000 / 25; // 截图操作的间隔时长（毫秒）
	queue<cv::Mat*> picture_cache;
	cv::Mat pic_arr[2];
	thread th;
	// 缓存中有多少内容
	HANDLE chache_size_semaphore = NULL;
	// 有多少剩余空间
	HANDLE chache_space_semaphore = NULL;
};
