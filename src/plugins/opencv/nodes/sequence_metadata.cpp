#include <possumwood_sdk/node_implementation.h>

#include <memory>

#include "maths/io/vec2.h"
#include "sequence.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_seq;
dependency_graph::OutAttr<unsigned> a_count;
dependency_graph::OutAttr<Imath::Vec2<unsigned>> a_size;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::Sequence& input = data.get(a_seq);

	unsigned count = 0;
	for(auto it = input.begin(); it != input.end(); ++it)
		++count;

	data.set(a_count, count);
	data.set(a_size, Imath::Vec2<unsigned>(input.meta().cols, input.meta().rows));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_size, "size", Imath::Vec2<unsigned>(0, 0));
	meta.addAttribute(a_count, "count", 0u);

	meta.addAttribute(a_seq, "sequence");

	meta.addInfluence(a_seq, a_count);
	meta.addInfluence(a_seq, a_size);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/sequence/metadata", init);

}  // namespace
