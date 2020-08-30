#include <actions/traits.h>
#include <possumwood_sdk/datatypes/enum.h>
#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include "frame.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame;
dependency_graph::InAttr<float> a_sigmaX, a_sigmaY;
dependency_graph::InAttr<possumwood::Enum> a_borderType;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

int borderToEnum(const std::string& type) {
	if(type == "BORDER_DEFAULT")
		return cv::BORDER_DEFAULT;
	else if(type == "BORDER_CONSTANT")
		return cv::BORDER_CONSTANT;
	else if(type == "BORDER_REPLICATE")
		return cv::BORDER_REPLICATE;
	else if(type == "BORDER_REFLECT")
		return cv::BORDER_REFLECT;
	else if(type == "BORDER_WRAP")
		return cv::BORDER_WRAP;
	else if(type == "BORDER_REFLECT_101")
		return cv::BORDER_REFLECT_101;
	else if(type == "BORDER_TRANSPARENT")
		return cv::BORDER_TRANSPARENT;
	else if(type == "BORDER_REFLECT101")
		return cv::BORDER_REFLECT101;
	else if(type == "BORDER_ISOLATED")
		return cv::BORDER_ISOLATED;

	throw std::runtime_error("Unknown border type " + type);
}

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat result = (*data.get(a_inFrame)).clone();

	int border = borderToEnum(data.get(a_borderType).value());

	cv::GaussianBlur(*data.get(a_inFrame), result, cv::Size(), data.get(a_sigmaX), data.get(a_sigmaY), border);

	data.set(a_outFrame, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_sigmaX, "sigma_x", 7.0f);
	meta.addAttribute(a_sigmaY, "sigma_y", 0.0f);
	meta.addAttribute(
	    a_borderType, "border",
	    possumwood::Enum({"BORDER_DEFAULT", "BORDER_CONSTANT", "BORDER_REPLICATE", "BORDER_REFLECT", "BORDER_WRAP",
	                      "BORDER_REFLECT_101", "BORDER_TRANSPARENT", "BORDER_REFLECT101", "BORDER_ISOLATED"}));
	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_sigmaX, a_outFrame);
	meta.addInfluence(a_sigmaY, a_outFrame);
	meta.addInfluence(a_borderType, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/filter/gaussian_blur", init);

}  // namespace
