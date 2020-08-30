#pragma once

#include <iomanip>
#include <iostream>
#include <opencv2/opencv.hpp>

#include "opencv/frame.h"

namespace possumwood {
namespace images {

class OpencvMatWrapper {
  public:
	OpencvMatWrapper() : m_constMat(new cv::Mat(1, 1, CV_8UC1)) {
	}

	OpencvMatWrapper(std::size_t width, std::size_t height, int type = CV_8U, int channels = 3)
	    : m_mat(new cv::Mat(height, width, CV_MAKETYPE(type, channels))) {
		m_constMat = m_mat;
		assert(type == CV_8U || type == CV_32F);
	}

	OpencvMatWrapper(const OpencvMatWrapper& i) : m_constMat(i.m_constMat) {
	}

	OpencvMatWrapper(const opencv::Frame& f) : m_constMat(new cv::Mat(*f)) {
	}

	OpencvMatWrapper& operator=(const OpencvMatWrapper& i) {
		m_constMat = i.m_constMat;
		return *this;
	}

	OpencvMatWrapper& operator=(const cv::Mat& m) {
		m_constMat = std::shared_ptr<const cv::Mat>(new cv::Mat(m));
		return *this;
	}

	operator opencv::Frame() const {
		return opencv::Frame(*m_constMat);
	}

	void setPixel(std::size_t x, std::size_t y, const luabind::object& value) {
		if(x >= width() || y >= height()) {
			std::stringstream ss;
			ss << "Coordinates " << x << ", " << y << " out of bounds of " << width() << ", " << height() << ".";
			throw std::runtime_error(ss.str().c_str());
		}

		if(luabind::type(value) != LUA_TTABLE)
			throw std::runtime_error("Pixel values have to be tables!");

		// copy on first write - to be used in "injection" to allow to seamlessly
		//   "overwrite" the original image, copying only when necessary
		if(m_mat == nullptr) {
			m_mat = std::shared_ptr<cv::Mat>(new cv::Mat(m_constMat->clone()));
			m_constMat = m_mat;
		}

		if(m_mat->depth() == CV_8U) {
			unsigned char* val = m_mat->ptr<unsigned char>(y, x);

			for(int i = 0; luabind::object(value[i + 1]).is_valid() && i < m_mat->channels(); ++i)
				val[i] = luabind::object_cast<unsigned char>(value[i + 1]);
		}
		else if(m_mat->depth() == CV_32F) {
			float* val = m_mat->ptr<float>(y, x);

			for(int i = 0; luabind::object(value[i + 1]).is_valid() && i < m_mat->channels(); ++i)
				val[i] = luabind::object_cast<float>(value[i + 1]);
		}
		else
			throw std::runtime_error("Only CV_8U or CV_32F types supported at the moment");
	}

	luabind::object pixel(std::size_t x, std::size_t y, lua_State* L) {
		if(x >= width() || y >= height()) {
			std::stringstream ss;
			ss << "Coordinates " << x << ", " << y << " out of bounds of " << width() << ", " << height() << ".";
			throw std::runtime_error(ss.str().c_str());
		}

		luabind::object result = luabind::newtable(L);

		if(m_constMat->depth() == CV_8U) {
			const unsigned char* val = m_constMat->ptr<const unsigned char>(y, x);

			for(int i = 0; i < m_constMat->channels(); ++i)
				result[i + 1] = val[i];
		}
		else if(m_constMat->depth() == CV_32F) {
			const float* val = m_constMat->ptr<const float>(y, x);

			for(int i = 0; i < m_constMat->channels(); ++i)
				result[i + 1] = val[i];
		}
		else
			throw std::runtime_error("Only CV_8U or CV_32F types supported at the moment");

		return result;
	}

	std::size_t width() const {
		return m_constMat->cols;
	}

	std::size_t height() const {
		return m_constMat->rows;
	}

	bool operator==(const OpencvMatWrapper& p) const {
		return m_constMat == p.m_constMat;
	}

  private:
	std::shared_ptr<const cv::Mat> m_constMat;
	std::shared_ptr<cv::Mat> m_mat;
};

inline std::string to_string(const OpencvMatWrapper& p) {
	return "(opencv Mat " + std::to_string(p.width()) + "x" + std::to_string(p.height()) + ")";
}

}  // namespace images
}  // namespace possumwood
