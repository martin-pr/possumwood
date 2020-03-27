#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include "frame.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	const cv::Mat& src = *data.get(a_in);
	if(src.type() != CV_32FC1)
		throw std::runtime_error("Only single-channel floats supported at the moment.");
	if(src.rows == 0 || src.cols == 0)
		throw std::runtime_error("Input should be non-empty.");

	std::multimap<float, cv::Vec2i> pixels;

	for(int y=0;y<src.rows;++y)
		for(int x=0;x<src.cols;++x)
			pixels.insert(std::make_pair(src.at<float>(y, x), cv::Vec2i(y, x)));

	cv::Mat mat = cv::Mat::zeros(src.rows, src.cols, CV_32FC1);
	{
		float dest_value = 0;
		float src_value = pixels.begin()->first;

		std::size_t counter = 0;
		for(const auto& p : pixels) {
			if(src_value != p.first) {
				src_value = p.first;
				dest_value = (float)counter / (float)pixels.size();
			}

			mat.at<float>(p.second[0], p.second[1]) = dest_value;

			++counter;
		}
	}

	data.set(a_out, possumwood::opencv::Frame(mat));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/equalize_float", init);

}
