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
Mat Thinedge, dx2_edge;
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

static void EdgeThinner(const Mat &input)
{
	Mat edge;
	input.copyTo(edge);
	int img_height = edge.rows;
	int img_width = edge.cols;
	Thinedge.create(edge.size(), CV_8UC1);
	Thinedge = Scalar::all(0);

	for (int y = 0; y < img_height; y++)
	{
		uchar *row = edge.ptr<uchar>(y);
		for (int x = 0; x < img_width; x++)
		{
			if (row[x] > 50)
				Thinedge.at<uchar>(y, x) = row[x] ;
		}
	}


}
static void CannyThreshold(int, void*)
{
	blur(src_gray, detected_edges, Size(3, 3));
	Scharr(detected_edges, dx_Scharr, CV_32F, 1, 0, 1, 0, BORDER_REPLICATE);
	Scharr(detected_edges, dy_Scharr, CV_32F, 0, 1, 1, 0, BORDER_REPLICATE);
	//sqrt(dx.mul(dx) + dy.mul(dy), sobel);
	sqrt(dx_Scharr.mul(dx_Scharr) + dy_Scharr.mul(dy_Scharr), scharr);
		
	double minVal_s, maxVal_s;
	minMaxLoc(scharr, &minVal_s, &maxVal_s);
		
	scharr.convertTo(draw1, CV_8U, 255.0 / (maxVal_s - minVal_s), 0);
	EdgeThinner(draw1);
	

	

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

	//2nd Derivative 이용해서 더 얇게 햇을때.
	namedWindow("2nd Edge", WINDOW_KEEPRATIO);
	imshow("2nd Edge", dx2_edge);

	
	
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
	

	// mat파일 Array로 변환해서 저장하는 부분
	
	
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
		cout << "각도 : " << theta_d << endl;
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
	imwrite("Houghline_byCanny.jpg", Hough_thin);


	namedWindow("Source", WINDOW_KEEPRATIO);
	namedWindow("Hough_edge", WINDOW_KEEPRATIO);
	//namedWindow("angle", WINDOW_KEEPRATIO);
	imshow("Source", src);
	imshow("Hough_edge", Hough_thin);
	//imshow("angle", angle);
	waitKey(0);
	return 0;
}




