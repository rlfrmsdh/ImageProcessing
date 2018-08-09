#include "stdafx.h"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

Mat src, src_gray;
Mat dst, detected_edges,mask,Hough_thin,Hough;
Mat dx_Scharr,dy_Scharr, scharr,dx2;
Mat draw1;
Mat dx2_edge;
int lowThreshold = 10;
const int max_lowThreshold = 100;
const int ratio = 3;
const int kernel_size = 3;
const char* window_name = "Edge Map";

static void Imgbin( Mat &input )
{
	Mat white;
	white.create(input.size(), CV_8UC1);
	white = Scalar::all(255);
	white.copyTo(input, input);
}

static void EdgeThinner(Mat &input)
{
	Mat edge;
	edge.create(input.size(), CV_8UC1);
	edge = Scalar::all(0);
	int img_height = input.rows;
	int img_width = input.cols;
	//Thinedge.create(edge.size(), CV_8UC1);
	//Thinedge = Scalar::all(0);

	for (int y = 0; y < img_height; y++)
	{
		uchar *row = input.ptr<uchar>(y);
		for (int x = 0; x < img_width; x++)
		{
			if (row[x] > 20)
				edge.at<uchar>(y, x) = row[x] ;
		}
	}
	edge.copyTo(input);


}
static void CannyThreshold(int, void*)
{
	blur(src_gray, detected_edges, Size(3, 3));
	Scharr(detected_edges, dx_Scharr, CV_32F, 1, 0, 1, 0, BORDER_REPLICATE);
	Scharr(detected_edges, dy_Scharr, CV_32F, 0, 1, 1, 0, BORDER_REPLICATE);   //dx_Scharr, dy_Scharr ���� CV_32F
	//sqrt(dx.mul(dx) + dy.mul(dy), sobel);
	sqrt(dx_Scharr.mul(dx_Scharr) + dy_Scharr.mul(dy_Scharr), scharr);
		
	double minVal_s, maxVal_s;
	minMaxLoc(scharr, &minVal_s, &maxVal_s);
		
	scharr.convertTo(draw1, CV_8U, 255.0 / (maxVal_s - minVal_s), 0); // draw1 �� Scharr ��� �����ϸ鼭 CV_8U�� ����ȯ
	EdgeThinner(draw1);
	
	Mat hor, ver;
	hor.create(dx_Scharr.size(), CV_8UC1);
	ver.create(dx_Scharr.size(), CV_8UC1);
	
	//namedWindow("dx", WINDOW_KEEPRATIO);
	//imshow("dx", dx_Scharr);
	//imwrite("dx_Scharr_FocusRange.jpg", dx_Scharr);
	//namedWindow("dy", WINDOW_KEEPRATIO);
	//imshow("dy", dy_Scharr);

	dx_Scharr = abs(dx_Scharr);
	dy_Scharr = abs(dy_Scharr);
	double maxVal_x, minVal_x;
	minMaxLoc(dx_Scharr, &minVal_x, &maxVal_x);
	cout << minVal_x << " " << maxVal_x << endl;
	double maxVal_y, minVal_y;
	minMaxLoc(dy_Scharr, &minVal_y, &maxVal_y);
	cout << minVal_y << " " << maxVal_y << endl;

	hor = Scalar::all(0);
	ver = Scalar::all(0);
	
	dx_Scharr.convertTo(ver, CV_8U, 255.0 / (maxVal_x - minVal_x), 0.0 );
	dy_Scharr.convertTo(hor, CV_8U, 255.0 / (maxVal_y - minVal_y), 0.0 ); 

	EdgeThinner(hor);
	EdgeThinner(ver);
	

	Canny(detected_edges, mask, lowThreshold, lowThreshold*ratio, kernel_size);
	dst = Scalar::all(0);
	src.copyTo(dst, mask);
	
	//Canny Edge Showing
	//namedWindow(window_name, WINDOW_KEEPRATIO);
	//imshow(window_name, dst);
	
	//Sobel Result Showing
	//namedWindow("Scharr grad", WINDOW_KEEPRATIO);
	//imshow("Scharr grad", draw1);
	
	//Edge Thinner Showing
	namedWindow("Thin Edge", WINDOW_KEEPRATIO);
	imshow("Thin Edge", draw1);

	namedWindow("Horizontal Edge", WINDOW_KEEPRATIO);
	imshow("Horizontal Edge", hor);

	namedWindow("Vertical Edge", WINDOW_KEEPRATIO);
	imshow("Vertical Edge", ver);

	
	//2nd Derivative �̿��ؼ� �� ��� ������.
	//namedWindow("2nd Edge", WINDOW_KEEPRATIO);
	//imshow("2nd Edge", dx2_edge);

	
	
}


