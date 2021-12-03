#include "CameraPictureGetter.h"
#include <stdio.h>
#include <iostream>
#include <string>


bool CameraPictureGetter::netSKD_inited() {
	return is_netSKD_inited;
}

// 设备断线回调函数
void CALLBACK DisConnectFunc(LLONG lLoginID, char* pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	printf("Call DisConnectFunc\n");
	printf("lLoginID[0x%x]", lLoginID);
	if (NULL != pchDVRIP)
	{
		printf("pchDVRIP[%s]\n", pchDVRIP);
	}
	printf("nDVRPort[%d]\n", nDVRPort);
	printf("dwUser[%p]\n", dwUser);
	printf("\n");
}

void CameraPictureGetter::chachePicture()
{
	while (!stop_thread)
	{
		DWORD res = WaitForSingleObject(chache_space_semaphore, 5000);
		if (res == WAIT_TIMEOUT)continue;

		pic_arr[0] = controller.getPicture();
		Sleep(interval);
		pic_arr[1] = controller.getPicture();
		picture_cache.push(pic_arr);

		ReleaseSemaphore(chache_size_semaphore, 1, NULL);
	}
	return;
	/* 保存原始图片文件*/
	//const char* pFileName = "123.jpg";
	//FILE* stream;
	//if (fopen_s(&stream, pFileName, "wb") == 0)
	//{
	//	int numwritten = fwrite(pBuffer, sizeof(char), stuOutParam.dwPicBufRetLen, stream);
	//	fclose(stream);
	//}
}

PictureGetter& CameraPictureGetter::operator>>(cv::Mat image[2])
{
	WaitForSingleObject(chache_size_semaphore, INFINITE);
	image[0] = picture_cache.front()[0];
	image[1] = picture_cache.front()[1];
	picture_cache.pop();
	ReleaseSemaphore(chache_space_semaphore, 1, NULL);
	return *this;
}

bool CameraPictureGetter::CameraPictureGetterInit(char device_ip[32], char username[64], char password[64])
{
	controller.initializeSDK(DisConnectFunc);
	DWORD err;
	if (controller.connect(device_ip, username, password, err) == 0) {
		return false;
	}
	is_netSKD_inited = true;
	chache_size_semaphore = CreateSemaphore(NULL          //信号量的安全特性
		, 0            //设置信号量的初始计数。可设置零到最大值之间的一个值
		, MAXINT            //设置信号量的最大计数
		, NULL         //指定信号量对象的名称
	);
	chache_space_semaphore = CreateSemaphore(NULL, MAX_CHCAHE_LEN, MAXINT, NULL);
	th = thread(std::mem_fn(&CameraPictureGetter::chachePicture), this);
	return true;
}

CameraPictureGetter::~CameraPictureGetter()
{
	stop_thread = true;
	th.join();
}
