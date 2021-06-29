#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>
#include <opencv2/imgproc/imgproc_c.h>

using namespace cv;
using namespace std;

void on_mouse(int event, int x, int y, int flags, void* ustc);

Mat frame1;
Mat maskImage;

int main()
{
    /* 初始化视频读取和写入 */
    VideoCapture capture("307f03da3543eb357df1ab02ecdb5fdf.mp4");
    VideoWriter wrt("res.mp4", VideoWriter::fourcc('H', '2', '6', '4'), capture.get(cv::CAP_PROP_FPS),
        cv::Size(capture.get(cv::CAP_PROP_FRAME_WIDTH), capture.get(cv::CAP_PROP_FRAME_HEIGHT)));
    if (!capture.isOpened()) {
        //error in opening the video input
        cerr << "Unable to open file!" << endl;
        return 0;
    }

    Mat prvs;
    capture >> frame1;
    cvtColor(frame1, prvs, COLOR_BGR2GRAY);
    /* 设置ROI */
    cv::namedWindow("图片");
    cv::setMouseCallback("图片", on_mouse, 0); // 调用回调函数    
    cv::imshow("图片", frame1);
    cv::waitKey(0);
    cv::setMouseCallback("图片", NULL, 0); // 解除回调函数 

    while (true) {
        Mat frame2_masked;
        Mat frame2, next;
        capture >> frame2;
        if (frame2.empty())
            break;
        else
        {
            frame2.copyTo(frame2_masked, maskImage);
            frame2 = frame2_masked;
        }
            
        cvtColor(frame2, next, COLOR_BGR2GRAY);
        Mat flow(prvs.size(), CV_32FC2);
        calcOpticalFlowFarneback(prvs, next, flow, 0.5, 3, 15, 3, 5, 1.2, 0);
        // visualization
        Mat flow_parts[2];
        split(flow, flow_parts);
        Mat magnitude, angle, magn_norm;
        cartToPolar(flow_parts[0], flow_parts[1], magnitude, angle, true);
        normalize(magnitude, magn_norm, 0.0f, 1.0f, NORM_MINMAX);
        angle *= ((1.f / 360.f) * (180.f / 255.f));
        //build hsv image
        Mat _hsv[3], hsv, hsv8, bgr;
        _hsv[0] = angle;
        _hsv[1] = Mat::ones(angle.size(), CV_32F);
        _hsv[2] = magn_norm;
        threshold(_hsv[2], _hsv[2], 0.9, 1, THRESH_TOZERO);
        merge(_hsv, 3, hsv);
        hsv.convertTo(hsv8, CV_8U, 255.0);
        cvtColor(hsv8, bgr, COLOR_HSV2BGR);
        //imshow("current", next);
        frame2.setTo(0, bgr);
        imshow("frame2", frame2);
        wrt << frame2;
        int keyboard = waitKey(30);
        if (keyboard == 'q' || keyboard == 27)
            break;
        prvs = next;
    }
    wrt.release();
}


void on_mouse(int event, int x, int y, int flags, void* ustc) // event鼠标事件代号，x,y鼠标坐标，flags拖拽和键盘操作的代号    
{
    static vector<vector<Point>> vctvctPoint; // 保存n个多边形的点
    static Point ptStart(-1, -1); // 初始化起点
    static Point cur_pt(-1, -1); // 初始化临时节点
    static vector<Point> vctPoint; // 路径点集合
    if (event == EVENT_LBUTTONDOWN)
    {
        ptStart = Point(x, y);
        vctPoint.push_back(ptStart);
        cv::circle(frame1, ptStart, 1, cv::Scalar(255, 0, 255), CV_FILLED, CV_AA, 0);
        cv::imshow("图片", frame1);
        //cv::putText(tmp, temp, ptStart, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0, 0), 1, 8);
    }
    else if (event == EVENT_MOUSEMOVE && (flags & EVENT_FLAG_LBUTTON))
    {
        cur_pt = Point(x, y);
        cv::line(frame1, vctPoint.back(), cur_pt, cv::Scalar(0, 255, 0, 0), 1, 8, 0);
        cv::circle(frame1, cur_pt, 1, cv::Scalar(255, 0, 255), CV_FILLED, CV_AA, 0);
        cv::imshow("图片", frame1);
        vctPoint.push_back(cur_pt);
        //cv::putText(tmp, temp, cur_pt, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0, 0));
    }
    else if (event == EVENT_LBUTTONUP)
    {
        cur_pt = Point(x, y);
        cv::line(frame1, ptStart, cur_pt, cv::Scalar(0, 255, 0, 0), 1, 8, 0);
        cv::circle(frame1, cur_pt, 1, cv::Scalar(255, 0, 255), CV_FILLED, CV_AA, 0);
        cv::imshow("图片", frame1);
        vctPoint.push_back(cur_pt);
        vctvctPoint.push_back(vctPoint);
        vctPoint.clear();
        maskImage = Mat::zeros(frame1.size(), frame1.type());
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
        frame1.copyTo(dst, maskImage);
        cv::imshow("ROI", dst);
    }
}