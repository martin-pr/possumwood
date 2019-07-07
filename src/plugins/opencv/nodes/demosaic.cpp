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
	if(mode == "BayerBG2BGR")
		return cv::COLOR_BayerBG2BGR;
	else if(mode == "BayerGB2BGR")
		return cv::COLOR_BayerGB2BGR;
	else if(mode == "BayerRG2BGR")
		return cv::COLOR_BayerRG2BGR;
	else if(mode == "BayerGR2BGR")
		return cv::COLOR_BayerGR2BGR;
	else if(mode == "BayerBG2BGR_VNG")
		return cv::COLOR_BayerBG2BGR_VNG;
	else if(mode == "BayerGB2BGR_VNG")
		return cv::COLOR_BayerGB2BGR_VNG;
	else if(mode == "BayerRG2BGR_VNG")
		return cv::COLOR_BayerRG2BGR_VNG;
	else if(mode == "BayerGR2BGR_VNG")
		return cv::COLOR_BayerGR2BGR_VNG;
	else if(mode == "BayerBG2BGR_EA")
		return cv::COLOR_BayerBG2BGR_EA;
	else if(mode == "BayerGB2BGR_EA")
		return cv::COLOR_BayerGB2BGR_EA;
	else if(mode == "BayerRG2BGR_EA")
		return cv::COLOR_BayerRG2BGR_EA;
	else if(mode == "BayerGR2BGR_EA")
		return cv::COLOR_BayerGR2BGR_EA;

	throw std::runtime_error("Unknown conversion mode " + mode);
}

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat result;

	cvtColor(*data.get(a_inFrame), result, modeToEnum(data.get(a_mode).value()));

	data.set(a_outFrame, possumwood::opencv::Frame(result));
	
	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_mode, "mode", 
		possumwood::Enum({
			"BayerBG2BGR", "BayerGB2BGR", "BayerRG2BGR", "BayerGR2BGR", 
			"BayerBG2BGR_VNG", "BayerGB2BGR_VNG", "BayerRG2BGR_VNG", 
			"BayerGR2BGR_VNG", "BayerBG2BGR_EA", "BayerGB2BGR_EA", 
			"BayerRG2BGR_EA", "BayerGR2BGR_EA"
		}));
	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_mode, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/demosaic", init);

}
