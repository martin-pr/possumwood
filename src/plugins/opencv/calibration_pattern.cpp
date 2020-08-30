#include "calibration_pattern.h"

#include "tools.h"

namespace possumwood {
namespace opencv {

CalibrationPattern::CalibrationPattern(const cv::Mat& data, const cv::Size& size, bool wasFound, const Type& type,
                                       const cv::Size& imageSize)
    : m_size(size), m_imageSize(imageSize), m_wasFound(wasFound), m_type(type) {
	m_features = std::shared_ptr<const cv::Mat>(new cv::Mat(data.clone()));
}

const cv::Mat& CalibrationPattern::operator*() const {
	return *m_features;
}

const cv::Size& CalibrationPattern::size() const {
	return m_size;
}

const cv::Size& CalibrationPattern::imageSize() const {
	return m_imageSize;
}

bool CalibrationPattern::wasFound() const {
	return m_wasFound;
}

CalibrationPattern::Type CalibrationPattern::type() const {
	return m_type;
}

bool CalibrationPattern::operator==(const CalibrationPattern& f) const {
	return m_features == f.m_features;
}

bool CalibrationPattern::operator!=(const CalibrationPattern& f) const {
	return m_features != f.m_features;
}

std::ostream& operator<<(std::ostream& out, const CalibrationPattern& f) {
	out << "(" << (*f).rows << " circles, image size = " << f.imageSize().width << "x" << f.imageSize().height
	    << ", found=" << f.wasFound() << ")";
	return out;
}

}  // namespace opencv
}  // namespace possumwood
