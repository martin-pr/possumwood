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

static std::vector<std::pair<std::string, int>> s_borderType = {{"BORDER_DEFAULT", cv::BORDER_DEFAULT},
                                                                {"BORDER_CONSTANT", cv::BORDER_CONSTANT},
                                                                {"BORDER_REPLICATE", cv::BORDER_REPLICATE},
                                                                {"BORDER_REFLECT", cv::BORDER_REFLECT},
                                                                {"BORDER_WRAP", cv::BORDER_WRAP},
                                                                {"BORDER_REFLECT_101", cv::BORDER_REFLECT_101},
                                                                {"BORDER_TRANSPARENT", cv::BORDER_TRANSPARENT},
                                                                {"BORDER_REFLECT101", cv::BORDER_REFLECT101},
                                                                {"BORDER_ISOLATED", cv::BORDER_ISOLATED}};

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat result = (*data.get(a_inFrame)).clone();

	int border = data.get(a_borderType).intValue();

	cv::GaussianBlur(*data.get(a_inFrame), result, cv::Size(), data.get(a_sigmaX), data.get(a_sigmaY), border);

	data.set(a_outFrame, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_sigmaX, "sigma_x", 7.0f);
	meta.addAttribute(a_sigmaY, "sigma_y", 0.0f);
	meta.addAttribute(a_borderType, "border", possumwood::Enum(s_borderType.begin(), s_borderType.end()));
	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_sigmaX, a_outFrame);
	meta.addInfluence(a_sigmaY, a_outFrame);
	meta.addInfluence(a_borderType, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/filter/gaussian_blur", init);

}  // namespace
