#include <possumwood_sdk/node_implementation.h>

#include <tbb/parallel_for.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include "maths/io/vec2.h"
#include "frame.h"
#include "lightfield_pattern.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::InAttr<lightfields::Pattern> a_pattern;
dependency_graph::InAttr<Imath::Vec2<unsigned>> a_size;
dependency_graph::InAttr<unsigned> a_elements;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out, a_norm;

void add1channel(float* color, uint16_t* n, int colorIndex, const float* value) {
	color[colorIndex] += value[0];
	n[colorIndex] += 1;
}

void add3channel(float* color, uint16_t* n, int colorIndex, const float* value) {
	color[0] += value[0];
	n[0] += 1;

	color[1] += value[1];
	n[1] += 1;

	color[2] += value[2];
	n[2] += 1;
}

dependency_graph::State compute(dependency_graph::Values& data) {
	const lightfields::Pattern& pattern = data.get(a_pattern);

	if((*data.get(a_in)).type() != CV_32FC1 && (*data.get(a_in)).type() != CV_32FC3)
		throw std::runtime_error("Only 32-bit single-float or 32-bit 3 channel float format supported on input, " + possumwood::opencv::type2str((*data.get(a_in)).type()) + " found instead!");

	auto applyFn = &add1channel;
	if((*data.get(a_in)).type() == CV_32FC3)
		applyFn = &add3channel;

	const cv::Mat& input = *data.get(a_in);

	const unsigned width = data.get(a_size)[0];
	const unsigned height = data.get(a_size)[1];
	const unsigned elements = data.get(a_elements);

	// TODO: for parallelization to work reliably, we need to use integer atomics here, unfortunately

	cv::Mat mat = cv::Mat::zeros(height*elements, width*elements, CV_32FC3);
	cv::Mat norm = cv::Mat::zeros(height*elements, width*elements, CV_16UC3);

	tbb::parallel_for(0, input.rows, [&](int y) {
		for(int x = 0; x < input.cols; ++x) {
			const Imath::V4d& sample = pattern.sample(Imath::V2i(x, y));

			const double uv_magnitude_2 = sample[2]*sample[2] + sample[3]*sample[3];
			if(uv_magnitude_2 < 1.0) {
				float target_x = sample[0] / (float)pattern.sensorResolution()[0] * (float)width;
				float target_y = sample[1] / (float)pattern.sensorResolution()[1] * (float)height;

				target_x = std::min(target_x, (float)(width-1));
				target_y = std::min(target_y, (float)(height-1));

				target_x = std::max(target_x, 0.0f);
				target_y = std::max(target_y, 0.0f);

				target_x += floor((sample[2] + 1.0) / 2.0 * (float)elements) * (float)width;
				target_y += floor((sample[3] + 1.0) / 2.0 * (float)elements) * (float)height;

				const int colorId = (x%2) + (y%2); // hardcoded bayer pattern, for now

				float* color = mat.ptr<float>(floor(target_y), floor(target_x));
				uint16_t* n = norm.ptr<uint16_t>(floor(target_y), floor(target_x));

				const float* value = input.ptr<float>(y, x);
				(*applyFn)(color, n, colorId, value);
			}
		}
	});

	tbb::parallel_for(0, mat.rows, [&](int y) {
		for(int x=0;x<mat.cols;++x)
			for(int a=0;a<3;++a)
				if(norm.ptr<uint16_t>(y,x)[a] > 0.0f)
					mat.ptr<float>(y,x)[a] /= (float)norm.ptr<uint16_t>(y,x)[a];
	});

	data.set(a_out, possumwood::opencv::Frame(mat));
	data.set(a_norm, possumwood::opencv::Frame(norm));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_pattern, "pattern", lightfields::Pattern(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_size, "size", Imath::Vec2<unsigned>(300u, 300u));
	meta.addAttribute(a_elements, "elements", 5u);
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_norm, "sample_count", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_pattern, a_out);
	meta.addInfluence(a_size, a_out);
	meta.addInfluence(a_elements, a_out);

	meta.addInfluence(a_in, a_norm);
	meta.addInfluence(a_pattern, a_norm);
	meta.addInfluence(a_size, a_out);
	meta.addInfluence(a_elements, a_norm);

	meta.setCompute(compute);
}


possumwood::NodeImplementation s_impl("opencv/lightfields/integrate_mosaic", init);

}


