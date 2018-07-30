#include "stdafx.h"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
using namespace std;
using namespace cv;

// vector.insert 함수 확인하려고 짜본 예제임 
int main(void)
{
	Mat mat;
	mat.create(10, 10, CV_8UC1);
	mat = Scalar::all(0);
	mat.at<uchar>(0, 0) = 1;
	mat.at<uchar>(0, 1) = 2;
	mat.at<uchar>(0, 2) = 3;
	mat.at<uchar>(0, 3) = 4;
	mat.at<uchar>(0, 4) = 5;
	mat.at<uchar>(0, 5) = 6;
	uchar *ptt = mat.ptr<uchar>(0);
	ptt[6] = 7;

	cout << ptt[6] << endl;
	//unsigned char mat[10] = {1, 2, 3,4,5,6,7,8,9,10};
	
	
	vector<int> array;
		
	array.insert(array.end(), mat.ptr<uchar>(0), mat.ptr<uchar>(0) + 6);
	
	for (int i = 0; i < 6; i++)
		cout << array[i] << " ";
	
	return 0;
}