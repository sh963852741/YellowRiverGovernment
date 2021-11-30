
//#include "OpticalFlowInterface.h"

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



void makeMask(cv::Mat picture);
void on_mouse(int event, int x, int y, int flags, void* ustc);
int opticalFlow(Mat* frame_arr, bool if_show_dyna);

Mat maskImage;

using namespace cv;
using namespace std;


int main() {
	VideoPictrueGetter c("1.mp4");
	PictureGetter& pg = c;
	Mat frame_arr[2];
	
	pg >> frame_arr;
	/* ����ROI */
	makeMask(frame_arr[0]);
	while (true) {
		pg >> frame_arr;
		opticalFlow(frame_arr, true);
	}
		
}

int opticalFlow(Mat* frame_arr, bool if_show_dyna) //�Ƿ���ʾ������
{
	static cv::Point history_center;
	static bool history[5] = { false };
	{
		int error_level = 0;
		int move_count = 0;
		int i_sum = 0, j_sum = 0;
		Mat frame_gray[2];
		Mat frame_masked_arr[2];
		
		if (frame_arr[1].empty()) return -1;

		// ��ͼ�������Ĥ����
		frame_arr[0].copyTo(frame_masked_arr[0], maskImage);
		frame_arr[1].copyTo(frame_masked_arr[1], maskImage);
		// ��Ϊ�Ҷ�ͼ��
		cvtColor(frame_masked_arr[0], frame_gray[0], COLOR_BGR2GRAY);
		cvtColor(frame_masked_arr[1], frame_gray[1], COLOR_BGR2GRAY);
		Mat flow(frame_gray[0].size(), CV_32FC2);
		calcOpticalFlowFarneback(frame_gray[0], frame_gray[1], flow, 0.5, 3, 15, 3, 5, 1.2, 0);
		// ����������ӻ�
		Mat flow_parts[2];
		split(flow, flow_parts); // ��Xֵ��Yֵ���������
		Mat magnitude, angle, magn_norm;
		cartToPolar(flow_parts[0], flow_parts[1], magnitude, angle, true);
		normalize(magnitude, magn_norm, 0.0f, 1.0f, NORM_MINMAX);
		angle *= ((1.f / 360.f) * (180.f / 255.f));
		//build hsv image
		Mat _hsv[3], hsv, hsv8, bgr;
		_hsv[0] = angle;
		_hsv[1] = Mat::ones(angle.size(), CV_32F);
		_hsv[2] = magn_norm;

		//threshold(_hsv[0], direction_mask, 90.f / 255.f, 1, THRESH_BINARY_INV); // ������180�ȵĶ���0
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
		/* ����ͼƬ�ϵ�ÿ�����ص� */
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
			error_level = 1;
			putText(res_img, "!", Point(0, 25), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 3);
		}
		else if (count > 5)
		{
			// ���˶�
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

		imshow("ʶ����", res_img);
		return error_level;
	}
}


void makeMask(cv::Mat picture)
{
	cv::namedWindow("����ѡ������");
	cv::setMouseCallback("����ѡ������", on_mouse, &picture); // ���ûص�����    
	cv::imshow("����ѡ������", picture);
	cv::waitKey(0);
	cv::setMouseCallback("����ѡ������", NULL, 0); // ����ص�����
	if (cv::getWindowProperty("����ѡ������", WND_PROP_VISIBLE) == 1)
		cv::destroyWindow("����ѡ������");
	if (cv::getWindowProperty("ROI", WND_PROP_VISIBLE) == 1)
		cv::destroyWindow("ROI");
}

void on_mouse(int event, int x, int y, int flags, void* ustc) // event����¼����ţ�x,y������꣬flags��ק�ͼ��̲����Ĵ���    
{
	Mat& pic = *((Mat*)ustc);
	//maskImage = Mat::zeros(pic.size(), pic.type());
	static vector<vector<Point>> vctvctPoint; // ����n������εĵ�
	static Point ptStart(-1, -1); // ��ʼ�����
	static Point cur_pt(-1, -1); // ��ʼ����ʱ�ڵ�
	static vector<Point> vctPoint; // ·���㼯��
	if (event == EVENT_LBUTTONDOWN)
	{
		ptStart = Point(x, y);
		vctPoint.push_back(ptStart);
		cv::circle(pic, ptStart, 1, cv::Scalar(255, 0, 255), CV_FILLED, CV_AA, 0);
		cv::imshow("����ѡ������", pic);
		//cv::putText(tmp, temp, ptStart, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0, 0), 1, 8);
	}
	else if (event == EVENT_MOUSEMOVE && (flags & EVENT_FLAG_LBUTTON))
	{
		cur_pt = Point(x, y);
		cv::line(pic, vctPoint.back(), cur_pt, cv::Scalar(0, 255, 0, 0), 1, 8, 0);
		cv::circle(pic, cur_pt, 1, cv::Scalar(255, 0, 255), CV_FILLED, CV_AA, 0);
		cv::imshow("����ѡ������", pic);
		vctPoint.push_back(cur_pt);
		//cv::putText(tmp, temp, cur_pt, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0, 0));
	}
	else if (event == EVENT_LBUTTONUP)
	{
		cur_pt = Point(x, y);
		cv::line(pic, ptStart, cur_pt, cv::Scalar(0, 255, 0, 0), 1, 8, 0);
		cv::circle(pic, cur_pt, 1, cv::Scalar(255, 0, 255), CV_FILLED, CV_AA, 0);
		cv::imshow("����ѡ������", pic);
		vctPoint.push_back(cur_pt);
		vctvctPoint.push_back(vctPoint);
		vctPoint.clear();
		maskImage = Mat::zeros(pic.size(), pic.type());
		//�ѵ㹹���������ν������
		for (int i = 0; i < vctvctPoint.size(); ++i)
		{
			const Point* ppt[1] = { &vctvctPoint[i][0] };//ȡ������׵�ַ
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
