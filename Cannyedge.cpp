#include "stdafx.h"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

Mat src, src_gray;
Mat dst, detected_edges,mask,Hough_thin;
Mat dx_Scharr,dy_Scharr, scharr;
Mat draw1;
int lowThreshold = 10;
//const int max_lowThreshold = 100;
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
	Scharr(detected_edges, dy_Scharr, CV_32F, 0, 1, 1, 0, BORDER_REPLICATE);   //dx_Scharr, dy_Scharr 형은 CV_32F
	//sqrt(dx.mul(dx) + dy.mul(dy), sobel);
	sqrt(dx_Scharr.mul(dx_Scharr) + dy_Scharr.mul(dy_Scharr), scharr);
		
	double minVal_s, maxVal_s;
	minMaxLoc(scharr, &minVal_s, &maxVal_s);
		
	scharr.convertTo(draw1, CV_8U, 255.0 / (maxVal_s - minVal_s), 0); // draw1 에 Scharr 결과 대입하면서 CV_8U로 형변환
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
	
	
	//namedWindow("dx", WINDOW_KEEPRATIO);
	//imshow("dx", dx_Scharr);

	//namedWindow("dy", WINDOW_KEEPRATIO);
	//imshow("dy", dy_Scharr);

	//Edge Thinner Showing
	//namedWindow("Thin Edge", WINDOW_KEEPRATIO);
	//imshow("Thin Edge", draw1);
}


int main(int argc, char** argv)
{

	Mat res_h, points_h, labels_h, centers_h;  //K-means algorithm variable
	int nPoints, cIndex_h, iTemp_h, k_hor = 1, k_ver = 1;
	Mat res_v, points_v, labels_v, centers_v;
	int cIndex_v, iTemp_v;


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
	
	dst.copyTo(Hough_thin);

	cvtColor(dst, dst, COLOR_BGR2GRAY);
	
	// Edge Image Horizontal Vertical Separation
	Mat hor, ver;
	hor.create(dx_Scharr.size(), CV_8UC1);
	ver.create(dx_Scharr.size(), CV_8UC1);
	hor = Scalar::all(0);
	ver = Scalar::all(0);
	//dx_Scharr = abs(dx_Scharr);
	//dy_Scharr = abs(dy_Scharr);

	double temp;
	for (int y = 0; y <draw1.rows; y++)
		for (int x = 0; x < draw1.cols; x++)
		{
			temp = atan2((double)(dy_Scharr.at<float>(y, x)), (double)(dx_Scharr.at<float>(y, x)));


			if ((temp > -CV_PI / 4 && temp <= CV_PI / 4) || (temp > CV_PI * 3 / 4) || (temp <= -CV_PI * 3 / 4))
				ver.at<uchar>(y, x) = draw1.at<uchar>(y, x);
			else
				hor.at<uchar>(y, x) = draw1.at<uchar>(y, x);
		}

	vector <int> arr;
	vector <int> arr2;
	
	//line 갯수 세기 & K-means clustering의 K 정하기
	int x, y;
	for ( x = 0; x < ver.cols; x++)
	{
		if (ver.at<uchar>(0, x) > 0)
		{
			arr.push_back(x);
		}
	}

	for ( y = 0; y < hor.rows; y++)
	{
		if (hor.at<uchar>(y, 0) > 0)
		{
			arr2.push_back(y);
		}
	}

	
	for ( x = 1; x < arr.size(); x++)
	{
		if ((arr[x] - arr[x-1]) > 3)
			k_ver++;
	}

	for (int y = 1; y < arr2.size(); y++)
	{
		if ((arr2[y] - arr2[y-1]) > 3)
			k_hor++;
	}

	cout << "k_ver:" << k_ver << endl;
	cout << "k_hor: " << k_hor << endl;
	nPoints = hor.rows*hor.cols;

	points_h.create(nPoints, 1, CV_32FC1);
	points_v.create(nPoints, 1, CV_32FC1);
	centers_h.create(k_hor, 1, points_h.type());
	centers_v.create(k_ver, 1, points_v.type());
	res_h.create(hor.rows, hor.cols, hor.type());
	res_v.create(ver.rows, ver.cols, ver.type());
	
	//k-means 함수에 맞게 데이터 변환
	int n;
	for (y = 0,  n = 0; y < hor.rows; y++)
	{
		for (int x = 0; x < hor.cols; x++, n++)
		{
			points_h.at<float>(n) = hor.at<uchar>(y, x);
			points_v.at<float>(n) = ver.at<uchar>(y, x);
		}
	}
	
	
	kmeans(points_h, k_hor, labels_h, TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 10, 1.0), 3,KMEANS_PP_CENTERS,centers_h);
	//kmeans(points_v, k_ver, labels_v, TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 10, 1.0), 3, KMEANS_PP_CENTERS, centers_v);

	for (y = 0, n = 0; y < hor.rows; y++)
	{
		for (x = 0; x < hor.cols; x++, n++)
		{
			cIndex_h = labels_h.at<int>(n);
			if (cIndex_h == 0)
			{
				//cout << centers_h.at<float>(cIndex_h) << endl;
				iTemp_h = cvRound(centers_h.at<float>(cIndex_h));
				iTemp_h = iTemp_h > 255 ? 255 : iTemp_h < 0 ? 0 : iTemp_h;
				res_h.at<uchar>(y, x) = (uchar)iTemp_h;
			}
			//cIndex_v = labels_v.at<int>(n);
			//iTemp_v = cvRound(centers_v.at<float>(cIndex_v));

		}
	}
	
	namedWindow("Cluster",WINDOW_KEEPRATIO);
	imshow("Cluster", res_h);
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
	


	/*  Sobel Edge로 결과 나온것 Binary로 만들고(Hough돌리기위해) Line 그리기 위한 RGB형의 Mat파일 만듬
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
		
		if ((theta_d <= 10 && theta_d >= -10) || (theta_d >= 170) || (theta_d <= -170))
		{
			cout << "각도 : " << theta_d << endl;
			cout << "r : " << rho << endl;
		}
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
	

	namedWindow("Horizontal", WINDOW_KEEPRATIO);
	imshow("Horizontal", hor);
	imwrite("Horizontal line.jpg", hor);

	namedWindow("Vertical", WINDOW_KEEPRATIO);
	imshow("Vertical", ver);
	imwrite("Vertical line.jpg", ver);
	
	
	//namedWindow("Source", WINDOW_KEEPRATIO);
	//namedWindow("Hough_edge", WINDOW_KEEPRATIO);
	//namedWindow("angle", WINDOW_KEEPRATIO);
	//imshow("Source", src);
	//imshow("Hough_edge", Hough_thin);
	
	//imshow("angle", angle);
	waitKey(0);
	return 0;
}




