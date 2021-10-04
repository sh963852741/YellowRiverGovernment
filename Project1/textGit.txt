////#include <opencv2/opencv.hpp>
////using namespace cv;
////
////int  WinMain() {
 ////   Mat img;
 ////   img.create(200, 300, CV_8UC3);
 ////   img.setTo(Scalar(255, 255, 255));

 ////   // draw lines
 ////   //line(img, Point(w / 4, w / 4), Point(3 * w / 4, w / 4), Scalar(255, 0, 0));


 ////   // draw rectangle
 ////   rectangle(img, Point(0,105), Point(50,200), Scalar(200, 200, 100), -1);
 ////   rectangle(img, Point(60, 80), Point(110, 200), Scalar(200, 200, 100), -1);
 ////   rectangle(img, Point(120, 44), Point(170, 200), Scalar(200, 200, 100), -1);
 ////   rectangle(img, Point(180, 86), Point(230, 200), Scalar(200, 200, 100), -1);
 ////   rectangle(img, Point(240, 130), Point(290, 200), Scalar(200, 200, 100), -1);

	////imshow("1",img);
////    img.setTo(Scalar(255, 255, 255));
////
////
////	return 0;
////}
//
//#include<opencv2/opencv.hpp>
//#include<opencv2/core/core.hpp>
//#include<opencv2/highgui/highgui.hpp>
//#include<iostream>
//using namespace cv;
//using namespace std;
//#define WINDOW_NAME "灰度阈值操作"
//
//void on_Threshold(int, void*);
//int threshold_lower = 0;     //上限滑条值
//int threshold_up = 255;     //下限滑条值
//const int MaxthresholdValue = 255;  // 滑条的最大值
//
//Mat image;   //原图像
//Mat gray;    //灰度图
//Mat result;  //显示结果
//int WinMain(int argc, char** argv)
//{
//    image = imread("C:/Users/ferster/Desktop/1.png", 1);    //读取原图像
//    cvtColor(image, gray, COLOR_BGR2GRAY);     //转换为灰度图像
//    namedWindow(WINDOW_NAME, 1);              //开启一窗口
//    imshow(WINDOW_NAME, gray);                //最初显示原灰度图
//    //创建滑动条来控制阈值
//    createTrackbar("阈值上限", WINDOW_NAME, &threshold_lower, MaxthresholdValue, on_Threshold);  //创建下限滑条
//    createTrackbar("阈值下限", WINDOW_NAME, &threshold_up, MaxthresholdValue, on_Threshold);     //创建上限滑条
//    //初始化 阈值回调函数
//    on_Threshold(0, 0);
//    waitKey(0);
//    return 0;
//}
////　阈值回调函数
//void on_Threshold(int, void*)
//{
//    //调用阈值函数
//    threshold(gray, result, threshold_lower, 0, THRESH_TOZERO);        //低于下限置0
//    threshold(result, result, threshold_up, 0, THRESH_TOZERO_INV);        //高于上限置0
//    //更新效果图
//    imshow(WINDOW_NAME, result);
//}
