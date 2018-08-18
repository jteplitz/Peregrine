#include "motionDetection.hpp"

int main(int argc, char** argv) {
	rng = cv::RNG(12345);

	const char* filename = argv[1];

	std::cout << "reading test video from: " << filename << std::endl;

	cv::VideoCapture vid(filename);

	cv::Ptr<cv::BackgroundSubtractorMOG2> background;
	background = cv::createBackgroundSubtractorMOG2(BACKGROUND_HISTORY,
													BACKGROUND_THRESHOLD,
													DETECT_SHADOWS);

	structuringElement_open = 
		cv::getStructuringElement(cv::MORPH_ELLIPSE,
						 	      cv::Size(ELLIPSE_OPEN_SIZE,
						 	      		   ELLIPSE_OPEN_SIZE));

	if (!vid.isOpened()) {
		std::cout << "Error" << std::endl;
		return -1;
	}

	cv::Mat rawFrame;
	cv::Mat processFrame;

	cv::Mat foreground;

	while(true) {
		vid >> rawFrame;

		processFrame = filterFrame(rawFrame);

		background->apply(processFrame, foreground);


		char c = (char)cv::waitKey(25);
		if (c==27) {
			break;
		}

		drawContours(foreground, rawFrame);
	}


	vid.release();
	cv::destroyAllWindows();
	return 0;
}

cv::Mat filterFrame(cv::Mat &inFrame) {
	cv::Mat outFrame;
	cv::GaussianBlur(inFrame, outFrame, 
					 cv::Size(BLUR_KERNEL, BLUR_KERNEL), 0, 0);
	return(outFrame);
}

void drawContours(cv::Mat &frame, cv::Mat &rawFrame) {
	cv::Mat edges;
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;

	cv::Mat opened;
	cv::morphologyEx(frame, opened, cv::MORPH_OPEN, structuringElement_open);

	cv::Canny(opened, edges, EDGE_THRESH, EDGE_THRESH*2, 3);
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

		cv::drawContours(edges, contours, i, color, 2, 8, 
						 hierarchy, 0, cv::Point());
		cv::rectangle(rawFrame, rects[i].tl(), rects[i].br(), color, 2, 8, 0);
	}

	cv::imshow("edges", edges);
	cv::imshow("raw", rawFrame);
}


