#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>

#include <GL/glew.h>
#include <GL/glut.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QStyle>

#include <OpenEXR/ImathMatrix.h>

#include "datatypes/shader.h"

namespace {

dependency_graph::InAttr<std::shared_ptr<const possumwood::FragmentShader>> a_fragmentShader;
dependency_graph::InAttr<std::shared_ptr<const possumwood::VertexShader>> a_vertexShader;

dependency_graph::State checkProgramState(GLuint& programId) {
	dependency_graph::State state;

	GLint isLinked = 0;
	glGetProgramiv(programId, GL_LINK_STATUS, &isLinked);

	if(isLinked == GL_FALSE) {
		GLint maxLength = 0;
		glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &maxLength);

		std::string error;
		error.resize(maxLength);
		glGetProgramInfoLog(programId, maxLength, &maxLength, &error[0]);

		state.addError(error);

		glDeleteProgram(programId);
		programId = 0;
	}

	return state;
}

struct Drawable : public possumwood::Drawable {
	Drawable(dependency_graph::Values&& vals) : possumwood::Drawable(std::move(vals)) {
		m_timeChangedConnection = possumwood::App::instance().onTimeChanged([this](float t) {
				refresh();
			});
	}

	~Drawable() {
		m_timeChangedConnection.disconnect();

		if(m_posBuffer != 0) {
			glDeleteBuffers(1, &m_posBuffer);
			m_posBuffer = 0;
		}

		if(m_programId != 0)
			glDeleteProgram(m_programId);

		if(m_vao != 0)
			glDeleteVertexArrays(1, &m_vao);
	}

