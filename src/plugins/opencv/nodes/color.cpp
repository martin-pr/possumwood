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
	else if(mode == "BGR2XYZ")
		return cv::COLOR_BGR2XYZ;
	else if(mode == "RGB2XYZ")
		return cv::COLOR_RGB2XYZ;
	else if(mode == "XYZ2BGR")
		return cv::COLOR_XYZ2BGR;
	else if(mode == "XYZ2RGB")
		return cv::COLOR_XYZ2RGB;
	else if(mode == "BGR2YCrCb")
		return cv::COLOR_BGR2YCrCb;
	else if(mode == "RGB2YCrCb")
		return cv::COLOR_RGB2YCrCb;
	else if(mode == "YCrCb2BGR")
		return cv::COLOR_YCrCb2BGR;
	else if(mode == "YCrCb2RGB")
		return cv::COLOR_YCrCb2RGB;
	else if(mode == "BGR2HSV")
		return cv::COLOR_BGR2HSV;
	else if(mode == "RGB2HSV")
		return cv::COLOR_RGB2HSV;
	else if(mode == "BGR2Lab")
		return cv::COLOR_BGR2Lab;
	else if(mode == "RGB2Lab")
		return cv::COLOR_RGB2Lab;
	else if(mode == "BGR2Luv")
		return cv::COLOR_BGR2Luv;
	else if(mode == "RGB2Luv")
		return cv::COLOR_RGB2Luv;
	else if(mode == "BGR2HLS")
		return cv::COLOR_BGR2HLS;
	else if(mode == "RGB2HLS")
		return cv::COLOR_RGB2HLS;
	else if(mode == "HSV2BGR")
		return cv::COLOR_HSV2BGR;
	else if(mode == "HSV2RGB")
		return cv::COLOR_HSV2RGB;
	else if(mode == "Lab2BGR")
		return cv::COLOR_Lab2BGR;
	else if(mode == "Lab2RGB")
		return cv::COLOR_Lab2RGB;
	else if(mode == "Luv2BGR")
		return cv::COLOR_Luv2BGR;
	else if(mode == "Luv2RGB")
		return cv::COLOR_Luv2RGB;
	else if(mode == "HLS2BGR")
		return cv::COLOR_HLS2BGR;
	else if(mode == "HLS2RGB")
		return cv::COLOR_HLS2RGB;
	else if(mode == "LBGR2Lab")
		return cv::COLOR_LBGR2Lab;
	else if(mode == "LRGB2Lab")
		return cv::COLOR_LRGB2Lab;
	else if(mode == "LBGR2Luv")
		return cv::COLOR_LBGR2Luv;
	else if(mode == "LRGB2Luv")
		return cv::COLOR_LRGB2Luv;
	else if(mode == "Lab2LBGR")
		return cv::COLOR_Lab2LBGR;
	else if(mode == "Lab2LRGB")
		return cv::COLOR_Lab2LRGB;
	else if(mode == "Luv2LBGR")
		return cv::COLOR_Luv2LBGR;
	else if(mode == "Luv2LRGB")
		return cv::COLOR_Luv2LRGB;
	else if(mode == "BGR2YUV")
		return cv::COLOR_BGR2YUV;
	else if(mode == "RGB2YUV")
		return cv::COLOR_RGB2YUV;
	else if(mode == "YUV2BGR")
		return cv::COLOR_YUV2BGR;
	else if(mode == "YUV2RGB")
		return cv::COLOR_YUV2RGB;

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
		possumwood::Enum({
			"BGR2GRAY", "BGR2RGB", "RGB2BGR", "GRAY2BGR", "GRAY2RGB", "BGR2XYZ", 
			"RGB2XYZ", "XYZ2BGR", "XYZ2RGB", "BGR2YCrCb", "RGB2YCrCb", "YCrCb2BGR", 
			"YCrCb2RGB", "BGR2HSV", "RGB2HSV", "BGR2Lab", "RGB2Lab", "BGR2Luv", 
			"RGB2Luv", "BGR2HLS", "RGB2HLS", "HSV2BGR", "HSV2RGB", "Lab2BGR", 
			"Lab2RGB", "Luv2BGR", "Luv2RGB", "HLS2BGR", "HLS2RGB", "LBGR2Lab", 
			"LRGB2Lab", "LBGR2Luv", "LRGB2Luv", "Lab2LBGR", "Lab2LRGB", "Luv2LBGR", 
			"Luv2LRGB", "BGR2YUV", "RGB2YUV", "YUV2BGR", "YUV2RGB"
		}));
	meta.addAttribute(a_outFrame, "out_frame");

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_mode, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/color", init);

}
