// openCV.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//
#include "stdafx.h"
#include "opencv2/opencv.hpp"

using namespace cv;

int main(int argv, char** argc)
{
	cv::Mat input =cv::Mat(512, 512, CV_8UC1, cv::Scalar(0));
	
	Mat test = imread("opencv-logo.png", CV_LOAD_IMAGE_GRAYSCALE);
	Mat test_gray;
	Mat test1(test.size(),CV_8UC3);
	std::cout << test.size << std::endl;
	Mat testcolor = imread("opencv-logo.png", CV_LOAD_IMAGE_ANYDEPTH);
	
//	imshow("Anydepth", testcolor);
	/* imshow("test", test);
	cvtColor(test, test_gray, CV_BGR2GRAY);
	imshow("test_gray", test_gray); */
	
	cvtColor(test, test1, CV_GRAY2BGR);
	imshow("test1", test1);
	//imwrite("Wow.jpg", testcolor);

	waitKey(0);
}
