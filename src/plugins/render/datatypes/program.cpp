#include "program.h"

namespace possumwood {

namespace {

dependency_graph::State checkProgramState(const GLuint& programId) {
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
	}

	return state;
}

}

Program::Program() {
	assert(glCreateProgram && "GLEW is probably not initialised");

	m_programId = glCreateProgram();

	m_state.addError("Program has not been linked yet.");
}

Program::~Program() {
	glDeleteProgram(m_programId);
}

const dependency_graph::State& Program::state() const {
	return m_state;
}

GLuint Program::id() const {
	return m_programId;
}

void Program::addShader(const Shader& s) {
	m_shaderIds.push_back(s.id());
}

/// links this program
void Program::link() {
	m_state = dependency_graph::State();

	for(auto& s : m_shaderIds)
		glAttachShader(m_programId, s);

	glLinkProgram(m_programId);
	m_state = checkProgramState(m_programId);

	for(auto& s : m_shaderIds)
		glDetachShader(m_programId, s);
}

}
