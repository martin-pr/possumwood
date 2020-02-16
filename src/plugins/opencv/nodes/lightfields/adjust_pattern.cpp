#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>
#include <tbb/parallel_for.h>

#include <actions/traits.h>

#include <lightfields/pattern.h>

#include "frame.h"
#include "tools.h"
#include "lightfields.h"

namespace {

dependency_graph::InAttr<lightfields::Pattern> a_inPattern;
dependency_graph::InAttr<float> a_scale;
dependency_graph::OutAttr<lightfields::Pattern> a_outPattern;

dependency_graph::State compute(dependency_graph::Values& data) {
	lightfields::Pattern pattern = data.get(a_inPattern);

	pattern.scale(data.get(a_scale));

	data.set(a_outPattern, pattern);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inPattern, "in_pattern", lightfields::Pattern(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_scale, "scale", 1.0f);
	meta.addAttribute(a_outPattern, "out_pattern", lightfields::Pattern(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inPattern, a_outPattern);
	meta.addInfluence(a_scale, a_outPattern);

	meta.setCompute(compute);
}


possumwood::NodeImplementation s_impl("opencv/lightfields/adjust_pattern", init);

}
