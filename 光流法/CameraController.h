#pragma once

#include <dhnetsdk.h>
#include <avglobal.h>
#include <dhconfigsdk.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

using namespace cv;

class CameraController
{
private:
	
	LLONG login_handle = 0;
	cv::Mat fileToMat(char buffer[], unsigned int len);
public:

	bool static hasSDKInitialized();
	bool static initializeSDK(fDisConnect disconnectCallback, LDWORD user = (LDWORD)nullptr);
	LLONG connect(char device_ip[32], char username[64], char password[64], DWORD& error_no, WORD port = 37777);
	Mat getPicture(unsigned int quality = 6, unsigned int channel = 0);
	~CameraController();
};