#include "stdafx.h"
#include "opencv/cv.h"
#include "opencv2/imgproc.hpp"
#include <iostream>
using namespace cv;
using namespace std;

int main (void)
{
	
	float Threshold;
	Threshold = 50;
Mat src, FMplot,FM_gray;  //FocusMeasure에서 구해낸 FMplot Mat파일을 읽어들임.
src = imread("Chessboard_focusmeasure2.jpg", IMREAD_COLOR);
FMplot = imread("FMplot.jpg", IMREAD_COLOR);

int x, y, i;

cvtColor(FMplot, FM_gray, COLOR_BGR2GRAY);
vector <Point> plot_pos;
vector <Point> Inrange;

for (y = 0; y < FM_gray.rows; y++)
{
	uchar *row = FM_gray.ptr<uchar>(y);
	for (x = 0; x < FM_gray.cols; x++)
	{
		if (row[x] > 0)
			plot_pos.push_back(Point(x, y));
	}
}

int min_x = FM_gray.cols, max_x = 0;  //Threshold 정해주기 위함
for (i = 0; i < plot_pos.size(); i++)
{
	if (plot_pos[i].x < min_x)
		min_x = plot_pos[i].x;
	if (plot_pos[i].x > max_x)
		max_x = plot_pos[i].x;
}

int limit_x;
limit_x = min_x + (Threshold / 100)*(max_x - min_x);
for (i = 0; i < plot_pos.size(); i++)
{
	if (plot_pos[i].x >= limit_x)
		Inrange.push_back(plot_pos[i]);
}

int lowbdry=FM_gray.cols, highbdry=0;
for (i = 0; i < Inrange.size(); i++)
{
	if (Inrange[i].y < lowbdry)
		lowbdry = Inrange[i].y;
	if (Inrange[i].y > highbdry)
		highbdry = Inrange[i].y;
}

Mat *FMrange = new Mat(src.rows, src.cols, CV_32FC3);

Mat alpha(src.rows, src.cols, CV_32FC3, Scalar(0, 0, 0));

for (y = lowbdry; y < highbdry; y++)
{
	Vec3f *row = alpha.ptr<Vec3f>(y);
	for (x = 0; x < src.cols; x++)
	{
		row[x] = Vec3f(100, 100, 100);

	}

}

alpha.convertTo(alpha, CV_32FC3, 1.0 / 255);
Mat red(src.rows, src.cols, CV_32FC3, Scalar(255, 0, 0));
multiply(red, alpha, red);



Mat src_alpha;
src_alpha.create(src.rows, src.cols, CV_32FC3);
src.convertTo(src, CV_32FC3);
multiply(src, Scalar::all(1) - alpha, src_alpha);
add(src_alpha, red, *FMrange);
FMrange->convertTo(*FMrange, CV_8UC3);


//namedWindow("FMplot", WINDOW_KEEPRATIO);
//imshow("FMplot", FMplot);

//namedWindow("src", WINDOW_KEEPRATIO);
//imshow("src", src);


namedWindow("FMrange", WINDOW_KEEPRATIO);
imshow("FMrange", *FMrange);
waitKey(0);

}