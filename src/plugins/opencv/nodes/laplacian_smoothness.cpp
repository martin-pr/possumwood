#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>
#include <possumwood_sdk/datatypes/enum.h>

#include "frame.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::InAttr<possumwood::Enum> a_kernel;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

cv::Mat normalize(const cv::Mat& in) {
	cv::Mat result = in.clone();

	float sum = 0.0f;
	for(int y=0;y<in.rows;++y)
		for(int x=0;x<in.cols;++x)
			sum += std::abs(result.at<float>(y, x));

	for(int y=0;y<in.rows;++y)
		for(int x=0;x<in.cols;++x)
			result.at<float>(y, x) /= sum;

	return result;
}

static const cv::Mat kernel_4cross = normalize((cv::Mat_<float>(3,3) <<
	 0.0, -1.0,  0.0,
	-1.0,  4.0, -1.0,
	 0.0, -1.0,  0.0
));

static const cv::Mat kernel_8uniform = normalize((cv::Mat_<float>(3,3) <<
	-1.0, -1.0, -1.0,
	-1.0,  8.0, -1.0,
	-1.0, -1.0, -1.0
));

static const cv::Mat kernel_8shaped = normalize((cv::Mat_<float>(3,3) <<
	-1.0, -2.0, -1.0,
	-2.0, 12.0, -2.0,
	-1.0, -2.0, -1.0
));

static const cv::Mat kernel_24 = normalize((cv::Mat_<float>(5,5) <<
	 0.0,  0.0,  1.0,  0.0,  0.0,
	 0.0,  2.0, -8.0,  2.0,  0.0,
	 1.0, -8.0, 20.0, -8.0,  1.0,
	 0.0,  2.0, -8.0,  2.0,  0.0,
	 0.0,  0.0,  1.0,  0.0,  0.0
));

const cv::Mat& kernelEnum(const std::string& mode) {
	if(mode == "4_cross")
		return kernel_4cross;
	else if(mode == "8_uniform")
		return kernel_8uniform;
	else if(mode == "8_shaped")
		return kernel_8shaped;
	else if(mode == "24_shaped")
		return kernel_24;

	throw std::runtime_error("Unknown kernel " + mode);
}

void convolution(const cv::Mat& src, cv::Mat& tgt, const cv::Mat& kernel) {
	assert(src.rows == tgt.rows && src.cols == tgt.cols);

	for(int y=0; y<src.rows; ++y)
		for(int x=0; x<src.cols; ++x) {
			float& target = tgt.at<float>(y, x);

			// convolution
			for(int yi=0; yi<kernel.rows; ++yi)
				for(int xi=0; xi<kernel.cols; ++xi) {
					int ypos = y + yi - kernel.rows/2;
					int xpos = x + xi - kernel.cols/2;

					// handling of edges - "clip" (or "mirror", commented out for now)
					if(ypos < 0)
						// ypos = -ypos;
						ypos = 0;
					if(ypos >= src.rows)
						// ypos = (image.rows-1) - (ypos-image.rows);
						ypos = src.rows - 1;

					if(xpos < 0)
						// xpos = -xpos;
						xpos = 0;
					if(xpos >= src.cols)
						// xpos = (image.cols-1) - (xpos-image.cols);
						xpos = src.cols - 1;

					const float& k = kernel.at<float>(yi, xi);
					const float* source = src.ptr<float>(ypos, xpos);

					for(int a=0;a<src.channels();++a)
						target += k * source[a];
				}

			target = std::abs(target);
		}
}

dependency_graph::State compute(dependency_graph::Values& data) {
	const cv::Mat& in = *data.get(a_in);

	if((in.type() != CV_32FC1) && (in.type() != CV_32FC3))
		throw std::runtime_error("Only works with CV_32FC1 and CV_32FC3");

	cv::Mat out = cv::Mat::zeros(in.rows, in.cols, CV_32FC1);

	convolution(in, out, kernelEnum(data.get(a_kernel).value()));

	data.set(a_out, possumwood::opencv::Frame(out));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_kernel, "kernel",
		possumwood::Enum({"4_cross", "8_uniform", "8_shaped", "24_shaped"}));
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_kernel, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/laplacian_smoothness", init);

}
