#include <actions/traits.h>
#include <lightfields/metadata.h>
#include <lightfields/pattern.h>
#include <lightfields/samples.h>
#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include "frame.h"
#include "lightfields.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::InAttr<lightfields::Metadata> a_meta;
dependency_graph::InAttr<float> a_scaleCompensation;
dependency_graph::OutAttr<lightfields::Samples> a_samples;
dependency_graph::OutAttr<float> a_lensPitch;

dependency_graph::State compute(dependency_graph::Values& data) {
	if((*data.get(a_in)).type() != CV_32FC1 && (*data.get(a_in)).type() != CV_32FC3)
		throw std::runtime_error("Only 32-bit single-float or 32-bit 3 channel float format supported on input, " +
		                         possumwood::opencv::type2str((*data.get(a_in)).type()) + " found instead!");

	// metadata instance - just a bunch of JSON dictionaries extracted from the image
	const lightfields::Metadata& meta = data.get(a_meta);

	// get the data from dicts and make it into a pattern
	lightfields::Pattern pattern = lightfields::Pattern::fromMetadata(meta);

	// compensate for metadata scale (user parameter)
	pattern.scale(data.get(a_scaleCompensation));

	// comvert the pattern to the samples instance
	data.set(a_samples, lightfields::Samples::fromPattern(pattern, *data.get(a_in)));
	data.set(a_lensPitch, pattern.lensPitch());

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_meta, "metadata", lightfields::Metadata(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_scaleCompensation, "scale_compensation", 1.0f);
	meta.addAttribute(a_samples, "samples", lightfields::Samples(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_lensPitch, "lens_pitch");

	meta.addInfluence(a_in, a_samples);
	meta.addInfluence(a_meta, a_samples);
	meta.addInfluence(a_scaleCompensation, a_samples);

	meta.addInfluence(a_in, a_lensPitch);
	meta.addInfluence(a_meta, a_lensPitch);
	meta.addInfluence(a_scaleCompensation, a_lensPitch);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/lightfields/samples_from_metadata", init);

}  // namespace
