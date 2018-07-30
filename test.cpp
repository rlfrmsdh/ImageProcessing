#include "stdafx.h"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
	// Declare the output variables
	Mat dst, cdst, cdstP;

	//![load]
	const char* default_file = "chessboard.jpg";
	const char* filename = argc >= 2 ? argv[1] : default_file;

	// Loads an image
	Mat src = imread(filename, IMREAD_GRAYSCALE);

	// Check if image is loaded fine
	if (src.empty()) {
		printf(" Error opening image\n");
		printf(" Program Arguments: [image_name -- default %s] \n", default_file);
		return -1;
	}
	//![load]

	//![edge_detection]
	// Edge detection
	Canny(src, dst, 50, 200, 3);
	//![edge_detection]

	// Copy edges to the images that will display the results in BGR
	cvtColor(dst, cdst, COLOR_GRAY2BGR);
	cdstP = cdst.clone();

	//![hough_lines]
	// Standard Hough Line Transform
	vector<Vec2f> lines; // will hold the results of the detection
	HoughLines(dst, lines, 1, CV_PI / 180, 150, 0, 0); // runs the actual detection
													   //![hough_lines]
													   //![draw_lines]
													   // Draw the lines
	for (size_t i = 0; i < lines.size(); i++)
	{
		float rho = lines[i][0], theta = lines[i][1];

		if ((lines[i][1])*180/CV_PI >= 85 && (lines[i][1]) * 180 / CV_PI <= 95)
		{
			cout << (lines[i][1]) * 180 / CV_PI << "(deg)" << endl;
			Point pt1, pt2;
			double a = cos(theta), b = sin(theta);
			double x0 = a * rho, y0 = b * rho;
			pt1.x = cvRound(x0 + 1000 * (-b));
			pt1.y = cvRound(y0 + 1000 * (a));
			pt2.x = cvRound(x0 - 1000 * (-b));
			pt2.y = cvRound(y0 - 1000 * (a));
			line(cdst, pt1, pt2, Scalar(0, 0, 255), 3, LINE_AA);

		}
	}
	//![draw_lines]
	
	cout << src.size() << endl;
	namedWindow("Source",WINDOW_KEEPRATIO);
	namedWindow("edge", WINDOW_KEEPRATIO);
	namedWindow("Detected Lines (in red) - Standard Hough Line Transform", WINDOW_KEEPRATIO);
	imshow("Source", src);
	imshow("edge", dst);
	imshow("Detected Lines (in red) - Standard Hough Line Transform", cdst);
//	imshow("Detected Lines (in red) - Probabilistic Line Transform", cdstP);
	waitKey(0);
	return 0;
}