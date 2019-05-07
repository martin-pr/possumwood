#include <possumwood_sdk/node_implementation.h>

#include <utility>
#include <random>

#include <possumwood_sdk/app.h>

#include "datatypes/skeleton.h"
#include "datatypes/animation.h"
#include "datatypes/motion_graph.h"
#include "ui/motion_map.h"

namespace {

dependency_graph::OutAttr<anim::Animation> a_outAnim;
dependency_graph::InAttr<anim::MotionGraph> a_mgraph;
dependency_graph::InAttr<unsigned> a_targetLength, a_randomSeed;
dependency_graph::InAttr<float> a_transitionProbability;

dependency_graph::State compute(dependency_graph::Values& values) {
	if(!values.get(a_mgraph).empty())
		values.set(a_outAnim, values.get(a_mgraph).randomWalk(values.get(a_targetLength), values.get(a_transitionProbability), values.get(a_randomSeed)));
	else
		values.set(a_outAnim, anim::Animation());

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	std::random_device rd;

	meta.addAttribute(a_outAnim, "out_anim", anim::Animation(24.0f), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_mgraph, "motion_graph", anim::MotionGraph(), possumwood::AttrFlags::kVertical);

	meta.addAttribute(a_targetLength, "target_length", 200u);
	meta.addAttribute(a_randomSeed, "random_seed", unsigned(rd() % 10000));
	meta.addAttribute(a_transitionProbability, "transition_probability", 0.1f);

	meta.addInfluence(a_mgraph, a_outAnim);
	meta.addInfluence(a_targetLength, a_outAnim);
	meta.addInfluence(a_transitionProbability, a_outAnim);
	meta.addInfluence(a_randomSeed, a_outAnim);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("anim/animation/motiongraph/random_walk", init);

}
