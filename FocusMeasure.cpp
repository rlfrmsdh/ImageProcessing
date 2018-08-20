#include "stdafx.h"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>

using namespace cv;
using namespace std;
Mat src,src_gray,src_part,lap,scharr,dx,dy;
vector <vector<float>> fmeas;

float LAP1(Mat &a)
{
	int sizemat = a.cols * a.rows;
	float fm = 0;
	for (int y = 0; y < a.rows; y++)
	{
		uchar * temp = a.ptr<uchar>(y);
		for (int x = 0; x < a.cols; x++)
		{
			fm += (temp[x])^2;
		}
	}

	return fm / sizemat;
}
bool sortcol2(vector<float> &a, vector <float> &b)
{
	return a[0] < b[0];
}

int main(void)
{
	int hor_part = 4;
	int ver_part = 6;
	float blur_limit;
	int i, j, cnt;
	src = imread("Chessboard_focusmeasure2.jpg", IMREAD_COLOR); // Load an image 
	src.convertTo(src_gray, CV_8UC1, 1.0, 0.0);
	lap.create(src.rows, src.cols, CV_16SC3);
	blur(src_gray, src_gray, Size(3, 3));

	Scharr(src_gray, dx, CV_32F, 1, 0, 1, 0, BORDER_REPLICATE);
	Scharr(src_gray, dy, CV_32F, 0, 1, 1, 0, BORDER_REPLICATE);   //dx_Scharr, dy_Scharr 형은 CV_32F
																			   //sqrt(dx.mul(dx) + dy.mul(dy), sobel);
	sqrt(dx.mul(dx) + dy.mul(dy), scharr);  // Mat.mul(Mat)는 element by element multiplication

	double minVal_s, maxVal_s;
	minMaxLoc(scharr, &minVal_s, &maxVal_s);

	scharr.convertTo(scharr, CV_8U, 255.0 / (maxVal_s - minVal_s), 0);

	//src_part.create(900, 850, CV_8UC1);
	//src_part = Scalar::all(0);
	//src_part = src_gray(Range(0, 1600), Range(300, 1700));     // 앞에가 Row, 뒤에가 Col range
		
	Laplacian(src_gray, lap, CV_16S, 3, 1.0, 0.0, BORDER_DEFAULT);
	convertScaleAbs(lap, lap, 1.0, 0.0);
	
	vector <Mat> sec;
	for (i = 0,cnt=0; i < ver_part; i++)
	{
		for (j = 0; j < hor_part; j++,cnt++)
		{
			vector <float> row;
			if (i == ver_part && j != hor_part)
				sec.push_back(lap(Range(i*floor(lap.rows / ver_part), lap.rows), Range(j*floor(lap.cols / hor_part), (j + 1)*floor(lap.cols / hor_part) - 1)));

			else if (i != ver_part && j == hor_part)
			{
				sec.push_back(lap(Range(i*floor(lap.rows / ver_part), (i + 1)*floor(lap.rows / ver_part) - 1), Range(j*floor(lap.cols / hor_part), lap.cols)));
			}

			else if (i != ver_part && j != hor_part)
			{
				sec.push_back(lap(Range(i*floor(lap.rows / ver_part), (i + 1)*floor(lap.rows / ver_part) - 1), Range(j*floor(lap.cols / hor_part), (j + 1)*floor(lap.cols / hor_part) - 1)));
			}

			else
				sec.push_back(lap(Range(i*floor(lap.rows / ver_part), lap.rows), Range(j*floor(lap.cols / hor_part), lap.cols)));
			row.push_back(LAP1(sec[cnt]));
			row.push_back(cnt);
			fmeas.push_back(row);
			
		}
	}
	
	sort(fmeas.begin(), fmeas.end(),sortcol2);
	blur_limit = (fmeas[fmeas.size()-1][0]-fmeas[0][0])*0.3 + fmeas[0][0];
	for (i=0; i< fmeas.size();i++)
	{	
		int seq = fmeas[i][1];
		int rem, div;
		rem = seq % hor_part;
		div = seq / hor_part;
		cout << "F-meas : " << fmeas[i][0] << "Position" << "[" << div + 1 << " , " << rem + 1 << "]" << endl;
		if (fmeas[i][0] >= blur_limit)
		{
			
			int x_lt, x_rt, x_lb, x_rb, y_lt, y_rt, y_lb, y_rb;
			Point lt, rt, lb, rb;
		
			
			if (div == ver_part - 1)
			{
				y_rt = div * floor(lap.rows / ver_part);
				y_lt = div * floor(lap.rows / ver_part);
				y_rb = lap.rows;
				y_lb = lap.rows;
			}

			else
			{
				y_rt = div * floor(lap.rows / ver_part);
				y_lt = div * floor(lap.rows / ver_part);
				y_rb = (div + 1)* floor(lap.rows / ver_part);
				y_lb = (div + 1)* floor(lap.rows / ver_part);
			}

			if (rem == hor_part - 1)
			{
				x_lb = rem * floor(lap.cols / hor_part);
				x_lt = rem * floor(lap.cols / hor_part);
				x_rb = lap.cols;
				x_rt = lap.cols;
			}

			else
			{
				x_lb = rem * floor(lap.cols / hor_part);
				x_lt = rem * floor(lap.cols / hor_part);
				x_rb = (rem + 1) * floor(lap.cols / hor_part);
				x_rt = (rem + 1) * floor(lap.cols / hor_part);
			}
			lb.x = x_lb;
			lb.y = y_lb;
			rb.x = x_rb;
			rb.y = y_rb;
			lt.x = x_lt;
			lt.y = y_lt;
			rt.x = x_rt;
			rt.y = y_rt;
			line(src, lb, rb, Scalar(0, 0, 255), 2, LINE_AA);
			line(src, lb, lt, Scalar(0, 0, 255), 2, LINE_AA);
			line(src, lt, rt, Scalar(0, 0, 255), 2, LINE_AA);
			line(src, rb, rt, Scalar(0, 0, 255), 2, LINE_AA);
		}
	}
	

	namedWindow("Laplacian", WINDOW_KEEPRATIO);
	imshow("Laplacian", lap);

	namedWindow("Scharr", WINDOW_KEEPRATIO);
	imshow("Scharr", scharr);
//namedWindow("Part", WINDOW_KEEPRATIO);
//imshow("Part", src_part);
//imwrite("Chessboard_focusmeasure1.jpg", src_part);
	namedWindow("Source", WINDOW_KEEPRATIO);
	imshow("Source", src);


	waitKey(0);
	return 0;


}
/*

*/

