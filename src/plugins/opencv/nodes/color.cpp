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

int modeToEnum(const std::string& mode) {
	if(mode == "BGR2GRAY")
		return cv::COLOR_BGR2GRAY;
	else if(mode == "BGR2RGB")
		return cv::COLOR_BGR2RGB;
	else if(mode == "RGB2BGR")
		return cv::COLOR_RGB2BGR;
	else if(mode == "GRAY2BGR")
		return cv::COLOR_GRAY2BGR;
	else if(mode == "GRAY2RGB")
		return cv::COLOR_GRAY2RGB;

	throw std::runtime_error("Unknown conversion mode " + mode);
}

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat result;

	cvtColor(*data.get(a_inFrame), result, modeToEnum(data.get(a_mode).value()));

	data.set(a_outFrame, possumwood::opencv::Frame(result));
	
	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "in_frame");
	meta.addAttribute(a_mode, "mode", 
		possumwood::Enum({"BGR2GRAY", "BGR2RGB", "RGB2BGR", "GRAY2BGR", "GRAY2RGB"}));
	meta.addAttribute(a_outFrame, "out_frame");

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_mode, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/color", init);

}