int main(int argc, char** argv)
{
	
	CommandLineParser parser(argc, argv, "{@input | Chessboard.jpg | input image}");
	src = imread(parser.get<String>("@input"), IMREAD_COLOR); // Load an image 
	if (src.empty())
	{
		std::cout << "Could not open or find the image!\n" << std::endl;
		std::cout << "Usage: " << argv[0] << " <Input image>" << std::endl;
		return -1;
	}

	dst.create(src.size(), src.type());
	cvtColor(src, src_gray, COLOR_BGR2GRAY);

	CannyThreshold(0, 0);
	cvtColor(dst, dst, COLOR_BGR2GRAY);
	

	// mat���� Array�� ��ȯ�ؼ� �����ϴ� �κ�
	
	
	/*
	Mat save;
	dx2.copyTo(save);
	vector<int> array;
	for (int i = 0; i < save.rows; i++)
	{
		array.insert(array.end(), save.ptr<float>(i), save.ptr<float>(i) + save.cols);
	}
	cout << "row : " << save.rows << "cols : " << save.cols << endl;
	
	cout << array.size() << endl;
	
	ofstream myfile("2nd deriv.txt");
	if (myfile.is_open())
	{
		//myfile << "Writing this to a file \n";
		
		for (int count = 0; count < array.size(); count++)
			myfile << array[count] << " ";
		
		//myfile << "\n End of file \n";
		
		myfile.close();
	}
	else cout << "Unable to open file";
	//
	*/
	
	/*
	Imgbin(Thinedge);
	Hough_thin.create(Thinedge.size(), CV_8UC3);
	for (int y = 0; y < Thinedge.rows; y++)
	{
		uchar * temp = Hough_thin.ptr<uchar>(y);
		uchar * temp1 = dst.ptr<uchar>(y);
		for (int x = 0; x < dst.cols; x++)
		{
			temp[3 * x + 0] = temp1[x];
			temp[3 * x + 1] = temp1[x];
			temp[3 * x + 2] = temp1[x];
		}
	}
	*/
	/*
	vector <Vec2f> lines;
	HoughLines(dst, lines, 1, CV_PI / 180, 250, 0, 0);
	//int cnt_out=0, cnt_in=0;
	for (int i = 0; i < lines.size(); i++)
	{
		float rho = lines[i][0], theta = lines[i][1], theta_d = (lines[i][1]) * 180 / CV_PI;
		//if (theta_d >= 85 && theta_d <= 95)
		//{
		//if (theta_d != 90)
		//	cnt_out++;
		//else
		//	cnt_in++;
		cout << "���� : " << theta_d << endl;
		cout << "r : " << rho << endl;
		Point  pt1, pt2;
		double a = cos(theta), b = sin(theta);
		double x0 = a * rho, y0 = b * rho;
		pt1.x = cvRound(x0 + 2500 * (-b));
		pt1.y = cvRound(y0 + 2000 * a);
		pt2.x = cvRound(x0 - 2500 * (-b));
		pt2.y = cvRound(y0 - 2000 * a);
		line(Hough_thin, pt1, pt2, Scalar(0, 0, 255), 2, LINE_AA);
		//}
	}
	*/
	/*
	bool compare(Vec2f &line1, Vec2f &line2)
	{
		line1
	}
	sort(lines[][0].begin(), lines[][0].end());
	*/
	

	/*
	namedWindow("Source", WINDOW_KEEPRATIO);
	namedWindow("Hough_edge", WINDOW_KEEPRATIO);
	//namedWindow("angle", WINDOW_KEEPRATIO);
	imshow("Source", src);
	imshow("Hough_edge", Hough_thin);
	*/
	//imshow("angle", angle);
	waitKey(0);
	return 0;
}




