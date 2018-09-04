#include "stdafx.h"
#include "opencv/cv.h"
#include <iostream>


using namespace cv;
using namespace std;

Mat blending(Mat &a, int lowbdry, int highbdry, Scalar colorname,int intensity)
{
	int x, y;
	Mat a_copy;
	a.convertTo(a_copy, CV_32FC3);
	Mat output;
	
	Mat alpha(a.rows, a.cols, CV_32FC3, Scalar(0, 0, 0));
	Mat color(a.rows, a.cols, CV_32FC3, Scalar(colorname));
	for (y = 0; y < alpha.rows; y++)
	{
		Vec3f *row = alpha.ptr<Vec3f>(y);
		for (x = lowbdry; x < highbdry; x++)
		{
			row[x] = Vec3f(intensity,intensity,intensity);
		}
	}
	alpha.convertTo(alpha, CV_32FC3, 1.0 / 255);

	multiply(color, alpha, color);
	multiply(a_copy, Scalar::all(1) - alpha, a_copy);
	add(color, a_copy, output);
	output.convertTo(output, CV_8UC3);

	return output;
}

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
	int hist_w = 1084, hist_h = 400;
	int blank = 60;
	int bin_w = cvRound((double) (hist_w-blank)/histSize);
	Mat histimg(hist_h, hist_w, CV_32FC3, Scalar(0, 0, 0));
	

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
	cout << hist << endl;
	//cout << "Normalize Àü: " << result << endl;
	normalize(result, result, 0, histimg.rows, NORM_MINMAX, -1, Mat());
	//cout << "Normalize ÈÄ : " << result << endl;
	histimg = blending(histimg, blank / 2, hist_w - blank / 2, Color::aqua,120);

	for (i = 1; i < histSize; i++)
	{
		line(histimg, Point(blank/2+bin_w*(i - 1), hist_h - cvRound(result.at<float>(i-1))), Point(blank / 2 +bin_w*i, hist_h - cvRound(result.at<float>(i))),Color::aliceblue,2,LINE_AA);
	}

	int black_low=40, black_high=80;
	int white_low=200, white_high=220;

	histimg = blending(histimg, black_low*bin_w, black_high*bin_w, Color::bisque,100);
	histimg = blending(histimg, white_low*bin_w, white_high*bin_w, Color::bisque,100);

	Mat *histo = new Mat;
	histimg.copyTo(*histo);


	namedWindow("Histogram", WINDOW_KEEPRATIO);
	imshow("Histogram", *histo);

	namedWindow("Source", WINDOW_KEEPRATIO);
	imshow("Source", src);

	

	waitKey(0);
}