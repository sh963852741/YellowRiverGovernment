#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <iostream>
#include <opencv2/imgproc/types_c.h>
using namespace cv;
using namespace std;

// ��ʷ��Ϣ֡��
const int HISTORY_NUM = 7;
// KNN������ж�Ϊ��������ֵ
const int nKNN = 3;
// �ҶȾ�����ֵ
const float defaultDist2Threshold = 20.0f;

// ������¼ͼ�����ʷ��Ϣ
struct PixelHistory
{
	unsigned char* gray; // ��ʷ�Ҷ�ֵ��0-255��
	unsigned char* IsBG; // 1-->������0-->ǰ��
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
		// �Ƚ�Ҫ�����ͼƬ��ɰ�ɫ��
		FGMask.setTo(Scalar(255));
		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < cols; j++)
			{
				gray = frame.at<unsigned char>(i, j);
				// �Ƿ���ǰ���ĸ���
				int foreground = 0;
				// �Ƿ��ϱ����ĸ���
				int background = 0;
				for (int n = 0; n < HISTORY_NUM; n++)
				{
					if (fabs(gray - framePixelHistory[i * cols + j].gray[n]) < defaultDist2Threshold)
					{
						// �жϵ�ǰ֡����ʷ�ϵ�ÿһ֡�ĻҶȲ���Ƿ�λ���趨��ֵ��
						foreground++;
						if (framePixelHistory[i * cols + j].IsBG[n])
						{
							// ����ʷ�Ͽ���֮ǰ������Ǳ���
							background++;
						}
					}
				}

				if (background >= nKNN) 
				{
					// ͶƱԭ�򣬵�ǰ���ж�Ϊ����
					FGMask.at<unsigned char>(i, j) = 0;
				}

				// ������ʷֵ
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