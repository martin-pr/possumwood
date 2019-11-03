#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>
#include <possumwood_sdk/datatypes/enum.h>

#include "frame.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in1, a_in2;
dependency_graph::InAttr<possumwood::Enum> a_mode;
dependency_graph::InAttr<unsigned> a_separation;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat mat;

	const cv::Mat& in1 = *data.get(a_in1);
	const cv::Mat& in2 = *data.get(a_in2);

	if(in1.type() != in2.type())
		throw std::runtime_error("Arrange only works on images of the same type (" + possumwood::opencv::type2str(in1.type()) + " vs " + possumwood::opencv::type2str(in2.type()) + ")");

	const unsigned sep = data.get(a_separation);

	if(sep == 0) {
		if(data.get(a_mode).value() == "Horizontal")
			cv::hconcat(in1, in2, mat);
		else
			cv::vconcat(in1, in2, mat);
	}
	else {
		if(data.get(a_mode).value() == "Horizontal") {
			const cv::Mat separator = cv::Mat::zeros(in1.rows, sep, in1.type());

			cv::hconcat(in1, separator, mat);
			cv::hconcat(mat, in2, mat);
		}
		else {
			const cv::Mat separator = cv::Mat::zeros(sep, in1.cols, in1.type());

			cv::vconcat(in1, separator, mat);
			cv::vconcat(mat, in2, mat);
		}
	}

	data.set(a_out, possumwood::opencv::Frame(mat));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in1, "in_frame_1", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_in2, "in_frame_2", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_mode, "mode", possumwood::Enum({"Horizontal", "Vertical"}));
	meta.addAttribute(a_separation, "separation", 0u);
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in1, a_out);
	meta.addInfluence(a_in2, a_out);
	meta.addInfluence(a_mode, a_out);
	meta.addInfluence(a_separation, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/concat", init);

}
