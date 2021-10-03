#include "VideoPictureGetter.h"


PictureGetter& VideoPictrueGetter::operator>>(cv::Mat image[2])
{
	if (this->image[0].empty() && this->image[1].empty())
	{
		capture >> image[0];
		capture >> image[1];
	}
	else
	{
		image[0] = image[1];
		capture >> image[1];
	}
	return *this;
}

VideoPictrueGetter::VideoPictrueGetter(string video_path)
{
	capture = cv::VideoCapture(video_path);
}