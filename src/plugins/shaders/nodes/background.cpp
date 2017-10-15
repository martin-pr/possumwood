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

namespace {

dependency_graph::InAttr<std::shared_ptr<const possumwood::Program>> a_program;

struct Drawable : public possumwood::Drawable {
	Drawable(dependency_graph::Values&& vals) : possumwood::Drawable(std::move(vals)) {
		m_timeChangedConnection = possumwood::App::instance().onTimeChanged([this](float t) {
				refresh();
			});
	}

	~Drawable() {
		m_timeChangedConnection.disconnect();

		if(m_vao != 0)
			glDeleteVertexArrays(1, &m_vao);
	}

	dependency_graph::State draw() {
		dependency_graph::State state;

		std::shared_ptr<const possumwood::Program> program = values().get(a_program);

		if(!program)
			state.addError("No GLSL program provided - cannot draw.");

		else if(program->state().errored())
			state.append(program->state());

		else {
			//////////////////////
			// setup - only once

			// first, the position buffer
			if(!m_posBuffer.isInitialised()) {
				m_posBuffer.init({
					Imath::V3f(-1,-1,1),
					Imath::V3f(1,-1,1),
					Imath::V3f(1,1,1),
					Imath::V3f(-1,1,1)
				});
			}

			// vertex array object to tie it all together
			if(m_vao == 0) {
				// make a new vertex array, and bind it - everything here is now
				//   stored inside that vertex array object
				glGenVertexArrays(1, &m_vao);
				glBindVertexArray(m_vao);

				// and tie it to the position attribute (will tie itself to CURRENT
				//   GL_ARRAY_BUFFER)
				glBindBuffer(GL_ARRAY_BUFFER, m_posBuffer.id());

				GLuint positionAttr = glGetAttribLocation(program->id(), "position");
				glEnableVertexAttribArray(positionAttr);
				glVertexAttribPointer(positionAttr, 3, GL_FLOAT, 0, 0, 0);

				glBindBuffer(GL_ARRAY_BUFFER, 0);// unnecessary?

				glBindVertexArray(0);
			}

			//////////
			// per-frame drawing

			// bind the VAO
			glBindVertexArray(m_vao);

			// use the program
			glUseProgram(program->id());

			// feed in the uniforms
			double modelview[16];
			glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

			{
				// modelview matrix
				GLint iModelViewAttr = glGetUniformLocation(program->id(), "iModelView");
				if(iModelViewAttr >= 0) {
					float modelviewf[16];
					for(unsigned a=0;a<16;++a)
						modelviewf[a] = modelview[a];
					glUniformMatrix4fv(iModelViewAttr, 1, false, modelviewf);
				}
			}

			double projection[16];
			glGetDoublev(GL_PROJECTION_MATRIX, projection);

			{
				// projection matrix
				GLint iProjectionAttr = glGetUniformLocation(program->id(), "iProjection");
				if(iProjectionAttr >= 0) {
					float projectionf[16];
					for(unsigned a=0;a<16;++a)
						projectionf[a] = projection[a];
					glUniformMatrix4fv(iProjectionAttr, 1, false, projectionf);
				}
			}

			GLint viewport[4];
			glGetIntegerv(GL_VIEWPORT, viewport);

			{
				// points on the near plane, corresponding to each fragment (useful for raytracing)
				Imath::V3d nearPosData[4];
				gluUnProject(0, 0, 0, modelview, projection, viewport, &nearPosData[0][0], &nearPosData[0][1], &nearPosData[0][2]);
				gluUnProject(width(), 0, 0, modelview, projection, viewport, &nearPosData[1][0], &nearPosData[1][1], &nearPosData[1][2]);
				gluUnProject(width(), height(), 0, modelview, projection, viewport, &nearPosData[2][0], &nearPosData[2][1], &nearPosData[2][2]);
				gluUnProject(0, height(), 0, modelview, projection, viewport, &nearPosData[3][0], &nearPosData[3][1], &nearPosData[3][2]);

				m_nearPosBuffer.init(nearPosData, nearPosData+4);

				glBindBuffer(GL_ARRAY_BUFFER, m_nearPosBuffer.id());

				GLint iNearPosAttr = glGetAttribLocation(program->id(), "iNearPositionVert");
				if(iNearPosAttr >= 0) {
					glEnableVertexAttribArray(iNearPosAttr);
					glVertexAttribPointer(iNearPosAttr, 3, GL_DOUBLE, 0, 0, 0);
				}

				glBindBuffer(GL_ARRAY_BUFFER, 0);
			}

			{
				// points on the far plane, corresponding to each fragment (useful for raytracing)
				Imath::V3d farPosData[4];
				gluUnProject(0, 0, 1, modelview, projection, viewport, &farPosData[0][0], &farPosData[0][1], &farPosData[0][2]);
				gluUnProject(width(), 0, 1, modelview, projection, viewport, &farPosData[1][0], &farPosData[1][1], &farPosData[1][2]);
				gluUnProject(width(), height(), 1, modelview, projection, viewport, &farPosData[2][0], &farPosData[2][1], &farPosData[2][2]);
				gluUnProject(0, height(), 1, modelview, projection, viewport, &farPosData[3][0], &farPosData[3][1], &farPosData[3][2]);

				m_farPosBuffer.init(farPosData, farPosData+4);

				glBindBuffer(GL_ARRAY_BUFFER, m_farPosBuffer.id());

				GLint iFarPosAttr = glGetAttribLocation(program->id(), "iFarPositionVert");
				if(iFarPosAttr >= 0) {
					glEnableVertexAttribArray(iFarPosAttr);
					glVertexAttribPointer(iFarPosAttr, 3, GL_DOUBLE, 0, 0, 0);

					glBindBuffer(GL_ARRAY_BUFFER, 0);
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

			// and execute draw
			glDrawArrays(GL_QUADS, 0, 4);

			// disconnect everything
			glUseProgram(0);
			glBindVertexArray(0);
		}

		return state;
	}

	boost::signals2::connection m_timeChangedConnection;

	GLuint m_vao = 0;

	possumwood::VBO<Imath::V3f> m_posBuffer;
	possumwood::VBO<Imath::V3d> m_nearPosBuffer, m_farPosBuffer;
};

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_program, "program");

	meta.setDrawable<Drawable>();
}

possumwood::NodeImplementation s_impl("shaders/background", init);

}
