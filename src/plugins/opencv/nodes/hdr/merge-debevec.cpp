#include <possumwood_sdk/node_implementation.h>

#include <opencv2/photo.hpp>

#include <actions/traits.h>

#include "sequence.h"
#include "camera_response.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_in;
dependency_graph::InAttr<possumwood::opencv::CameraResponse> a_response;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {

	cv::Ptr<cv::MergeDebevec> merger = cv::createMergeDebevec();

	// doesn't copy, just uses shared references
	std::vector<cv::Mat> inputs;
	for(auto& in : data.get(a_in))
		inputs.push_back(*in);

	cv::Mat result;
	merger->process(inputs, result, data.get(a_response).exposures(), data.get(a_response).matrix());

	data.set(a_out, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "sequence", possumwood::opencv::Sequence());
	meta.addAttribute(a_response, "camera_response");
	meta.addAttribute(a_out, "frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_response, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/hdr/merge_debevec", init);

}
