#include "shader.h"

#include <cassert>

namespace possumwood {

struct Shader::Pimpl : public boost::noncopyable {
	Pimpl() : m_shaderId(0) {
		m_state.addError("Shader has not been compilet yet.");
	}

	~Pimpl() {
		if(m_shaderId != 0)
			glDeleteShader(m_shaderId);
	}

	void compile(GLenum shaderType, const std::string& source) {
		assert(m_shaderId == 0);

		m_shaderId = glCreateShader(shaderType);

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
		}
	}

	dependency_graph::State m_state;
	GLuint m_shaderId;
};

Shader::Shader() {
}

Shader::Shader(GLenum shaderType, const std::string& source) : m_pimpl(new Pimpl()), m_source(source), m_shaderType(shaderType) {
	assert(glCreateShader && "GLEW is probably not initialised");
};

Shader::~Shader() {
}

const dependency_graph::State& Shader::state() const {
	if(m_pimpl != nullptr)
		return m_pimpl->m_state;

	static const dependency_graph::State s_defaultState;
	return s_defaultState;
}

GLuint Shader::id() const {
	if(m_pimpl != nullptr)
		return m_pimpl->m_shaderId;
	return 0;
}

void Shader::compile() {
	// only compile if not compiled already
	if(m_pimpl != nullptr && id() == 0)
		m_pimpl->compile(m_shaderType, m_source);
}

std::ostream& operator <<(std::ostream& out, const Shader& s) {
	out << "(shader " << s.id() << ")";

	return out;
}

}
