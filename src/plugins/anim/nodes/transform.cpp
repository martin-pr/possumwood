#include <possumwood_sdk/node_implementation.h>

#include <dependency_graph/values.inl>
#include <dependency_graph/attr.inl>
#include <dependency_graph/datablock.inl>
#include <dependency_graph/port.inl>
#include <dependency_graph/node.inl>

#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathEuler.h>

#include "datatypes/animation.h"
#include "3d/io/vec3.h"

namespace {

dependency_graph::InAttr<std::shared_ptr<const anim::Animation>> a_inAnim;
dependency_graph::InAttr<Imath::Vec3<float>> a_translation, a_rotation, a_scale;
dependency_graph::OutAttr<std::shared_ptr<const anim::Animation>> a_outAnim;

dependency_graph::State compute(dependency_graph::Values& data) {
	const std::shared_ptr<const anim::Animation> anim = data.get(a_inAnim);
	const Imath::Vec3<float> tr = data.get(a_translation);
	const Imath::Vec3<float> rot = data.get(a_rotation);
	const Imath::Vec3<float> sc = data.get(a_scale);

	if(anim) {
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
		std::unique_ptr<anim::Animation> newAnim(new anim::Animation(*anim));
		newAnim->base *= matrix;
		for(auto& f : newAnim->frames)
			f *= matrix;

		data.set(a_outAnim, std::shared_ptr<const anim::Animation>(newAnim.release()));
	}
	else
		data.set(a_outAnim, std::shared_ptr<const anim::Animation>());

	return dependency_graph::State();

}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inAnim, "in_anim");
	meta.addAttribute(a_translation, "translation", Imath::Vec3<float>(0, 0, 0));
	meta.addAttribute(a_rotation, "rotation", Imath::Vec3<float>(0, 0, 0));
	meta.addAttribute(a_scale, "scale", Imath::Vec3<float>(1, 1, 1));
	meta.addAttribute(a_outAnim, "out_anim");

	meta.addInfluence(a_inAnim, a_outAnim);
	meta.addInfluence(a_translation, a_outAnim);
	meta.addInfluence(a_rotation, a_outAnim);
	meta.addInfluence(a_scale, a_outAnim);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("anim/transform", init);

}
