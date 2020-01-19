#pragma once

#include <array>
#include <memory>

#include <opencv2/opencv.hpp>

namespace lightfields {

class LensletGraph {
	public:
		LensletGraph(cv::Vec2i sensorSize, int exclusionBorder);
		~LensletGraph();

		LensletGraph(const LensletGraph&) = delete;
		LensletGraph& operator = (const LensletGraph&) = delete;

		void addLenslet(const cv::Vec2f& center);

		cv::Matx<double, 3, 3> fit();

		void drawCenters(cv::Mat& target) const;
		void drawEdges(cv::Mat& target) const;
		void drawFit(cv::Mat& target) const;

	private:
		struct Pimpl;
		std::unique_ptr<Pimpl> m_pimpl;
};

}
