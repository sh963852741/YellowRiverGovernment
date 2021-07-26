#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>
#include <opencv2/imgproc/imgproc_c.h>

#include <thread>

#include "PictureGetter.h"

using namespace cv;
using namespace std;
void on_mouse(int event, int x, int y, int flags, void* ustc);

Mat frame_arr[2];
Mat maskImage;

int main()
{
	PictureGetter pg;
	pg.init();
	
	/* ��ʼ����Ƶ��ȡ��д�� */
	//VideoCapture capture("������.mp4");
	VideoWriter wrt("res.mp4", VideoWriter::fourcc('H', '2', '6', '4'), 24,// capture.get(cv::CAP_PROP_FPS),
		cv::Size(1280, 720));
		//cv::Size(capture.get(cv::CAP_PROP_FRAME_WIDTH), capture.get(cv::CAP_PROP_FRAME_HEIGHT)));
	//if (!capture.isOpened()) {
	//	//error in opening the video input
	//	cerr << "Unable to open file!" << endl;
	//	return 0;
	//}
	
	Mat direction_mask, magn_mask, final_mask;
	pg >> frame_arr;
	/* ����ROI */
	cv::namedWindow("ͼƬ");
	cv::setMouseCallback("ͼƬ", on_mouse, 0); // ���ûص�����    
	cv::imshow("ͼƬ", frame_arr[0]);
	cv::waitKey(0);
	cv::setMouseCallback("ͼƬ", NULL, 0); // ����ص����� 

	Mat frame_gray[2];
	Mat frame_masked_arr[2];
	while (true) {
		
		pg >> frame_arr;

		// ��ͼ�������Ĥ����
		frame_arr[0].copyTo(frame_masked_arr[0], maskImage);
		frame_arr[1].copyTo(frame_masked_arr[1], maskImage);
		// ��Ϊ�Ҷ�ͼ��
		cvtColor(frame_masked_arr[0], frame_gray[0], COLOR_BGR2GRAY);
		cvtColor(frame_masked_arr[1], frame_gray[1], COLOR_BGR2GRAY);
		Mat flow(frame_arr[0].size(), CV_32FC2);
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

		
		for (int i = 0; i < frame_masked_arr[1].rows; i++) {
			for (int j = 0; j < frame_masked_arr[1].cols; j++) {
				if (angle.at<float>(i, j) < 90.f / 255.f && angle.at<float>(i, j) > 0 && magn_norm.at<float>(i, j) > 0.8)
				{
					frame_masked_arr[1].at<Vec3b>(i, j)[0] = bgr.at<Vec3b>(i, j)[0];
					frame_masked_arr[1].at<Vec3b>(i, j)[1] = bgr.at<Vec3b>(i, j)[1];
					frame_masked_arr[1].at<Vec3b>(i, j)[2] = bgr.at<Vec3b>(i, j)[2];
				}
			}
		}

		imshow("result", frame_masked_arr[1]);
		wrt << frame_masked_arr[1];
		int keyboard = waitKey(30);
		if (keyboard == 'q' || keyboard == 27)
			break;
	}
	wrt.release();
}

void on_mouse(int event, int x, int y, int flags, void* ustc) // event����¼����ţ�x,y������꣬flags��ק�ͼ��̲����Ĵ���    
{
	static vector<vector<Point>> vctvctPoint; // ����n������εĵ�
	static Point ptStart(-1, -1); // ��ʼ�����
	static Point cur_pt(-1, -1); // ��ʼ����ʱ�ڵ�
	static vector<Point> vctPoint; // ·���㼯��
	if (event == EVENT_LBUTTONDOWN)
	{
		ptStart = Point(x, y);
		vctPoint.push_back(ptStart);
		cv::circle(frame_arr[0], ptStart, 1, cv::Scalar(255, 0, 255), CV_FILLED, CV_AA, 0);
		cv::imshow("ͼƬ", frame_arr[0]);
		//cv::putText(tmp, temp, ptStart, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0, 0), 1, 8);
	}
	else if (event == EVENT_MOUSEMOVE && (flags & EVENT_FLAG_LBUTTON))
	{
		cur_pt = Point(x, y);
		cv::line(frame_arr[0], vctPoint.back(), cur_pt, cv::Scalar(0, 255, 0, 0), 1, 8, 0);
		cv::circle(frame_arr[0], cur_pt, 1, cv::Scalar(255, 0, 255), CV_FILLED, CV_AA, 0);
		cv::imshow("ͼƬ", frame_arr[0]);
		vctPoint.push_back(cur_pt);
		//cv::putText(tmp, temp, cur_pt, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0, 0));
	}
	else if (event == EVENT_LBUTTONUP)
	{
		cur_pt = Point(x, y);
		cv::line(frame_arr[0], ptStart, cur_pt, cv::Scalar(0, 255, 0, 0), 1, 8, 0);
		cv::circle(frame_arr[0], cur_pt, 1, cv::Scalar(255, 0, 255), CV_FILLED, CV_AA, 0);
		cv::imshow("ͼƬ", frame_arr[0]);
		vctPoint.push_back(cur_pt);
		vctvctPoint.push_back(vctPoint);
		vctPoint.clear();
		maskImage = Mat::zeros(frame_arr[0].size(), frame_arr[0].type());
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
		frame_arr[0].copyTo(dst, maskImage);
		cv::imshow("ROI", dst);
	}
}