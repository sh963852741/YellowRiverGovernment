#pragma once
#include "PictrueGetter.h"
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>
#include <opencv2/imgproc/imgproc_c.h>

using namespace std;
class VideoPictrueGetter : public PictureGetter
{
private:
	cv::VideoCapture capture;
	cv::Mat image[2];
public:
	PictureGetter& operator >> (cv::Mat image[2]);
	VideoPictrueGetter(std::string video_path);
};