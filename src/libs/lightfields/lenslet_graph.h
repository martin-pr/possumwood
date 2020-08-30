#pragma once

#include <OpenEXR/ImathVec.h>

#include <array>
#include <memory>
#include <opencv2/opencv.hpp>

namespace lightfields {

/// Triangulates lenslet centers detected through image processing, and fits a matrix
/// representing the hexagonal grid pattern present in Lytro cameras.
class LensletGraph {
  public:
	LensletGraph(Imath::V2i sensorSize, int exclusionBorder);
	~LensletGraph();

	LensletGraph(const LensletGraph&) = delete;
	LensletGraph& operator=(const LensletGraph&) = delete;

	void addLenslet(const cv::Vec2f& center);

	void fit();

	const cv::Matx<double, 3, 3>& fittedMatrix() const;
	double lensPitch() const;
	const Imath::V2i& sensorResolution() const;

	void drawCenters(cv::Mat& target) const;
	void drawEdges(cv::Mat& target) const;
	void drawFit(cv::Mat& target) const;

  private:
	struct Pimpl;
	std::unique_ptr<Pimpl> m_pimpl;
};

}  // namespace lightfields
