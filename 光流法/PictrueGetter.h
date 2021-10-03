#pragma once
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
class PictureGetter
{
public:
	virtual PictureGetter& operator >> (cv::Mat image[2]) = 0;
};