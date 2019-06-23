#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include "frame.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame1, a_inFrame2, a_inFrame3, a_inFrame4;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

dependency_graph::State compute(dependency_graph::Values& data) {
	std::vector<cv::Mat> in;

	const cv::Mat& in1 = *data.get(a_inFrame1);
	if(in1.cols > 0 && in1.rows > 0)
		in.push_back(*data.get(a_inFrame1));

	const cv::Mat& in2 = *data.get(a_inFrame2);
	if(in2.cols > 0 && in2.rows > 0)
		in.push_back(*data.get(a_inFrame2));

	const cv::Mat& in3 = *data.get(a_inFrame3);
	if(in3.cols > 0 && in3.rows > 0)
		in.push_back(*data.get(a_inFrame3));

	const cv::Mat& in4 = *data.get(a_inFrame4);
	if(in4.cols > 0 && in4.rows > 0)
		in.push_back(*data.get(a_inFrame4));

	// handling of 2D arrays - merge needs 1, 3 or 4 valid inputs, fails with 2
	if(in.size() == 2)
		in.push_back(cv::Mat::zeros(in[0].rows, in[0].cols, in[0].type()));

	cv::Mat out;

	cv::merge(in, out);

	data.set(a_outFrame, possumwood::opencv::Frame(out));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame1, "in_1");
	meta.addAttribute(a_inFrame2, "in_2");
	meta.addAttribute(a_inFrame3, "in_3");
	meta.addAttribute(a_inFrame4, "in_4");
	meta.addAttribute(a_outFrame, "out");

	meta.addInfluence(a_inFrame1, a_outFrame);
	meta.addInfluence(a_inFrame2, a_outFrame);
	meta.addInfluence(a_inFrame3, a_outFrame);
	meta.addInfluence(a_inFrame4, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/merge", init);

}
