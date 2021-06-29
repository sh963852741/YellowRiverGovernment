#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main()
{
    String filepath = "307f03da3543eb357df1ab02ecdb5fdf.mp4";
    VideoCapture capture(filepath);

    if (capture.isOpened())
    {
        VideoWriter wrt("res.mp4", VideoWriter::fourcc('a', 'c', 'v', 'l'), 30.0, cv::Size(544, 960));
        Mat first_frame, second_frame; // 每一帧图像

        
        capture >> first_frame;
        while (true)
        {
            capture >> second_frame;

            if (!second_frame.empty())
            {

                first_frame = second_frame;
            }
        }
    }
    else
    {

    }
}




