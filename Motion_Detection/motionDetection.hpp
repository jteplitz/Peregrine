#include <iostream>

#include <opencv2/opencv.hpp>

static const int BACKGROUND_HISTORY = 3000;
static const int BACKGROUND_THRESHOLD = 150;
static const bool DETECT_SHADOWS = false;

static const int ELLIPSE_OPEN_SIZE = 10;

static const int BLUR_KERNEL = 11;

static const int EDGE_THRESH = 10;

static const double MERGE_EPS = 0.01;

cv::Mat structuringElement_open;

// void rawToMat();

void preFilterFrame(cv::Mat&, cv::Mat&);
void postFilterFrame(cv::Mat&, cv::Mat&);

void getEdges(cv::Mat&, cv::Mat&);

void getContours(cv::Mat&, 
				 std::vector<std::vector<cv::Point>>&,
				 std::vector<cv::Vec4i>&);


void getBoundingBoxes(std::vector<std::vector<cv::Point>>&,
					  std::vector<cv::Rect>&);

// void mergeBoundingBoxes(std::vector<cv::Rect>&,
// 						std::vector<cv::Rect>&);

void exportBoundingBoxes(cv::Mat, std::vector<cv::Rect>&);