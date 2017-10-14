#include "shader.h"

namespace possumwood {

Shader::Shader(GLenum shaderType) {
	assert(glCreateShader && "GLEW is probably not initialised");

	m_shaderId = glCreateShader(shaderType);

	m_state.addError("Shader has not been compilet yet.");
};

Shader::~Shader() {
	glDeleteShader(m_shaderId);
}

const dependency_graph::State& Shader::state() const {
	return m_state;
}

GLuint Shader::id() const {
	return m_shaderId;
}

void Shader::compile(const std::string& source) {
	// compile the shader
	const char* src = source.c_str();
	glShaderSource(m_shaderId, 1, &src, 0);
	glCompileShader(m_shaderId);

	// and check the state
	m_state = dependency_graph::State();

	GLint isCompiled = 0;
	glGetShaderiv(m_shaderId, GL_COMPILE_STATUS, &isCompiled);

	if(isCompiled == GL_FALSE) {
		GLint maxLength = 0;
		glGetShaderiv(m_shaderId, GL_INFO_LOG_LENGTH, &maxLength);

		std::string error;
		error.resize(maxLength);
		glGetShaderInfoLog(m_shaderId, maxLength, &maxLength, &error[0]);

		m_state.addError(error);

		glDeleteShader(m_shaderId);
		m_shaderId = 0;
	}
}

}
