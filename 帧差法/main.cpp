#include "opencv2/opencv.hpp"
using namespace std;
using namespace cv;

//计算二值化图中白色像素点个数
double bSums(Mat src)
{
    int counter = 0;
    //迭代器访问像素点  
    Mat_<uchar>::iterator it = src.begin<uchar>();
    Mat_<uchar>::iterator itend = src.end<uchar>();
    for (; it != itend; ++it)
    {
        if ((*it) == 255) counter += 1;//二值化后，像素点是0或者255  
    }
    int size = src.cols * src.rows;
    return 1.0 * counter / size;
}


int main(int argc, char** argv)
{
    String path ="123.avi";
    VideoCapture capture(path);
    if (!capture.isOpened())
    {
        cout << "File not found\n" << endl;
        return -1;
    }
    Mat frame;
    Mat gray; //灰度图像
    Mat gray_dilate1;
    Mat gray_dilate2;
    Mat gray_dilate3;
    Mat background, foreground, foreground_BW;
    Mat mid_filter;   //中值滤波法后的照片
    Mat frame_0, frame_1;//Mat m(3, 5, CV_32FC1, 1);
    int num = 0;

    capture >> frame;
    while (!frame.empty())
    {
        //imshow("frame_resize", frame);
        cvtColor(frame, gray, COLOR_RGB2GRAY);

        //选择前一帧作为背景（读入第一帧时，第一帧作为背景）
        if (num == 0)
        {
            background = gray.clone();
            frame_0 = background;
        }
        else
        {
            background = frame_0;
        }
        absdiff(gray, background, foreground);//用帧差法求前景  做差求绝对值
        //imshow("foreground", foreground);

        threshold(foreground, foreground_BW, 30, 255, 0);//二值化通常设置为50  255
        //threshold(foreground, foreground_BW, 0, 255 ,CV_THRESH_BINARY | CV_THRESH_OTSU) ;  //此处使用大津法  自适应取阈值
        //imshow("foreground_BW", foreground_BW);
        medianBlur(foreground_BW, mid_filter, 3);     //中值滤波法

        //double whilenum = bSums(mid_filter);
        //cout << "white pixel num rate:" << whilenum << endl;

        //寻找最外层轮廓  
        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        findContours(mid_filter, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE, Point());
        //Mat imageContours = Mat::zeros(mid_filter.size(), CV_8UC1); //最小外接矩形画布  

        for (int i = 0; i < contours.size(); i++)
        {
            //绘制轮廓  
            drawContours(mid_filter, contours, i, Scalar(255), 1, 8, hierarchy);
            //绘制轮廓的最小外结矩形  
            RotatedRect rect = minAreaRect(contours[i]);
            Point2f P[4];
            rect.points(P);
            for (int j = 0; j <= 3; j++)
            {
                line(mid_filter, P[j], P[(j + 1) % 4], Scalar(255), 2);
            }
        }
        imshow("mid_filter", mid_filter);
        frame_0 = gray.clone();
        num++;
        char c = waitKey(33);
        if (c == 27) break;

        capture >> frame;
    }
}