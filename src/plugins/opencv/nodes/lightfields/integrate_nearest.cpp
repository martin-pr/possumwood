#include <possumwood_sdk/node_implementation.h>

#include <tbb/parallel_for.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include "frame.h"
#include "lightfield_samples.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::InAttr<possumwood::opencv::LightfieldSamples> a_samples;
dependency_graph::InAttr<unsigned> a_width, a_height;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::LightfieldSamples& samples = data.get(a_samples);

	if((*data.get(a_in)).type() != CV_32FC1)
		throw std::runtime_error("Only 16-bit single-float format supported on input, " + possumwood::opencv::type2str((*data.get(a_in)).type()) + " found instead!");

	const cv::Mat& input = *data.get(a_in);

	const unsigned width = data.get(a_width);
	const unsigned height = data.get(a_height);

	// TODO: for parallelization to work reliably, we need to use integer atomics here, unfortunately

	cv::Mat mat = cv::Mat::zeros(height, width, CV_32FC3);
	cv::Mat norm = cv::Mat::zeros(height, width, CV_32FC3);

	tbb::parallel_for(0, input.rows, [&](int y) {
		const auto end = samples.end(y);
		for(auto it = samples.begin(y); it != end; ++it) {
			const float target_x = it->target[0] * (float)width;
			const float target_y = it->target[1] * (float)height;

			float* color = mat.ptr<float>(floor(target_y), floor(target_x));
			float* n = norm.ptr<float>(floor(target_y), floor(target_x));

			const float value = input.at<float>(it->source[1], it->source[0]);
			color[(int)it->color] += value;
			n[(int)it->color] += 1.0f;
		}
	});

	tbb::parallel_for(0, mat.rows, [&](int y) {
		for(int x=0;x<mat.cols;++x)
			for(int a=0;a<3;++a)
				if(norm.ptr<float>(y,x)[a] > 0.0f)
					mat.ptr<float>(y,x)[a] /= norm.ptr<float>(y,x)[a];
	});

	data.set(a_out, possumwood::opencv::Frame(mat));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_samples, "samples", possumwood::opencv::LightfieldSamples(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_width, "size/width", 300u);
	meta.addAttribute(a_height, "size/height", 300u);
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_samples, a_out);
	meta.addInfluence(a_width, a_out);
	meta.addInfluence(a_height, a_out);

	meta.setCompute(compute);
}


possumwood::NodeImplementation s_impl("opencv/lightfields/integrate_nearest", init);

}
