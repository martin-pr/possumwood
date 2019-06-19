#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include "frame.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame1, a_outFrame2, a_outFrame3, a_outFrame4;

dependency_graph::State compute(dependency_graph::Values& data) {
	std::vector<cv::Mat> mat(4);

	const cv::Mat& in = *data.get(a_inFrame);

	if(in.rows > 0 && in.cols > 0)
		cv::split(in, mat);

	data.set(a_outFrame1, possumwood::opencv::Frame(mat[0]));
	data.set(a_outFrame2, possumwood::opencv::Frame(mat[1]));
	data.set(a_outFrame3, possumwood::opencv::Frame(mat[2]));
	data.set(a_outFrame4, possumwood::opencv::Frame(mat[3]));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "in");
	meta.addAttribute(a_outFrame1, "out_1");
	meta.addAttribute(a_outFrame2, "out_2");
	meta.addAttribute(a_outFrame3, "out_3");
	meta.addAttribute(a_outFrame4, "out_4");

	meta.addInfluence(a_inFrame, a_outFrame1);
	meta.addInfluence(a_inFrame, a_outFrame2);
	meta.addInfluence(a_inFrame, a_outFrame3);
	meta.addInfluence(a_inFrame, a_outFrame4);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/split", init);

}
	