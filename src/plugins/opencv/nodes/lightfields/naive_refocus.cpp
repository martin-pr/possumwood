#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include "frame.h"
#include "lightfield_pattern.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::InAttr<lightfields::Pattern> a_pattern;
dependency_graph::InAttr<unsigned> a_width, a_height;
dependency_graph::InAttr<float> a_uvScale;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	const lightfields::Pattern& pattern = data.get(a_pattern);

	if((*data.get(a_in)).type() != CV_32FC1)
		throw std::runtime_error("Only 16-bit single-float format supported on input, " + possumwood::opencv::type2str((*data.get(a_in)).type()) + " found instead!");

	const cv::Mat& input = *data.get(a_in);

	const unsigned width = data.get(a_width);
	const unsigned height = data.get(a_height);

	cv::Mat mat = cv::Mat::zeros(height, width, CV_32FC3);
	cv::Mat norm = cv::Mat::zeros(height, width, CV_32FC3);

	const float uvScale = data.get(a_uvScale);

	for(int y=0;y<input.rows;++y)
		for(int x=0;x<input.cols;++x) {
			const Imath::V4f coords = pattern.sample(Imath::V2i(x, y)).pos;

			const Imath::V2f pos(coords[0] + coords[2]*uvScale, coords[1] + coords[3]*uvScale);

			const double uv_magnitude_2 = coords[2]*coords[2] + coords[3]*coords[3];

			if(uv_magnitude_2 <= 1.0 && pos[0] >= 0.0f && pos[1] >= 0.0f && pos[0] < (float)input.cols && pos[1] < (float)input.rows) {
				const float value = input.at<float>(y, x);

				const float target_x = pos[0]/input.cols * (float)width;
				const float target_y = pos[1]/input.rows * (float)height;

				float* color = mat.ptr<float>(floor(target_y), floor(target_x));
				float* n = norm.ptr<float>(floor(target_y), floor(target_x));

				// hardcoded bayer pattern - blue
				if(x%2 == 0 && y%2 == 0) {
					color[0] += value;
					n[0] += 1.0f;
				}
				// hardcoded bayer pattern - red
				else if(x%2 == 1 && y%2 == 1) {
					color[2] += value;
					n[2] += 1.0f;
				}
				// hardcoded bayer pattern - green
				else {
					color[1] += value;
					n[1] += 1.0f;
				}
			}
		}

	for(int y=0;y<mat.rows;++y)
		for(int x=0;x<mat.cols;++x)
			for(int a=0;a<3;++a)
				if(norm.ptr<float>(y,x)[a] > 0.0f)
					mat.ptr<float>(y,x)[a] /= norm.ptr<float>(y,x)[a];

	data.set(a_out, possumwood::opencv::Frame(mat));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_pattern, "pattern", lightfields::Pattern(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_width, "size/width", 300u);
	meta.addAttribute(a_height, "size/height", 300u);
	meta.addAttribute(a_uvScale, "uv_scale", 0.0f);
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_pattern, a_out);
	meta.addInfluence(a_width, a_out);
	meta.addInfluence(a_height, a_out);
	meta.addInfluence(a_uvScale, a_out);

	meta.setCompute(compute);
}


possumwood::NodeImplementation s_impl("opencv/lightfields/naive_refocus", init);

}
