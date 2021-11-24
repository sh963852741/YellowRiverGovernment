#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>
#include <opencv2/imgproc/imgproc_c.h>

#include <thread>

#include "CameraPictureGetter.h"
#include "VideoPictureGetter.h"

using namespace cv;
using namespace std;
void makeMask(cv::Mat picture);
void on_mouse(int event, int x, int y, int flags, void* ustc);

Mat frame_arr[2];
Mat maskImage;
bool history[5] = { false };

int main()
{
	//Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
	//Ptr<BackgroundSubtractorKNN> fgbg = createBackgroundSubtractorKNN();

	VideoPictrueGetter c("0705cut.mp4");
	PictureGetter& pg = c;

	/* 初始化视频读取和写入 */
	//VideoCapture capture("大土坡.mp4");
	VideoWriter wrt("res.mp4", VideoWriter::fourcc('H', '2', '6', '4'), 25,// capture.get(cv::CAP_PROP_FPS),
		cv::Size(1280, 720));
	//cv::Size(capture.get(cv::CAP_PROP_FRAME_WIDTH), capture.get(cv::CAP_PROP_FRAME_HEIGHT)));
//if (!capture.isOpened()) {
//	//error in opening the video input
//	cerr << "Unable to open file!" << endl;
//	return 0;
//}

	Mat direction_mask, magn_mask, final_mask;
	pg >> frame_arr;
	//maskImage = Mat::ones(frame_arr[0].size(), frame_arr[0].type());
	/* 设置ROI */
	makeMask(frame_arr[0]);

	Mat frame_gray[2];
	Mat frame_masked_arr[2];
	// 一幅图像中运动的点的个数
	int move_count = 0;
	int i_sum = 0, j_sum = 0;
	cv::Point history_center;
	while (true) {

		pg >> frame_arr;
		if (frame_arr[1].empty()) break;

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
				if (angle.at<float>(i, j) < 90.f / 255.f && angle.at<float>(i, j) > 0 && magnitude.at<float>(i, j) > 0.5)
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
		i_sum = j_sum = move_count = 0;

		imshow("result", res_img);
		wrt << res_img;
		int keyboard = waitKey(30);
		if (keyboard == 'q' || keyboard == 27)
			break;
	}
	wrt.release();
}

void makeMask(cv::Mat picture)
{
	cv::namedWindow("图片");
	cv::setMouseCallback("图片", on_mouse, &picture); // 调用回调函数    
	cv::imshow("图片", picture);
	cv::waitKey(0);
	cv::setMouseCallback("图片", NULL, 0); // 解除回调函数 
}

void on_mouse(int event, int x, int y, int flags, void* ustc) // event鼠标事件代号，x,y鼠标坐标，flags拖拽和键盘操作的代号    
{
	Mat& pic = *((Mat*)ustc);
	//maskImage = Mat::zeros(pic.size(), pic.type());
	static vector<vector<Point>> vctvctPoint; // 保存n个多边形的点
	static Point ptStart(-1, -1); // 初始化起点
	static Point cur_pt(-1, -1); // 初始化临时节点
	static vector<Point> vctPoint; // 路径点集合
	if (event == EVENT_LBUTTONDOWN)
	{
		ptStart = Point(x, y);
		vctPoint.push_back(ptStart);
		cv::circle(pic, ptStart, 1, cv::Scalar(255, 0, 255), CV_FILLED, CV_AA, 0);
		cv::imshow("图片", pic);
		//cv::putText(tmp, temp, ptStart, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0, 0), 1, 8);
	}
	else if (event == EVENT_MOUSEMOVE && (flags & EVENT_FLAG_LBUTTON))
	{
		cur_pt = Point(x, y);
		cv::line(pic, vctPoint.back(), cur_pt, cv::Scalar(0, 255, 0, 0), 1, 8, 0);
		cv::circle(pic, cur_pt, 1, cv::Scalar(255, 0, 255), CV_FILLED, CV_AA, 0);
		cv::imshow("图片", pic);
		vctPoint.push_back(cur_pt);
		//cv::putText(tmp, temp, cur_pt, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0, 0));
	}
	else if (event == EVENT_LBUTTONUP)
	{
		cur_pt = Point(x, y);
		cv::line(pic, ptStart, cur_pt, cv::Scalar(0, 255, 0, 0), 1, 8, 0);
		cv::circle(pic, cur_pt, 1, cv::Scalar(255, 0, 255), CV_FILLED, CV_AA, 0);
		cv::imshow("图片", pic);
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
		frame_arr[0].copyTo(dst, maskImage);
		cv::imshow("ROI", dst);
	}
}