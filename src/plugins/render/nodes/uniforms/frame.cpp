#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>

#include <GL/glew.h>
#include <GL/glu.h>

#include <ImathMatrix.h>

#include "datatypes/uniforms.inl"
#include "anim/datatypes/skeleton.h"

namespace {

dependency_graph::InAttr<anim::Skeleton> a_skeleton;
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

	anim::Skeleton frame = data.get(a_skeleton);

	// convert to world space
	for(auto& b : frame)
		if(b.hasParent())
			b.tr() = b.parent().tr() * b.tr();

	std::vector<Imath::M44f> matrices(frame.size());
	for(unsigned a=0;a<frame.size();++a)
		matrices[a] = frame[a].tr().toMatrix44();

	uniforms->addUniform<Imath::M44f>(
		"frame",
		frame.size(),
		possumwood::Uniforms::kPerFrame,
		[matrices](Imath::M44f* data, std::size_t size) {
			for(std::size_t i=0;i<matrices.size();++i)
				/// OpenGL matrices are transposed (?!)
				data[i] = matrices[i].transposed();
		}
	);

	data.set(a_outUniforms, std::shared_ptr<const possumwood::Uniforms>(uniforms.release()));

	return result;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_skeleton, "frame");
	meta.addAttribute(a_inUniforms, "in_uniforms");
	meta.addAttribute(a_outUniforms, "out_uniforms");

	meta.addInfluence(a_skeleton, a_outUniforms);
	meta.addInfluence(a_inUniforms, a_outUniforms);

	meta.setCompute(&compute);
}

possumwood::NodeImplementation s_impl("render/uniforms/anim_frame", init);

}
