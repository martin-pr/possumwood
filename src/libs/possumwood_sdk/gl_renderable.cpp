#include "gl_renderable.h"

#include <cassert>
#include <stdexcept>

#include <GL/glew.h>
#include <GL/gl.h>

#include "gl.h"

namespace possumwood {

namespace {

GLuint compileShader(GLenum shaderType, const std::string& source) {
	GLuint shaderId = glCreateShader(shaderType);

	const GLchar* src = &source[0];
	glShaderSource(shaderId, 1, &src, 0);
	glCompileShader(shaderId);

	int isCompiled;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &isCompiled);

	if(isCompiled == false) {
		GLint maxLength;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &maxLength);

		std::string infoLog;
		infoLog.resize(maxLength);

		glGetShaderInfoLog(shaderId, maxLength, &maxLength, &infoLog[0]);

		glDeleteShader(shaderId);

		throw std::runtime_error("Shader compilation error:\n" + infoLog);
	}

	return shaderId;
}
}

GLRenderable::GLRenderable(GLenum drawType, const std::string& vertexShaderSrc,
                           const std::string& fragmentShaderSrc)
    : m_vao(0), m_vertexShader(0), m_fragmentShader(0), m_program(0),
      m_vertexShaderSrc(vertexShaderSrc), m_fragmentShaderSrc(fragmentShaderSrc),
      m_drawType(drawType) {
}

GLRenderable::~GLRenderable() {
	GL_CHECK_ERR;

	if(m_vertexShader != 0)
		glDeleteShader(m_vertexShader);

	if(m_fragmentShader != 0)
		glDeleteShader(m_fragmentShader);

	if(m_program != 0)
		glDeleteProgram(m_program);

	for(auto& vbo : m_vbos)
		if(vbo.second.VBOId != 0)
			glDeleteBuffers(1, &vbo.second.VBOId);

	if(m_vao != 0)
		glDeleteVertexArrays(1, &m_vao);

	GL_CHECK_ERR;
}

void GLRenderable::initialise() {
	GL_CHECK_ERR;

	// make sure nothing is initialised - this should be run only once
	assert(m_vao == 0);
	assert(m_program == 0);
	assert(m_vertexShader == 0);
	assert(m_fragmentShader == 0);

	// make an Vertex Array Object
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	// make and compile the vertex shader (throws on error)
	m_vertexShader = compileShader(GL_VERTEX_SHADER, m_vertexShaderSrc);

	// make and compile the fragment shader (throws on error)
	m_fragmentShader = compileShader(GL_FRAGMENT_SHADER, m_fragmentShaderSrc);

	// make a program and link the shaders
	m_program = glCreateProgram();

	glAttachShader(m_program, m_vertexShader);
	glAttachShader(m_program, m_fragmentShader);

	{
		GLuint counter = 0;
		for(auto& vbo : m_vbos) {
			glBindAttribLocation(m_program, counter, vbo.first.c_str());
			++counter;
		}
	}

	glLinkProgram(m_program);
	GLint isLinked;
	glGetProgramiv(m_program, GL_LINK_STATUS, (int*)&isLinked);
	if(isLinked == false) {
		GLint maxLength;
		glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &maxLength);

		std::string infoLog;
		infoLog.resize(maxLength);
		glGetProgramInfoLog(m_program, maxLength, &maxLength, &infoLog[0]);

		glDeleteProgram(m_program);

		throw std::runtime_error(infoLog);
	}

	glUseProgram(m_program);

	glBindVertexArray(0);

	GL_CHECK_ERR;
}

void GLRenderable::updateVBO(GLuint index, VBOData& data) {
	GL_CHECK_ERR;

	assert(data.needsUpdate);

	// generate the buffer, if needed
	if(data.VBOId == 0)
		glGenBuffers(1, &data.VBOId);

	// update the content
	glBindBuffer(GL_ARRAY_BUFFER, data.VBOId);
	glBufferData(GL_ARRAY_BUFFER, data.data.size() * 3 * sizeof(GLfloat), &data.data[0],
	             GL_STATIC_DRAW);
	glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(index);

	data.needsUpdate = false;

	GL_CHECK_ERR;
}

