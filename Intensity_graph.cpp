#include "stdafx.h"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

int main(void)
{
	Mat src;
	float a, b;
	Point pt1, pt2;
	pt1.x = 100;
	pt1.y = 100;
	pt2.x = 500;
	pt2.y = 500;
	src = imread("Chessboard.jpg", IMREAD_COLOR);
	line(src, pt1, pt2, Color::antiquewhite, 2, LINE_AA);

	


}