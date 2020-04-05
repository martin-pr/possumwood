#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include <possumwood_sdk/datatypes/enum.h>
#include <actions/traits.h>

#include "frame.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame;
dependency_graph::InAttr<possumwood::Enum> a_type;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

static const std::vector<std::pair<std::string, int>> s_modes {
	{"COLORMAP_AUTUMN", cv::COLORMAP_AUTUMN},
	{"COLORMAP_BONE", cv::COLORMAP_BONE},
	{"COLORMAP_JET", cv::COLORMAP_JET},
	{"COLORMAP_WINTER", cv::COLORMAP_WINTER},
	{"COLORMAP_RAINBOW", cv::COLORMAP_RAINBOW},
	{"COLORMAP_OCEAN", cv::COLORMAP_OCEAN},
	{"COLORMAP_SUMMER", cv::COLORMAP_SUMMER},
	{"COLORMAP_SPRING", cv::COLORMAP_SPRING},
	{"COLORMAP_COOL", cv::COLORMAP_COOL},
	{"COLORMAP_HSV", cv::COLORMAP_HSV},
	{"COLORMAP_PINK", cv::COLORMAP_PINK},
	{"COLORMAP_HOT", cv::COLORMAP_HOT},
};

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat result = (*data.get(a_inFrame)).clone();

	cv::applyColorMap(*data.get(a_inFrame), result, data.get(a_type).intValue());

	data.set(a_outFrame, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_type, "type", possumwood::Enum(s_modes.begin(), s_modes.end()));
	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_type, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/colormap", init);

}
