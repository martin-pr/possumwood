#include <actions/traits.h>
#include <possumwood_sdk/datatypes/enum.h>
#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include "sequence.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_inSequence;
dependency_graph::InAttr<possumwood::Enum> a_mode;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

enum Modes { kHorizontalID, kVerticalID };

static std::vector<std::pair<std::string, int>> s_mode{
    {"horizontal, ID-based", kHorizontalID},
    {"vertical, ID-based", kVerticalID},
};

dependency_graph::State compute(dependency_graph::Values& data) {
	// opencv creates shared (shallow) copies of its inputs -> copies are "free"
	std::vector<cv::Mat> mats;
	for(auto& f : data.get(a_inSequence))
		mats.push_back(*f);

	// result
	cv::Mat result;
	if(data.get(a_mode).intValue() == kHorizontalID)
		cv::hconcat(mats, result);
	else if(data.get(a_mode).intValue() == kVerticalID)
		cv::vconcat(mats, result);
	else
		throw std::runtime_error("Unknown mode '" + data.get(a_mode).value() + "'.");

	data.set(a_outFrame, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inSequence, "seq", possumwood::opencv::Sequence());
	meta.addAttribute(a_mode, "mode", possumwood::Enum(s_mode.begin(), s_mode.end()));
	meta.addAttribute(a_outFrame, "frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inSequence, a_outFrame);
	meta.addInfluence(a_mode, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/sequence/mosaic", init);

}  // namespace
