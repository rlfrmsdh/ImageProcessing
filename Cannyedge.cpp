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
	
	/*
	angle.create(dx.size(), CV_8UC3);
	double temp;
	for (int y = 0; y <angle.rows; y++)
		for (int x = 0; x < angle.cols; x++)
		{
			temp = atan2((double)(dy.at<float>(y, x)), (double)(dx.at<float>(y, x)));
			if ((temp >= -CV_PI / 4 && temp <= CV_PI / 4) || (temp >= CV_PI * 3 / 4 && temp <= -CV_PI * 3 / 4))
				angle.at<Vec3b>(y, x)[0] = temp * 180 / CV_PI;
			else
				angle.at<Vec3b>(y, x)[2] = temp * 180 / CV_PI;
		}
	

	double minValx, maxValx;
	minMaxLoc(dx, &minValx, &maxValx);
	dx.convertTo(dx, CV_8U, 255.0 / (maxValx - minValx), -minValx*255.0 / (maxValx - minValx));
	cout << "Maxval : " << maxValx << "Minval : " << minValx << endl;

	double minValy, maxValy;
	minMaxLoc(dy, &minValy, &maxValy);
	dy.convertTo(dy, CV_8U, 255.0 / (maxValy - minValy), -minValy*255.0 / (maxValy - minValy));
	cout << "Maxval : " << maxValy << "Minval : " << minValy << endl;
	*/
	
	double minVal_s, maxVal_s;
	minMaxLoc(scharr, &minVal_s, &maxVal_s);
		
	scharr.convertTo(draw1, CV_8U, 255.0 / (maxVal_s - minVal_s), 0);
	EdgeThinner(draw1);
	
	/*
	Thinedge.copyTo(dx2_edge);

	Scharr(dx2_edge, dx2, CV_32F, 1, 0, 1, 0, BORDER_REPLICATE);
	cout << "height : " << dx2.rows << "width : " << dx2.cols << endl;
	for (int y = 0; y < dx2.rows; y++)
	{
		float *onerow = dx2.ptr<float>(y);
		uchar *edge_row = dx2_edge.ptr<uchar>(y);
		for (int x = 0; x < dx2.cols; x++)
		{
			if (abs(onerow[x]) > 50)
				edge_row[x] = 0;
				
		}
	}
	*/
	
	

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

/* HoughLinesGrad 함수 짜다가 중지
static void HoughLinesGrad(const Mat &edge, const Mat &dx, const Mat &dy,
	float rho, float theta, int threshold, vector<Vec2f> &lines)
{
	float max_theta = CV_PI/2, min_theta = -CV_PI/2;
	int accm_width = (max_theta - min_theta) / theta +1;
	float rmax = sqrt((pow((double)(edge.rows),2.0) + pow((double)(edge.cols),2.0)));
	int accm_height = cvRound(rmax / rho)+1;
	float dtheta = 10*(CV_PI/180); //(rad) 

	CV_Assert(edge.type() == CV_8UC1);
	Mat accm ;
	Mat image;
	edge.copyTo(image);
	accm.create(accm_height, accm_width, CV_16UC1);
	
	int img_height = edge.rows;
	int img_width = edge.cols;

	accm = Scalar::all(0);

	float r;
	int rnum;
	int angnum;
	for (int y = 0; y < img_height; y++)
	{
		uchar* img_row = image.ptr<uchar>(y);
		for (int x = 0; x < img_width; x++)
		{
			float cen_ang = (float)atan((double)(dy.at<float>(y, x))/ (double)(dx.at<float>(y, x)));
			if (img_row[x] != 0)
			{
				for (float ang = (cen_ang - dtheta); ang < cen_ang + dtheta; ang += theta)
				{
					r = (float)(x * cos(ang) + y * sin(ang));
					if ((r / rho - (int)(r / rho)) != 0)
						rnum = cvRound(r / rho);
					else
						rnum = r / rho;
					
					if ((ang / theta - (int)(ang / theta)) != 0)
						angnum = cvRound(ang / theta);
					else
						angnum = ang / theta;
					
					if (angnum >= 0)
						angnum = angnum + (accm_width - 1) / 2;
					else
						angnum = (accm_width - 1) / 2 + angnum;
					
					//cout << "x :" << x << "y : " << y << endl;
					if (rnum < 0)
					{
						cout << "rnum: " << rnum << "angnum : " << angnum << endl;
						cout << "x :" << x << "y: " << y << endl;
						cout << "cen_angle : " << cen_ang <<" angle :"<<ang<< endl;
						cout << endl;
					}
					//accm.at<ushort>(rnum, angnum) += 1;
					
				}
				
			}
		}

	}

	imshow("Accumulator",accm);

}
*/



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

	/*
	// Probabilistic Line Transform
	vector<Vec4i> linesP; // will hold the results of the detection
	HoughLinesP(dst, linesP, 1, CV_PI / 180, 50, 100, 10); // runs the actual detection
														  // Draw the lines
	int cntP = 0;
	for (size_t i = 0; i < linesP.size(); i++)
	{
		Vec4i l = linesP[i];
		line(cdstP, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 3, LINE_AA);
		double thetaP = atan2(l[3] - l[1], l[2] - l[0]);
		double thetaP_d = thetaP * 180 / CV_PI;
		if (thetaP_d == 0 || thetaP_d == -90)
			cntP++;
		
		cout << "각도 : " << thetaP_d << endl;
	}
	cout << "평행or 수직 : " << cntP << endl;
	cout << "전체 갯수 : " << linesP.size() << endl;


	//cout << "90도 : " << cnt_in << endl;
	//cout << "90도이외 : " << cnt_out << endl;

	
	namedWindow("Detected Lines (in red) - Probabilistic Line Transform", WINDOW_KEEPRATIO);
	
	imshow("Detected Lines (in red) - Probabilistic Line Transform", cdstP);
	*/


	//namedWindow("Detected Lines (in red) - Standard Hough Line Transform", WINDOW_KEEPRATIO);
	//imshow("Detected Lines (in red) - Standard Hough Line Transform", cdst);

	namedWindow("Source", WINDOW_KEEPRATIO);
	namedWindow("Hough_edge", WINDOW_KEEPRATIO);
	//namedWindow("angle", WINDOW_KEEPRATIO);
	imshow("Source", src);
	imshow("Hough_edge", Hough_thin);
	//imshow("angle", angle);
	waitKey(0);
	return 0;
}




