#include <possumwood_sdk/node_implementation.h>

#include <GL/glew.h>
#include <GL/glu.h>

#include <ImathMatrix.h>

#include "datatypes/uniforms.inl"

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

	uniforms->addUniform<std::array<double, 16>>(
		"iProjection",
		possumwood::Uniforms::kPerDraw,
		[]() {
			std::array<double, 16> projection;
			glGetDoublev(GL_PROJECTION_MATRIX, projection.data());

			return projection;
		}
	);

	uniforms->addUniform<std::array<double, 16>>(
		"iModelView",
		possumwood::Uniforms::kPerDraw,
		[]() {
			std::array<double, 16> modelview;
			glGetDoublev(GL_MODELVIEW_MATRIX, modelview.data());

			return modelview;
		}
	);

	uniforms->addUniform<std::array<double, 16>>(
		"iModelViewNormal",
		possumwood::Uniforms::kPerDraw,
		[]() {
			std::array<double, 16> modelview;
			glGetDoublev(GL_MODELVIEW_MATRIX, modelview.data());

			// normal matrix has to be inverted and transposed
			Imath::M44d mv;
			for(unsigned a=0;a<16;++a)
				mv[a/4][a%4] = modelview[a];

			mv = mv.inverse().transposed();

			for(unsigned a=0;a<16;++a)
				modelview[a] = mv[a/4][a%4];

			return modelview;
		}
	);

	uniforms->addUniform<Imath::V2f>(
		"iResolution",
		possumwood::Uniforms::kPerDraw,
		[]() {
			GLint viewport[4];
			glGetIntegerv(GL_VIEWPORT, viewport);

			return Imath::V2f(viewport[2], viewport[3]);
		}
	);

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
