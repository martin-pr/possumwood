#include <possumwood_sdk/app.h>
#include <possumwood_sdk/node_implementation.h>

#include "datatypes/animation.h"
#include "datatypes/constraints.h"

namespace {

Imath::V3f velocity(const anim::constraints::Frames& source, std::size_t frame, float fps) {
	assert(frame < source.size());

	if(source.size() <= 1)
		return Imath::V3f(0, 0, 0);

	else if(frame == 0)
		return (source[1].tr().translation - source[0].tr().translation) * fps;

	else if(frame == source.size() - 1)
		return (source[source.size() - 1].tr().translation - source[source.size() - 2].tr().translation) * fps;

	// frames in the middle - average velocity from previous and next frame differential
	return (source[frame].tr().translation - source[frame - 1].tr().translation + source[frame + 1].tr().translation -
	        source[frame].tr().translation) /
	       2.0f * fps;
}

dependency_graph::InAttr<anim::Constraints> a_inConstraints;
dependency_graph::InAttr<std::string> a_joint;
dependency_graph::InAttr<float> a_velocityThreshold;
dependency_graph::OutAttr<anim::Constraints> a_outConstraints;

dependency_graph::State compute(dependency_graph::Values& data) {
	anim::Constraints constraints = data.get(a_inConstraints);
	const float velocityThreshold = data.get(a_velocityThreshold);

	constraints.process(data.get(a_joint), [&](anim::constraints::Frames& frames) {
		for(std::size_t frameIndex = 0; frameIndex < frames.size(); ++frameIndex) {
			const float currentVelocity = velocity(frames, frameIndex, constraints.anim().fps()).length();
			frames[frameIndex].setValue(currentVelocity / velocityThreshold);
		}
	});

	std::cout << "Stationary (" << data.get(a_joint) << "):" << std::endl;
	std::cout << constraints << std::endl;
	std::cout << std::endl;

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

}  // namespace
