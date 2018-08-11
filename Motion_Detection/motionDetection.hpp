#include <iostream>

#include <opencv2/opencv.hpp>

static const int BLUR_KERNEL = 7;
static const int EDGE_THRESH = 100;
cv::RNG rng;

cv::Mat filterFrame(cv::Mat);
cv::Mat diffFrame(cv::Mat*);
void drawContours(cv::Mat, cv::Mat);