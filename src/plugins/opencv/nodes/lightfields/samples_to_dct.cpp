#include <possumwood_sdk/node_implementation.h>

#include <tbb/parallel_for.h>

#include <lightfields/dct.h>

#include "datatypes/dct.h"
#include "lightfields.h"

namespace {

dependency_graph::InAttr<lightfields::Samples> a_samples;
dependency_graph::InAttr<unsigned> a_xyRes, a_uvRes;
dependency_graph::OutAttr<lightfields::DCT> a_dct;

dependency_graph::State compute(dependency_graph::Values& data) {
	data.set(a_dct, lightfields::dct(data.get(a_samples), data.get(a_xyRes), data.get(a_uvRes)));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_samples, "samples", lightfields::Samples(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_xyRes, "resolution/xy", 10u);
	meta.addAttribute(a_uvRes, "resolution/uv", 10u);
	meta.addAttribute(a_dct, "dct", lightfields::DCT(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_samples, a_dct);
	meta.addInfluence(a_xyRes, a_dct);
	meta.addInfluence(a_uvRes, a_dct);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/lightfields/samples_to_dct", init);

}  // namespace
