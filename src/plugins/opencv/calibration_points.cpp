#include "calibration_points.h"

namespace possumwood {
namespace opencv {

void CalibrationPoints::Layer::add(const cv::Vec3f& objectPoint, const cv::Vec2f& cameraPoint) {
	m_objectPoints.push_back(objectPoint);
	m_cameraPoints.push_back(cameraPoint);
}

CalibrationPoints::CalibrationPoints(const cv::Size& imageSize) : m_imageSize(imageSize) {
}

void CalibrationPoints::addLayer(const Layer& l) {
	m_cameraPoints.push_back(l.m_cameraPoints);
	m_objectPoints.push_back(l.m_objectPoints);
}

const std::vector<std::vector<cv::Vec3f>>& CalibrationPoints::objectPoints() const {
	return m_objectPoints;
}

const std::vector<std::vector<cv::Vec2f>>& CalibrationPoints::cameraPoints() const {
	return m_cameraPoints;
}

const cv::Size& CalibrationPoints::imageSize() const {
	return m_imageSize;
}

bool CalibrationPoints::empty() const {
	return m_cameraPoints.empty();
}

std::size_t CalibrationPoints::size() const {
	return m_cameraPoints.size();
}

bool CalibrationPoints::operator==(const CalibrationPoints& f) const {
	return m_objectPoints == f.m_objectPoints && m_cameraPoints == f.m_cameraPoints && m_imageSize == f.m_imageSize;
}

bool CalibrationPoints::operator!=(const CalibrationPoints& f) const {
	return m_objectPoints != f.m_objectPoints || m_cameraPoints != f.m_cameraPoints || m_imageSize != f.m_imageSize;
}

std::ostream& operator<<(std::ostream& out, const CalibrationPoints& f) {
	out << "(" << f.size() << " calibration point set(s), image size=" << f.imageSize().width << "x"
	    << f.imageSize().height << ")" << std::endl;

	return out;
}

}  // namespace opencv
}  // namespace possumwood
