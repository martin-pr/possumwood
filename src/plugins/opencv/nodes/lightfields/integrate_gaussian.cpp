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
dependency_graph::InAttr<float> a_sigma2;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

void add1channel(float* color, float* n, int colorIndex, const float* value, float gauss) {
	color[colorIndex] += value[0] * gauss;
	n[colorIndex] += gauss;
}

void add3channel(float* color, float* n, int colorIndex, const float* value, float gauss) {
	color[0] += value[0] * gauss;
	n[0] += gauss;

	color[1] += value[1] * gauss;
	n[1] += gauss;

	color[2] += value[2] * gauss;
	n[2] += gauss;
}

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::LightfieldSamples& samples = data.get(a_samples);

	if((*data.get(a_in)).type() != CV_32FC1 && (*data.get(a_in)).type() != CV_32FC3)
		throw std::runtime_error("Only 32-bit single-float or 32-bit 3 channel float format supported on input, " + possumwood::opencv::type2str((*data.get(a_in)).type()) + " found instead!");

	auto applyFn = &add1channel;
	if((*data.get(a_in)).type() == CV_32FC3)
		applyFn = &add3channel;

	const cv::Mat& input = *data.get(a_in);

	const unsigned width = data.get(a_width);
	const unsigned height = data.get(a_height);

	// TODO: for parallelization to work reliably, we need to use integer atomics here, unfortunately

	cv::Mat mat = cv::Mat::zeros(height, width, CV_32FC3);
	cv::Mat norm = cv::Mat::zeros(height, width, CV_32FC3);

	const float sigma2 = data.get(a_sigma2) * (float)width / (float)input.cols;

	tbb::parallel_for(0, input.rows, [&](int y) {
		const auto end = samples.end(y);
		for(auto it = samples.begin(y); it != end; ++it) {
			const float target_x = it->target[0] * (float)width;
			const float target_y = it->target[1] * (float)height;

			int xFrom = std::max((int)floor(target_x - 3.0f*sigma2), 0);
			int xTo = std::min((int)ceil(target_x + 3.0f*sigma2 + 1.0f), (int)width);
			int yFrom = std::max((int)floor(target_y - 3.0f*sigma2), 0);
			int yTo = std::min((int)ceil(target_y + 3.0f*sigma2 + 1.0f), (int)height);

			for(int yt=yFrom; yt<yTo; ++yt) {
				for(int xt=xFrom; xt<xTo; ++xt) {
					const float dist2 = std::pow((float)xt - target_x, 2) + std::pow((float)yt - target_y, 2);
					const float gauss = std::exp(-dist2/(2.0f*sigma2));

					float* color = mat.ptr<float>(yt, xt);
					float* n = norm.ptr<float>(yt, xt);

					const float* value = input.ptr<float>(it->source[1], it->source[0]);
					(*applyFn)(color, n, it->color, value, gauss);
				}
			}
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
	meta.addAttribute(a_sigma2, "sigma2", 2.0f);
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_samples, a_out);
	meta.addInfluence(a_width, a_out);
	meta.addInfluence(a_height, a_out);
	meta.addInfluence(a_sigma2, a_out);

	meta.setCompute(compute);
}


possumwood::NodeImplementation s_impl("opencv/lightfields/integrate_gaussian", init);

}
