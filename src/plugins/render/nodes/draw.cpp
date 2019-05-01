#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>
#include <possumwood_sdk/gl.h>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>

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
#include "uniforms.h"

#include "default_shaders.h"

namespace {

dependency_graph::InAttr<std::shared_ptr<const possumwood::Program>> a_program;
dependency_graph::InAttr<std::shared_ptr<const possumwood::VertexData>> a_vertexData;
dependency_graph::InAttr<std::shared_ptr<const possumwood::Uniforms>> a_uniforms;

std::shared_ptr<const possumwood::Uniforms> defaultUniforms() {
	static std::shared_ptr<const possumwood::Uniforms> s_uniforms;
	if(s_uniforms == nullptr) {
		std::unique_ptr<possumwood::Uniforms> newUniforms(new possumwood::Uniforms());

		possumwood::addViewportUniforms(*newUniforms);

		s_uniforms = std::shared_ptr<const possumwood::Uniforms>(newUniforms.release());
	}

	return s_uniforms;
}

std::shared_ptr<const possumwood::Program> defaultProgram() {
	static std::shared_ptr<const possumwood::Program> s_program;

	if(s_program == nullptr) {
		std::unique_ptr<possumwood::Program> program(new possumwood::Program());

		program->addShader(possumwood::defaultVertexShader());
		program->addShader(possumwood::defaultFragmentShader());

		program->link();

		assert(!program->state().errored());

		s_program = std::shared_ptr<const possumwood::Program>(program.release());
	}

	return s_program;
}

// source: https://www.khronos.org/opengl/wiki/Program_Introspection#Interface_query
std::set<std::string> getUniforms(GLuint prog) {
	std::set<std::string> result;

	GLint numUniforms = 0;
	glGetProgramInterfaceiv(prog, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);
	const GLenum properties[4] = {GL_BLOCK_INDEX, GL_TYPE, GL_NAME_LENGTH,
	                              GL_LOCATION};

	for(int unif = 0; unif < numUniforms; ++unif) {
		GLint values[4];
		glGetProgramResourceiv(prog, GL_UNIFORM, unif, 4, properties, 4, NULL, values);

		// Skip any uniforms that are in a block.
		if(values[0] != -1)
			continue;

		// Get the name.
		std::vector<char> tmp(values[2]);
		glGetProgramResourceName(prog, GL_UNIFORM, unif, tmp.size(), NULL, &tmp[0]);

		result.insert(std::string(&tmp.front()));
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
	Drawable(dependency_graph::Values&& vals) : possumwood::Drawable(std::move(vals)), m_vao(0) {
		m_timeChangedConnection = possumwood::App::instance().onTimeChanged([this](float t) {
				refresh();
			});
	}

	~Drawable() {
		m_timeChangedConnection.disconnect();
	}

	dependency_graph::State draw() override {
		GL_CHECK_ERR;

		dependency_graph::State state;

		std::shared_ptr<const possumwood::Program> program = values().get(a_program);
		std::shared_ptr<const possumwood::VertexData> vertexData = values().get(a_vertexData);
		std::shared_ptr<const possumwood::Uniforms> uniforms = values().get(a_uniforms);

		if(!program)
			program = defaultProgram();

		if(program->state().errored())
			state.append(program->state());

		else if(!vertexData)
			state.addError("No vertex data provided - cannot draw.");

		else if(!uniforms)
			state.addError("No uniform data provided - cannot draw.");

		else {
			GL_CHECK_ERR;

			if(m_vao == 0)
				glGenVertexArrays(1, &m_vao);
			assert(m_vao != 0);

			GL_CHECK_ERR;

			// use the program
			glUseProgram(program->id());

			GL_CHECK_ERR;

			// feed in the uniforms
			assert(uniforms);
			dependency_graph::State uniState = uniforms->use(program->id(), viewport());
			state.append(uniState);

			// get all the uniforms and test them
			{
				const std::set<std::string> usedUniforms = getUniforms(program->id());
				const std::set<std::string> inputUniforms = uniforms->names();

				std::set<std::string> undefinedUniforms;
				for(auto& u : usedUniforms)
					if(inputUniforms.find(u) == inputUniforms.end() && !boost::algorithm::starts_with(u, "gl_"))
						undefinedUniforms.insert(u);

				if(!undefinedUniforms.empty())
					state.addWarning("These uniforms are used in the shader code, but are not defined: " + boost::algorithm::join(undefinedUniforms, ", "));
			}

			GL_CHECK_ERR;

			// get all the inputs and test them
			{
				const std::set<std::string> usedInputs = getInputs(program->id());
				const std::set<std::string> inputInputs = vertexData->names();

				std::set<std::string> undefinedInputs;
				for(auto& u : usedInputs)
					if(inputInputs.find(u) == inputInputs.end())
						undefinedInputs.insert(u);

				if(!undefinedInputs.empty())
					state.addWarning("These shader inputs are used in the shader code, but are not defined: " + boost::algorithm::join(undefinedInputs, ", "));
			}

			GL_CHECK_ERR;

			glBindVertexArray(m_vao);

			GL_CHECK_ERR;

			// use the vertex data
			dependency_graph::State vdState = vertexData->use(program->id(), viewport());
			state.append(vdState);

			// and execute draw
			glDrawArrays(vertexData->drawElementType(), 0, vertexData->size());

			GL_CHECK_ERR;

			// disconnect everything
			glUseProgram(0);

			GL_CHECK_ERR;

			glBindVertexArray(0);

			GL_CHECK_ERR;

			// and undo the params
			// glPopAttrib();

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

	meta.setDrawable<Drawable>();
}

possumwood::NodeImplementation s_impl("render/draw", init);

}
