#include <possumwood_sdk/node_implementation.h>

#include <sstream>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include "frame.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in1, a_in2;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	const cv::Mat& in1 = *data.get(a_in1);
	const cv::Mat& in2 = *data.get(a_in2);

	if(in1.type() != in2.type())
		throw std::runtime_error("Cannot subtract images of different types (" + possumwood::
			opencv::type2str(in1.type()) + " and " + possumwood::opencv::type2str(in2.type()) + ")");

	if(in1.rows != in2.rows || in1.cols != in2.cols) {
		std::stringstream ss;
		ss << "Cannot subtract images of different size (" << in1.rows << "x" << in1.cols << " and " << in2.rows << "x" << in2.cols << ")";

		throw std::runtime_error(ss.str());
	}

	cv::Mat mat;

	cv::subtract(in1, in2, mat);

	data.set(a_out, possumwood::opencv::Frame(mat));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in1, "in_frame_1");
	meta.addAttribute(a_in2, "in_frame_2");
	meta.addAttribute(a_out, "out_frame");

	meta.addInfluence(a_in1, a_out);
	meta.addInfluence(a_in2, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/subtract", init);

}
