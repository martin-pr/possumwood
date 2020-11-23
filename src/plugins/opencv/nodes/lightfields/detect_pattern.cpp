#include <actions/traits.h>
#include <lightfields/lenslet_graph.h>
#include <lightfields/pattern.h>
#include <lightfields/samples.h>
#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include "frame.h"
#include "lightfields.h"

namespace {

struct Vec2Compare {
	bool operator()(const cv::Vec2f& v1, const cv::Vec2f& v2) const {
		if(v1[0] != v2[0])
			return v1[0] < v2[0];
		return v1[1] < v2[1];
	}
};

struct Vec4Compare {
	bool operator()(const cv::Vec4f& v1, const cv::Vec4f& v2) const {
		for(int a = 0; a < 3; ++a)
			if(v1[a] != v2[a])
				return v1[a] < v2[a];
		return v1[3] < v2[3];
	}
};

dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::InAttr<possumwood::opencv::Frame> a_calibration;
dependency_graph::InAttr<unsigned> a_border;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;
dependency_graph::OutAttr<lightfields::Samples> a_samples;
dependency_graph::OutAttr<float> a_lensPitch;

dependency_graph::State compute(dependency_graph::Values& data) {
	const cv::Mat& input = *data.get(a_in);
	if(input.type() != CV_32FC1)
		throw std::runtime_error("Only CV_32FC1 type allowed on input");

	const cv::Mat& calibration = *data.get(a_calibration);
	if(calibration.type() != CV_8UC1)
		throw std::runtime_error("Only CV_8UC1 type allowed on calibration input");

	const std::size_t border = data.get(a_border);
	if(border < 1)
		throw std::runtime_error("Border value needs to be non-zero");
	if(std::size_t(calibration.rows) < 2 * border + 1 || std::size_t(calibration.cols) < 2 * border + 1)
		throw std::runtime_error("Border value too large for the size of the calibration image");

	// make a lenslet data structure
	lightfields::LensletGraph lenslets(Imath::V2i(calibration.cols, calibration.rows), border);
	// feed all the local minima into it, except the ones too close to the border
	for(std::size_t y = 1; y < std::size_t(calibration.rows) - 1; ++y)
		for(std::size_t x = 1; x < std::size_t(calibration.cols) - 1; ++x) {
			const unsigned char current = calibration.at<unsigned char>(y, x);
			bool isMaximum = true;
			for(std::size_t a = 0; a < 9; ++a)
				if(a != 4 && calibration.at<unsigned char>(y + a / 3 - 1, x + a % 3 - 1) >= current)
					isMaximum = false;

			if(isMaximum)
				lenslets.addLenslet(cv::Vec2i(x, y));
		}

	// debug drawing
	cv::Mat mat(input.size(), CV_8UC1, cv::Scalar(0));
	lenslets.drawCenters(mat);

	// delaunay drawing
	lenslets.fit();
	lenslets.drawFit(mat);

	data.set(a_out, possumwood::opencv::Frame(mat));

	// pattern
	const lightfields::Pattern pattern = lightfields::Pattern::fromFit(lenslets);
	data.set(a_samples, lightfields::Samples::fromPattern(pattern, input));

	data.set(a_lensPitch, pattern.lensPitch());

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_calibration, "calibration", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_border, "border", 20u);
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_samples, "samples", lightfields::Samples(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_lensPitch, "lens_pitch");

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_calibration, a_out);
	meta.addInfluence(a_border, a_out);

	meta.addInfluence(a_in, a_samples);
	meta.addInfluence(a_calibration, a_samples);
	meta.addInfluence(a_border, a_samples);

	meta.addInfluence(a_in, a_lensPitch);
	meta.addInfluence(a_calibration, a_lensPitch);
	meta.addInfluence(a_border, a_lensPitch);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/lightfields/detect_pattern", init);

}  // namespace
