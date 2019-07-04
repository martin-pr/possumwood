#include <possumwood_sdk/node_implementation.h>

#include <opencv2/photo.hpp>

#include <actions/traits.h>

#include "sequence.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_in;
dependency_graph::InAttr<float> a_contrastWeight, a_saturationWeight, a_exposureWeight;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {

	cv::Ptr<cv::MergeMertens> merger = cv::createMergeMertens();
	merger->setContrastWeight(data.get(a_contrastWeight));
	merger->setExposureWeight(data.get(a_exposureWeight));
	merger->setSaturationWeight(data.get(a_saturationWeight));

	// doesn't copy, just uses shared references
	std::vector<cv::Mat> inputs;
	for(auto& in : data.get(a_in))
		inputs.push_back(*in);

	cv::Mat result;
	merger->process(inputs, result);

	data.set(a_out, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	cv::Ptr<cv::MergeMertens> dummy = cv::createMergeMertens();

	meta.addAttribute(a_in, "sequence", possumwood::opencv::Sequence());
	meta.addAttribute(a_contrastWeight, "weights/contrast", dummy->getContrastWeight());
	meta.addAttribute(a_saturationWeight, "weights/saturation", dummy->getSaturationWeight());
	meta.addAttribute(a_exposureWeight, "weights/exposure", dummy->getExposureWeight());
	meta.addAttribute(a_out, "frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_contrastWeight, a_out);
	meta.addInfluence(a_saturationWeight, a_out);
	meta.addInfluence(a_exposureWeight, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/hdr/merge_mertens", init);

}
