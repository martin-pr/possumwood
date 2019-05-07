#include <possumwood_sdk/node_implementation.h>

#include "datatypes/animation.h"

namespace {

dependency_graph::InAttr<anim::Animation> a_inAnim;
dependency_graph::InAttr<float> a_retime;
dependency_graph::OutAttr<anim::Animation> a_outAnim;

dependency_graph::State compute(dependency_graph::Values& data) {
	anim::Animation anim = data.get(a_inAnim);

	anim.setFps(anim.fps() * data.get(a_retime));

	data.set(a_outAnim, anim);

	return dependency_graph::State();

}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inAnim, "anim", anim::Animation(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_retime, "retime", 1.0f);
	meta.addAttribute(a_outAnim, "out_anim", anim::Animation(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inAnim, a_outAnim);
	meta.addInfluence(a_retime, a_outAnim);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("anim/animation/retime", init);

}
