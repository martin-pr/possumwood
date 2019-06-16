#pragma once

#include <memory>
#include <vector>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

namespace possumwood { namespace opencv {

class Frame {
	public:
		Frame(const cv::Mat& data = cv::Mat());

		const cv::Mat& operator*() const;

		bool operator == (const Frame& f) const;
		bool operator != (const Frame& f) const;

	private:
		std::shared_ptr<const cv::Mat> m_frame;
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
