#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>

#include <GL/glew.h>
#include <GL/glu.h>

#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathEuler.h>

#include "maths/io/vec3.h"

#include "datatypes/uniforms.inl"

namespace {

dependency_graph::InAttr<std::string> a_name;
dependency_graph::InAttr<Imath::Vec3<float>> a_translation, a_rotation, a_scale;
dependency_graph::InAttr<std::shared_ptr<const possumwood::Uniforms>> a_inUniforms;
dependency_graph::OutAttr<std::shared_ptr<const possumwood::Uniforms>> a_outUniforms;

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State result;

	std::unique_ptr<possumwood::Uniforms> uniforms;

	std::shared_ptr<const possumwood::Uniforms> inUniforms = data.get(a_inUniforms);
	if(inUniforms != nullptr)
		uniforms = std::unique_ptr<possumwood::Uniforms>(new possumwood::Uniforms(*inUniforms));
	else
		uniforms = std::unique_ptr<possumwood::Uniforms>(new possumwood::Uniforms());

	const Imath::Vec3<float> tr = data.get(a_translation);
	const Imath::Vec3<float> rot = data.get(a_rotation);
	const Imath::Vec3<float> sc = data.get(a_scale);

	Imath::Matrix44<float> m1, m2, m3;
	m1 =
	    Imath::Euler<float>(Imath::Vec3<float>(rot.x * M_PI / 180.0, rot.y * M_PI / 180.0,
	                                           rot.z * M_PI / 180.0))
	        .toMatrix44();
	m2.setScale(sc);
	m3.setTranslation(tr);

	const Imath::Matrix44<float> matrix = m1 * m2 * m3;

	uniforms->addUniform<Imath::Matrix44<float>>(
		data.get(a_name),
		1,
		possumwood::Uniforms::kPerFrame,
		[matrix](Imath::Matrix44<float>* data, std::size_t size, const possumwood::Drawable::ViewportState& vs) {
			assert(size == 1);
			*data = matrix;
		}
	);

	data.set(a_outUniforms, std::shared_ptr<const possumwood::Uniforms>(uniforms.release()));

	return result;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_name, "name", std::string("uniform_value"));
	meta.addAttribute(a_translation, "translation", Imath::Vec3<float>(0, 0, 0));
	meta.addAttribute(a_rotation, "rotation", Imath::Vec3<float>(0, 0, 0));
	meta.addAttribute(a_scale, "scale", Imath::Vec3<float>(1, 1, 1));
	meta.addAttribute(a_inUniforms, "in_uniforms");
	meta.addAttribute(a_outUniforms, "out_uniforms");

	meta.addInfluence(a_name, a_outUniforms);
	meta.addInfluence(a_translation, a_outUniforms);
	meta.addInfluence(a_rotation, a_outUniforms);
	meta.addInfluence(a_scale, a_outUniforms);
	meta.addInfluence(a_inUniforms, a_outUniforms);

	meta.setCompute(&compute);
}

possumwood::NodeImplementation s_impl("render/uniforms/transform", init);

}
