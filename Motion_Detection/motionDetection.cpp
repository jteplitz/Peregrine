#include "motionDetection.hpp"

int main(int argc, char** argv) {
	rng = cv::RNG(12345);

	const char* filename = argv[1];

	std::cout << "reading test video from: " << filename << std::endl;

	cv::VideoCapture vid(filename);

	if (!vid.isOpened()) {
		std::cout << "Error" << std::endl;
		return -1;
	}

	cv::Mat frameSet[3];
	cv::Mat rawFrame;

	for (int i = 0; i < 3; i++) {
		vid >> rawFrame;

		frameSet[i] = filterFrame(rawFrame);
	}

	do {
		cv::Mat diffs = diffFrame(frameSet);
		drawContours(diffs, rawFrame);

		char c = (char)cv::waitKey(25);
		if (c==27) {
			break;
		}

		vid >> rawFrame;

		frameSet[2] = frameSet[1];
		frameSet[1] = frameSet[0];
		frameSet[0] = filterFrame(rawFrame);
	}
	while(!frameSet[0].empty());

	vid.release();
	cv::destroyAllWindows();
	return 0;
}

cv::Mat filterFrame(cv::Mat frame) {
	cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
	cv::GaussianBlur(frame, frame, cv::Size(BLUR_KERNEL, BLUR_KERNEL), 0, 0);
	return(frame);
}

cv::Mat diffFrame(cv::Mat* frameSet) {
	cv::Mat structuringElement = cv::getStructuringElement(cv::MORPH_ELLIPSE, 
														   cv::Size(50, 50));

	cv::Mat d1;
	cv::Mat d2;

	cv::Mat diffs;

	cv::absdiff(frameSet[0], frameSet[1], d1);
	cv::absdiff(frameSet[1], frameSet[2], d2);

	cv::bitwise_xor(d1, d2, diffs);

	cv::Mat dilated;

	cv::morphologyEx(diffs, dilated, cv::MORPH_CLOSE, structuringElement);

	return(dilated);
}

void drawContours(cv::Mat frame, cv::Mat orgFrame) {
	cv::Mat edges;
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;


	cv::Canny(frame, edges, EDGE_THRESH, EDGE_THRESH*2, 3);
	cv::findContours(edges, contours, hierarchy, cv::RETR_TREE, 
					 cv::CHAIN_APPROX_SIMPLE, cv::Point(0,0));

	std::vector<std::vector<cv::Point>> contours_poly(contours.size());
	std::vector<cv::Rect> rects(contours.size());

	for (unsigned int i = 0; i < contours.size(); i++) {
		cv::Scalar color = cv::Scalar(rng.uniform(0, 255),
									  rng.uniform(0, 255),
									  rng.uniform(0, 255));

		cv::approxPolyDP(cv::Mat(contours[i]), contours_poly[i], 3, true);
		rects[i] = cv::boundingRect(cv::Mat(contours_poly[i]));

		// cv::drawContours(orgFrame, contours, i, color, 2, 8, 
		// 				 hierarchy, 0, cv::Point());
		cv::rectangle(orgFrame, rects[i].tl(), rects[i].br(), color, 2, 8, 0);
	}

	cv::imshow("contours", orgFrame);
}


