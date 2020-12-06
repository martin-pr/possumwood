#include <GL/glew.h>
#include <GL/glut.h>
#include <OpenEXR/ImathMatrix.h>
#include <possumwood_sdk/app.h>
#include <possumwood_sdk/gl.h>
#include <possumwood_sdk/node_implementation.h>

#include <QHBoxLayout>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "datatypes/program.h"
#include "datatypes/setup.h"
#include "datatypes/uniforms.inl"
#include "datatypes/vbo.inl"
#include "datatypes/vertex_data.inl"
#include "default_shaders.h"
#include "uniforms.h"

namespace {

dependency_graph::InAttr<possumwood::Program> a_program;
dependency_graph::InAttr<possumwood::VertexData> a_vertexData;
dependency_graph::InAttr<possumwood::Uniforms> a_uniforms;
dependency_graph::InAttr<possumwood::GLSetup> a_setup;

const possumwood::Uniforms& defaultUniforms() {
	static possumwood::Uniforms s_uniforms;
	if(s_uniforms.empty())
		possumwood::addViewportUniforms(s_uniforms);

	return s_uniforms;
}

const possumwood::Program& defaultProgram() {
	static possumwood::Program s_program;

	if(s_program.id() == 0) {
		std::vector<possumwood::Shader> shaders;
		shaders.push_back(possumwood::defaultVertexShader());
		shaders.push_back(possumwood::defaultFragmentShader());

		s_program = possumwood::Program(shaders);

		s_program.link();
	}

	return s_program;
}

struct UniformProperties {
	std::string name;
	GLint size;

	bool operator<(const UniformProperties& u) const {
		if(name != u.name)
			return name < u.name;
		return size < u.size;
	}
};

// source: https://www.khronos.org/opengl/wiki/Program_Introspection#Interface_query
std::set<UniformProperties> getUniforms(GLuint prog) {
	std::set<UniformProperties> result;

	GLint numUniforms = 0;
	glGetProgramInterfaceiv(prog, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);
	const GLenum properties[5] = {GL_BLOCK_INDEX, GL_TYPE, GL_NAME_LENGTH, GL_LOCATION, GL_ARRAY_SIZE};

	for(int unif = 0; unif < numUniforms; ++unif) {
		GLint values[5];
		glGetProgramResourceiv(prog, GL_UNIFORM, unif, 5, properties, 5, NULL, values);

		// Skip any uniforms that are in a block.
		if(values[0] != -1)
			continue;

		// Get the name.
		std::vector<char> tmp(values[2]);
		glGetProgramResourceName(prog, GL_UNIFORM, unif, tmp.size(), NULL, &tmp[0]);

		result.insert(UniformProperties{std::string(&tmp.front()), values[4]});
	}

	return result;
}

// source: https://www.khronos.org/opengl/wiki/Program_Introspection#Interface_query
std::set<std::string> getInputs(GLuint prog) {
	std::set<std::string> result;

	GLint numInputs = 0;
	glGetProgramInterfaceiv(prog, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &numInputs);
	const GLenum properties[4] = {GL_TYPE, GL_NAME_LENGTH, GL_LOCATION};

	for(int unif = 0; unif < numInputs; ++unif) {
		GLint values[3];
		glGetProgramResourceiv(prog, GL_PROGRAM_INPUT, unif, 3, properties, 3, NULL, values);

		// Get the name.
		std::vector<char> tmp(values[1]);
		glGetProgramResourceName(prog, GL_PROGRAM_INPUT, unif, tmp.size(), NULL, &tmp[0]);

		result.insert(std::string(&tmp.front()));
	}

	return result;
}

struct Drawable : public possumwood::Drawable {
	explicit Drawable(dependency_graph::Values&& vals) : possumwood::Drawable(std::move(vals)), m_vao(0) {
		m_timeChangedConnection = possumwood::App::instance().onTimeChanged([](float t) { refresh(); });
	}

	~Drawable() {
		m_timeChangedConnection.disconnect();
	}

	dependency_graph::State draw() override {
		GL_CHECK_ERR;

		dependency_graph::State state;

		possumwood::Program program = values().get(a_program);
		const possumwood::VertexData& vertexData = values().get(a_vertexData);
		possumwood::Uniforms uniforms = values().get(a_uniforms);

		/// RAII scoped holder, to guarantee state restoration when it goes out of scope
		auto originalState = values().get(a_setup).apply();

		program.link();

		GL_CHECK_ERR;

		if(program.id() == 0) {
			program = defaultProgram();
			program.link();
		}

		GL_CHECK_ERR;

		if(program.state().errored())
			state.append(program.state());

		else {
			GL_CHECK_ERR;

			if(m_vao == 0)
				glGenVertexArrays(1, &m_vao);
			assert(m_vao != 0);

			GL_CHECK_ERR;

			// use the program
			glUseProgram(program.id());
			GL_CHECK_ERR;

			// feed in the uniforms
			dependency_graph::State uniState = uniforms.use(program.id(), viewport());
			GL_CHECK_ERR;

			state.append(uniState);

			// get all the uniforms and test them
			{
				const std::set<UniformProperties> usedUniforms = getUniforms(program.id());
				const std::set<std::string> inputUniforms = uniforms.names();

				std::set<UniformProperties> undefinedUniforms;
				for(auto& u : usedUniforms)
					if(inputUniforms.find(u.name) == inputUniforms.end() &&
					   !boost::algorithm::starts_with(u.name, "gl_"))
						undefinedUniforms.insert(u);

				if(!undefinedUniforms.empty()) {
					std::stringstream msg;

					msg << "These uniforms are used in the shader code, but are not defined: " << std::endl;
					for(auto& u : undefinedUniforms)
						msg << "  " << u.name << " (size=" << u.size << ")" << std::endl;
					msg << std::endl;

					msg << "List of defined uniforms: " << std::endl;
					for(auto& u : inputUniforms)
						msg << "  " << u << std::endl;

					state.addWarning(msg.str());
				}
			}

			GL_CHECK_ERR;

			// get all the inputs and test them
			{
				const std::set<std::string> usedInputs = getInputs(program.id());
				const std::set<std::string> inputInputs = vertexData.names();

				std::set<std::string> undefinedInputs;
				for(auto& u : usedInputs)
					if(inputInputs.find(u) == inputInputs.end())
						undefinedInputs.insert(u);

				if(!undefinedInputs.empty())
					state.addWarning("These shader inputs are used in the shader code, but are not defined: " +
					                 boost::algorithm::join(undefinedInputs, ", "));
			}

			GL_CHECK_ERR;

			glBindVertexArray(m_vao);

			GL_CHECK_ERR;

			// use the vertex data
			dependency_graph::State vdState = vertexData.use(program.id(), viewport());
			state.append(vdState);

			// and execute draw
			glDrawArrays(vertexData.drawElementType(), 0, vertexData.size());

			GL_CHECK_ERR;

			// disconnect everything
			glUseProgram(0);

			GL_CHECK_ERR;

			glBindVertexArray(0);

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
	meta.addAttribute(a_uniforms, "uniforms", defaultUniforms(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_setup, "setup");

	meta.setDrawable<Drawable>();
}

possumwood::NodeImplementation s_impl("render/draw", init);

}  // namespace
