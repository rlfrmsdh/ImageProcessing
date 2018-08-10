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
	/*
	Mat hor, ver;
	hor.create(dx_Scharr.size(), CV_8UC1);
	ver.create(dx_Scharr.size(), CV_8UC1);
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
				ver.at<uchar>(y, x) = draw1.at<uchar>(y, x);
			else
				hor.at<uchar>(y, x) = draw1.at<uchar>(y, x);
		}
		*/
	
	/////////////////////////////////////////////////////////////////////end



	//line 갯수 세기 & vector 채워넣기
	//////////////////////////////////////////////////////////////////////////////////
	/*
	vector <vector <float> > arr_v;
	vector <vector <float> > arr_h;
	//arr 과 arr2에 저장순서 : [pixel위치(x or y), angle, 몇번째 line인지]


	int x, y;
	int i = 1, j = 1, cnt = 0;
	for (x = 0; x < ver.cols; x++)
	{
		if (ver.at<uchar>(testpoint, x) > 0)
		{
			vector <float> row;
			temp = atan2((double)(dy_Scharr.at<float>(testpoint, x)), (double)(dx_Scharr.at<float>(testpoint, x)));
			temp = temp * 180 / CV_PI;
			if (temp >= 90 && temp <= 180)
				temp = temp - 180;
			if (temp <= -90 && temp >= -180)
				temp = temp + 180;

			row.push_back(x);
			row.push_back(temp);
			arr_v.push_back(row);
		}

	}
	for (y = 0; y < hor.rows; y++)
	{
		if (hor.at<uchar>(y, testpoint) > 0)
		{
			vector <float> row;
			temp = atan2((double)(dy_Scharr.at<float>(y, testpoint)), (double)(dx_Scharr.at<float>(y, testpoint)));
			temp = temp * 180 / CV_PI;
			if (temp >= 90 && temp <= 180)
				temp = temp - 180;
			if (temp <= -90 && temp >= -180)
				temp = temp + 180;

			row.push_back(y);
			row.push_back(temp);
			arr_h.push_back(row);
		}

	}
	i = 1, j = 1;
	for (x = 1; x < arr_v.size(); x++)
	{
		arr_v[x - 1].push_back(i);
		if ((arr_v[x][0] - arr_v[x - 1][0]) > 3)
		{
			i++;
		}
	}
	arr_v[arr_v.size() - 1].push_back(i);
	for (int y = 1; y < arr_h.size(); y++)
	{
		arr_h[y - 1].push_back(j);
		if ((arr_h[y][0] - arr_h[y - 1][0]) > 3)
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
	
	 
	cout << "arr2 출력 : " << endl;
	for (int y = 0; y < arr_h.size(); y++)
	{
		for (int x = 0; x < arr_h[y].size(); x++)
			cout << arr_h[y][x] << " ";
		cout << endl;
	}
	*/


	//////////////////////////////////////////////////////////////////////////////////

	//namedWindow("Thin Edge", WINDOW_KEEPRATIO);
	//imshow("Thin Edge", draw1);



	// Horizontal line과 Vertical line의 각도 뽑아내기
	///////////////////////////////////////////////////////////////////////////////
