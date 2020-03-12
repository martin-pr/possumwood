#pragma once

#include <memory>
#include <vector>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

namespace possumwood { namespace opencv {

class Frame {
	public:
		Frame(const cv::Mat& data = cv::Mat(), bool copy = true);

		const cv::Mat& operator*() const;
		const cv::Mat* operator->() const;

		cv::Mat& operator*();
		cv::Mat* operator->();

		cv::Size size() const;
		bool empty() const;

		bool operator == (const Frame& f) const;
		bool operator != (const Frame& f) const;

	private:
		cv::Mat m_frame;
};

std::ostream& operator << (std::ostream& out, const Frame& f);

}

template<>
struct Traits<opencv::Frame> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 1, 0}};
	}
};

}
