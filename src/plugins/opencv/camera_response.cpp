#include "camera_response.h"

namespace possumwood {
namespace opencv {

namespace {
bool isSame(const float& f1, const float& f2) {
	if(std::isinf(f1) && std::isinf(f2))
		return true;
	if(std::isnan(f1) && std::isnan(f2))
		return true;

	return f1 == f2;
}

bool sameMatrix(const cv::Mat& m1, const cv::Mat& m2) {
	if(m1.rows != m2.rows || m1.cols != m2.cols)
		return false;

	for(int r = 0; r < m1.rows; ++r)
		for(int c = 0; c < m1.cols; ++c)
			if(!isSame(m1.at<float>(r, c), m2.at<float>(r, c)))
				return false;

	return true;
}
}  // namespace

CameraResponse::CameraResponse() {
}

CameraResponse::CameraResponse(const cv::Mat& responseCurve, const std::vector<float>& exposures)
    : m_matrix(responseCurve), m_exposures(exposures) {
}

const cv::Mat& CameraResponse::matrix() const {
	return m_matrix;
}

const std::vector<float>& CameraResponse::exposures() const {
	return m_exposures;
}

bool CameraResponse::operator==(const CameraResponse& f) const {
	return sameMatrix(m_matrix, f.m_matrix) && m_exposures == f.m_exposures;
}

bool CameraResponse::operator!=(const CameraResponse& f) const {
	return !sameMatrix(m_matrix, f.m_matrix) || m_exposures != f.m_exposures;
}

std::ostream& operator<<(std::ostream& out, const CameraResponse& f) {
	out << "(response curve)";
	return out;
}

}  // namespace opencv
}  // namespace possumwood
