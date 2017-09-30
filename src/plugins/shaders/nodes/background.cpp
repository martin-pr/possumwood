#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>

#include <GL/glew.h>
#include <GL/glut.h>

namespace {

dependency_graph::InAttr<std::string> a_src;

struct Drawable : public possumwood::Drawable {
	Drawable(dependency_graph::Values&& vals) : possumwood::Drawable(std::move(vals)) {
		m_timeChangedConnection = possumwood::App::instance().onTimeChanged([this](float t) {
		 refresh();
		});
	}

	~Drawable() {
		m_timeChangedConnection.disconnect();

		if(m_programId != 0 && m_vertexShaderId) {
			glDetachShader(m_programId, m_vertexShaderId);
			glDeleteShader(m_vertexShaderId);
		}

		if(m_programId != 0 && m_fragmentShaderId) {
			glDetachShader(m_programId, m_fragmentShaderId);
			glDeleteShader(m_fragmentShaderId);
		}

		if(m_programId != 0)
			glDeleteProgram(m_programId);
	}

	dependency_graph::State draw() {
		dependency_graph::State state;

		if(m_vertexShaderId == 0) {
			m_vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
			const GLchar* src = "void main() {gl_Position = ftransform();}";
			glShaderSource(m_vertexShaderId, 1, &src, 0);
			glCompileShader(m_vertexShaderId);

			GLint isCompiled = 0;
			glGetShaderiv(m_vertexShaderId, GL_COMPILE_STATUS, &isCompiled);

			if(isCompiled == GL_FALSE) {
				GLint maxLength = 0;
				glGetShaderiv(m_vertexShaderId, GL_INFO_LOG_LENGTH, &maxLength);

				std::string error;
				error.resize(maxLength);
				glGetShaderInfoLog(m_vertexShaderId, maxLength, &maxLength, &error[0]);

				state.addError(error);

				glDeleteShader(m_vertexShaderId);
				m_vertexShaderId = 0;
			}
		}

		if(m_fragmentShaderId == 0)
			m_fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

		const std::string& src = values().get(a_src);
		if(src != m_currentFragmentSource) {
			const char* srcPtr = src.c_str();
			glShaderSource(m_fragmentShaderId, 1, &srcPtr, 0);
			glCompileShader(m_fragmentShaderId);

			{
				GLint isCompiled = 0;
				glGetShaderiv(m_fragmentShaderId, GL_COMPILE_STATUS, &isCompiled);

				if(isCompiled == GL_FALSE) {
					GLint maxLength = 0;
					glGetShaderiv(m_fragmentShaderId, GL_INFO_LOG_LENGTH, &maxLength);

					std::string error;
					error.resize(maxLength);
					glGetShaderInfoLog(m_fragmentShaderId, maxLength, &maxLength, &error[0]);

					state.addError(error);

					glDeleteShader(m_fragmentShaderId);
					m_fragmentShaderId = 0;
				}
			}
		}

		if(m_programId == 0)
			m_programId = glCreateProgram();

		if(src != m_currentFragmentSource && m_fragmentShaderId != 0 && m_vertexShaderId != 0) {
			glAttachShader(m_programId, m_vertexShaderId);
			glAttachShader(m_programId, m_fragmentShaderId);

			glLinkProgram(m_programId);

			GLint isLinked = 0;
			glGetProgramiv(m_programId, GL_LINK_STATUS, &isLinked);

			if(isLinked == GL_FALSE) {
				GLint maxLength = 0;
				glGetProgramiv(m_programId, GL_INFO_LOG_LENGTH, &maxLength);

				std::string error;
				error.resize(maxLength);
				glGetProgramInfoLog(m_programId, maxLength, &maxLength, &error[0]);

				state.addError(error);

				glDeleteProgram(m_programId);
				m_programId = 0;
			}

			glDetachShader(m_programId, m_vertexShaderId);
			glDetachShader(m_programId, m_fragmentShaderId);


			m_currentFragmentSource = src;
		}

		if(!state.errored()) {

			glUseProgram(m_programId);

			glBegin(GL_QUADS);
			glVertex3f(-1,-1,0);
			glVertex3f(1,-1,0);
			glVertex3f(1,1,0);
			glVertex3f(-1,1,0);
			glEnd();

			glUseProgram(0);
		}

		return state;
	}

	boost::signals2::connection m_timeChangedConnection;

	GLuint m_programId = 0;
	GLuint m_vertexShaderId = 0;
	GLuint m_fragmentShaderId = 0;

	std::string m_currentFragmentSource;
};

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_src, "source", std::string("void main() {gl_FragColor = vec4(1,0,1,1);}"));

	meta.setDrawable<Drawable>();
}

possumwood::NodeImplementation s_impl("shaders/background", init);

}
