#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include <possumwood_sdk/datatypes/enum.h>
#include <actions/traits.h>

#include "frame.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame;
dependency_graph::InAttr<possumwood::Enum> a_type;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

int modeToEnum(const std::string& mode) {
	if(mode == "COLORMAP_AUTUMN")
		return cv::COLORMAP_AUTUMN;
	if(mode == "COLORMAP_BONE")
		return cv::COLORMAP_BONE;
	if(mode == "COLORMAP_JET")
		return cv::COLORMAP_JET;
	if(mode == "COLORMAP_WINTER")
		return cv::COLORMAP_WINTER;
	if(mode == "COLORMAP_RAINBOW")
		return cv::COLORMAP_RAINBOW;
	if(mode == "COLORMAP_OCEAN")
		return cv::COLORMAP_OCEAN;
	if(mode == "COLORMAP_SUMMER")
		return cv::COLORMAP_SUMMER;
	if(mode == "COLORMAP_SPRING")
		return cv::COLORMAP_SPRING;
	if(mode == "COLORMAP_COOL")
		return cv::COLORMAP_COOL;
	if(mode == "COLORMAP_HSV")
		return cv::COLORMAP_HSV;
	if(mode == "COLORMAP_PINK")
		return cv::COLORMAP_PINK;
	if(mode == "COLORMAP_HOT")
		return cv::COLORMAP_HOT;

	throw std::runtime_error("Unknown colormap mode " + mode);
}

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat result = (*data.get(a_inFrame)).clone();

	int mode = modeToEnum(data.get(a_type).value());

	cv::applyColorMap(*data.get(a_inFrame), result, mode);

	data.set(a_outFrame, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_type, "type", possumwood::Enum({"COLORMAP_JET", "COLORMAP_AUTUMN", "COLORMAP_BONE", "COLORMAP_WINTER", "COLORMAP_RAINBOW",
		"COLORMAP_OCEAN", "COLORMAP_SUMMER", "COLORMAP_SPRING", "COLORMAP_COOL", "COLORMAP_HSV", "COLORMAP_PINK", "COLORMAP_HOT"}));
	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_type, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/colormap", init);

}