	dependency_graph::State draw() {
		dependency_graph::State state;

		std::shared_ptr<const possumwood::FragmentShader> fragmentShader = values().get(a_fragmentShader);
		std::shared_ptr<const possumwood::VertexShader> vertexShader = values().get(a_vertexShader);

		if(!fragmentShader)
			state.addError("No fragment shader provided - cannot draw.");

		else if(!vertexShader)
			state.addError("No vertex shader provided - cannot draw.");

		else if(fragmentShader->state().errored())
			state.append(fragmentShader->state());

		else if(vertexShader->state().errored())
			state.append(vertexShader->state());

		// use the provided fragment shader
		else {
			assert(fragmentShader->id() != 0);
			assert(vertexShader->id() != 0);

			if(m_programId == 0)
				m_programId = glCreateProgram();

			// HACK, for now - COMPILES ON EACH DRAW!
			if(true) {
				glAttachShader(m_programId, vertexShader->id());
				glAttachShader(m_programId, fragmentShader->id());

				glLinkProgram(m_programId);
				state.append(checkProgramState(m_programId));

				glDetachShader(m_programId, vertexShader->id());
				glDetachShader(m_programId, fragmentShader->id());

				// VAO will need updating
				if(m_vao != 0)
					glDeleteVertexArrays(1, &m_vao);
				m_vao = 0;
			}

			if(!state.errored()) {
				//////////////////////
				// setup - only once

				// first, the position buffer
				if(m_posBuffer == 0) {
					glGenBuffers(1, &m_posBuffer);

					glBindBuffer(GL_ARRAY_BUFFER, m_posBuffer);
					static const float vertices[] = {
						-1,-1,1,
						1,-1,1,
						1,1,1,
						-1,1,1
					};
					glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

					glBindBuffer(GL_ARRAY_BUFFER, 0);
				}

				// vertex array object to tie it all together
				if(m_vao == 0) {
					// make a new vertex array, and bind it - everything here is now
					//   stored inside that vertex array object
					glGenVertexArrays(1, &m_vao);
					glBindVertexArray(m_vao);

					// and tie it to the position attribute (will tie itself to CURRENT
					//   GL_ARRAY_BUFFER)
					glBindBuffer(GL_ARRAY_BUFFER, m_posBuffer);

					GLuint positionAttr = glGetAttribLocation(m_programId, "position");
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
				glUseProgram(m_programId);

				// feed in the uniforms
				double modelview[16];
				glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

				{
					// modelview matrix
					GLint iModelViewAttr = glGetUniformLocation(m_programId, "iModelView");
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
					GLint iProjectionAttr = glGetUniformLocation(m_programId, "iProjection");
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
					double nearPosData[12];
					gluUnProject(0, 0, 0, modelview, projection, viewport, nearPosData, nearPosData+1, nearPosData+2);
					gluUnProject(width(), 0, 0, modelview, projection, viewport, nearPosData+3, nearPosData+4, nearPosData+5);
					gluUnProject(width(), height(), 0, modelview, projection, viewport, nearPosData+6, nearPosData+7, nearPosData+8);
					gluUnProject(0, height(), 0, modelview, projection, viewport, nearPosData+9, nearPosData+10, nearPosData+11);

					if(m_nearPosBuffer == 0)
						glGenBuffers(1, &m_nearPosBuffer);
					glBindBuffer(GL_ARRAY_BUFFER, m_nearPosBuffer);

					glBufferData(GL_ARRAY_BUFFER, sizeof(nearPosData), nearPosData, GL_STATIC_DRAW);

					GLint iNearPosAttr = glGetAttribLocation(m_programId, "iNearPositionVert");
					if(iNearPosAttr >= 0) {
						glEnableVertexAttribArray(iNearPosAttr);
						glVertexAttribPointer(iNearPosAttr, 3, GL_DOUBLE, 0, 0, 0);
					}

					glBindBuffer(GL_ARRAY_BUFFER, 0);
				}

				{
					// points on the far plane, corresponding to each fragment (useful for raytracing)
					double farPosData[12];
					gluUnProject(0, 0, 1, modelview, projection, viewport, farPosData, farPosData+1, farPosData+2);
					gluUnProject(width(), 0, 1, modelview, projection, viewport, farPosData+3, farPosData+4, farPosData+5);
					gluUnProject(width(), height(), 1, modelview, projection, viewport, farPosData+6, farPosData+7, farPosData+8);
					gluUnProject(0, height(), 1, modelview, projection, viewport, farPosData+9, farPosData+10, farPosData+11);

					if(m_farPosBuffer == 0)
						glGenBuffers(1, &m_farPosBuffer);
					glBindBuffer(GL_ARRAY_BUFFER, m_farPosBuffer);

					glBufferData(GL_ARRAY_BUFFER, sizeof(farPosData), farPosData, GL_STATIC_DRAW);

					GLint iFarPosAttr = glGetAttribLocation(m_programId, "iFarPositionVert");
					if(iFarPosAttr >= 0) {
						glEnableVertexAttribArray(iFarPosAttr);
						glVertexAttribPointer(iFarPosAttr, 3, GL_DOUBLE, 0, 0, 0);

						glBindBuffer(GL_ARRAY_BUFFER, 0);
					}
				}

				{
					// viewport resolution - useful for screen-space shaders
					GLint attr = glGetUniformLocation(m_programId, "iResolution");
					if(attr >= 0) {
						float res[2] = {(float)width(), (float)height()};
						glUniform2fv(attr, 1, res);
					}
				}

				{
					// time
					GLint attr = glGetUniformLocation(m_programId, "iTime");
					if(attr >= 0)
						glUniform1f(attr, possumwood::App::instance().time());
				}

				// and execute draw
				glDrawArrays(GL_QUADS, 0, 4);

				// disconnect everything
				glUseProgram(0);
				glBindVertexArray(0);
			}
		}

		return state;
	}

	boost::signals2::connection m_timeChangedConnection;

	GLuint m_programId = 0;

	GLuint m_vao = 0;
	GLuint m_posBuffer = 0;
	GLuint m_nearPosBuffer = 0;
	GLuint m_farPosBuffer = 0;
};

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_fragmentShader, "fragment_shader");
	meta.addAttribute(a_vertexShader, "vertex_shader");

	meta.setDrawable<Drawable>();
}

possumwood::NodeImplementation s_impl("shaders/background", init);

}
