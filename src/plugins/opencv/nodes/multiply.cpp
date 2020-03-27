#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/enum.h>

#include <sstream>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include "frame.h"
#include "tools.h"

namespace {

static const std::vector<std::pair<std::string, int>> s_output {{
	std::pair<std::string, int>("Automatic", -1),
	std::pair<std::string, int>("8UC1", CV_8UC1),
	std::pair<std::string, int>("8UC3", CV_8UC3),
	std::pair<std::string, int>("32FC1", CV_32FC1),
	std::pair<std::string, int>("32FC3", CV_32FC3),
}};

dependency_graph::InAttr<possumwood::opencv::Frame> a_in1, a_in2;
dependency_graph::InAttr<float> a_scale;
dependency_graph::InAttr<possumwood::Enum> a_outputType;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat mat;
	cv::multiply(*data.get(a_in1), *data.get(a_in2), mat, data.get(a_scale), data.get(a_outputType).intValue());

	data.set(a_out, possumwood::opencv::Frame(mat));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in1, "in_frame_1", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_in2, "in_frame_2", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_scale, "scale", 1.0f);
	meta.addAttribute(a_outputType, "output_type", possumwood::Enum(s_output.begin(), s_output.end()));
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in1, a_out);
	meta.addInfluence(a_in2, a_out);
	meta.addInfluence(a_scale, a_out);
	meta.addInfluence(a_outputType, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/multiply", init);

}
