
//#include "OpticalFlowInterface.h"

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <fstream>
#include <thread>

#include "CameraPictureGetter.h"
#include "VideoPictureGetter.h"



void makeMask(cv::Mat picture);
void on_mouse(int event, int x, int y, int flags, void* ustc);
int opticalFlow(Mat* frame_arr, bool if_show_dyna, bool if_save_warning, bool use_magnitude, int sensitivity, int degree1, int degree2);

Mat maskImage;
vector<vector<Point>> vctvctPoint; // 保存n个多边形的点
Point ptStart(-1, -1); // 初始化起点
Point cur_pt(-1, -1); // 初始化临时节点
vector<Point> vctPoint; // 路径点集合

using namespace cv;
using namespace std;


int main() {
	CameraPictureGetter c;
	PictureGetter& pg = c;
	Mat frame_arr[2];
	pg >> frame_arr;
	while (frame_arr[0].empty()) {
		Sleep(3000);
		pg >> frame_arr;
	}
		/* 设置ROI */
	makeMask(frame_arr[0]);
	while (true) {
		pg >> frame_arr;
		opticalFlow(frame_arr, true, true, true, 80, 0, 180);
	}
		
}

int opticalFlow(Mat* frame_arr, bool if_show_dyna, bool if_save_warning, bool use_magnitude, int sensitivity, int degree1, int degree2) //是否显示动检结果
{
	static cv::Point history_center;
	static bool history[5] = { false };
	static time_t last_warning;

	{
		int error_level = 0;
		int move_count = 0;
		int i_sum = 0, j_sum = 0;
		Mat frame_gray[2];
		Mat frame_masked_arr[2];
		
		if (frame_arr[1].empty()) return -1;

		// 对图像进行掩膜操作
		frame_arr[0].copyTo(frame_masked_arr[0], maskImage);
		frame_arr[1].copyTo(frame_masked_arr[1], maskImage);
		// 变为灰度图像
		cvtColor(frame_masked_arr[0], frame_gray[0], COLOR_BGR2GRAY);
		cvtColor(frame_masked_arr[1], frame_gray[1], COLOR_BGR2GRAY);
		Mat flow(frame_gray[0].size(), CV_32FC2);
		calcOpticalFlowFarneback(frame_gray[0], frame_gray[1], flow, 0.5, 3, 15, 3, 5, 1.2, 0);
		// 光流结果可视化
		Mat flow_parts[2];
		split(flow, flow_parts); // 将X值和Y值分离成数组
		Mat magnitude, angle, magn_norm;
		cartToPolar(flow_parts[0], flow_parts[1], magnitude, angle, true);
		normalize(magnitude, magn_norm, 0.0f, 1.0f, NORM_MINMAX);
		angle *= ((1.f / 360.f) * (180.f / 255.f));
		//build hsv image
		Mat _hsv[3], hsv, hsv8, bgr;
		_hsv[0] = angle;
		_hsv[1] = Mat::ones(angle.size(), CV_32F);
		_hsv[2] = magn_norm;

		//threshold(_hsv[0], direction_mask, 90.f / 255.f, 1, THRESH_BINARY_INV); // 凡大于180度的都是0
		//direction_mask.convertTo(direction_mask, CV_8UC1, 255);
		//threshold(_hsv[2], magn_mask, 0.9, 1, THRESH_BINARY);
		//magn_mask.convertTo(magn_mask, CV_8UC1, 255);
		merge(_hsv, 3, hsv);
		hsv.convertTo(hsv8, CV_8U, 255.0);
		cvtColor(hsv8, bgr, COLOR_HSV2BGR);
		//imshow("current", next);
		//bitwise_and(direction_mask, magn_mask, final_mask);
		//frame2.setTo(Scalar(0,0,255), final_mask);
		history[0] = false;
		Mat res_img = frame_masked_arr[1].clone();
		/* 对于图片上的每个像素点 */
		for (int i = 0; i < res_img.rows; i++) {
			for (int j = 0; j < res_img.cols; j++) {
				bool temp = false;
				if (use_magnitude) {
					temp = magnitude.at<float>(i, j) > (float)sensitivity / 50.f;
				}
				else {
					temp = magn_norm.at<float>(i, j) > (float)sensitivity / 100.f;
				}
				if (angle.at<float>(i, j) < (float)degree2 / 255.f && angle.at<float>(i, j) > (float)degree1 / 255.f && temp)
				{
					res_img.at<Vec3b>(i, j)[0] = bgr.at<Vec3b>(i, j)[0];
					res_img.at<Vec3b>(i, j)[1] = bgr.at<Vec3b>(i, j)[1];
					res_img.at<Vec3b>(i, j)[2] = bgr.at<Vec3b>(i, j)[2];
					history[0] = true;
					++move_count;
					i_sum += i;
					j_sum += j;
				}
			}
		}


		int count = history[0] ? 5 : 0;
		for (int i = 4; i > 0; --i)
		{
			history[i] = history[i - 1];
			if (history[i] == true) ++count;
		}
		if (count > 7)
		{
			putText(res_img, "!", Point(0, 25), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 3);
			time_t t;
			time(&t);//获取Unix时间戳。
			if (t - last_warning > 10) { // 10s内只报警一次
				error_level = 1;
				time(&last_warning);
			}
		}
		else if (count > 5)
		{
			// 有运动
			//putText(res_img, "?", Point(0, 25), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 165, 255), 3);
		}


		if (move_count > 0)
		{
			cv::Point point(j_sum / move_count, i_sum / move_count);
			if (abs(point.x - history_center.x) + abs(point.y - history_center.y) < 300)
			{
				cv::circle(res_img, point, 5, cv::Scalar(0, 0, 255), 2);
			}
			history_center = point;
		}
		if (if_save_warning && error_level > 0) //保存报警信息
		{
			ofstream outfile("../报警日志.txt", ios::app);
			struct tm* lt = new tm();
			localtime_s(lt, &last_warning);//转为时间结构。
			char tempStr[100];
			sprintf_s(tempStr, "%d-%d-%d %d:%d:%d\n", lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);//输出结果    
			outfile << tempStr;

			sprintf_s(tempStr, "../log picture/%d %d %d %d %d %d(1).jpg", lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);//输出结果    
			cv::imwrite(tempStr, frame_arr[0]);
			sprintf_s(tempStr, "../log picture/%d %d %d %d %d %d(2).jpg", lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);//输出结果    
			cv::imwrite(tempStr, frame_arr[1]);
			sprintf_s(tempStr, "../log picture/%d %d %d %d %d %d(3).jpg", lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);//输出结果    
			cv::imwrite(tempStr, res_img);
		}
		if(if_show_dyna) {
			int nCols = 600;
			int nRows = frame_arr[1].rows * nCols / frame_arr[1].cols;
			Mat show_res = Mat(nRows, nCols, frame_arr[1].type());
			resize(res_img, show_res, show_res.size(), 0, 0, INTER_LINEAR);
			imshow("识别结果", show_res);
		}
		return error_level;
	}
}


