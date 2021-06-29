// 程序描述：来自OpenCV安装目录下Samples文件夹中的官方示例程序-利用光流法进行运动目标检测
//  描述：包含程序所使用的头文件和命名空间
#include <opencv2/video/video.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <iostream>
#include <cstdio>
using namespace std;
using namespace cv;
//  描述：声明全局函数
void trackingPyrLK(Mat& frame, Mat& output);
void trackingFarneback(Mat& frame, Mat& output);
bool addNewPoints();
bool acceptTrackedPoint(int i);
//  描述：声明全局变量
string window_name = "optical flow tracking";
Mat gray;   // 当前图片
Mat gray_prev;  // 预测图片
vector<Point2f> points[2];  // point0为特征点的原来位置，point1为特征点的新位置
vector<Point2f> initial;    // 初始化跟踪点的位置
vector<Point2f> features;   // 检测的特征
int maxCount = 500; // 检测的最大特征数
double qLevel = 0.01;   // 特征检测的等级
double minDist = 10.0;  // 两特征点之间的最小距离
vector<uchar> status;   // 跟踪特征的状态，特征的流发现为1，否则为0
vector<float> err;
//输出相应信息和OpenCV版本-----
static void helpinformation()
{
    cout << "\n\n\t\t\t 光流法跟踪运动目标检测\n"
        << "\n\n\t\t\t 当前使用的OpenCV版本为：" << CV_VERSION
        << "\n\n";
}

//main( )函数，程序入口
int main()
{
    Mat frame;
    Mat result;
    VideoCapture capture("VID_20210627_202501.mp4");
    helpinformation();

    if (capture.isOpened())
    {
        VideoWriter wrt("res_refined.mp4", VideoWriter::fourcc('H', '2', '6', '4'), 25.0, cv::Size(522, 922));
        while (true)
        {
            capture >> frame;
            

            if (!frame.empty())
            {
                trackingFarneback(frame, result);
                wrt << result; // where image should be cv::Mat class
            }
            else
            {
                printf(" --(!) No captured frame -- Break!");
                break;
            }
            int c = waitKey(1);
            if ((char)c == 27)
            {
                wrt.release();
                break;
            }
        }
        wrt.release();
    }
    return 0;
}

void trackingFarneback(Mat& frame, Mat& output)
{
    // 转换为灰度图像
    cvtColor(frame, gray, COLOR_RGB2GRAY);
    frame.copyTo(output);

    if (gray_prev.empty())
    {
        gray.copyTo(gray_prev);
    }

    Mat result;//保存结果 CF_32FC2
    calcOpticalFlowFarneback(gray_prev, gray, result, 0.5, 6, 7, 6, 5, 1.1, OPTFLOW_FARNEBACK_GAUSSIAN);
    for (int i = 0; i < gray_prev.rows; ++i)
    {
        for (int j = 0; j < gray_prev.cols; ++j)
        {
            Point2f pt = result.at<Point2f>(i, j);
            //if(pt.y > 0 && pt.x* pt.x + pt.y * pt.y > 9)
            if (abs(pt.y) > 1 && abs(pt.x) > 1)
                line(output, Point2f(j, i), Point2f(j + pt.x, i + pt.y), Scalar(0, 255, 0), 1);
        }
    }

    swap(gray_prev, gray);
    imshow("Result", output);
}

// parameter: frame 输入的视频帧 output 有跟踪结果的视频帧
void trackingPyrLK(Mat& frame, Mat& output)
{
    // 转换为灰度图像
    cvtColor(frame, gray, COLOR_RGB2GRAY);
    frame.copyTo(output);
    // 添加特征点
    if (addNewPoints())
    {
        goodFeaturesToTrack(gray, features, maxCount, qLevel, minDist);
        for (int i = 0; i < features.size(); ++i)
        {
            circle(output, features[i], 5, Scalar(255, 0, 0), -1);
        }
        points[0].insert(points[0].end(), features.begin(), features.end());
        initial.insert(initial.end(), features.begin(), features.end());
    }

    // 函数第一次被调用
    if (gray_prev.empty())
    {
        gray.copyTo(gray_prev);
    }
    // l-k光流法运动估计
    calcOpticalFlowPyrLK(gray_prev, gray, points[0], points[1], status, err);
    // 去掉一些不好的特征点
    int k = 0;
    for (size_t i = 0; i < points[1].size(); i++)
    {
        if (acceptTrackedPoint(i))
        {
            initial[k] = initial[i];
            points[1][k++] = points[1][i];
        }
    }
    points[1].resize(k);
    initial.resize(k);
    // 显示特征点和运动轨迹
    for (size_t i = 0; i < points[1].size(); i++)
    {
        line(output, initial[i], points[1][i], Scalar(0, 0, 255));
        circle(output, points[1][i], 1, Scalar(0, 255, 0), -1);
    }

    // 把当前跟踪结果作为下一次的参考
    swap(points[1], points[0]);
    swap(gray_prev, gray);
    imshow(window_name, output);
}

//  检测新点是否应该被添加
// return: 是否被添加标志
bool addNewPoints()
{
    return points[0].size() <= 10;
}

//决定哪些跟踪点被接受
bool acceptTrackedPoint(int i)
{
    return status[i] && ((abs(points[0][i].x - points[1][i].x) + abs(points[0][i].y - points[1][i].y)) > 2);
}