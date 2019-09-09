#include <memory>

#include <possumwood_sdk/node_implementation.h>

#include "frame.h"
#include "maths/io/vec2.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_frame;
dependency_graph::OutAttr<Imath::Vec2<unsigned>> a_size;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::Frame& input = data.get(a_frame);

	data.set(a_size, Imath::Vec2<unsigned>((*input).cols, (*input).rows));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_size, "size", Imath::Vec2<unsigned>(0, 0));

	meta.addAttribute(a_frame, "frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_frame, a_size);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/metadata", init);

}
