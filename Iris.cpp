#include "stdafx.h"
#include "opencv/cv.h"
#include <iostream>


using namespace cv;
using namespace std;

int main(void)
{
	int x, y,i;
	Mat src,src_gray;
	src = imread("Chessboard_iris1.jpg", IMREAD_COLOR);
	cvtColor(src, src_gray, COLOR_BGR2GRAY);
	vector <float> intensity;
	int histSize = 256;
	float range[] = { 0, 256 };
	const float* histRange = range;
	bool uniform = true;
	bool accumulate = false;

	//Draw the Histogram
	int hist_w = 1024, hist_h = 400;
	int bin_w = cvRound((double) hist_w/histSize);
	Mat histimg(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));
	

	Point pt1, pt2;
	pt1.x = 300;
	pt2.x = 750;
	pt1.y = 600;
	pt2.y = 600;
	line(src, pt1, pt2, Color::aqua, 2, LINE_AA);
	float a, b;
	if (pt2.y == pt1.y) // y=ax+b
	{ 
		a = (pt2.y - pt1.y) / (pt2.x - pt1.x);
		b = pt2.y - a * pt2.x;
		int temp,temp2;
		temp = pt1.x < pt2.x ? pt1.x : pt2.x;
		temp2 = pt1.x < pt2.x ? pt2.x : pt1.x;
		for (x = temp; x < temp2; x++)
		{
			y = a * x + b;
			intensity.push_back(src_gray.at<uchar>(y, x));
		}

	}
	else  //x= ay+b
	{
		a =  (pt2.x - pt1.x) / (pt2.y - pt1.y);
		b = pt2.x - a * pt2.y;
		int temp, temp2;
		temp = pt1.y < pt2.y ? pt1.y : pt2.y;
		temp2 = pt1.y < pt2.y ? pt2.y : pt1.y;
		for (y = temp; y < temp2; y++)
		{
			x = a * y + b;
			intensity.push_back(src_gray.at<uchar>(y, x));
		}

	}
	Mat hist(1, intensity.size(), CV_32FC1, Scalar(0, 0, 0));
	memcpy(hist.data, intensity.data(), intensity.size() * sizeof(float));
	//for (i = 0; i < intensity.size(); i++)
		//cout << (intensity[i]) << " ";
	cout << hist <<endl;
	Mat result;
	calcHist(&hist, 1, 0, Mat(), result, 1, &histSize, &histRange, uniform, accumulate);
	cout << "hist size : " << hist.size() << endl;
	cout << "result size : " << result.size() << endl;

	//cout << "Normalize Àü: " << result << endl;
	normalize(result, result, 0, histimg.rows, NORM_MINMAX, -1, Mat());
	//cout << "Normalize ÈÄ : " << result << endl;
	for (i = 1; i < histSize; i++)
	{
		line(histimg, Point(bin_w*(i - 1), hist_h - cvRound(result.at<float>(i-1))), Point(bin_w*i, hist_h - cvRound(result.at<float>(i))),Color::aliceblue,2,LINE_AA);
	}
	

	namedWindow("Source", WINDOW_KEEPRATIO);
	imshow("Source", src);

	namedWindow("Histogram", WINDOW_KEEPRATIO);
	imshow("Histogram", histimg);

	waitKey(0);
}