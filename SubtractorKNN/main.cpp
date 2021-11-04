#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <iostream>
#include <opencv2/imgproc/types_c.h>
using namespace cv;
using namespace std;

// 历史信息帧数
const int HISTORY_NUM = 7;
// KNN聚类后判断为背景的阈值
const int nKNN = 3;
// 灰度聚类阈值
const float defaultDist2Threshold = 20.0f;

// 用来记录图像的历史信息
struct PixelHistory
{
	unsigned char* gray; // 历史灰度值（0-255）
	unsigned char* IsBG; // 1-->背景，0-->前景
};


int main()
{
	PixelHistory* framePixelHistory = NULL;
	cv::Mat frame, FGMask, FGMask_KNN;
	int keyboard = 0;
	short rows, cols;
	rows = cols = 0;
	int frameCount = 0;
	int gray = 0;
	VideoCapture capture("2e8e4df3f151c39a9dee2ae6cae9dd81.mp4");
	Ptr<BackgroundSubtractorKNN> pBackgroundKnn = createBackgroundSubtractorKNN();
	pBackgroundKnn->setHistory(200);
	pBackgroundKnn->setDist2Threshold(600);
	pBackgroundKnn->setShadowThreshold(0.5);

	if (!capture.isOpened())
	{
		return -1;
	}

	capture >> frame;
	cvtColor(frame, frame, CV_BGR2GRAY);

	rows = frame.rows;
	cols = frame.cols;
	FGMask.create(rows, cols, CV_8UC1);

	framePixelHistory = (PixelHistory*)malloc(rows * cols * sizeof(PixelHistory));
	for (int i = 0; i < rows * cols; i++)
	{
		framePixelHistory[i].gray = (unsigned char*)malloc(HISTORY_NUM * sizeof(unsigned char));
		framePixelHistory[i].IsBG = (unsigned char*)malloc(HISTORY_NUM * sizeof(unsigned char));
		memset(framePixelHistory[i].gray, 0, HISTORY_NUM * sizeof(unsigned char));
		memset(framePixelHistory[i].IsBG, 0, HISTORY_NUM * sizeof(unsigned char));
	}

	while ((char)keyboard != 'q' && (char)keyboard != 27)
	{
		// 先将要输出的图片变成白色的
		FGMask.setTo(Scalar(255));
		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < cols; j++)
			{
				gray = frame.at<unsigned char>(i, j);
				// 记符合前景的个数
				int foreground = 0;
				// 记符合背景的个数
				int background = 0;
				for (int n = 0; n < HISTORY_NUM; n++)
				{
					if (fabs(gray - framePixelHistory[i * cols + j].gray[n]) < defaultDist2Threshold)
					{
						// 判断当前帧与历史上的每一帧的灰度差别是否位于设定阈值内
						foreground++;
						if (framePixelHistory[i * cols + j].IsBG[n])
						{
							// 从历史上看，之前这个点是背景
							background++;
						}
					}
				}

				if (background >= nKNN) 
				{
					// 投票原则，当前点判断为背景
					FGMask.at<unsigned char>(i, j) = 0;
				}

				// 更新历史值
				int index = frameCount % HISTORY_NUM;
				framePixelHistory[i * cols + j].gray[index] = gray;
				framePixelHistory[i * cols + j].IsBG[index] = foreground >= nKNN ? 1 : 0;
			}
		}
		capture >> frame;
		cvtColor(frame, frame, CV_BGR2GRAY);

		pBackgroundKnn->apply(frame, FGMask_KNN);
		imshow("Frame", frame);
		imshow("FGMask", FGMask);
		imshow("FGMask_KNN", FGMask_KNN);

		keyboard = waitKey(30);
		frameCount++;
	}

	capture.release();

	return 0;
}