#include <actions/traits.h>
#include <possumwood_sdk/datatypes/enum.h>
#include <possumwood_sdk/datatypes/filename.h>
#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include "frame.h"
#include "scoped_error_redirect.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame;
dependency_graph::InAttr<possumwood::Enum> a_mode;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

static const std::vector<std::pair<std::string, int>> s_colorEnum{
    {"BGR2GRAY", cv::COLOR_BGR2GRAY},   {"BGR2RGB", cv::COLOR_BGR2RGB},     {"RGB2BGR", cv::COLOR_RGB2BGR},
    {"GRAY2BGR", cv::COLOR_GRAY2BGR},   {"GRAY2RGB", cv::COLOR_GRAY2RGB},   {"BGR2XYZ", cv::COLOR_BGR2XYZ},
    {"RGB2XYZ", cv::COLOR_RGB2XYZ},     {"XYZ2BGR", cv::COLOR_XYZ2BGR},     {"XYZ2RGB", cv::COLOR_XYZ2RGB},
    {"BGR2YCrCb", cv::COLOR_BGR2YCrCb}, {"RGB2YCrCb", cv::COLOR_RGB2YCrCb}, {"YCrCb2BGR", cv::COLOR_YCrCb2BGR},
    {"YCrCb2RGB", cv::COLOR_YCrCb2RGB}, {"BGR2HSV", cv::COLOR_BGR2HSV},     {"RGB2HSV", cv::COLOR_RGB2HSV},
    {"BGR2Lab", cv::COLOR_BGR2Lab},     {"RGB2Lab", cv::COLOR_RGB2Lab},     {"BGR2Luv", cv::COLOR_BGR2Luv},
    {"RGB2Luv", cv::COLOR_RGB2Luv},     {"BGR2HLS", cv::COLOR_BGR2HLS},     {"RGB2HLS", cv::COLOR_RGB2HLS},
    {"HSV2BGR", cv::COLOR_HSV2BGR},     {"HSV2RGB", cv::COLOR_HSV2RGB},     {"Lab2BGR", cv::COLOR_Lab2BGR},
    {"Lab2RGB", cv::COLOR_Lab2RGB},     {"Luv2BGR", cv::COLOR_Luv2BGR},     {"Luv2RGB", cv::COLOR_Luv2RGB},
    {"HLS2BGR", cv::COLOR_HLS2BGR},     {"HLS2RGB", cv::COLOR_HLS2RGB},     {"LBGR2Lab", cv::COLOR_LBGR2Lab},
    {"LRGB2Lab", cv::COLOR_LRGB2Lab},   {"LBGR2Luv", cv::COLOR_LBGR2Luv},   {"LRGB2Luv", cv::COLOR_LRGB2Luv},
    {"Lab2LBGR", cv::COLOR_Lab2LBGR},   {"Lab2LRGB", cv::COLOR_Lab2LRGB},   {"Luv2LBGR", cv::COLOR_Luv2LBGR},
    {"Luv2LRGB", cv::COLOR_Luv2LRGB},   {"BGR2YUV", cv::COLOR_BGR2YUV},     {"RGB2YUV", cv::COLOR_RGB2YUV},
    {"YUV2BGR", cv::COLOR_YUV2BGR},     {"YUV2RGB", cv::COLOR_YUV2RGB}};

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State state;

	cv::Mat result;

	{
		possumwood::opencv::ScopedErrorRedirect errors;
		cvtColor(*data.get(a_inFrame), result, data.get(a_mode).intValue());
		state.append(errors.state());
	}

	data.set(a_outFrame, possumwood::opencv::Frame(result));

	return state;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_mode, "mode", possumwood::Enum(s_colorEnum.begin(), s_colorEnum.end()));
	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_mode, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/color", init);

}  // namespace
