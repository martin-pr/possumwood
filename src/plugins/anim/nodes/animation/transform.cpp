#include <possumwood_sdk/node_implementation.h>

#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathEuler.h>

#include "datatypes/animation.h"
#include "maths/io/vec3.h"

namespace {

dependency_graph::InAttr<anim::Animation> a_inAnim;
dependency_graph::InAttr<Imath::Vec3<float>> a_translation, a_rotation;
dependency_graph::InAttr<float> a_scale;
dependency_graph::OutAttr<anim::Animation> a_outAnim;

dependency_graph::State compute(dependency_graph::Values& data) {
	const anim::Animation anim = data.get(a_inAnim);
	const Imath::Vec3<float> tr = data.get(a_translation);
	const Imath::Vec3<float> rot = data.get(a_rotation);
	const float sc = data.get(a_scale);

	if(!anim.empty()) {
		// assemble the transform
		Imath::Matrix44<float> m1, m2, m3;
		m1 = Imath::Euler<float>(Imath::Vec3<float>(
			rot.y * M_PI / 180.0,
			rot.x * M_PI / 180.0,
			rot.z * M_PI / 180.0
		)).toMatrix44();
		m2.setScale(sc);
		m3.setTranslation(tr);

		const Imath::Matrix44<float> matrix = m1 * m2 * m3;

		// and construct the new animation
		anim::Animation newAnim(anim.fps());
		for(auto& f : anim) {
			auto frame = f;
			frame *= matrix;
			newAnim.addFrame(frame);
		}

		data.set(a_outAnim, newAnim);
	}
	else
		data.set(a_outAnim, anim::Animation());

	return dependency_graph::State();

}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inAnim, "in_anim", anim::Animation(), possumwood::Metadata::Flags::kVertical);
	meta.addAttribute(a_translation, "translation", Imath::Vec3<float>(0, 0, 0));
	meta.addAttribute(a_rotation, "rotation", Imath::Vec3<float>(0, 0, 0));
	meta.addAttribute(a_scale, "scale", 1.0f);
	meta.addAttribute(a_outAnim, "out_anim", anim::Animation(), possumwood::Metadata::Flags::kVertical);

	meta.addInfluence(a_inAnim, a_outAnim);
	meta.addInfluence(a_translation, a_outAnim);
	meta.addInfluence(a_rotation, a_outAnim);
	meta.addInfluence(a_scale, a_outAnim);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("anim/animation/transform", init);

}
