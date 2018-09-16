#include <peregrine.hpp>

#include <opencv2/opencv.hpp>
#include <tensyr/memory.h>

// TODO(jason): Use graph constants!
#define FRAME_WIDTH 640
#define FRAME_HEIGHT 360

// opencv constants
static const int BACKGROUND_HISTORY = 3000;
static const int BACKGROUND_THRESHOLD = 150;
static const bool DETECT_SHADOWS = false;

static const int ELLIPSE_OPEN_SIZE = 10;

static const int BLUR_KERNEL = 11;

static const int EDGE_THRESH = 10;

static const double MERGE_EPS = 0.01;



namespace Peregrine {
  static const cv::Mat FrameToCvMat(const unsigned char* frame, const int kFrameWidth, const int kFrameHeight) {
    const cv::Mat mat(kFrameHeight, kFrameWidth,  CV_MAKETYPE(CV_8U, 3), const_cast<unsigned char*>(frame));
    return mat;
  }

  static cv::Mat FrameToCvMat(unsigned char* frame, const int kFrameWidth, const int kFrameHeight) {
    const cv::Mat mat(kFrameHeight, kFrameWidth,  CV_MAKETYPE(CV_8U, 3), frame);
    return mat;
  }

  static void PreFilterFrame(const cv::Mat &inFrame, cv::Mat &outFrame)
  {
    cv::GaussianBlur(inFrame, outFrame,
             cv::Size(BLUR_KERNEL, BLUR_KERNEL), 0, 0);
  }

  static void PostFilterFrame(const cv::Mat &inFrame, cv::Mat &outFrame, cv::Mat& structuringElement_open)
  {
    cv::morphologyEx(inFrame, outFrame, cv::MORPH_OPEN,
             structuringElement_open);
  }

  static void GetEdges(const cv::Mat &inFrame, cv::Mat &edges)
  {
    cv::Canny(inFrame, edges, EDGE_THRESH, EDGE_THRESH*2, 3);
  }

  static void GetContours(const cv::Mat &edges, 
           std::vector<std::vector<cv::Point>> &contours,
           std::vector<cv::Vec4i> &hierarchy)
  {
    cv::findContours(edges, contours, hierarchy,
             cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE,
             cv::Point(0, 0));
  }

  // Copies the c++ vector of opencv rectangles into a contiguous block
  // of memory for HALO
  static Boxes* ToBoxes(const std::vector<cv::Rect>& rects) {
    auto output_size = sizeof(Boxes) + (sizeof(Box) * rects.size());
    auto boxes = reinterpret_cast<Boxes*>(HaloMalloc(output_size));
    if (boxes == nullptr) {
      fprintf(stderr, "OOM in HALO heap\n");
      return nullptr;
    }
    boxes->num_boxes = rects.size();
    auto curr_box = reinterpret_cast<Box*>(boxes + 1);
    for (const auto& rect: rects) {
      curr_box->x = rect.x;
      curr_box->y = rect.y;
      curr_box->width = rect.width;
      curr_box->height = rect.height;
      curr_box++;
    }
    return boxes;
  }


  static Boxes* GetBoundingBoxes(std::vector<std::vector<cv::Point>> &contours)
  {
    cv::Scalar color(255, 255, 255);

    std::vector<cv::Rect> rects;
    for (unsigned int i = 0; i < contours.size(); i++) {
      std::vector<cv::Point> polys;
      cv::approxPolyDP(cv::Mat(contours[i]), polys, 3, true);
      rects.push_back(cv::boundingRect(cv::Mat(polys)));
    }

    cv::groupRectangles(rects, 0, MERGE_EPS);
    return ToBoxes(rects);
  }

  static void DrawBoundingBoxes(cv::Mat& frame,
      const Boxes* boxes) {
    cv::Scalar color(255, 255, 255);

    auto box_arr = reinterpret_cast<const Box*>(boxes + 1);
    for (unsigned int i = 0; i < boxes->num_boxes; i++) {
      const auto& curr_box = box_arr[i];
      cv::Point tl(curr_box.x, curr_box.y);
      cv::Point br(curr_box.x + curr_box.width, curr_box.y + curr_box.height);
      cv::rectangle(frame, tl, br, color, 2, 8, 0);
    }
  }
}

extern "C"{
// TODO(jason): Move to KV store once it's released
static cv::Mat structuringElement_open;
static cv::Ptr<cv::BackgroundSubtractorMOG2> background;
static bool motion_detector_initialized = false;

Boxes* DetectMotion(const unsigned char* frame) {
  if (!motion_detector_initialized) {
    background = cv::createBackgroundSubtractorMOG2(BACKGROUND_HISTORY,
                            BACKGROUND_THRESHOLD,
                            DETECT_SHADOWS);

    structuringElement_open = 
      cv::getStructuringElement(cv::MORPH_ELLIPSE,
                      cv::Size(ELLIPSE_OPEN_SIZE,
                             ELLIPSE_OPEN_SIZE));
    motion_detector_initialized = true;
  }
  // Convert & apply to background
  auto rawFrame = Peregrine::FrameToCvMat(frame, FRAME_WIDTH, FRAME_HEIGHT);
  cv::Mat foreground;
  Peregrine::PreFilterFrame(rawFrame, foreground);
  background->apply(foreground, foreground);
  Peregrine::PostFilterFrame(foreground, foreground, structuringElement_open);

  // Get edges
  cv::Mat edges;
  Peregrine::GetEdges(foreground, edges);

  // Get Contours
	std::vector<cv::Vec4i> hierarchy;
  std::vector<std::vector<cv::Point>> contours;
  Peregrine::GetContours(edges, contours, hierarchy);
  return Peregrine::GetBoundingBoxes(contours);
}

unsigned char* DrawBoxes(unsigned char* frame, const Boxes* boxes) {
  auto rawFrame = Peregrine::FrameToCvMat(frame, FRAME_WIDTH, FRAME_HEIGHT);
  Peregrine::DrawBoundingBoxes(rawFrame, boxes);
  return frame;
}

}
