#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>

#include <GL/glew.h>
#include <GL/glu.h>

#include "datatypes/uniforms.inl"

namespace {

dependency_graph::InAttr<std::string> a_name;
dependency_graph::InAttr<float> a_value;
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

	const float value = data.get(a_value);

	uniforms->addUniform<float>(
		data.get(a_name),
		1,
		possumwood::Uniforms::kPerFrame,
		[value](float* data, std::size_t size, const possumwood::ViewportState& vs) {
			assert(size == 1);
			return *data = value;
		}
	);

	data.set(a_outUniforms, std::shared_ptr<const possumwood::Uniforms>(uniforms.release()));

	return result;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_name, "name", std::string("uniform_value"));
	meta.addAttribute(a_value, "value", 0.0f);
	meta.addAttribute(a_inUniforms, "in_uniforms", std::shared_ptr<const possumwood::Uniforms>(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_outUniforms, "out_uniforms", std::shared_ptr<const possumwood::Uniforms>(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_name, a_outUniforms);
	meta.addInfluence(a_value, a_outUniforms);
	meta.addInfluence(a_inUniforms, a_outUniforms);

	meta.setCompute(&compute);
}

possumwood::NodeImplementation s_impl("render/uniforms/float", init);

}
