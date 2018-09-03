#include "stdafx.h"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;


struct FM {
	float fm;
	int pixelno;
};

FM LAP1(Mat &a)
{
	int sizemat = a.cols * a.rows;
	int pixelno=0;
	float fm = 0;
	for (int y = 0; y < a.rows; y++)
	{
		uchar * temp = a.ptr<uchar>(y);
		for (int x = 0; x < a.cols; x++)
		{
			if (temp[x] > 0)
				pixelno++;
			fm += (temp[x])*(temp[x]);
		}
	} 
	return { fm,pixelno };
}

static void EdgeThinner(Mat &input)
{
	Mat edge;
	edge.create(input.rows,input.cols, CV_8UC1);
	//cout << edge.size() << endl;
	edge = Scalar::all(0);
	int img_height = input.rows;
	int img_width = input.cols;
	
	for (int y = 0; y < img_height; y++)
	{
		uchar *row = input.ptr<uchar>(y);
		for (int x = 0; x < img_width; x++)
		{
			if (row[x] > 25)
				edge.at<uchar>(y, x) = row[x];
		}
	}
	//namedWindow("Laplace Thin",WINDOW_KEEPRATIO);
	//imshow("Laplace Thin", edge);
	edge.copyTo(input);


}


int main(void)
{
	int windowsize = 300;
	int windowdel = 75;
	Mat src, src_gray, src_part, lap, lap2, scharr, dx, dy;
	Mat hor, ver;
	vector <Vec2f> fmeas;
	

	int i, j, cnt;
	src = imread("Chessboard_focusmeasure2.jpg", IMREAD_COLOR); // Load an image 
	//src = src(Range(0,src.rows), Range(0,src.cols-100));
	//src.convertTo(src_gray, CV_8UC1, 1.0, 0.0);  // 이렇게 하면 Channel은 안바뀜!! 형변환만 일어남!
	cvtColor(src,src_gray,COLOR_BGR2GRAY);
	lap.create(src.rows, src.cols, CV_16SC1);
	lap2.create(src.rows, src.cols, CV_8UC3);
	hor.create(src.rows, src.cols, CV_8UC1);
	ver.create(src.rows, src.cols, CV_8UC1);
	hor = Scalar::all(0);
	ver = Scalar::all(0);

	blur(src_gray, src_gray, Size(3, 3));
	//cout << src_gray.size() << endl;
	Scharr(src_gray, dx, CV_32F, 1, 0, 1, 0, BORDER_REPLICATE);
	Scharr(src_gray, dy, CV_32F, 0, 1, 1, 0, BORDER_REPLICATE);   //dx_Scharr, dy_Scharr 형은 CV_32F
																			   //sqrt(dx.mul(dx) + dy.mul(dy), sobel);
	sqrt(dx.mul(dx) + dy.mul(dy), scharr);  // Mat.mul(Mat)는 element by element multiplication

	double minVal_s, maxVal_s;
	minMaxLoc(scharr, &minVal_s, &maxVal_s);

	scharr.convertTo(scharr, CV_8U, 255.0 / (maxVal_s - minVal_s), 0);
	EdgeThinner(scharr);
	double temp;
	for (int y = 0; y < scharr.rows; y++)
		for (int x = 0; x < scharr.cols; x++)
		{
			temp = atan2((double)(dy.at<float>(y, x)), (double)(dx.at<float>(y, x)));
			temp = temp * 180 / CV_PI;
			if (temp >= 90 && temp <= 180)
				temp = temp - 180;
			if (temp <= -90 && temp >= -180)
				temp = temp + 180;

			if ((temp > -45 && temp <= 45))
			{
				ver.at<uchar>(y, x) = scharr.at<uchar>(y, x);
			
			}
			else
			{
				hor.at<uchar>(y, x) = scharr.at<uchar>(y, x);
				
			}
		}



	
	
		
	Laplacian(src_gray, lap, CV_16S, 3, 1.0, 0.0, BORDER_DEFAULT);
	//lap = abs(lap);
	//lap.convertTo(lap, CV_8U, 1.0, 0.0);
	convertScaleAbs(lap, lap, 1.0, 0.0); // Scaling - Taking an absolute value - 8-bit conversion

	lap2 = Scalar::all(0);
	////lap를 lap2로 옮기면서 Channel3 , Scharr에서 mask 따기.
	for (int y = 0; y < ver.rows; y++)
	{
		uchar *temp = lap.ptr<uchar>(y);
		uchar *temp1 = ver.ptr<uchar>(y);
		for (int x = 0; x < ver.cols; x++)
		{
			if (temp1[x] > 0)
			{
				lap2.at<Vec3b>(y, x)[0] = temp[x];
				lap2.at<Vec3b>(y, x)[1] = temp[x];
				lap2.at<Vec3b>(y, x)[2] = temp[x];
			}
		}
	}

	Mat FMmask;
	FMmask.create(lap2.rows, lap2.cols, CV_8UC1);
	cvtColor(lap2, FMmask,COLOR_BGR2GRAY);

	vector <Mat> sec;
	int finpixel,cen,pixelno;
	float fm;
	
	cen = (windowsize/2)/windowdel;
	cnt = 0;
	finpixel = cen * windowdel + windowsize / 2;
	while (finpixel <= FMmask.rows)
	{
		sec.push_back(FMmask(Range(cen*windowdel - windowsize / 2, cen*windowdel + windowsize / 2), Range(0, FMmask.cols)));
		FM result = LAP1(sec[cnt]); // F-measure 값과 pixel의 갯수

		fmeas.push_back(Vec2f(result.fm/result.pixelno,cen*windowdel));
		
		
		if(cen>=7 && cen<=9)
		{ 
		Point top_i, top_f, bottom_i, bottom_f;
		top_i.y = cen * windowdel - windowsize / 2;
		top_f.y = cen * windowdel - windowsize / 2;
		top_i.x = 0;
		top_f.x = FMmask.cols;
		bottom_i.y = cen * windowdel + windowsize / 2;
		bottom_f.y = cen * windowdel + windowsize / 2;
		bottom_i.x = 0;
		bottom_f.x = FMmask.cols;
		/*
			if (cen == 8)
			{
				line(src, top_i, top_f, Color::indigo, 2, LINE_AA);
				line(src, bottom_i, bottom_f, Color::indigo, 2, LINE_AA);
				cout << "Highest value FM : " << result.fm << "pixel no : " << result.pixelno << endl;
			}
			else
			{
				line(src, top_i, top_f, Color::coral, 2, LINE_AA);
				line(src, bottom_i, bottom_f, Color::coral, 2, LINE_AA);
				cout << "normal value FM : " << result.fm << "pixel no : " << result.pixelno << endl;

			}
			*/
		}
		
		cnt++;
		cen++;
		finpixel = cen * windowdel + windowsize / 2;
	}


	float threshold;
	threshold = 800;
	vector <Vec2f> focusrange;
	for (i = 0; i < fmeas.size(); i++)
	{
		if (fmeas[i][0] >= threshold)
			focusrange.push_back(fmeas[i]);
	}

	Point top_l, top_r, bottom_l, bottom_r;
	top_l.x = 0;
	top_r.x = FMmask.cols;
	top_l.y = focusrange[0][1] - windowsize / 2;
	top_r.y = focusrange[0][1] - windowsize / 2;
	bottom_l.x = 0;
	bottom_r.x = FMmask.cols;
	bottom_l.y = focusrange[focusrange.size() - 1][1] + windowsize / 2;
	bottom_r.y = focusrange[focusrange.size() - 1][1] + windowsize / 2;
	line(src, top_l, top_r, Scalar(0, 255, 0), 2, LINE_AA);
	line(src, bottom_l, bottom_r, Scalar(0, 255, 0), 2, LINE_AA);


	Mat *FMplot = new Mat;
	(*FMplot).create(src.rows, src.cols, CV_8UC3);
	*FMplot = Scalar::all(0);
	int bin_width = cvRound((double)src.rows / fmeas.size());

	Mat FMval;
	FMval.create(1, fmeas.size(), CV_32FC1);
	for (i = 0; i < fmeas.size(); i++)
	{
		FMval.at<float>(0, i) = fmeas[i][0];
	}
	normalize(FMval, FMval, 0, src.cols, NORM_MINMAX, -1, Mat());
	for (i = 1; i < fmeas.size(); i++)
	{
		line(*FMplot, Point(FMval.at<float>(0,i-1), bin_width*(i - 1)), Point(FMval.at<float>(0,i), bin_width*i), Color::aliceblue, 2, LINE_AA);
	}
	


	//for (i = 0; i < fmeas.size(); i++)
	//	cout << fmeas[i] << endl;

	/*
	ofstream myfile("Fmeas_ver.txt");
	if (myfile.is_open())
	{
		for (i = 0; i < fmeas.size(); i++)
			myfile << (fmeas[i][0]) << " ";
		for (i = 0; i < fmeas.size(); i++)
			myfile << (fmeas[i][1]) << " ";
		myfile.close();
	}
	else cout << "Unable to open file";
	*/
	/*
	sort(fmeas.begin(), fmeas.end(),sortcol2);
	blur_limit = (fmeas[0][0]-fmeas[fmeas.size()-1][0])*0.3 + fmeas[fmeas.size() - 1][0];
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
				y_rt = div * floor(FMval.rows / ver_part);
				y_lt = div * floor(FMval.rows / ver_part);
				y_rb = FMval.rows;
				y_lb = FMval.rows;
			}

			else
			{
				y_rt = div * floor(FMval.rows / ver_part);
				y_lt = div * floor(FMval.rows / ver_part);
				y_rb = (div + 1)* floor(FMval.rows / ver_part);
				y_lb = (div + 1)* floor(FMval.rows / ver_part);
			}

			if (rem == hor_part - 1)
			{
				x_lb = rem * floor(FMval.cols / hor_part);
				x_lt = rem * floor(FMval.cols / hor_part);
				x_rb = FMval.cols;
				x_rt = FMval.cols;
			}

			else
			{
				x_lb = rem * floor(FMval.cols / hor_part);
				x_lt = rem * floor(FMval.cols / hor_part);
				x_rb = (rem + 1) * floor(FMval.cols / hor_part);
				x_rt = (rem + 1) * floor(FMval.cols / hor_part);
			}
			lb.x = x_lb;
			lb.y = y_lb;
			rb.x = x_rb;
			rb.y = y_rb;
			lt.x = x_lt;
			lt.y = y_lt;
			rt.x = x_rt;
			rt.y = y_rt;
			line(lap2, lb, rb, Scalar(0, 0, 255), 2, LINE_AA);
			line(lap2, lb, lt, Scalar(0, 0, 255), 2, LINE_AA);
			line(lap2, lt, rt, Scalar(0, 0, 255), 2, LINE_AA);
			line(lap2, rb, rt, Scalar(0, 0, 255), 2, LINE_AA);

			line(src, lb, rb, Scalar(0, 0, 255), 2, LINE_AA);
			line(src, lb, lt, Scalar(0, 0, 255), 2, LINE_AA);
			line(src, lt, rt, Scalar(0, 0, 255), 2, LINE_AA);
			line(src, rb, rt, Scalar(0, 0, 255), 2, LINE_AA);

		
		}
	}
	*/


	//namedWindow("Laplacian2", WINDOW_KEEPRATIO);
	//imshow("Laplacian2", FMval);

	//namedWindow("Hor", WINDOW_KEEPRATIO);
	//imshow("Hor", hor);

	//namedWindow("Ver", WINDOW_KEEPRATIO);
	//imshow("Ver", ver);
	//namedWindow("Laplacian", WINDOW_KEEPRATIO);
	//imshow("Laplacian", lap);

	//namedWindow("Scharr", WINDOW_KEEPRATIO);
	//imshow("Scharr", scharr);
//namedWindow("Part", WINDOW_KEEPRATIO);
//imshow("Part", src_part);
//imwrite("Chessboard_focusmeasure1.jpg", src_part);
	namedWindow("Source", WINDOW_KEEPRATIO);
	imshow("Source", src);

	namedWindow("FMplot", WINDOW_KEEPRATIO);
	imshow("FMplot", *FMplot);
	waitKey(0);
	return 0;


}
/*

*/

