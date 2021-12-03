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
			// 根据错误码，可以在 dhnetsdk.h 中找到相应的解释，此处打印的是 16 进制，头文件中是十进制，其中的转换需注意
			// 例如：
			// #define NET_NOT_SUPPORTED_EC(23) // 当前 SDK 未支持该功能，对应的错误码为 0x80000017, 23 对应的 16 进制为 0x17
			error_no = CLIENT_GetLastError();
		}
		else
		{
			error_no = 0;
		}
		// 用户初次登录设备，需要初始化一些数据才能正常实现业务功能，建议登录后等待一小段时间，具体等待时间因设备而异。
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
	stuSnapParams.Channel = channel; // 以第一个通道为例
	stuSnapParams.Quality = quality; // 截图质量

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
	cv::Mat picture = fileToMat(pBuffer, nBufferLen);
	delete[] pBuffer;
	return picture;
}

CameraController::~CameraController()
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
	if (has_sdk_been_initialized)
	{
		CLIENT_Cleanup();
		has_sdk_been_initialized = false;
	}
}
