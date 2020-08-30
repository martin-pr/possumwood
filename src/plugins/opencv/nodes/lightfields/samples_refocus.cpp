#include <actions/traits.h>
#include <lightfields/pattern.h>
#include <lightfields/samples.h>
#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include "lightfields.h"

namespace {

dependency_graph::InAttr<lightfields::Samples> a_inSamples;
dependency_graph::InAttr<float> a_uvOffset, a_uvThreshold, a_xyScale;
dependency_graph::OutAttr<lightfields::Samples> a_outSamples;

dependency_graph::State compute(dependency_graph::Values& data) {
	lightfields::Samples samples = data.get(a_inSamples);

	samples.offset(data.get(a_uvOffset));
	samples.scale(data.get(a_xyScale));
	samples.threshold(data.get(a_uvThreshold));
	samples.filterInvalid();

	data.set(a_outSamples, std::move(samples));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inSamples, "in_samples", lightfields::Samples(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_uvOffset, "uv_offset", 0.0f);
	meta.addAttribute(a_uvThreshold, "uv_threshold", 1.0f);
	meta.addAttribute(a_xyScale, "xy_scale", 1.0f);
	meta.addAttribute(a_outSamples, "out_samples", lightfields::Samples(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inSamples, a_outSamples);
	meta.addInfluence(a_uvOffset, a_outSamples);
	meta.addInfluence(a_uvThreshold, a_outSamples);
	meta.addInfluence(a_xyScale, a_outSamples);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/lightfields/samples_refocus", init);

}  // namespace