/*
	//angle sorting
	int size_v, size_h;
	size_v = arr_v.size();
	size_h = arr_h.size();

	vector <float> angle_v;
	vector <float> angle_h;
	cout << endl;

	for (int i = 0; i < size_v; i++)
	{
		angle_v.push_back(arr_v[i][1]);
	}

	for (int j = 0; j < size_h; j++)
	{
		angle_h.push_back(arr_h[j][1]);
	}
	sort(angle_v.begin(), angle_v.end());
	sort(angle_h.begin(), angle_h.end());



	//sorting하고난 후 갯수 세기
	vector <vector<float>> theta_sort_v;            //이런형태에서 한 row를 없애면 그다음 row가 순차적으로 올라오지 않음. 이걸 어떻게 해결하면좋을까
	vector <vector<float>> theta_sort_h;
	
	vector <float> initial_v;
	initial_v.push_back(angle_v[0]);
	initial_v.push_back(1);
	theta_sort_v.push_back(initial_v);

	cnt = 0;
	for (int i=0; i<angle_v.size()-1; i++)
	{
		vector <float> row;

		if (angle_v[i] == angle_v[i + 1])
		{
			theta_sort_v[cnt][1]++;

		}
		else
		{
			row.push_back(angle_v[i+1]);
			row.push_back(1);
			theta_sort_v.push_back(row);
			cnt++;
		}
	}
	vector <float> initial_h;
	initial_h.push_back(angle_h[0]);
	initial_h.push_back(1);
	theta_sort_h.push_back(initial_h);

	cnt = 0;
	for (int i = 0; i<angle_h.size() - 1; i++)
	{
		vector <float> row;

		if (angle_h[i] == angle_h[i + 1])
		{
			theta_sort_h[cnt][1]++;

		}
		else
		{
			row.push_back(angle_h[i + 1]);
			row.push_back(1);
			theta_sort_h.push_back(row);
			cnt++;
		}
	}




	sort(theta_sort_v.begin(), theta_sort_v.end(), sortcol2);
	sort(theta_sort_h.begin(), theta_sort_h.end(), sortcol2);

	
	cout << "theta_sort printing :  " << endl;
	for (int i = 0; i < theta_sort_v.size(); i++)
		cout << theta_sort_v[i][0] << " " << theta_sort_v[i][1] << endl;
	cout << "theta_sort2 printing :  " << endl;
	for (int i = 0; i < theta_sort_h.size(); i++)
		cout << theta_sort_h[i][0] << " " << theta_sort_h[i][1] << endl;
		
	float Horizontal_theta = theta_sort_h[0][0];
	float Vertical_theta = theta_sort_v[0][0];

	cout << "Horizontal theta: " << Horizontal_theta << endl;
	cout << "Vertical theta: " << Vertical_theta << endl;
	*/
	/////////////////////////////////////////////////////////////////////////////////////////

	/*
	// r계산 후 영역 정하고  영역별 pixel position 뽑아내기.
	////////////////////////////////////////////////////////////////////////////////////start
	vector <float> r_v;
	vector <float> r_h;
	
	//Vertical
	r_v.push_back(cos(Vertical_theta*CV_PI / 180)*arr_v[0][0]);
	cnt = 0;
	for (i = 1; i < size_v; i++)
	{
		if (arr_v[i][0] - arr_v[i - 1][0] > 10)
		{
			r_v.push_back(cos(Vertical_theta*CV_PI / 180)*arr_v[i-1][0]);
			r_v.push_back(cos(Vertical_theta*CV_PI / 180)*arr_v[i][0]);
			cnt = cnt + 2;

		}	
	}
	r_v.push_back(cos(Vertical_theta*CV_PI / 180)*arr_v[size_v-1][0]);

	int size_rv = r_v.size();
	r_v[0] = r_v[0] < 5 ? r_v[0] : r_v[0] - 5;
	r_v[size_rv -1] = r_v[size_rv -1] >= width - 5 ? r_v[size_rv -1] : r_v[size_rv -1] + 5;

	for (i = 1; i < size_rv-1; i++)
	{
		r_v[i] = i % 2 == 0 ? r_v[i] - 5 : r_v[i] + 5;

	}

	//Horizontal
	r_h.push_back(sin(Horizontal_theta*CV_PI / 180)*arr_h[0][0]);
	cnt = 0;
	for (i = 1; i < size_h; i++)
	{
		if (arr_h[i][0] - arr_h[i - 1][0] > 10)
		{
			r_h.push_back(sin(Horizontal_theta*CV_PI / 180)*arr_h[i - 1][0]);
			r_h.push_back(sin(Horizontal_theta*CV_PI / 180)*arr_h[i][0]);
			cnt = cnt + 2;

		}
	}
	r_h.push_back(sin(Horizontal_theta*CV_PI / 180)*arr_h[size_h - 1][0]);

	int size_rh = r_h.size();
	r_h[0] = r_h[0] < 5 ? r_h[0] : r_h[0] - 5;
	r_h[size_rh - 1] = r_h[size_rh - 1] >= height - 5 ? r_h[size_rh - 1] : r_h[size_rh - 1] + 5;

	for (i = 1; i < size_rh - 1; i++)
	{
		r_h[i] = i % 2 == 0 ? r_h[i] - 5 : r_h[i] + 5;

	}
	
	
	cout << "Vertical radius printing : ";
	for (i = 0; i < size_rv; i++)
		cout << r_v[i] << endl;

	cout << "Horizontal radius printing : ";
	for (j = 0; j < size_rh; j++)
		cout << r_h[j] << endl;
		
	char dir[10];
	int linenum;

	strcpy_s(dir, "Hor");
	linenum = 1;

	Mat A, B, X;
	Mat chosenline;
	vector <Vec2f> vec_A;
	vector <float> vec_B;
	chosenline.create(height, width, CV_8UC3);
	chosenline = Scalar::all(0);
	for (y = 0; y < draw1.rows; y++)
	{
		uchar *imagerow = draw1.ptr<uchar>(y);
		for (x=0; x<draw1.cols; x++)
		{
			chosenline.at<Vec3b>(y,x)[0] = imagerow[x];
			chosenline.at<Vec3b>(y,x)[1] = imagerow[x];
			chosenline.at<Vec3b>(y,x)[2] = imagerow[x];
		}
	}


	if (!strcmp(dir,"Hor"))
	{
		for (int r = r_h[(linenum - 1) * 2]; r < r_h[(linenum - 1) * 2 + 1]; r++)
		{
			for (x = 0; x < width; x++)
			{ 
				y = (-cos(Horizontal_theta*CV_PI / 180) / sin(Horizontal_theta*CV_PI / 180))*x + r / sin(Horizontal_theta*CV_PI / 180);
				//if (y % 1 != 0)
					//cout << "y 출력 : " << endl;
					
				if (hor.at<uchar>(y, x) != 0)
				{
					vec_A.push_back(Vec2f(x, 1));
					vec_B.push_back(y);
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

		namedWindow("Line Fitting Result",WINDOW_KEEPRATIO);
		imshow("Line Fitting Result", src);
		
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
			//myfile << "Writing this to a file \n";

			for (int count = 0; count < diff.size(); count++)
				myfile << diff[count] << " ";

			//myfile << "\n End of file \n";

			myfile.close();
		}
		else cout << "Unable to open file";
		//
	}

	else
	{
		
		for (int r = r_v[(linenum - 1) * 2]; r < r_v[(linenum - 1) * 2 + 1]; r++)
		{
			for (y = 0; y < height; y++)
			{
				x = (-sin(Vertical_theta*CV_PI / 180) / cos(Vertical_theta*CV_PI / 180))*y + r / cos(Vertical_theta*CV_PI / 180);
				//if (y % 1 != 0)
				//cout << "y 출력 : " << endl;

				if (ver.at<uchar>(y, x) != 0)
				{
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
		line(chosenline, pt1, pt2, Scalar(0, 0, 255), 2, LINE_AA);

		namedWindow("Line Fitting Result", WINDOW_KEEPRATIO);
		imshow("Line Fitting Result", chosenline);

		Mat differ;
		differ.create(B.rows, 1, CV_32FC1);
		differ = A*X-B;
		
		vector <float> diff;
		diff.assign((float*)differ.datastart, (float*)differ.dataend);

		ofstream myfile("Line Fitting Result.txt");
		if (myfile.is_open())
		{
			//myfile << "Writing this to a file \n";

			for (int count = 0; count < diff.size(); count++)
				myfile << diff[count] << " ";

			//myfile << "\n End of file \n";

			myfile.close();
		}
		else cout << "Unable to open file";
		//
		
		
	}*/
	////////////////////////////////////////////////////////////////////////
	
	/*
	cout << "Horizontal line(1) or Vertical line(2): ";
	int classify;
	cin >> classify;

	cout << "몇번째 line ? ";
	int linenum; 
	cin >> linenum;
	
	if (classify == 1)   // Horizontal line
	{

	}
	else   // Vertical line
	{

	}
	*/



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




	//namedWindow("Horizontal", WINDOW_KEEPRATIO);
	//imshow("Horizontal", hor);

	//namedWindow("Vertical", WINDOW_KEEPRATIO);
	//imshow("Vertical", ver);
	
	
	//namedWindow("Source", WINDOW_KEEPRATIO);
	namedWindow("Hough_edge", WINDOW_KEEPRATIO);
	//namedWindow("angle", WINDOW_KEEPRATIO);
	//imshow("Source", src);
	imshow("Hough_edge", Hough_thin);
	
	//imshow("angle", angle);
	waitKey(0);
	return 0;
}




