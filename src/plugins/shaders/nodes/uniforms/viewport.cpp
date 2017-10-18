#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>

#include <GL/glew.h>
#include <GL/glu.h>

#include "datatypes/uniforms.inl"

namespace {

dependency_graph::OutAttr<std::shared_ptr<const possumwood::Uniforms>> a_uniforms;

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State result;

	// we're drawing quads
	std::unique_ptr<possumwood::Uniforms> uniforms(new possumwood::Uniforms());

	uniforms->addUniform<float>(
		"iTime",
		possumwood::Uniforms::kPerFrame,
		[]() {
			return possumwood::App::instance().time();
		}
	);

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

	uniforms->addUniform<Imath::V2f>(
		"iResolution",
		possumwood::Uniforms::kPerDraw,
		[]() {
			GLint viewport[4];
			glGetIntegerv(GL_VIEWPORT, viewport);

			return Imath::V2f(viewport[2], viewport[3]);
		}
	);

	data.set(a_uniforms, std::shared_ptr<const possumwood::Uniforms>(uniforms.release()));

	return result;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_uniforms, "uniforms");

	meta.setCompute(&compute);
}

possumwood::NodeImplementation s_impl("shaders/uniforms/viewport", init);

}
