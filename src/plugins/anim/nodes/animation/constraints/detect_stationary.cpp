#include <possumwood_sdk/node_implementation.h>

#include <possumwood_sdk/app.h>

#include "datatypes/constraints.h"

namespace {

dependency_graph::InAttr<anim::Constraints> a_inConstraints;
dependency_graph::InAttr<std::string> a_joint;
dependency_graph::InAttr<float> a_velocityThreshold;
dependency_graph::OutAttr<anim::Constraints> a_outConstraints;

dependency_graph::State compute(dependency_graph::Values& data) {
	anim::Constraints constraints = data.get(a_inConstraints);

	constraints.addVelocityConstraint(data.get(a_joint), data.get(a_velocityThreshold));

	data.set(a_outConstraints, constraints);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inConstraints, "in_constraints", anim::Constraints(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_joint, "joint", std::string());
	meta.addAttribute(a_velocityThreshold, "velocity_threshold", 0.1f);

	meta.addAttribute(a_outConstraints, "out_constraints", anim::Constraints(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inConstraints, a_outConstraints);
	meta.addInfluence(a_joint, a_outConstraints);
	meta.addInfluence(a_velocityThreshold, a_outConstraints);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("anim/constraints/detect_stationary", init);

}
