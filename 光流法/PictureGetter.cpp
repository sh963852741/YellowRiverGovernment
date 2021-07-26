#include "PictureGetter.h"
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
			// ���ݴ����룬������ dhnetsdk.h ���ҵ���Ӧ�Ľ��ͣ��˴���ӡ���� 16 ���ƣ�ͷ�ļ�����ʮ���ƣ����е�ת����ע��
			// ���磺
			// #define NET_NOT_SUPPORTED_EC(23) // ��ǰ SDK δ֧�ָù��ܣ���Ӧ�Ĵ�����Ϊ 0x80000017, 23 ��Ӧ�� 16 ����Ϊ 0x17
			printf("CLIENT_LoginWithHighLevelSecurity %s[%d]Failed!Last Error[%x]\n", device_ip, port, CLIENT_GetLastError());
		}
		else
		{
			printf("CLIENT_LoginWithHighLevelSecurity %s[%d] Success\n", device_ip, port);
		}
		// �û����ε�¼�豸����Ҫ��ʼ��һЩ���ݲ�������ʵ��ҵ���ܣ������¼��ȴ�һС��ʱ�䣬����ȴ�ʱ�����豸���졣
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
		stuSnapParams.Channel = 0; // �Ե�һ��ͨ��Ϊ��
		stuSnapParams.Quality = 6; // ��ͼ����

		int nBufferLen = 2 * 1024 * 1024;
		char* pBuffer = new char[nBufferLen]; // ͼƬ����
		memset(pBuffer, 0, nBufferLen);
		stuOutParam.szPicBuf = pBuffer;
		stuOutParam.dwPicBufLen = nBufferLen;

		/* ��ͼ */
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
	/* ����ԭʼͼƬ�ļ�*/
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
	// �˳��豸
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
	// �����ʼ����Դ
	if (is_netSKD_inited)
	{
		CLIENT_Cleanup();
		is_netSKD_inited = false;
	}
}

PictureGetter::PictureGetter()
{
	chache_size_semaphore = CreateSemaphore(NULL          //�ź����İ�ȫ����
		, 0            //�����ź����ĳ�ʼ�������������㵽���ֵ֮���һ��ֵ
		, MAXINT            //�����ź�����������
		, NULL         //ָ���ź������������
	);
	chache_space_semaphore = CreateSemaphore(NULL, MAX_CHCAHE_LEN, MAXINT, NULL);
}

// ��ͼƬ���ļ���תΪC++��Mat
cv::Mat PictureGetter::fileToMat(char buffer[], unsigned int len)
{
	cv::_InputArray pic_arr(buffer, len);
	cv::Mat src_mat = cv::imdecode(pic_arr, cv::IMREAD_COLOR);
	return src_mat;
}
