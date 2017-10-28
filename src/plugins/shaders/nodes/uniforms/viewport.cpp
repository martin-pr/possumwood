#include <possumwood_sdk/node_implementation.h>

#include <GL/glew.h>
#include <GL/glu.h>

#include <ImathMatrix.h>

#include "datatypes/uniforms.inl"
#include "uniforms.h"

namespace {

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

	possumwood::addViewportUniforms(*uniforms);

	data.set(a_outUniforms, std::shared_ptr<const possumwood::Uniforms>(uniforms.release()));

	return result;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inUniforms, "in_uniforms");
	meta.addAttribute(a_outUniforms, "out_uniforms");

	meta.addInfluence(a_inUniforms, a_outUniforms);

	meta.setCompute(&compute);
}

possumwood::NodeImplementation s_impl("shaders/uniforms/viewport", init);

}
