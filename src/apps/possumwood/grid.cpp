#include "grid.h"

#include <vector>
#include <cassert>

namespace possumwood {

namespace {

static const GLchar* fragmentSrc = " \
#version 150 \n \
in float color; \n \
\n \
void main(void) { \n \
	float c = 0.5*color + 0.5; \n \
    gl_FragColor = vec4(c,c,c,1.0); \n \
}";

static const GLchar* vertexSrc = " \
#version 150 \n \
in vec3 in_Position; \n \
uniform mat4 in_Projection; \n \
uniform mat4 in_Modelview; \n \
out float color; \n \
\n \
void main(void) { \n \
    gl_Position = in_Projection * in_Modelview * vec4(in_Position.x, 0.0, in_Position.y, 1.0); \n \
    color = in_Position.z; \n \
}";

const std::vector<Imath::V3f>& makeGrid() {
	static std::vector<Imath::V3f> result;

	if(result.empty()) {
		for(int a = -10; a <= 10; ++a) {
			result.push_back(Imath::V3f(a, -10.0f, a%10==0));
			result.push_back(Imath::V3f(a, 10.0f, a%10==0));
			result.push_back(Imath::V3f(-10.0f, a, a%10==0));
			result.push_back(Imath::V3f(10.0f, a, a%10==0));
		}
	}

	return result;
}

}

Grid::Grid() {
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glGenBuffers(1, &m_verticesVBO);

	glBindBuffer(GL_ARRAY_BUFFER, m_verticesVBO);
	const std::vector<Imath::V3f>& grid = makeGrid();
	glBufferData(GL_ARRAY_BUFFER, grid.size() * 3 * sizeof(GLfloat), &grid[0],
	             GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(m_vertexShader, 1, &vertexSrc, 0);
	glCompileShader(m_vertexShader);

	int IsCompiled_VS;
	glGetShaderiv(m_vertexShader, GL_COMPILE_STATUS, &IsCompiled_VS);
	if(IsCompiled_VS == false) {
		GLint maxLength;
		glGetShaderiv(m_vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

		std::string infoLog;
		infoLog.resize(maxLength);

		glGetShaderInfoLog(m_vertexShader, maxLength, &maxLength, &infoLog[0]);

		throw std::runtime_error("VERTEX SHADER:\n" + infoLog);
	}

	m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(m_fragmentShader, 1, &fragmentSrc, 0);
	glCompileShader(m_fragmentShader);

	int IsCompiled_FS;
	glGetShaderiv(m_fragmentShader, GL_COMPILE_STATUS, &IsCompiled_FS);
	if(IsCompiled_FS == false) {
		GLint maxLength;
		glGetShaderiv(m_fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

		std::string infoLog;
		infoLog.resize(maxLength);

		glGetShaderInfoLog(m_fragmentShader, maxLength, &maxLength, &infoLog[0]);

		throw std::runtime_error("FRAGMENT SHADER:\n" + infoLog);
	}

	m_program = glCreateProgram();

	glAttachShader(m_program, m_vertexShader);
	glAttachShader(m_program, m_fragmentShader);

	glBindAttribLocation(m_program, 0, "in_Position");

	glLinkProgram(m_program);
	GLint IsLinked;
	glGetProgramiv(m_program, GL_LINK_STATUS, (int*)&IsLinked);
	if(IsLinked == false) {
		GLint maxLength;
		glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &maxLength);

		std::string infoLog;
		infoLog.resize(maxLength);
		glGetProgramInfoLog(m_program, maxLength, &maxLength, &infoLog[0]);

		throw std::runtime_error(infoLog);
	}

	glUseProgram(m_program);

	glBindVertexArray(0);
}

void Grid::draw(const Imath::M44f& projection, const Imath::M44f& modelview) {
	glBindVertexArray(m_vao);

	GLint projectionLoc = glGetUniformLocation(m_program, "in_Projection");
	assert(projectionLoc != -1);
	glUniformMatrix4fv(projectionLoc, 1, false, projection.getValue());

	GLint modelviewLoc = glGetUniformLocation(m_program, "in_Modelview");
	assert(modelviewLoc != -1);
	glUniformMatrix4fv(modelviewLoc, 1, false, modelview.getValue());

	glDrawArrays(GL_LINES, 0, makeGrid().size());

	glBindVertexArray(0);
}

}
