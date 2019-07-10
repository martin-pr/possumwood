#include <possumwood_sdk/node_implementation.h>

#include <fstream>

#include <tbb/parallel_for.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include "frame.h"
#include "lightfield_samples.h"
#include "tools.h"
#include "bspline_hierarchy.h"
#include "bspline.inl"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::InAttr<possumwood::opencv::LightfieldSamples> a_samples;
dependency_graph::InAttr<unsigned> a_width, a_height, a_levels, a_offset;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

struct Sample {
	cv::Vec2d target;
	float value;
};

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::LightfieldSamples& samples = data.get(a_samples);

	if((*data.get(a_in)).type() != CV_32FC1 && (*data.get(a_in)).type() != CV_32FC3)
		throw std::runtime_error("Only 32-bit single-float or 32-bit 3 channel float format supported on input, " + possumwood::opencv::type2str((*data.get(a_in)).type()) + " found instead!");

	const cv::Mat& input = *data.get(a_in);

	const unsigned width = data.get(a_width);
	const unsigned height = data.get(a_height);

	// TODO: for parallelization to work reliably, we need to use integer atomics here, unfortunately

	{
		std::vector<std::pair<cv::Vec2d, float>> points;
		for(auto& s : samples)
			if(s.color == 0) {
				cv::Vec2d pt = s.target;

				if(pt[0] > 0.48 && pt[0] < 0.52 && pt[1] > 0.48 && pt[1] < 0.52)
					points.push_back(std::make_pair(pt, input.at<float>(s.source[1], s.source[0])));
			}

		std::ofstream data("data.txt");
		for(auto& p : points)
			data << std::setprecision(15) << p.first[0] << "\t" << std::setprecision(15) << p.first[1] << "\t" << p.second << std::endl;
	}

	std::vector<Sample> cache[3];
	if((*data.get(a_in)).type() == CV_32FC1)
		for(auto s : samples)
			cache[s.color].push_back(Sample{
				s.target,
				input.at<float>(s.source[1], s.source[0])
			});
	else
		for(auto s : samples)
			for(int c=0;c<3;++c)
				cache[c].push_back(Sample{
					s.target,
					*(input.ptr<float>(s.source[1], s.source[0]) + c)
				});

	const std::size_t levels = data.get(a_levels);
	const std::size_t offset = data.get(a_offset);

	// lets make a hierarchy of b splines
	possumwood::opencv::BSplineHierarchy splines[3] = {
		possumwood::opencv::BSplineHierarchy(levels, offset),
		possumwood::opencv::BSplineHierarchy(levels, offset),
		possumwood::opencv::BSplineHierarchy(levels, offset)
	};

	for(std::size_t c=0;c<3;++c) {
		for(std::size_t a=0;a<levels;++a) {
			auto& spline = splines[c].level(a);

			tbb::parallel_for(std::size_t(0), cache[c].size(), [&](std::size_t a) {
				auto& s = cache[c][a];
				spline.addSample({s.target[0], s.target[1]}, s.value);
			});

			if(a < levels-1)
				tbb::parallel_for(std::size_t(0), cache[c].size(), [&](std::size_t a) {
					auto& s = cache[c][a];
					s.value -= spline.sample({s.target[0], s.target[1]});
				});
		}
	}

	cv::Mat mat = cv::Mat::zeros(height, width, CV_32FC3);
	for(int a=0;a<3;++a)
		tbb::parallel_for(0, mat.rows, [&](int y) {
			for(int x=0;x<mat.cols;++x)
				// mat.ptr<float>(y,x)[a] = splines[a].level(levels-1).sample(
				mat.ptr<float>(y,x)[a] = splines[a].sample(
					(float)x / (float)(mat.cols-1),
					(float)y / (float)(mat.rows-1)
				);
		});

	data.set(a_out, possumwood::opencv::Frame(mat));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_samples, "samples", possumwood::opencv::LightfieldSamples(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_width, "size/width", 300u);
	meta.addAttribute(a_height, "size/height", 300u);
	meta.addAttribute(a_levels, "levels", 3u);
	meta.addAttribute(a_offset, "offset", 6u);
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_samples, a_out);
	meta.addInfluence(a_width, a_out);
	meta.addInfluence(a_height, a_out);
	meta.addInfluence(a_levels, a_out);
	meta.addInfluence(a_offset, a_out);

	meta.setCompute(compute);
}


possumwood::NodeImplementation s_impl("opencv/lightfields/integrate_bezier", init);

}
