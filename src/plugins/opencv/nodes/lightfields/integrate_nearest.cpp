#include <possumwood_sdk/node_implementation.h>

#include <tbb/parallel_for.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include "maths/io/vec2.h"
#include "frame.h"
#include "lightfield_samples.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::InAttr<possumwood::opencv::LightfieldSamples> a_samples;
dependency_graph::InAttr<Imath::Vec2<unsigned>> a_size;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out, a_norm;

void add1channel(float* color, uint16_t* n, int colorIndex, const float* value) {
	color[colorIndex] += value[0];
	++n[colorIndex];
}

void add3channel(float* color, uint16_t* n, int colorIndex, const float* value) {
	color[0] += value[0];
	++n[0];

	color[1] += value[1];
	++n[1];

	color[2] += value[2];
	++n[2];
}

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::LightfieldSamples& samples = data.get(a_samples);

	if((*data.get(a_in)).type() != CV_32FC1 && (*data.get(a_in)).type() != CV_32FC3)
		throw std::runtime_error("Only 32-bit single-float or 32-bit 3 channel float format supported on input, " + possumwood::opencv::type2str((*data.get(a_in)).type()) + " found instead!");

	auto applyFn = &add1channel;
	if((*data.get(a_in)).type() == CV_32FC3)
		applyFn = &add3channel;

	const cv::Mat& input = *data.get(a_in);

	const unsigned width = data.get(a_size)[0];
	const unsigned height = data.get(a_size)[1];

	// TODO: for parallelization to work reliably, we need to use integer atomics here, unfortunately

	cv::Mat mat = cv::Mat::zeros(height, width, CV_32FC3);
	cv::Mat norm = cv::Mat::zeros(height, width, CV_16UC3);

	tbb::parallel_for(0, input.rows, [&](int y) {
		const auto end = samples.end(y);
		const auto begin = samples.begin(y);
		assert(begin <= end);

		for(auto it = begin; it != end; ++it) {
			const float target_x = it->target[0] * (float)width;
			const float target_y = it->target[1] * (float)height;

			// sanity checks
			assert(it->source[0] < input.rows);
			assert(it->source[1] < input.cols);
			assert(floor(target_x) >= 0);
			assert(floor(target_y) >= 0);
			assert(floor(target_x) < width);
			assert(floor(target_y) < height);

			float* color = mat.ptr<float>(floor(target_y), floor(target_x));
			uint16_t* n = norm.ptr<uint16_t>(floor(target_y), floor(target_x));

			const float* value = input.ptr<float>(it->source[1], it->source[0]);
			(*applyFn)(color, n, it->color, value);
		}
	});

	tbb::parallel_for(0, mat.rows, [&](int y) {
		for(int x=0;x<mat.cols;++x)
			for(int a=0;a<3;++a)
				if(norm.ptr<float>(y,x)[a] > 0.0f)
					mat.ptr<float>(y,x)[a] /= (float)norm.ptr<uint16_t>(y,x)[a];
	});

	data.set(a_out, possumwood::opencv::Frame(mat));
	data.set(a_norm, possumwood::opencv::Frame(norm));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_samples, "samples", possumwood::opencv::LightfieldSamples(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_size, "size", Imath::Vec2<unsigned>(300u, 300u));
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_norm, "sample_count", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_samples, a_out);
	meta.addInfluence(a_size, a_out);

	meta.addInfluence(a_in, a_norm);
	meta.addInfluence(a_samples, a_norm);
	meta.addInfluence(a_size, a_norm);

	meta.setCompute(compute);
}


possumwood::NodeImplementation s_impl("opencv/lightfields/integrate_nearest", init);

}