void makeMask(cv::Mat picture)
{
	maskImage = Mat();
	vctvctPoint.clear(); //没有释放空间，以后愿意改再改吧
	vctPoint.clear();  //没有释放空间，以后愿意改再改吧
	cv::namedWindow("设置选定区域");
	cv::setMouseCallback("设置选定区域", on_mouse, &picture); // 调用回调函数    
	cv::imshow("设置选定区域", picture);
	cv::waitKey(0);
	cv::setMouseCallback("设置选定区域", NULL, 0); // 解除回调函数
	if (cv::getWindowProperty("设置选定区域", WND_PROP_VISIBLE) == 1)
		cv::destroyWindow("设置选定区域");
	if (cv::getWindowProperty("ROI", WND_PROP_VISIBLE) == 1)
		cv::destroyWindow("ROI");
}

void on_mouse(int event, int x, int y, int flags, void* ustc) // event鼠标事件代号，x,y鼠标坐标，flags拖拽和键盘操作的代号    
{
	Mat& pic = *((Mat*)ustc);
	//maskImage = Mat::zeros(pic.size(), pic.type());
	if (event == EVENT_LBUTTONDOWN)
	{
		ptStart = Point(x, y);
		vctPoint.push_back(ptStart);
		cv::circle(pic, ptStart, 1, cv::Scalar(255, 0, 255), CV_FILLED, CV_AA, 0);
		cv::imshow("设置选定区域", pic);
		//cv::putText(tmp, temp, ptStart, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0, 0), 1, 8);
	}
	else if (event == EVENT_MOUSEMOVE && (flags & EVENT_FLAG_LBUTTON))
	{
		cur_pt = Point(x, y);
		cv::line(pic, vctPoint.back(), cur_pt, cv::Scalar(0, 255, 0, 0), 1, 8, 0);
		cv::circle(pic, cur_pt, 1, cv::Scalar(255, 0, 255), CV_FILLED, CV_AA, 0);
		cv::imshow("设置选定区域", pic);
		vctPoint.push_back(cur_pt);
		//cv::putText(tmp, temp, cur_pt, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0, 0));
	}
	else if (event == EVENT_LBUTTONUP)
	{
		cur_pt = Point(x, y);
		cv::line(pic, ptStart, cur_pt, cv::Scalar(0, 255, 0, 0), 1, 8, 0);
		cv::circle(pic, cur_pt, 1, cv::Scalar(255, 0, 255), CV_FILLED, CV_AA, 0);
		cv::imshow("设置选定区域", pic);
		vctPoint.push_back(cur_pt);
		vctvctPoint.push_back(vctPoint);
		vctPoint.clear();
		maskImage = Mat::zeros(pic.size(), pic.type());
		//把点构成任意多边形进行填充
		for (int i = 0; i < vctvctPoint.size(); ++i)
		{
			const Point* ppt[1] = { &vctvctPoint[i][0] };//取数组的首地址
			int len = vctvctPoint[i].size();
			int npt[] = { len };
			//cv::polylines(frame1, ppt, npt, 1, 1, cv::Scalar(0, 0, 0, 0), 1, 8, 0);
			cv::fillPoly(maskImage, ppt, npt, 1, cv::Scalar(255, 255, 255, 255));
		}

		Mat dst;
		pic.copyTo(dst, maskImage);
		cv::imshow("ROI", dst);
	}
}
