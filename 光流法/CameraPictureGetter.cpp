#include "CameraPictureGetter.h"
#include <stdio.h>
#include <iostream>
#include <string>

// �豸���߻ص�����
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
	/* ����ԭʼͼƬ�ļ�*/
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

CameraPictureGetter::CameraPictureGetter(CameraController controller)
{
	this->controller = controller;
	controller.initializeSDK(DisConnectFunc);
	DWORD err;
	controller.connect(this->device_ip, this->username, this->password, err);
	chache_size_semaphore = CreateSemaphore(NULL          //�ź����İ�ȫ����
		, 0            //�����ź����ĳ�ʼ�������������㵽���ֵ֮���һ��ֵ
		, MAXINT            //�����ź�����������
		, NULL         //ָ���ź������������
	);
	chache_space_semaphore = CreateSemaphore(NULL, MAX_CHCAHE_LEN, MAXINT, NULL);

	th = thread(std::mem_fn(&CameraPictureGetter::chachePicture), this);
}

CameraPictureGetter::~CameraPictureGetter()
{
	stop_thread = true;
	th.join();
}
