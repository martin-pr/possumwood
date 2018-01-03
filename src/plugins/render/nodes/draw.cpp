#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>
#include <possumwood_sdk/gl.h>

#include <GL/glew.h>
#include <GL/glut.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QStyle>

#include <OpenEXR/ImathMatrix.h>

#include "datatypes/program.h"
#include "datatypes/vbo.inl"
#include "datatypes/vertex_data.inl"
#include "datatypes/uniforms.inl"
#include "datatypes/gl_parameters.h"
#include "uniforms.h"

#include "default_shaders.h"

namespace {

dependency_graph::InAttr<std::shared_ptr<const possumwood::Program>> a_program;
dependency_graph::InAttr<std::shared_ptr<const possumwood::VertexData>> a_vertexData;
dependency_graph::InAttr<std::shared_ptr<const possumwood::Uniforms>> a_uniforms;
dependency_graph::InAttr<possumwood::GLParameters> a_params;

namespace {
	std::shared_ptr<const possumwood::Uniforms> defaultUniforms() {
		static std::shared_ptr<const possumwood::Uniforms> s_uniforms;
		if(s_uniforms == nullptr) {
			std::unique_ptr<possumwood::Uniforms> newUniforms(new possumwood::Uniforms());

			possumwood::addViewportUniforms(*newUniforms);

			s_uniforms = std::shared_ptr<const possumwood::Uniforms>(newUniforms.release());
		}

		return s_uniforms;
	}

	std::shared_ptr<const possumwood::Program> defaultProgram() {
		static std::shared_ptr<const possumwood::Program> s_program;

		if(s_program == nullptr) {
			std::unique_ptr<possumwood::Program> program(new possumwood::Program());

			program->link(possumwood::defaultVertexShader(), possumwood::defaultFragmentShader());

			assert(!program->state().errored());

			s_program = std::shared_ptr<const possumwood::Program>(program.release());
		}

		return s_program;
	}
}

struct Drawable : public possumwood::Drawable {
	Drawable(dependency_graph::Values&& vals) : possumwood::Drawable(std::move(vals)), m_vao(0) {
		m_timeChangedConnection = possumwood::App::instance().onTimeChanged([this](float t) {
				refresh();
			});
	}

	~Drawable() {
		m_timeChangedConnection.disconnect();
	}

	dependency_graph::State draw() override {
		GL_CHECK_ERR;

		dependency_graph::State state;

		std::shared_ptr<const possumwood::Program> program = values().get(a_program);
		std::shared_ptr<const possumwood::VertexData> vertexData = values().get(a_vertexData);
		std::shared_ptr<const possumwood::Uniforms> uniforms = values().get(a_uniforms);

		if(!program)
			program = defaultProgram();

		if(program->state().errored())
			state.append(program->state());

		else if(!vertexData)
			state.addError("No vertex data provided - cannot draw.");

		else if(!uniforms)
			state.addError("No uniform data provided - cannot draw.");

		else {
			GL_CHECK_ERR;

			if(m_vao == 0)
				glGenVertexArrays(1, &m_vao);
			assert(m_vao != 0);

			// apply the parameters
			auto scopedParams = values().get(a_params).apply();

			// use the program
			glUseProgram(program->id());

			// feed in the uniforms
			assert(uniforms);
			uniforms->use(program->id(), viewport());

			glBindVertexArray(m_vao);

			// use the vertex data
			vertexData->use(program->id(), viewport());

			// and execute draw
			glDrawArrays(vertexData->drawElementType(), 0, vertexData->size());

			// disconnect everything
			glUseProgram(0);

			GL_CHECK_ERR;

			glBindVertexArray(0);

			GL_CHECK_ERR;

			// and undo the params
			// glPopAttrib();

			GL_CHECK_ERR;
		}

		return state;
	}

	GLuint m_vao;

	boost::signals2::connection m_timeChangedConnection;
};

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_program, "program");
	meta.addAttribute(a_vertexData, "vertex_data");
	meta.addAttribute(a_uniforms, "uniforms", defaultUniforms());
	meta.addAttribute(a_params, "gl_params");

	meta.setDrawable<Drawable>();
}

possumwood::NodeImplementation s_impl("render/draw", init);

}
