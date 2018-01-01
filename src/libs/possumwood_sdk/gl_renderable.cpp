#include "gl_renderable.h"

#include <cassert>

namespace possumwood {

namespace {

GLuint compileShader(GLenum shaderType, const std::string& source) {
	GLuint shaderId = glCreateShader(shaderType);

	const GLchar* const src = &source[0];
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

GLRenderable::GLRenderable()
    : m_vao(0), m_verticesVBO(0), m_vertexShader(0), m_fragmentShader(0), m_program(0),
      m_needsVBOUpdate(true) {
}

GLRenderable::~GLRenderable() {
	if(m_vertexShader != 0)
		glDeleteShader(m_vertexShader);

	if(m_fragmentShader != 0)
		glDeleteShader(m_fragmentShader);

	if(m_program != 0)
		glDeleteProgram(m_program);

	if(m_verticesVBO != 0)
		glDeleteBuffers(1, &m_verticesVBO);

	if(m_vao != 0)
		glDeleteVertexArrays(1, &m_vao);
}

void GLRenderable::initialise() {
	// make sure nothing is initialised - this should be run only once
	assert(m_vao == 0);
	assert(m_program == 0);
	assert(m_vertexShader == 0);
	assert(m_fragmentShader == 0);
	assert(m_verticesVBO == 0);

	// make an Vertex Array Object
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	// make and compile the vertex shader (throws on error)
	m_vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource());

	// make and compile the fragment shader (throws on error)
	m_fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource());

	// make a program and link the shaders
	m_program = glCreateProgram();

	glAttachShader(m_program, m_vertexShader);
	glAttachShader(m_program, m_fragmentShader);

	glBindAttribLocation(m_program, 0, "in_Position");

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
}

void GLRenderable::updateVBO() {
	assert(m_needsVBOUpdate);

	// generate the buffer, if needed
	if(m_verticesVBO == 0)
		glGenBuffers(1, &m_verticesVBO);

	// update the content
	glBindBuffer(GL_ARRAY_BUFFER, m_verticesVBO);
	glBufferData(GL_ARRAY_BUFFER, m_vboData.size() * 3 * sizeof(GLfloat), &m_vboData[0],
	             GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	m_needsVBOUpdate = false;
}

GLRenderable::VBO GLRenderable::updateVertexData() {
	return GLRenderable::VBO(this);
}

void GLRenderable::draw(const Imath::M44f& projection, const Imath::M44f& modelview) {
	// initialisation, only to be done first time around
	if(m_vao == 0) {
		assert(m_program == 0);
		assert(m_vertexShader == 0);
		assert(m_fragmentShader == 0);
		assert(m_verticesVBO == 0);

		initialise();
	}

	// use the VAO
	glBindVertexArray(m_vao);

	// update the VBO, if needed
	if(m_needsVBOUpdate)
		updateVBO();
	assert(m_verticesVBO != 0);

	// upload the projection and modelview uniforms
	GLint projectionLoc = glGetUniformLocation(m_program, "in_Projection");
	assert(projectionLoc != -1);
	glUniformMatrix4fv(projectionLoc, 1, false, projection.getValue());

	GLint modelviewLoc = glGetUniformLocation(m_program, "in_Modelview");
	assert(modelviewLoc != -1);
	glUniformMatrix4fv(modelviewLoc, 1, false, modelview.getValue());

	// do the drawing
	glDrawArrays(m_drawType, 0, m_vboData.size());

	// and unbind the vertex array
	glBindVertexArray(0);
}

const std::string& GLRenderable::vertexShaderSource() const {
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

const std::string& GLRenderable::fragmentShaderSource() const {
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

GLRenderable::VBO::VBO(GLRenderable* parent)
    : data(std::move(parent->m_vboData)), drawType(parent->m_drawType), m_parent(parent) {
	// lets make sure the vector in parent is useable after the move
	parent->m_vboData = std::vector<Imath::V3f>();

	parent->m_needsVBOUpdate = true;
}

GLRenderable::VBO::VBO(VBO&& src)
    : data(std::move(src.data)), drawType(src.drawType), m_parent(src.m_parent) {
}

GLRenderable::VBO::~VBO() {
	m_parent->m_vboData = std::move(data);
	m_parent->m_drawType = drawType;

	m_parent->m_needsVBOUpdate = true;
}
}
