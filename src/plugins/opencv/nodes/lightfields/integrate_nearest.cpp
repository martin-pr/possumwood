#include <actions/traits.h>
#include <lightfields/nearest_integration.h>
#include <lightfields/samples.h>
#include <possumwood_sdk/node_implementation.h>
#include <tbb/parallel_for.h>

#include <opencv2/opencv.hpp>

#include "frame.h"
#include "lightfields.h"
#include "maths/io/vec2.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::InAttr<lightfields::Samples> a_samples;
dependency_graph::InAttr<Imath::Vec2<unsigned>> a_size;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out, a_norm, a_correspondence;

dependency_graph::State compute(dependency_graph::Values& data) {
	if((*data.get(a_in)).type() != CV_32FC1 && (*data.get(a_in)).type() != CV_32FC3)
		throw std::runtime_error("Only 32-bit single-float or 32-bit 3 channel float format supported on input, " +
		                         possumwood::opencv::type2str((*data.get(a_in)).type()) + " found instead!");

	// integration
	auto tmp = lightfields::nearest::integrate(data.get(a_samples), data.get(a_size), *data.get(a_in));

	data.set(a_out, possumwood::opencv::Frame(tmp.average));
	data.set(a_norm, possumwood::opencv::Frame(tmp.samples));

	// correspondence
	auto corresp = lightfields::nearest::correspondence(data.get(a_samples), *data.get(a_in), tmp);

	data.set(a_correspondence, possumwood::opencv::Frame(corresp));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_samples, "samples", lightfields::Samples(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_size, "size", Imath::Vec2<unsigned>(300u, 300u));
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_norm, "sample_count", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_correspondence, "correspondence", possumwood::opencv::Frame(),
	                  possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_samples, a_out);
	meta.addInfluence(a_size, a_out);

	meta.addInfluence(a_in, a_norm);
	meta.addInfluence(a_samples, a_norm);
	meta.addInfluence(a_size, a_norm);

	meta.addInfluence(a_in, a_correspondence);
	meta.addInfluence(a_samples, a_correspondence);
	meta.addInfluence(a_size, a_correspondence);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/lightfields/integrate_nearest", init);

}  // namespace
