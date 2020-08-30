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

}  // namespace

struct Program::Pimpl : public boost::noncopyable {
	Pimpl(const std::vector<Shader>& shaders) : m_programId(0), m_shaders(shaders) {
		m_state.addError("Program has not been linked yet.");
	}

	~Pimpl() {
		if(m_programId != 0)
			glDeleteProgram(m_programId);
	}

	void link() {
		assert(m_programId == 0);

		m_programId = glCreateProgram();

		m_state = dependency_graph::State();

		for(auto& s : m_shaders) {
			s.compile();

			if(!s.state().errored())
				glAttachShader(m_programId, s.id());
		}

		glLinkProgram(m_programId);
		m_state = checkProgramState(m_programId);

		for(auto& s : m_shaders)
			glDetachShader(m_programId, s.id());
	}

	dependency_graph::State m_state;
	GLuint m_programId;
	std::vector<Shader> m_shaders;
};

Program::Program() {
}

Program::Program(const std::vector<Shader>& shaders) : m_pimpl(new Pimpl(shaders)) {
	assert(glCreateProgram && "GLEW is probably not initialised");
}

Program::~Program() {
}

const dependency_graph::State& Program::state() const {
	if(m_pimpl != nullptr)
		return m_pimpl->m_state;

	static const dependency_graph::State s_state;
	return s_state;
}

GLuint Program::id() const {
	if(m_pimpl != nullptr)
		return m_pimpl->m_programId;
	return 0;
}

/// links this program
void Program::link() {
	if(m_pimpl != nullptr && m_pimpl->m_programId == 0)
		m_pimpl->link();
}

std::ostream& operator<<(std::ostream& out, const Program& p) {
	out << "(program " << p.id() << ")";

	return out;
}

}  // namespace possumwood
