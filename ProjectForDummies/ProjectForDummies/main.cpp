#include "opencv2\highgui\highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
using namespace cv;
using namespace std;

/**
TODO:
- fix concave contour angles (180-angle)
- refine epsilon for approxPolyDp
*/

double calcAngle(Vec2f v1, Vec2f v2) {
	normalize(v1, v1);
	normalize(v2, v2);
	return acos(v1.dot(v2)) * 180 / CV_PI;
}

bool isContourADuplicate(vector<Point> c1, vector<Point> c2) {
	Rect r1 = boundingRect(c1);
	Rect r2 = boundingRect(c2);
	double tolerance = r1.width / 8;
	return sqrt((r1.x - r2.x) * (r1.x - r2.x) + (r1.y - r2.y) * (r1.y - r2.y)) < tolerance
		&& abs(r1.width - r2.width) < tolerance
		&& abs(r1.height - r2.height) < tolerance;
}

int main(int args, const char** argv) {
	//Mat img = imread("irregular-shapes.png", CV_LOAD_IMAGE_UNCHANGED);
	//Mat img = imread("shapes.png", CV_LOAD_IMAGE_UNCHANGED);
	//Mat img = imread("images.jpg", CV_LOAD_IMAGE_UNCHANGED);
	//Mat img = imread("example-pentagons.png", CV_LOAD_IMAGE_UNCHANGED);
	//Mat img = imread("concave-problematic.png", CV_LOAD_IMAGE_UNCHANGED);
	Mat img = imread("concave.png", CV_LOAD_IMAGE_UNCHANGED);
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
	vector<vector<Point> > approximatedContours;
	vector<Vec4i> hierarchy;

	// Detect edges using canny
	Canny(img_gray, canny_output, thresh, thresh * 2, 3);

	// Find contours
	findContours(canny_output, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	if (contours.size() == 0)
		return -1;

	// Drop nested contours
	for (int i = 0; i < contours.size(); i++) {
		for (int j = i + 1; j < contours.size(); j++) {
			if (isContourADuplicate(contours[j], contours[i])) {
				cout << "Found overlapping contours." << endl;
				contours.erase(contours.begin() + (j--));
			}
		}
	}

	cout << endl;

	// Draw contours
	Mat drawing = img_gray;// Mat::zeros(canny_output.size(), CV_8UC3);
	cvtColor(drawing, drawing, CV_GRAY2BGR);

	for (int id = 0; id < contours.size(); id++)
	{
		vector<Point> contour = contours[id];
		drawContours(drawing, contours, id, Scalar(0, 0, 255), 2);

		double arclen = arcLength(contour, true);

		vector<Point> approximatedContour;
		approxPolyDP(contours[id], approximatedContour, arclen/90, true);

		// Number of angles
		const int numOfAngles = approximatedContour.size();

		// Display number of angles in the centroid of the contour
		string text = to_string(numOfAngles);
		int fontFace = FONT_HERSHEY_TRIPLEX;
		double fontScale = 0.8;
		int thickness = 1;
		int baseline = 0;
		Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseline);
		baseline += thickness;
		Moments m = moments(contour);
		Point centroid = Point(m.m10 / m.m00, m.m01 / m.m00);
		putText(drawing, text, Point(centroid.x - textSize.width/2, centroid.y + textSize.height/2), fontFace, fontScale, Scalar(0, 0, 0), thickness);

		// Draw approximated contour
		approximatedContours.push_back(approximatedContour);
		drawContours(drawing, approximatedContours, id, Scalar(255, 0, 0), 2);

		// Calculate and draw angles

		if (approximatedContour.size() < 2)
			break;

		fontFace = FONT_HERSHEY_DUPLEX;
		fontScale = 0.4;

		cout << "#" << id << " contour: " << endl;

		cout << "Sides: " << numOfAngles << endl;

		// Calculate the vectors of the polygon's sides
		vector<Vec2f> vectors;
		vectors.push_back((Point2f)approximatedContour[0] - (Point2f)approximatedContour[approximatedContour.size() - 1]);
		for (int i = 1; i < approximatedContour.size(); i++) {
			vectors.push_back((Point2f)approximatedContour[i] - (Point2f)approximatedContour[i - 1]);
		}
		vectors.push_back((Point2f)approximatedContour[0] - (Point2f)approximatedContour[approximatedContour.size() - 1]);
		vectors.push_back((Point2f)approximatedContour[1] - (Point2f)approximatedContour[0]);

		// Calculate the angles between the vectors
		for (int i = 0; i < approximatedContour.size(); i++) {
			double angle = 180 - calcAngle(vectors[i], vectors[i+1]);
			double angleWithNext = calcAngle(vectors[i], vectors[i + 2]);
			cout << angle << ", " << angleWithNext << endl;

			putText(drawing, to_string((int)angle), approximatedContour[i], fontFace, fontScale, Scalar(0, 0, 0), thickness);
			putText(drawing, to_string((int)angle), approximatedContour[i] + Point(1, 1), fontFace, fontScale, Scalar(0, 255, 0), thickness);
		}

		cout << endl;
	}

	// Show in a window
	namedWindow("Contours", CV_WINDOW_AUTOSIZE);
	imshow("Contours", drawing);

	waitKey(0);
	//destroyWindow("Canny output");
	destroyWindow("Contours");
	return 0;
}