#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>

#include <GL/glew.h>
#include <GL/glu.h>

#include <ImathMatrix.h>

#include "possumwood_sdk/datatypes/enum.h"
#include "datatypes/uniforms.inl"
#include "anim/datatypes/skeleton.h"

namespace {

dependency_graph::InAttr<anim::Skeleton> a_frame, a_base;
dependency_graph::InAttr<possumwood::Enum> a_mode;
dependency_graph::InAttr<possumwood::Uniforms> a_inUniforms;
dependency_graph::OutAttr<possumwood::Uniforms> a_outUniforms;

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State result;

	possumwood::Uniforms uniforms = data.get(a_inUniforms);

	anim::Skeleton frame = data.get(a_frame);
	std::vector<Imath::M44f> matrices(frame.size());

	if(data.get(a_mode).value() == "Skinning") {
		anim::Skeleton base = data.get(a_base);

		if(base.empty())
			throw std::runtime_error("Base pose input required for skinning matrices computation");

		if(not base.isCompatibleWith(frame))
			throw std::runtime_error("Base and Frame hierarchies are not compatible");

		// convert both to world space
		for(auto& b : frame)
			if(b.hasParent())
				b.tr() = b.parent().tr() * b.tr();

		for(auto& b : base)
			if(b.hasParent())
				b.tr() = b.parent().tr() * b.tr();

		// and compute skinning transforms into frame
		for(unsigned b=0;b<frame.size();++b)
			frame[b].tr() = frame[b].tr() * base[b].tr().inverse();
	}

	else {
		// convert to world space
		for(auto& b : frame)
			if(b.hasParent())
				b.tr() = b.parent().tr() * b.tr();
	}

	for(unsigned a=0;a<frame.size();++a)
		matrices[a] = frame[a].tr().toMatrix44();

	uniforms.addUniform<Imath::M44f>(
		"frame",
		frame.size(),
		possumwood::Uniforms::kPerFrame,
		[matrices](Imath::M44f* data, std::size_t size, const possumwood::ViewportState& vs) {
			for(std::size_t i=0;i<matrices.size();++i)
				/// OpenGL matrices are transposed (?!)
				data[i] = matrices[i].transposed();
		}
	);

	data.set(a_outUniforms, uniforms);

	return result;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_frame, "frame");
	meta.addAttribute(a_base, "base_pose");
	meta.addAttribute(a_mode, "mode",
	                  possumwood::Enum({"World-space", "Skinning"}));
	meta.addAttribute(a_inUniforms, "in_uniforms", possumwood::Uniforms(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_outUniforms, "out_uniforms", possumwood::Uniforms(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_frame, a_outUniforms);
	meta.addInfluence(a_base, a_outUniforms);
	meta.addInfluence(a_mode, a_outUniforms);
	meta.addInfluence(a_inUniforms, a_outUniforms);

	meta.setCompute(&compute);
}

possumwood::NodeImplementation s_impl("render/uniforms/anim_frame", init);

}
