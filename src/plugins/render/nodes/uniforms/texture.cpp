#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>

#include <GL/glew.h>
#include <GL/glu.h>

#include "datatypes/uniforms.inl"

#include <images/datatypes/pixmap.h>

namespace {

template<typename PIXMAP>
struct Params {
	dependency_graph::InAttr<std::string> a_name;
	dependency_graph::InAttr<std::shared_ptr<const PIXMAP>> a_value;
	dependency_graph::InAttr<std::shared_ptr<const possumwood::Uniforms>> a_inUniforms;
	dependency_graph::OutAttr<std::shared_ptr<const possumwood::Uniforms>> a_outUniforms;
};

Params<possumwood::LDRPixmap> s_ldrParams;
Params<possumwood::HDRPixmap> s_hdrParams;

template<typename PIXMAP>
dependency_graph::State compute(dependency_graph::Values& data, Params<PIXMAP>& params) {
	dependency_graph::State result;

	std::unique_ptr<possumwood::Uniforms> uniforms;

	std::shared_ptr<const possumwood::Uniforms> inUniforms = data.get(params.a_inUniforms);
	if(inUniforms != nullptr)
		uniforms = std::unique_ptr<possumwood::Uniforms>(new possumwood::Uniforms(*inUniforms));
	else
		uniforms = std::unique_ptr<possumwood::Uniforms>(new possumwood::Uniforms());

	std::shared_ptr<const PIXMAP> value = data.get(params.a_value);

	if(value) {
		uniforms->addTexture(
			data.get(params.a_name),
			&(*value)(0,0).value()[0],
			value->width(),
			value->height()
		);
	}

	data.set(params.a_outUniforms, std::shared_ptr<const possumwood::Uniforms>(uniforms.release()));

	return result;
}

template<typename PIXMAP>
void init(possumwood::Metadata& meta, Params<PIXMAP>& params) {
	meta.addAttribute(params.a_name, "name", std::string("image"));
	meta.addAttribute(params.a_value, "value");
	meta.addAttribute(params.a_inUniforms, "in_uniforms", std::shared_ptr<const possumwood::Uniforms>(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(params.a_outUniforms, "out_uniforms", std::shared_ptr<const possumwood::Uniforms>(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(params.a_name, params.a_outUniforms);
	meta.addInfluence(params.a_value, params.a_outUniforms);
	meta.addInfluence(params.a_inUniforms, params.a_outUniforms);

	meta.setCompute([&params](dependency_graph::Values& data) {
		return compute<PIXMAP>(data, params);
	});
}

possumwood::NodeImplementation s_impl("render/uniforms/texture", [](possumwood::Metadata& meta) {
	init<possumwood::LDRPixmap>(meta, s_ldrParams);
});

possumwood::NodeImplementation s_impl_hdr("render/uniforms/texture_hdr", [](possumwood::Metadata& meta) {
	init<possumwood::HDRPixmap>(meta, s_hdrParams);
});

}
