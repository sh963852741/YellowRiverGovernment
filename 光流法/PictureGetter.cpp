#include "PictureGetter.h"
#include <stdio.h>
#include <iostream>
#include <string>

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

void PictureGetter::init()
{
	is_netSKD_inited = CLIENT_Init(DisConnectFunc, (LDWORD)this);
	if (!is_netSKD_inited)
	{
		printf("Initialize client SDK fail; \n");
		return;
	}
	else
	{
		printf("Initialize client SDK done; \n");
	}

	NET_IN_LOGIN_WITH_HIGHLEVEL_SECURITY stInparam;
	memset(&stInparam, 0, sizeof(stInparam));
	stInparam.dwSize = sizeof(stInparam);
	strncpy_s(stInparam.szIP, device_ip, sizeof(stInparam.szIP) - 1);
	strncpy_s(stInparam.szPassword, password, sizeof(stInparam.szPassword) - 1);
	strncpy_s(stInparam.szUserName, username, sizeof(stInparam.szUserName) - 1);
	stInparam.nPort = port;
	stInparam.emSpecCap = EM_LOGIN_SPEC_CAP_TCP;

	while (login_handle == 0)
	{
		NET_OUT_LOGIN_WITH_HIGHLEVEL_SECURITY stOutparam;
		login_handle = CLIENT_LoginWithHighLevelSecurity(&stInparam, &stOutparam);

		if (login_handle == 0)
		{
			// 根据错误码，可以在 dhnetsdk.h 中找到相应的解释，此处打印的是 16 进制，头文件中是十进制，其中的转换需注意
			// 例如：
			// #define NET_NOT_SUPPORTED_EC(23) // 当前 SDK 未支持该功能，对应的错误码为 0x80000017, 23 对应的 16 进制为 0x17
			printf("CLIENT_LoginWithHighLevelSecurity %s[%d]Failed!Last Error[%x]\n", device_ip, port, CLIENT_GetLastError());
		}
		else
		{
			printf("CLIENT_LoginWithHighLevelSecurity %s[%d] Success\n", device_ip, port);
		}
		// 用户初次登录设备，需要初始化一些数据才能正常实现业务功能，建议登录后等待一小段时间，具体等待时间因设备而异。
		Sleep(1000);
		printf("\n");
	}

	th = thread(std::mem_fn(&PictureGetter::chachePicture), this);
}


void PictureGetter::chachePicture()
{
	while (true)
	{
		WaitForSingleObject(chache_space_semaphore, INFINITE);
		NET_IN_SNAP_PIC_TO_FILE_PARAM stuInParam = { sizeof(stuInParam) };
		NET_OUT_SNAP_PIC_TO_FILE_PARAM stuOutParam = { sizeof(stuOutParam) };
		SNAP_PARAMS stuSnapParams = { 0 };
		stuSnapParams.Channel = 0; // 以第一个通道为例
		stuSnapParams.Quality = 6; // 截图质量

		int nBufferLen = 2 * 1024 * 1024;
		char* pBuffer = new char[nBufferLen]; // 图片缓存
		memset(pBuffer, 0, nBufferLen);
		stuOutParam.szPicBuf = pBuffer;
		stuOutParam.dwPicBufLen = nBufferLen;

		/* 截图 */
		if (!CLIENT_SnapPictureToFile(login_handle, &stuInParam, &stuOutParam, 5000))
		{
			printf("CLIENT_SnapPictureToFile Failed!Last Error[% x]\n", CLIENT_GetLastError());
		}
		cv::Mat picture1 = fileToMat(pBuffer, nBufferLen);
		Sleep(interval);
		if (!CLIENT_SnapPictureToFile(login_handle, &stuInParam, &stuOutParam, 5000))
		{
			printf("CLIENT_SnapPictureToFile Failed!Last Error[% x]\n", CLIENT_GetLastError());
		}
		cv::Mat picture2 = fileToMat(pBuffer, nBufferLen);

		pic_arr[0] = picture1;
		pic_arr[1] = picture2;
		picture_cache.push(pic_arr);
		ReleaseSemaphore(chache_size_semaphore, 1, NULL);
		delete[] pBuffer;
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

PictureGetter& PictureGetter::operator>>(cv::Mat image[2])
{
	WaitForSingleObject(chache_size_semaphore, INFINITE);
	image[0] = picture_cache.front()[0];
	image[1] = picture_cache.front()[1];
	picture_cache.pop();
	ReleaseSemaphore(chache_space_semaphore, 1, NULL);
	return *this;
}

PictureGetter::~PictureGetter()
{
	// 退出设备
	if (login_handle != 0)
	{
		if (!CLIENT_Logout(login_handle))
		{
			printf("CLIENT_Logout Failed!Last Error[%x]\n", CLIENT_GetLastError());
		}
		else
		{
			login_handle = 0;
		}
	}
	// 清理初始化资源
	if (is_netSKD_inited)
	{
		CLIENT_Cleanup();
		is_netSKD_inited = false;
	}
}

PictureGetter::PictureGetter()
{
	chache_size_semaphore = CreateSemaphore(NULL          //信号量的安全特性
		, 0            //设置信号量的初始计数。可设置零到最大值之间的一个值
		, MAXINT            //设置信号量的最大计数
		, NULL         //指定信号量对象的名称
	);
	chache_space_semaphore = CreateSemaphore(NULL, MAX_CHCAHE_LEN, MAXINT, NULL);
}

// 将图片的文件流转为C++的Mat
cv::Mat PictureGetter::fileToMat(char buffer[], unsigned int len)
{
	cv::_InputArray pic_arr(buffer, len);
	cv::Mat src_mat = cv::imdecode(pic_arr, cv::IMREAD_COLOR);
	return src_mat;
}
