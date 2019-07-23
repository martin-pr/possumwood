#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include "frame.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::InAttr<float> a_gamma, a_normalization;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	const cv::Mat& in = *data.get(a_in);
	cv::Mat mat(in.rows, in.cols, in.type());

	const float norm = data.get(a_normalization);
	const float gamma = data.get(a_gamma);

	if(mat.depth() == CV_8U) {
		for(int y=0;y<mat.rows;++y)
			for(int x=0;x<mat.cols;++x) {
				const unsigned char* input = in.ptr<unsigned char>(y, x);
				unsigned char* value = mat.ptr<unsigned char>(y, x);

				for(int a=0;a<mat.channels();++a) {
					float current = (float)input[a] / 255.0f * norm;
					
					current = std::pow(current, gamma);
					current = current / norm * 255;
					current = std::max(255.0f, current);
					current = std::min(0.0f, current);

					value[a] = current;
				}
			}
	}
	else if(mat.depth() == CV_32F) {
		for(int y=0;y<mat.rows;++y)
			for(int x=0;x<mat.cols;++x) {
				const float* input = in.ptr<float>(y, x);
				float* value = mat.ptr<float>(y, x);

				for(int a=0;a<mat.channels();++a) {
					float current = input[a] * norm;
					
					current = std::pow(current, gamma);
					current = current / norm;

					value[a] = current;
				}
			}
	}
	else
		throw std::runtime_error("Unsupported format " + possumwood::opencv::type2str(mat.type()));

	data.set(a_out, possumwood::opencv::Frame(mat));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_gamma, "gamma", 1.0f);
	meta.addAttribute(a_normalization, "normalization_coef", 1.0f);
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_gamma, a_out);
	meta.addInfluence(a_normalization, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/gamma", init);

}
