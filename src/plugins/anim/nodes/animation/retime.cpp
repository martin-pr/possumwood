#include <possumwood_sdk/node_implementation.h>

#include "datatypes/animation.h"

namespace {

dependency_graph::InAttr<std::shared_ptr<const anim::Animation>> a_inAnim;
dependency_graph::InAttr<float> a_retime;
dependency_graph::OutAttr<std::shared_ptr<const anim::Animation>> a_outAnim;

dependency_graph::State compute(dependency_graph::Values& data) {
	std::unique_ptr<anim::Animation> result;

	std::shared_ptr<const anim::Animation> anim = data.get(a_inAnim);
	if(anim) {
		result = std::unique_ptr<anim::Animation>(new anim::Animation(*anim));
		result->fps *= data.get(a_retime);
	}

	data.set(a_outAnim, std::shared_ptr<const anim::Animation>(result.release()));

	return dependency_graph::State();

}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inAnim, "anim");
	meta.addAttribute(a_retime, "retime", 1.0f);
	meta.addAttribute(a_outAnim, "out_anim");

	meta.addInfluence(a_inAnim, a_outAnim);
	meta.addInfluence(a_retime, a_outAnim);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("anim/animation/retime", init);

}
