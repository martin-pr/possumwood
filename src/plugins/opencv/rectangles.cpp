#include "rectangles.h"

namespace cv {

std::ostream& operator<<(std::ostream& out, const cv::Rect& rect) {
	out << rect.x << "," << rect.y << " " << rect.width << "x" << rect.height;

	return out;
}

}  // namespace cv

namespace std {

std::ostream& operator<<(std::ostream& out, const std::vector<cv::Rect>& rect) {
	for(auto& r : rect)
		out << r << std::endl;

	return out;
}

}  // namespace std