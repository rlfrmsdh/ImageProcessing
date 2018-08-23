#include "stdafx.h"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;
Mat src,src_gray,src_part,lap,lap2,scharr,dx,dy;
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

	return fm ;
}
bool sortcol2(vector<float> &a, vector <float> &b)
{
	return a[0] > b[0];
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
static void Imgbin(Mat &input)
{
	Mat white;
	white.create(input.size(), CV_8UC1);
	white = Scalar::all(255);
	white.copyTo(input, input);
}

int main(void)
{
	int hor_part = 4;
	int ver_part = 6;
	float blur_limit;
	int i, j, cnt;
	src = imread("Chessboard_focusmeasure2.jpg", IMREAD_COLOR); // Load an image 
	//src = src(Range(0,src.rows), Range(0,src.cols-100));
	cout << src.rows << endl;
	//src.convertTo(src_gray, CV_8UC1, 1.0, 0.0);  // 이렇게 하면 Channel은 안바뀜!! 형변환만 일어남!
	cvtColor(src,src_gray,COLOR_BGR2GRAY);
	lap.create(src.rows, src.cols, CV_16SC1);
	lap2.create(src.rows, src.cols, CV_8UC3);
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
	
	

	//src_part.create(900, 850, CV_8UC1);
	//src_part = Scalar::all(0);
	//src_part = src_gray(Range(0, 1600), Range(300, 1700));     // 앞에가 Row, 뒤에가 Col range
		
	Laplacian(src_gray, lap, CV_16S, 3, 1.0, 0.0, BORDER_DEFAULT);
	//lap = abs(lap);
	//lap.convertTo(lap, CV_8U, 1.0, 0.0);
	convertScaleAbs(lap, lap, 1.0, 0.0); // Scaling - Taking an absolute value - 8-bit conversion

	lap2 = Scalar::all(0);
	
	for (int y = 0; y < lap.rows; y++)
	{
		uchar *temp = lap.ptr<uchar>(y);
		uchar *temp1 = scharr.ptr<uchar>(y);
		for (int x = 0; x < lap.cols; x++)
		{
			if (temp1[x] > 0)
			{
				lap2.at<Vec3b>(y, x)[0] = temp[x];
				lap2.at<Vec3b>(y, x)[1] = temp[x];
				lap2.at<Vec3b>(y, x)[2] = temp[x];
			}
		}
	}


	
	

	//namedWindow("Laplace signedint",WINDOW_FREERATIO);
	//imshow("Laplace signedint", lap);

	//lap.convertTo(lap, CV_8U, 1, 0);    // convertTo는 타입만 바꿔주지 Channel의 갯수는 input의 채널 갯수와 같음
	
	/*
	vector <short> diff;
	diff.assign((short*)lap.datastart, (short*)lap.dataend);
	cout << diff.size() << endl;
	ofstream myfile("Laplacian.txt");
	if (myfile.is_open())
	{
		for (int count = 0; count < diff.size(); count++)
			myfile << (diff[count]) << " ";
		myfile.close();
	}
	else cout << "Unable to open file";
	*/

	Mat FMval;
	FMval.create(lap2.rows, lap2.cols, CV_8UC1);
	cvtColor(lap2, FMval,COLOR_BGR2GRAY);
	//lap.copyTo(FMval);
	vector <Mat> sec;
	for (i = 0,cnt=0; i < ver_part; i++)
	{
		for (j = 0; j < hor_part; j++,cnt++)
		{
			vector <float> row;
			if (i == ver_part-1 && j != hor_part-1)
				sec.push_back(FMval(Range(i*floor(FMval.rows / ver_part), FMval.rows), Range(j*floor(FMval.cols / hor_part), (j + 1)*floor(FMval.cols / hor_part) - 1)));

			else if (i != ver_part-1 && j == hor_part-1)
			{
				sec.push_back(FMval(Range(i*floor(FMval.rows / ver_part), (i + 1)*floor(FMval.rows / ver_part) - 1), Range(j*floor(FMval.cols / hor_part), FMval.cols)));
			}

			else if (i != ver_part-1 && j != hor_part-1)
			{
				sec.push_back(FMval(Range(i*floor(FMval.rows / ver_part), (i + 1)*floor(FMval.rows / ver_part) - 1), Range(j*floor(FMval.cols / hor_part), (j + 1)*floor(FMval.cols / hor_part) - 1)));
			}

			else
				sec.push_back(FMval(Range(i*floor(FMval.rows / ver_part), FMval.rows), Range(j*floor(FMval.cols / hor_part), FMval.cols)));
			row.push_back(LAP1(sec[cnt]));
			row.push_back(cnt);
			fmeas.push_back(row);
			
		}
	}

	Ptr<LineSegmentDetector> ls = createLineSegmentDetector(LSD_REFINE_NONE);
	vector <Vec4f> lines_std;
	vector <Vec4f> hor_lines;
	vector <Vec4f> ver_lines;
	Mat ldm=FMval(Range((1)*floor(FMval.rows / ver_part) , (2)*floor(FMval.rows / ver_part) - 1), Range(3 * floor(FMval.cols / hor_part), FMval.cols));
	//cout << ldm.type() << endl;
	ls->detect(ldm, lines_std);
	for (int i = 0; i < lines_std.size(); i++)
	{
		if (abs(lines_std[i][0] - lines_std[i][2]) > abs(lines_std[i][1] - lines_std[i][3]))
			hor_lines.push_back(lines_std[i]);
		else
			ver_lines.push_back(lines_std[i]);
	}
	
	ls->drawSegments(ldm, hor_lines);
	




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

			/*
			if(i==0)
				{
				for (int y = y_lt; y < y_lb; y++)
					{
						Vec3b *temp = src.ptr<Vec3b>(y);
						for (int x = x_lt; x < x_rt; x++)
						{
							temp[x] = Vec<uchar,3>(0,0,50); 
						}
					}
				}
				*/
		}
	}
	

	namedWindow("Laplacian2", WINDOW_KEEPRATIO);
	imshow("Laplacian2", ldm);

	//namedWindow("Laplacian", WINDOW_KEEPRATIO);
	//imshow("Laplacian", lap);

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

