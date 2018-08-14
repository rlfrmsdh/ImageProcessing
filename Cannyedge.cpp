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
			if (row[x] > 25)
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

}
bool sortcol(const vector<float> &a, const vector<float> &b)
{
	return a[0] < b[0];
}

bool sortcol2(const vector<float> &a, const vector<float> &b)
{
	return a[1] > b[1];
}


int main(int argc, char** argv)
{
	int height, width;
	int testpoint = 0;
	int thick_blur = 30;  //blur해지면 해질수록 이 값을 키워서 line fitting 할 영역을 정해야함

	CommandLineParser parser(argc, argv, "{@input | Chessboard_tilt.jpg | input image}");
	src = imread(parser.get<String>("@input"), IMREAD_COLOR); // Load an image 
	if (src.empty())
	{
		std::cout << "Could not open or find the image!\n" << std::endl;
		std::cout << "Usage: " << argv[0] << " <Input image>" << std::endl;
		return -1;
	}

	height = src.rows;
	width = src.cols;
	dst.create(src.size(), src.type());
	cvtColor(src, src_gray, COLOR_BGR2GRAY);

	CannyThreshold(0, 0);

	dst.copyTo(Hough_thin);

	cvtColor(dst, dst, COLOR_BGR2GRAY);


	// Edge Image Horizontal Vertical Separation
	///////////////////////////////////////////////////////////////////

	Mat hor, ver;
	hor.create(dx_Scharr.size(), CV_8UC3);
	ver.create(dx_Scharr.size(), CV_8UC3);
	hor = Scalar::all(0);
	ver = Scalar::all(0);
	//dx_Scharr = abs(dx_Scharr);
	//dy_Scharr = abs(dy_Scharr);
	double temp;
	for (int y = 0; y < draw1.rows; y++)
		for (int x = 0; x < draw1.cols; x++)
		{
			temp = atan2((double)(dy_Scharr.at<float>(y, x)), (double)(dx_Scharr.at<float>(y, x)));
			temp = temp * 180 / CV_PI;
			if (temp >= 90 && temp <= 180)
				temp = temp - 180;
			if (temp <= -90 && temp >= -180)
				temp = temp + 180;

			if ((temp > -45 && temp <= 45))
			{
				ver.at<Vec3b>(y, x)[0] = draw1.at<uchar>(y, x);
				ver.at<Vec3b>(y, x)[1] = draw1.at<uchar>(y, x);
				ver.at<Vec3b>(y, x)[2] = draw1.at<uchar>(y, x);
			}
			else
			{
				hor.at<Vec3b>(y, x)[0] = draw1.at<uchar>(y, x);
				hor.at<Vec3b>(y, x)[1] = draw1.at<uchar>(y, x);
				hor.at<Vec3b>(y, x)[2] = draw1.at<uchar>(y, x);
			}
		}


	/////////////////////////////////////////////////////////////////////end

	//Hough Transfer
	//////////////////////////////////////////////////////////////////
	vector <Vec2f> lines;
	vector <vector<float>> line_cpy;
	HoughLines(dst, lines, 1, CV_PI / 180, 250, 0, 0);
	for (int i = 0; i < lines.size(); i++)
	{
		vector<float> row;
		row.push_back(lines[i][0]);
		row.push_back(lines[i][1]);
		line_cpy.push_back(row);
	}
	sort(line_cpy.begin(), line_cpy.end(), sortcol2);
	//////////////////////////////////////////////////////////////


	//line 갯수 세기 & vector 채워넣기
	//////////////////////////////////////////////////////////////////////////////////

	vector <vector <float> > arr_v;
	vector <vector <float> > arr_h;
	//arr 과 arr2에 저장순서 : [pixel위치(x or y), 몇번째 line인지]

	int x, y;
	int i = 1, j = 1, cnt = 0;
	for (x = 0; x < ver.cols; x++)
	{
		if (ver.at<Vec3b>(testpoint, x)[0] > 0 || ver.at<Vec3b>(testpoint, x)[1] > 0 || ver.at<Vec3b>(testpoint, x)[2] > 0)
		{
			vector <float> row;
			row.push_back(x);
			arr_v.push_back(row);
		}

	}
	for (y = 0; y < hor.rows; y++)
	{
		if (hor.at<Vec3b>(y, testpoint)[0] > 0 || hor.at<Vec3b>(y, testpoint)[1] > 0 || hor.at<Vec3b>(y, testpoint)[2] > 0)
		{

			vector <float> row;
			row.push_back(y);
			arr_h.push_back(row);
		}

	}
	i = 1, j = 1;
	for (x = 1; x < arr_v.size(); x++)
	{
		arr_v[x - 1].push_back(i);
		if ((arr_v[x][0] - arr_v[x - 1][0]) > 200)
		{
			i++;
		}
	}
	arr_v[arr_v.size() - 1].push_back(i);
	for (int y = 1; y < arr_h.size(); y++)
	{
		arr_h[y - 1].push_back(j);
		if ((arr_h[y][0] - arr_h[y - 1][0]) > 200)
		{
			j++;
		}
	}
	arr_h[arr_h.size() - 1].push_back(j);

	//Vector Printing
	
	/*
	cout << "arr_v 출력 : " << endl;
	for (int y = 0; y < arr_v.size(); y++)
	{
		for (int x = 0; x < arr_v[y].size(); x++)
			cout << arr_v[y][x] << " ";
		cout << endl;
	}
	*/
	/*
	cout << "arr2_h 출력 : " << endl;
	for (int y = 0; y < arr_h.size(); y++)
	{
		for (int x = 0; x < arr_h[y].size(); x++)
			cout << arr_h[y][x] << " ";
		cout << endl;
	}*/
	//////////////////////////////////////////////////////////////////////////////////

	// Horizontal line과 Vertical line의 각도 뽑아내기 (by. Hough)
	///////////////////////////////////////////////////////////////////////////////
	vector <float> theta_sort_v;
	vector <float> theta_sort_h;
	for (i = 0; i < line_cpy.size(); i++)
	{
		if (line_cpy[i][1] < CV_PI / 4 || line_cpy[i][1]> CV_PI * 3 / 4) // Ver
		{ 
			line_cpy[i][1] = line_cpy[i][0] < 0 ? line_cpy[i][1] - CV_PI : line_cpy[i][1];
			theta_sort_v.push_back(line_cpy[i][1]);
		}
		else
			theta_sort_h.push_back(line_cpy[i][1]);
	}
	sort(theta_sort_h.begin(), theta_sort_h.end()); // 오름차순
	sort(theta_sort_v.begin(), theta_sort_v.end());


	vector <float> Horizontal_theta;
	vector <float> Vertical_theta;
	
	Horizontal_theta.push_back(theta_sort_h[0] - CV_PI/180); //최소값-1
	Horizontal_theta.push_back(theta_sort_h[theta_sort_h.size() - 1] + CV_PI/180); //최댓값+1
	cout << "Horizontal theta : " << Horizontal_theta[0] * 180 / CV_PI <<"," << Horizontal_theta[1] * 180 / CV_PI << endl;

	Vertical_theta.push_back(theta_sort_v[0] - CV_PI/180);
	Vertical_theta.push_back(theta_sort_v[theta_sort_v.size() - 1] + CV_PI/180);
	cout << "Vertical theta : " << Vertical_theta[0] * 180 / CV_PI << "," << Vertical_theta[1] * 180 / CV_PI << endl;
	/////////////////////////////////////////////////////////////////////////////////////////


	// Line들의 시작, 끝 포인트 뽑고. 위에서 구한 각도를 통해 영역 계산  영역별 pixel position 뽑아내기.   
	////////////////////////////////////////////////////////////////////////////////////start
	vector <float> point_v;
	vector <float> point_h;
	vector <Vec2f> line_plot_v;
	vector <Vec2f> line_plot_h;

	int size_v, size_h;
	size_v = arr_v.size();
	size_h = arr_h.size();

	//Vertical line 을 위한 pt
	point_v.push_back(arr_v[0][0] < 15 ? arr_v[0][0] : arr_v[0][0] - 15);
	for (i = 1; i < size_v; i++)
	{

		if (arr_v[i][0] - arr_v[i - 1][0] > 100)
		{
			point_v.push_back(arr_v[i - 1][0]+15);
			point_v.push_back(arr_v[i][0]-15);
		}
	}
	point_v.push_back(arr_v[size_v - 1][0] < width - 15 ? arr_v[size_v - 1][0] + 15 : arr_v[size_v - 1][0]);

	//Horizontal line을 위한 pt
	point_h.push_back(arr_h[0][0] < 15 ? arr_h[0][0] : arr_h[0][0] - 15);
	for (i = 1; i < size_h; i++)
	{
		if (arr_h[i][0] - arr_h[i - 1][0] > 100)
		{
			point_h.push_back(arr_h[i - 1][0]+15);
			point_h.push_back(arr_h[i][0]-15);
		}
	}
	point_h.push_back(arr_h[size_h - 1][0] < height - 15 ? arr_h[size_h - 1][0] + 15 : arr_h[size_h - 1][0]);



	//영역별 계산
	char dir[10];
	int linenum;

	strcpy_s(dir, "Hor");
	linenum = 3;

	Mat A, B, X;
	Mat chosenline;
	vector <Vec2f> vec_A;
	vector <float> vec_B;
	chosenline.create(height, width, CV_8UC1);
	chosenline = Scalar::all(0);
	
	
	if (!strcmp(dir,"Hor"))   //Horizontal line Line fitting
	{
		float r1, r2;
		r1 = sin(Horizontal_theta[0])*point_h[2 * (linenum - 1)]; // cos(theta)*x + sin(theta)*y = r 그러나 Horizontal line에서는 x=0
		r2 = sin(Horizontal_theta[1])*point_h[2 * (linenum - 1) + 1]; /// Horizontal 에서는 작은 각도가 영역의 시작 모서리, 큰 각도가 끝 모서리


		//Section 표시
		/*
		Point  pt1, pt2;
		double a = cos(Horizontal_theta[0]), b = sin(Horizontal_theta[0]);
		double x0 = a * sin(Horizontal_theta[0])*point_h[2 * (linenum - 1)], y0 = b * sin(Horizontal_theta[0])*point_h[2 * (linenum - 1)];
		pt1.x = cvRound(x0 + 2500 * (-b));
		pt1.y = cvRound(y0 + 2000 * a);
		pt2.x = cvRound(x0 - 2500 * (-b));
		pt2.y = cvRound(y0 - 2000 * a);
		line(hor, pt1, pt2, Scalar(0, 0, 255), 2, LINE_AA);

		a = cos(Horizontal_theta[1]), b = sin(Horizontal_theta[1]);
		x0 = a * sin(Horizontal_theta[1])*point_h[2 * (linenum - 1)+1], y0 = b * sin(Horizontal_theta[1])*point_h[2 * (linenum - 1)+1];
		pt1.x = cvRound(x0 + 2500 * (-b));
		pt1.y = cvRound(y0 + 2000 * a);
		pt2.x = cvRound(x0 - 2500 * (-b));
		pt2.y = cvRound(y0 - 2000 * a);
		line(hor, pt1, pt2, Scalar(0, 0, 255), 2, LINE_AA);
		*/

		
		for (x = 0; x < width; x++)
			{
			int y1, y2;
				y1 = (-cos(Horizontal_theta[0]) / sin(Horizontal_theta[0]))*x + r1 / sin(Horizontal_theta[0]);
				y2 = (-cos(Horizontal_theta[1]) / sin(Horizontal_theta[1]))*x + r2 / sin(Horizontal_theta[1]);
				//cout << "y1 :" << y1 << "y2 : " << y2 << endl;
				y1 = y1 < 0 ? 0 : y1;
				y1 = y1 > height ? height : y1;
				y2 = y2 < 0 ? 0 : y2;
				y2 = y2 > height ? height : y2;
				if(y2<=height)
				{ 
					for (y = y1; y < y2; y++)
					{
						if (hor.at<Vec3b>(y, x)[0]!= 0)
						{
							chosenline.at<uchar>(y, x) = hor.at<Vec3b>(y, x)[0];
							vec_A.push_back(Vec2f(x, 1));
							vec_B.push_back(y);
						}
					}
				}
			}
	
	cout << "Vec A size:" << vec_A.size() << endl;
	cout << "Vec B size: " << vec_B.size() << endl;
	
	
	//create Mat
	A.create(vec_A.size(), 2,CV_32FC1);
	B.create(vec_B.size(), 1, CV_32FC1);
	//copy vector to mat
	memcpy(A.data, vec_A.data(), vec_A.size() * sizeof(float)*2);
	memcpy(B.data, vec_B.data(), vec_B.size() * sizeof(float));

	X = A.inv(DECOMP_SVD)*B;

	cout << X.size() << endl;
	cout << X << endl; /// X.at<float>(0,0)= a  X.at<float>(1,0) = b

	float a, b;
	a = X.at<float>(0, 0);
	b = X.at<float>(1, 0);

	Point pt1, pt2;
	pt1.x = 0;
	pt2.x = width;
	pt1.y = a * pt1.x + b;
	pt2.y = a * pt2.x + b;
	line(src, pt1, pt2, Scalar(0, 0, 255), 2, LINE_AA);

	namedWindow("Chosen line", WINDOW_KEEPRATIO);
	imshow("Chosen line", chosenline);

	namedWindow("Line Fitting Result",WINDOW_KEEPRATIO);
	imshow("Line Fitting Result",src);

	//cout << "A size : " << A.cols << endl;
	//cout << "B size : " << B.size() << endl;

	Mat differ;
	differ.create(B.rows, 1, CV_32FC1);
	differ = A*X - B;

	vector <float> diff;
	diff.assign((float*)differ.datastart, (float*)differ.dataend);



	ofstream myfile("Line Fitting Result.txt");
	if (myfile.is_open())
	{
	for (int count = 0; count < diff.size(); count++)
	myfile << diff[count] << " ";
	myfile.close();
	}
	else cout << "Unable to open file";
	
	}
	
	else        //Vertical Line Fitting
	{
		
		float r1, r2;
		r1 = cos(Vertical_theta[1])*point_v[2 * (linenum - 1)]; // cos(theta)*x + sin(theta)*y = r 그러나 Vertical line에서는 y=0
		r2 = cos(Vertical_theta[0])*point_v[2 * (linenum - 1) + 1];  // Vertical 에서는 큰 각도가 영역의 시작 모서리, 작은 각도가 영역의 끝 모서리

		//Section 표시
		
		/*
		Point  pt1, pt2;
		double a = cos(Vertical_theta[0]), b = sin(Vertical_theta[0]);
		double x0 = a * r2, y0 = b * r2;
		pt1.x = cvRound(x0 + 2500 * (-b));
		pt1.y = cvRound(y0 + 2000 * a);
		pt2.x = cvRound(x0 - 2500 * (-b));
		pt2.y = cvRound(y0 - 2000 * a);
		line(ver, pt1, pt2, Scalar(0, 0, 255), 2, LINE_AA);

		a = cos(Vertical_theta[1]), b = sin(Vertical_theta[1]);
		x0 = a * r1, y0 = b * r1;
		pt1.x = cvRound(x0 + 2500 * (-b));
		pt1.y = cvRound(y0 + 2000 * a);
		pt2.x = cvRound(x0 - 2500 * (-b));
		pt2.y = cvRound(y0 - 2000 * a);
		line(ver, pt1, pt2, Scalar(0, 0, 255), 2, LINE_AA);
		*/
		
		for (y = 0; y < height; y++)
		{
			int x1, x2;
			x2 = (-sin(Vertical_theta[0]) / cos(Vertical_theta[0]))*y + r2 / cos(Vertical_theta[0]);
			x1 = (-sin(Vertical_theta[1]) / cos(Vertical_theta[1]))*y + r1 / cos(Vertical_theta[1]);
			x1 = x1 < 0 ? 0 : x1;
			x1 = x1 > width ? width : x1;
			x2 = x2 < 0 ? 0 : x2;
			x2 = x2 > width ? width : x2;
			//cout << "x1 :" << x1 << "x2 : " << x2 << endl;
				for (x = x1; x < x2; x++)
				{
					if (ver.at<Vec3b>(y, x)[0] != 0)
					{
						chosenline.at<uchar>(y, x) = ver.at<Vec3b>(y, x)[0];
						vec_A.push_back(Vec2f(y, 1));
						vec_B.push_back(x);
					}
				}
			
			

		}

		cout << "Vec A size:" << vec_A.size() << endl;
		cout << "Vec B size: " << vec_B.size() << endl;

		
		//create Mat
		A.create(vec_A.size(), 2, CV_32FC1);
		B.create(vec_B.size(), 1, CV_32FC1);
		//copy vector to mat
		memcpy(A.data, vec_A.data(), vec_A.size() * sizeof(float) * 2);
		memcpy(B.data, vec_B.data(), vec_B.size() * sizeof(float));

		X = A.inv(DECOMP_SVD)*B;

		cout << X.size() << endl;
		cout << X << endl; /// X.at<float>(0,0)= a  X.at<float>(1,0) = b

		float a, b;
		a = X.at<float>(0, 0);
		b = X.at<float>(1, 0);

		Point pt1, pt2;
		pt1.y = 0;
		pt2.y = height;
		pt1.x = a * pt1.y + b;
		pt2.x = a * pt2.y + b;
		line(src, pt1, pt2, Scalar(0, 0, 255), 2, LINE_AA);

		namedWindow("Chosen line", WINDOW_KEEPRATIO);
		imshow("Chosen line", chosenline);

		namedWindow("Line Fitting Result", WINDOW_KEEPRATIO);
		imshow("Line Fitting Result", src);

		//cout << "A size : " << A.cols << endl;
		//cout << "B size : " << B.size() << endl;

		Mat differ;
		differ.create(B.rows, 1, CV_32FC1);
		differ = A * X - B;

		vector <float> diff;
		diff.assign((float*)differ.datastart, (float*)differ.dataend);



		ofstream myfile("Line Fitting Result.txt");
		if (myfile.is_open())
		{
			for (int count = 0; count < diff.size(); count++)
				myfile << diff[count] << " ";
			myfile.close();
		}
		else cout << "Unable to open file";
		
	}

	//namedWindow("Horizontal", WINDOW_KEEPRATIO);
	//imshow("Horizontal", hor);
//	imwrite("Horizontal Section.jpg", hor);

	//namedWindow("Vertical", WINDOW_KEEPRATIO);
	//imshow("Vertical", ver);
	//imwrite("Vertical Section.jpg", ver);


	waitKey(0);
	return 0;
}




