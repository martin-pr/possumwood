#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>

#include "datatypes/animation.h"
#include "datatypes/constraints.h"

namespace {

dependency_graph::InAttr<anim::Animation> a_anim;
dependency_graph::OutAttr<anim::Constraints> a_constraints;

dependency_graph::State compute(dependency_graph::Values& data) {
	data.set(a_constraints, anim::Constraints(data.get(a_anim)));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_anim, "animation", anim::Animation(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_constraints, "constraints", anim::Constraints(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_anim, a_constraints);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("anim/constraints/from_animation", init);

}
