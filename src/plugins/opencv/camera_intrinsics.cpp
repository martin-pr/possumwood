#include "camera_intrinsics.h"

namespace possumwood { namespace opencv {

namespace {
	bool isSame(const float& f1, const float& f2) {
		if(std::isinf(f1) && std::isinf(f2))
			return true;
		if(std::isnan(f1) && std::isnan(f2))
			return true;

		return f1 == f2;
	}

	bool sameVector(const std::vector<float>& v1, const std::vector<float>& v2) {
		if(v1.size() != v2.size())
			return false;

		auto i1 = v1.begin();
		auto i2 = v2.begin();
		while(i1 != v1.end()) {
			if(!isSame(*i1, *i2))
				return false;

			++i1;
			++i2;
		}

		return true;
	}

	bool sameMatrix(const cv::Mat& m1, const cv::Mat& m2) {
		if(m1.rows != m2.rows || m1.cols != m2.cols)
			return false;

		for(int r=0;r<m1.rows;++r)
			for(int c=0;c<m1.cols;++c)
				if(!isSame(m1.at<double>(r, c), m2.at<double>(r, c)))
					return false;

		return true;
	}
}

CameraIntrinsics::CameraIntrinsics() : m_imageSize(0,0) {
	m_matrix = cv::Mat::eye(3, 3, CV_64F);
}

CameraIntrinsics::CameraIntrinsics(const cv::Mat& matrix, const std::vector<float>& distCoeffs, const cv::Size& imageSize) : m_matrix(matrix), m_distCoeffs(distCoeffs), m_imageSize(imageSize) {
}

const cv::Mat& CameraIntrinsics::matrix() const {
	return m_matrix;
}

const std::vector<float>& CameraIntrinsics::distCoeffs() const {
	return m_distCoeffs;
}

const cv::Size& CameraIntrinsics::imageSize() const {
	return m_imageSize;
}

bool CameraIntrinsics::operator == (const CameraIntrinsics& f) const {
	return sameMatrix(m_matrix, f.m_matrix) && sameVector(m_distCoeffs, f.m_distCoeffs) && m_imageSize == f.m_imageSize;
}

bool CameraIntrinsics::operator != (const CameraIntrinsics& f) const {
	return !sameMatrix(m_matrix, f.m_matrix) || !sameVector(m_distCoeffs, f.m_distCoeffs) || m_imageSize != f.m_imageSize;
}

std::ostream& operator << (std::ostream& out, const CameraIntrinsics& f) {
	out << "(camera intrinsics)";

	return out;
}

} }
