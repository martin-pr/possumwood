#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include <lightfields/metadata.h>
#include <lightfields/pattern.h>
#include "tools.h"
#include "lightfields.h"

namespace {

dependency_graph::InAttr<lightfields::Metadata> a_meta;
dependency_graph::OutAttr<lightfields::Pattern> a_pattern;

dependency_graph::State compute(dependency_graph::Values& data) {
	data.set(a_pattern, lightfields::Pattern::fromMetadata(data.get(a_meta)));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_meta, "metadata", lightfields::Metadata(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_pattern, "pattern", lightfields::Pattern(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_meta, a_pattern);

	meta.setCompute(compute);
}


possumwood::NodeImplementation s_impl("opencv/lightfields/pattern_from_metadata", init);

}
