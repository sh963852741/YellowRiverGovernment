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
	// ��ͼƬ�������
	void chachePicture();
	~CameraPictureGetter();
	CameraPictureGetter(CameraController controller);
	PictureGetter& operator >> (cv::Mat image[2]);
	bool stop_thread = false;
private:
	CameraController controller;
	// SDK��ʼ���Ƿ�ɹ�
	bool is_netSKD_inited = false;
	LLONG login_handle = 0L;
	char device_ip[32] = "169.254.210.127";
	WORD port = 37777; // tcp ���Ӷ˿ڣ�����������¼�豸ҳ�� tcp �˿�����һ��
	char username[64] = "admin"; // �û���
	char password[64] = "IO0n2a4G"; // ����
	const int interval = 1000 / 25; // ��ͼ�����ļ��ʱ�������룩
	queue<cv::Mat*> picture_cache;
	cv::Mat pic_arr[2];
	thread th;
	// �������ж�������
	HANDLE chache_size_semaphore = NULL;
	// �ж���ʣ��ռ�
	HANDLE chache_space_semaphore = NULL;
};
