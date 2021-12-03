#include "CameraController.h"

bool static has_sdk_been_initialized;
bool CameraController::initializeSDK(fDisConnect disconnectCallback, LDWORD user)
{
	if (has_sdk_been_initialized)
	{
		return false;
	}
    bool is_netSKD_inited = CLIENT_Init(disconnectCallback, (LDWORD)user);
	has_sdk_been_initialized = is_netSKD_inited;
    return is_netSKD_inited;
}

cv::Mat CameraController::fileToMat(char buffer[], unsigned int len)
{
	cv::_InputArray pic_arr(buffer, len);
	cv::Mat src_mat = cv::imdecode(pic_arr, cv::IMREAD_COLOR);
	return src_mat;
}

bool CameraController::hasSDKInitialized()
{
	return has_sdk_been_initialized;
}

LLONG CameraController::connect(char device_ip[32], char username[64], char password[64], DWORD& error_no, WORD port)
{
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
			error_no = CLIENT_GetLastError();
		}
		else
		{
			error_no = 0;
		}
		// �û����ε�¼�豸����Ҫ��ʼ��һЩ���ݲ�������ʵ��ҵ���ܣ������¼��ȴ�һС��ʱ�䣬����ȴ�ʱ�����豸���졣
		Sleep(1000);
		return login_handle;
	}
}

Mat CameraController::getPicture(unsigned int quality, unsigned int channel)
{
	if (login_handle == 0) return Mat();

	NET_IN_SNAP_PIC_TO_FILE_PARAM stuInParam = { sizeof(stuInParam) };
	NET_OUT_SNAP_PIC_TO_FILE_PARAM stuOutParam = { sizeof(stuOutParam) };
	SNAP_PARAMS stuSnapParams = { 0 };
	stuSnapParams.Channel = channel; // �Ե�һ��ͨ��Ϊ��
	stuSnapParams.Quality = quality; // ��ͼ����

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
	cv::Mat picture = fileToMat(pBuffer, nBufferLen);
	delete[] pBuffer;
	return picture;
}

CameraController::~CameraController()
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
	if (has_sdk_been_initialized)
	{
		CLIENT_Cleanup();
		has_sdk_been_initialized = false;
	}
}
