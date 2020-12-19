#include <actions/traits.h>
#include <possumwood_sdk/datatypes/enum.h>
#include <possumwood_sdk/datatypes/filename.h>
#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include "datatypes/mozaic_type.h"
#include "frame.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame;
dependency_graph::InAttr<possumwood::Enum> a_mode;
dependency_graph::InAttr<possumwood::MozaicType> a_mozaic;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

static std::vector<std::pair<std::string, int>> s_modes{{"Basic", cv::COLOR_BayerBG2BGR},
                                                        {"Variable Number of Gradients", cv::COLOR_BayerBG2BGR_VNG},
                                                        {"Edge-Aware", cv::COLOR_BayerBG2BGR_EA}};

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat result;

	cvtColor(*data.get(a_inFrame), result, data.get(a_mode).intValue() + static_cast<int>(data.get(a_mozaic)));

	data.set(a_outFrame, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_mode, "mode", possumwood::Enum(s_modes.begin(), s_modes.end()));
	meta.addAttribute(a_mozaic, "mozaic", possumwood::MozaicType::BG, possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_mode, a_outFrame);
	meta.addInfluence(a_mozaic, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/demosaic", init);

}  // namespace
