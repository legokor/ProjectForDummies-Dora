#include "opencv2\highgui\highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
using namespace cv;
using namespace std;

int main(int args, const char** argv) {
	Mat img = imread("irregular-shapes.png", CV_LOAD_IMAGE_UNCHANGED);
	if (img.empty()) {
		cout << "Error: Image cannot be loaded..." << endl;
		return -1;
	}

	Mat img_gray;
	// Convert image to gray and blur it
	cvtColor(img, img_gray, CV_BGR2GRAY);
	blur(img_gray, img_gray, Size(3, 3));

	Mat canny_output;
	const int thresh = 100;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	// Detect edges using canny
	Canny(img_gray, canny_output, thresh, thresh * 2, 3);

	namedWindow("Canny output", CV_WINDOW_AUTOSIZE);
	imshow("Canny output", canny_output);

	// Find contours
	findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	// Draw contours
	Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
	RNG rng(12345);
	for (int i = 0; i< contours.size(); i++)
	{
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());
	}

	// Show in a window
	namedWindow("Contours", CV_WINDOW_AUTOSIZE);
	imshow("Contours", drawing);

	waitKey(0);
	destroyWindow("Canny output");
	destroyWindow("Contours");
	return 0;
}