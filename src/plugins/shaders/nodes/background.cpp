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

namespace {

dependency_graph::InAttr<std::shared_ptr<const possumwood::Program>> a_program;
dependency_graph::InAttr<std::shared_ptr<const possumwood::VertexData>> a_vertexData;

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

		if(!program)
			state.addError("No GLSL program provided - cannot draw.");

		else if(program->state().errored())
			state.append(program->state());

		else if(!vertexData)
			state.addError("No vertex data provided - cannot draw.");

		else {

			//////////
			// per-frame drawing

			// use the program
			glUseProgram(program->id());

			// feed in the uniforms
			{
				// modelview matrix
				double modelview[16];
				glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

				GLint iModelViewAttr = glGetUniformLocation(program->id(), "iModelView");
				if(iModelViewAttr >= 0) {
					float modelviewf[16];
					for(unsigned a=0;a<16;++a)
						modelviewf[a] = modelview[a];
					glUniformMatrix4fv(iModelViewAttr, 1, false, modelviewf);
				}
			}

			{
				// projection matrix
				double projection[16];
				glGetDoublev(GL_PROJECTION_MATRIX, projection);

				GLint iProjectionAttr = glGetUniformLocation(program->id(), "iProjection");
				if(iProjectionAttr >= 0) {
					float projectionf[16];
					for(unsigned a=0;a<16;++a)
						projectionf[a] = projection[a];
					glUniformMatrix4fv(iProjectionAttr, 1, false, projectionf);
				}
			}

			{
				// viewport resolution - useful for screen-space shaders
				GLint attr = glGetUniformLocation(program->id(), "iResolution");
				if(attr >= 0) {
					float res[2] = {(float)width(), (float)height()};
					glUniform2fv(attr, 1, res);
				}
			}

			{
				// time
				GLint attr = glGetUniformLocation(program->id(), "iTime");
				if(attr >= 0)
					glUniform1f(attr, possumwood::App::instance().time());
			}

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

	meta.setDrawable<Drawable>();
}

possumwood::NodeImplementation s_impl("shaders/background", init);

}