GLRenderable::VBO GLRenderable::updateVertexData(const std::string& attrName) {
	return GLRenderable::VBO(this, attrName);
}

void GLRenderable::setUniform(const std::string& name, const Imath::V3f& value) {
	m_uniforms[name] = value;
}

void GLRenderable::draw(const Imath::M44f& projection, const Imath::M44f& modelview) {
	if(!m_vbos.empty()) {
		GL_CHECK_ERR;

		// initialisation, only to be done first time around
		if(m_vao == 0) {
			assert(m_program == 0);
			assert(m_vertexShader == 0);
			assert(m_fragmentShader == 0);

			initialise();
		}

		assert(m_vao != 0);
		assert(m_program != 0);
		assert(m_vertexShader != 0);
		assert(m_fragmentShader != 0);

		GL_CHECK_ERR;

		// use the program
		glUseProgram(m_program);

		GL_CHECK_ERR;

		// upload the projection and modelview uniforms
		GLint projectionLoc = glGetUniformLocation(m_program, "in_Projection");
		assert(projectionLoc != -1);
		glUniformMatrix4fv(projectionLoc, 1, false, projection.getValue());

		GL_CHECK_ERR;

		GLint modelviewLoc = glGetUniformLocation(m_program, "in_Modelview");
		assert(modelviewLoc != -1);
		glUniformMatrix4fv(modelviewLoc, 1, false, modelview.getValue());

		GL_CHECK_ERR;

		// apply all uniforms
		for(auto uniform : m_uniforms) {
			GLint attr = glGetUniformLocation(m_program, uniform.first.c_str());
			if(attr >= 0)
				glUniform3fv(attr, 1, reinterpret_cast<float*>(&uniform.second));
		}

		GL_CHECK_ERR;

		// use the VAO
		glBindVertexArray(m_vao);

		GL_CHECK_ERR;

		// update the VBOs, if needed
		const std::size_t size = m_vbos.begin()->second.data.size();
		{
			GLuint counter = 0;
			for(auto& vbo : m_vbos) {
				if(vbo.second.needsUpdate) {
					updateVBO(counter, vbo.second);
					assert(!vbo.second.needsUpdate);
				}
				++counter;

				assert(vbo.second.VBOId != 0);
				assert(vbo.second.data.size() == size && "All VBOs should be the same size");
			}
			assert(counter == m_vbos.size());
		}

		GL_CHECK_ERR;

		// do the drawing
		glDrawArrays(m_drawType, 0, size);

		GL_CHECK_ERR;

		// and unbind the vertex array
		glBindVertexArray(0);

		GL_CHECK_ERR;

		glUseProgram(0);

		GL_CHECK_ERR;
	}
}

const std::string& GLRenderable::defaultVertexShader() {
	static const std::string s_source =
	    " \
		#version 150 \n \
		in vec3 in_Position; \n \
		uniform mat4 in_Projection; \n \
		uniform mat4 in_Modelview; \n \
		\n \
		void main(void) { \n \
		    gl_Position = in_Projection * in_Modelview * vec4(in_Position, 1.0); \n \
		}";

	return s_source;
}

const std::string& GLRenderable::defaultFragmentShader() {
	static const std::string s_source =
	    " \
		#version 150 \n \
		out vec4 out_color; \n \
		\n \
		void main(void) { \n \
		    out_color = vec4(1.0,1.0,1.0,1.0); \n \
		}";

	return s_source;
}

/////////

GLRenderable::VBO::VBO(GLRenderable* parent, const std::string& name)
    : data(std::move(parent->m_vbos[name].data)), m_parent(parent), m_name(name) {
	// lets make sure the vector in parent is useable after the move
	parent->m_vbos[name].data = std::vector<Imath::V3f>();

	parent->m_vbos[name].needsUpdate = true;
}

GLRenderable::VBO::VBO(VBO&& src) : data(std::move(src.data)), m_parent(src.m_parent) {
}

GLRenderable::VBO::~VBO() {
	m_parent->m_vbos[m_name].data = std::move(data);

	m_parent->m_vbos[m_name].needsUpdate = true;
}
}
