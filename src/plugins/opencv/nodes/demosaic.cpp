#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filename.h>

#include <opencv2/opencv.hpp>

#include <possumwood_sdk/datatypes/enum.h>
#include <actions/traits.h>

#include "frame.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame;
dependency_graph::InAttr<possumwood::Enum> a_mode;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

static std::vector<std::pair<std::string, int>> s_modes {
	{"BayerBG2BGR", cv::COLOR_BayerBG2BGR},
	{"BayerGB2BGR", cv::COLOR_BayerGB2BGR},
	{"BayerRG2BGR", cv::COLOR_BayerRG2BGR},
	{"BayerGR2BGR", cv::COLOR_BayerGR2BGR},
	{"BayerBG2BGR_VNG", cv::COLOR_BayerBG2BGR_VNG},
	{"BayerGB2BGR_VNG", cv::COLOR_BayerGB2BGR_VNG},
	{"BayerRG2BGR_VNG", cv::COLOR_BayerRG2BGR_VNG},
	{"BayerGR2BGR_VNG", cv::COLOR_BayerGR2BGR_VNG},
	{"BayerBG2BGR_EA", cv::COLOR_BayerBG2BGR_EA},
	{"BayerGB2BGR_EA", cv::COLOR_BayerGB2BGR_EA},
	{"BayerRG2BGR_EA", cv::COLOR_BayerRG2BGR_EA},
	{"BayerGR2BGR_EA", cv::COLOR_BayerGR2BGR_EA},
};

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat result;

	cvtColor(*data.get(a_inFrame), result, data.get(a_mode).intValue());

	data.set(a_outFrame, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_mode, "mode", possumwood::Enum(s_modes.begin(), s_modes.end()));
	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_mode, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/demosaic", init);

}
