#include <iostream>

#include <opencv2/opencv.hpp>

static const int BACKGROUND_HISTORY = 5000;
static const int BACKGROUND_THRESHOLD = 100;
static const bool DETECT_SHADOWS = false;

static const int ELLIPSE_OPEN_SIZE = 10;

static const int BLUR_KERNEL = 11;

static const int EDGE_THRESH = 10;

cv::RNG rng;
cv::Mat structuringElement_open;


cv::Mat filterFrame(cv::Mat&);
void drawContours(cv::Mat&, cv::Mat& rawFrame);