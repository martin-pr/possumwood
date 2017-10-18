#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>

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

namespace {

dependency_graph::InAttr<std::shared_ptr<const possumwood::Program>> a_program;
dependency_graph::InAttr<std::shared_ptr<const possumwood::VertexData>> a_vertexData;
dependency_graph::InAttr<std::shared_ptr<const possumwood::Uniforms>> a_uniforms;

struct Drawable : public possumwood::Drawable {
	Drawable(dependency_graph::Values&& vals) : possumwood::Drawable(std::move(vals)) {
		m_timeChangedConnection = possumwood::App::instance().onTimeChanged([this](float t) {
				refresh();
			});
	}

	~Drawable() {
		m_timeChangedConnection.disconnect();
	}

	dependency_graph::State draw() {
		dependency_graph::State state;

		std::shared_ptr<const possumwood::Program> program = values().get(a_program);
		std::shared_ptr<const possumwood::VertexData> vertexData = values().get(a_vertexData);
		std::shared_ptr<const possumwood::Uniforms> uniforms = values().get(a_uniforms);

		if(!program)
			state.addError("No GLSL program provided - cannot draw.");

		else if(program->state().errored())
			state.append(program->state());

		else if(!vertexData)
			state.addError("No vertex data provided - cannot draw.");

		else {
			// use the program
			glUseProgram(program->id());

			// feed in the uniforms
			if(uniforms)
				uniforms->use(program->id());

			// use the vertex data
			vertexData->use(program->id());

			// and execute draw
			glDrawArrays(vertexData->drawElementType(), 0, vertexData->size());

			// disconnect everything
			glUseProgram(0);
		}

		return state;
	}

	boost::signals2::connection m_timeChangedConnection;

	// possumwood::VBO<Imath::V3f> m_posBuffer;
	possumwood::VBO<Imath::V3d> m_nearPosBuffer, m_farPosBuffer;
};

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_program, "program");
	meta.addAttribute(a_vertexData, "vertex_data");
	meta.addAttribute(a_uniforms, "uniforms");

	meta.setDrawable<Drawable>();
}

possumwood::NodeImplementation s_impl("shaders/background", init);

}
