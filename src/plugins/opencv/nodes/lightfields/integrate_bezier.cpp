#include <actions/traits.h>
#include <lightfields/samples.h>
#include <possumwood_sdk/node_implementation.h>
#include <tbb/parallel_for.h>

#include <fstream>
#include <opencv2/opencv.hpp>

#include "bspline.inl"
#include "bspline_hierarchy.h"
#include "frame.h"
#include "lightfields.h"
#include "maths/io/vec2.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::InAttr<lightfields::Samples> a_samples;
dependency_graph::InAttr<Imath::Vec2<unsigned>> a_size;
dependency_graph::InAttr<unsigned> a_levels, a_offset;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

struct Sample {
	Imath::V2d target;
	float value;
};

dependency_graph::State compute(dependency_graph::Values& data) {
	const lightfields::Samples& samples = data.get(a_samples);

	if((*data.get(a_in)).type() != CV_32FC1 && (*data.get(a_in)).type() != CV_32FC3)
		throw std::runtime_error("Only 32-bit single-float or 32-bit 3 channel float format supported on input, " +
		                         possumwood::opencv::type2str((*data.get(a_in)).type()) + " found instead!");

	const cv::Mat& input = *data.get(a_in);

	const unsigned width = data.get(a_size)[0];
	const unsigned height = data.get(a_size)[1];

	const float x_scale = 1.0f / (float)samples.sensorSize()[0];
	const float y_scale = 1.0f / (float)samples.sensorSize()[1];

	std::vector<Sample> cache[3];
	if((*data.get(a_in)).type() == CV_32FC1)
		for(auto s : samples)
			cache[s.color].push_back(
			    Sample{Imath::V2f(s.xy[0] * x_scale, s.xy[1] * y_scale), input.at<float>(s.source[1], s.source[0])});
	else
		for(auto s : samples)
			for(int c = 0; c < 3; ++c)
				cache[c].push_back(Sample{Imath::V2f(s.xy[0] * x_scale, s.xy[1] * y_scale),
				                          *(input.ptr<float>(s.source[1], s.source[0]) + c)});

	const std::size_t levels = data.get(a_levels);
	const std::size_t offset = data.get(a_offset);

	// lets make a hierarchy of b splines
	possumwood::opencv::BSplineHierarchy splines[3] = {possumwood::opencv::BSplineHierarchy(levels, offset),
	                                                   possumwood::opencv::BSplineHierarchy(levels, offset),
	                                                   possumwood::opencv::BSplineHierarchy(levels, offset)};

	for(std::size_t c = 0; c < 3; ++c) {
		for(std::size_t a = 0; a < levels; ++a) {
			auto& spline = splines[c].level(a);

			tbb::parallel_for(std::size_t(0), cache[c].size(), [&](std::size_t a) {
				auto& s = cache[c][a];
				spline.addSample({{s.target[0], s.target[1]}}, s.value);
			});

			if(a < levels - 1)
				tbb::parallel_for(std::size_t(0), cache[c].size(), [&](std::size_t a) {
					auto& s = cache[c][a];
					s.value -= spline.sample({{s.target[0], s.target[1]}});
				});
		}
	}

	cv::Mat mat = cv::Mat::zeros(height, width, CV_32FC3);
	for(int a = 0; a < 3; ++a)
		tbb::parallel_for(0, mat.rows, [&](int y) {
			for(int x = 0; x < mat.cols; ++x)
				// mat.ptr<float>(y,x)[a] = splines[a].level(levels-1).sample(
				mat.ptr<float>(y, x)[a] =
				    splines[a].sample((float)x / (float)(mat.cols - 1), (float)y / (float)(mat.rows - 1));
		});

	data.set(a_out, possumwood::opencv::Frame(mat));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_samples, "samples", lightfields::Samples(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_size, "size", Imath::Vec2<unsigned>(300u, 300u));
	meta.addAttribute(a_levels, "levels", 3u);
	meta.addAttribute(a_offset, "offset", 6u);
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_samples, a_out);
	meta.addInfluence(a_size, a_out);
	meta.addInfluence(a_levels, a_out);
	meta.addInfluence(a_offset, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/lightfields/integrate_bezier", init);

}  // namespace
