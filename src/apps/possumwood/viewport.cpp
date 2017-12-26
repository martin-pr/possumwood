#include <GL/glew.h>

#include "viewport.h"

#include <cassert>
#include <iostream>
#include <cmath>

#include <QtGui/QMouseEvent>

#include <GL/glu.h>
#include <GL/gl.h>

namespace {

QGLFormat getGLFormat() {
	QGLFormat format = QGLFormat::defaultFormat();
	format.setVersion(4, 5); // currently the latest
	format.setProfile(QGLFormat::CoreProfile);
	return format;
}

}

Viewport::Viewport(QWidget* parent)
    : QGLWidget(getGLFormat(), parent),
      m_sceneDistance(10), m_sceneRotationX(30), m_sceneRotationY(30), m_origin(0, 0, 0),
      m_mouseX(0), m_mouseY(0), m_groundVao(0), m_groundVbo(0) {

	setMouseTracking(true);
}

Viewport::~Viewport() {
}

void Viewport::initializeGL() {
	// glGetError

	glClearColor(0.1, 0, 0, 0);
	resizeGL(width(), height());

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// float white[3] = {1, 1, 1};
	// float position[4] = {1, 1, 1, 0};

	// glEnable(GL_LIGHT0);
	// glLightfv(GL_LIGHT0, GL_AMBIENT, white);
	// glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
	// glLightfv(GL_LIGHT0, GL_POSITION, position);

	// glEnable(GL_COLOR_MATERIAL);

	m_timer = boost::posix_time::microsec_clock::universal_time();
}

namespace {

Imath::V3f eyePosition(float sceneRotX, float sceneRotY, float sceneDist) {
	Imath::V3f eye;

	eye.x = sin(-sceneRotX / 180.0f * M_PI) * sceneDist;
	eye.z = cos(-sceneRotX / 180.0f * M_PI) * sceneDist;

	eye.y = sin(sceneRotY / 180.0f * M_PI) * sceneDist;
	eye.x *= cos(sceneRotY / 180.0f * M_PI);
	eye.z *= cos(sceneRotY / 180.0f * M_PI);

	return eye;
}

Imath::M44f lookAt(const Imath::V3f& eyePosition, const Imath::V3f& lookAt,
                   const Imath::V3f& upVector) {
	Imath::V3f forward = lookAt - eyePosition;
	forward.normalize();

	Imath::V3f side = forward.cross(upVector);
	side.normalize();

	Imath::V3f up = side.cross(forward);
	up.normalize();

	Imath::M44f rotmat;
	rotmat.makeIdentity();

	rotmat[0][0] = side.x;
	rotmat[1][0] = side.y;
	rotmat[2][0] = side.z;

	rotmat[0][1] = up.x;
	rotmat[1][1] = up.y;
	rotmat[2][1] = up.z;

	rotmat[0][2] = -forward.x;
	rotmat[1][2] = -forward.y;
	rotmat[2][2] = -forward.z;

	Imath::M44f transmat;
	transmat.setTranslation(Imath::V3f(-eyePosition.x, -eyePosition.y, -eyePosition.z));

	return transmat * rotmat;
}

Imath::M44f perspective(float fovyInDegrees, float aspectRatio, float znear, float zfar) {
	const float f = 1.0 / tanf(fovyInDegrees * M_PI / 360.0);
	const float A = (zfar + znear) / (znear - zfar);
	const float B = 2.0 * zfar * znear / (znear - zfar);

	return Imath::M44f(f / aspectRatio, 0, 0, 0, 0, f, 0, 0, 0, 0, A, -1, 0, 0, B, 0);
}

static const GLchar* fragmentSrc = " \
#version 150 \n \
//out vec4 gl_FragColor; \n \
\n \
void main(void) { \n \
    gl_FragColor = vec4(0,1,0,1.0); \n \
}";

static const GLchar* vertexSrc = " \
#version 150 \n \
in vec3 in_Position; \n \
\n \
void main(void) { \n \
    gl_Position = vec4(in_Position, 1.0); \n \
}";

const std::vector<Imath::V3f>& makeGrid() {
	static std::vector<Imath::V3f> result;

	if(result.empty()) {
		for(int a = -10; a <= 10; ++a) {
			result.push_back(Imath::V3f((float)a / 10.0f, -10.0f, 0));
			result.push_back(Imath::V3f((float)a / 10.0f, 10.0f, 0));
			result.push_back(Imath::V3f(-10.0f, (float)a / 10.0f, 0));
			result.push_back(Imath::V3f(10.0f, (float)a / 10.0f, 0));
		}
	}

	return result;
}

}

void Viewport::paintGL() {
	// first initialisation
	if(m_groundVao == 0) {
		// make the vertex array object
		glGenVertexArrays(1, &m_groundVao);
		// and bind it to work in it now
		glBindVertexArray(m_groundVao);

		// allocate the vertex data buffer
		glGenBuffers(1, &m_groundVbo);

		// bind the buffer and transfer the data
		glBindBuffer(GL_ARRAY_BUFFER, m_groundVbo);
		const std::vector<Imath::V3f>& grid = makeGrid();
		glBufferData(GL_ARRAY_BUFFER, grid.size() * 3 * sizeof(GLfloat), &grid[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		GLuint vertexshader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexshader, 1, &vertexSrc, 0);
		glCompileShader(vertexshader);

		int IsCompiled_VS;
		glGetShaderiv(vertexshader, GL_COMPILE_STATUS, &IsCompiled_VS);
		if(IsCompiled_VS == false) {
			GLint maxLength;
			glGetShaderiv(vertexshader, GL_INFO_LOG_LENGTH, &maxLength);

			std::string infoLog;
			infoLog.resize(maxLength);

			glGetShaderInfoLog(vertexshader, maxLength, &maxLength, &infoLog[0]);

			throw std::runtime_error("VERTEX SHADER:\n" + infoLog);
		}

		GLuint fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentshader, 1, &fragmentSrc, 0);
		glCompileShader(fragmentshader);

		int IsCompiled_FS;
		glGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &IsCompiled_FS);
		if(IsCompiled_FS == false) {
			GLint maxLength;
			glGetShaderiv(fragmentshader, GL_INFO_LOG_LENGTH, &maxLength);

			std::string infoLog;
			infoLog.resize(maxLength);

			glGetShaderInfoLog(fragmentshader, maxLength, &maxLength, &infoLog[0]);

			throw std::runtime_error("FRAGMENT SHADER:\n" + infoLog);
		}

		GLint shaderprogram = glCreateProgram();

		/* Attach our shaders to our program */
		glAttachShader(shaderprogram, vertexshader);
		glAttachShader(shaderprogram, fragmentshader);

		glBindAttribLocation(shaderprogram, 0, "in_Position");

		glLinkProgram(shaderprogram);
		GLint IsLinked;
		glGetProgramiv(shaderprogram, GL_LINK_STATUS, (int*)&IsLinked);
		if(IsLinked == false) {
			GLint maxLength;
			glGetProgramiv(shaderprogram, GL_INFO_LOG_LENGTH, &maxLength);

			std::string infoLog;
			infoLog.resize(maxLength);
			glGetProgramInfoLog(shaderprogram, maxLength, &maxLength,
			                    &infoLog[0]);

			throw std::runtime_error(infoLog);
		}

		glUseProgram(shaderprogram);

		glBindVertexArray(0);
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// update the projection matrix
	m_projection =
	    perspective(45, (float)width() / (float)height(), m_sceneDistance * 0.1f,
	                std::max(m_sceneDistance * 2.0f, 1000.0f));
	// glMatrixMode(GL_PROJECTION);
	// glLoadMatrixf(m_projection.getValue());

	m_modelview = lookAt(
	    eyePosition(m_sceneRotationX, m_sceneRotationY, m_sceneDistance) + m_origin,
	    m_origin, Imath::V3f(0, 1, 0));
	// glMatrixMode(GL_MODELVIEW);
	// glLoadMatrixf(m_modelview.getValue());

	const boost::posix_time::ptime t(boost::posix_time::microsec_clock::universal_time());
	// const float dt = (float)(t - m_timer).total_microseconds() / 1e6f;
	m_timer = t;

	// glPushAttrib(GL_ALL_ATTRIB_BITS);
	//emit render(dt);

	glBindVertexArray(m_groundVao);

	glDrawArrays(GL_LINES, 0, makeGrid().size());

	glBindVertexArray(0);

	// glPopAttrib();
}

void Viewport::resizeGL(int w, int h) {
	QGLWidget::resizeGL(w, h);

	glViewport(0, 0, w, h);
}

void Viewport::mouseMoveEvent(QMouseEvent* event) {
	if(event->buttons() & Qt::LeftButton) {
		m_sceneRotationX += (float)(event->x() - m_mouseX) / (float)width() * 360.0;
		m_sceneRotationY += (float)(event->y() - m_mouseY) / (float)height() * 360.0;

		update();
	}
	m_sceneRotationY = std::max(-89.0f, m_sceneRotationY);
	m_sceneRotationY = std::min(89.0f, m_sceneRotationY);

	if(event->buttons() & Qt::RightButton) {
		m_sceneDistance /= pow(10, (float)(event->y() - m_mouseY) / (float)height());

		update();
	}

	if(event->buttons() & Qt::MiddleButton) {
		const Imath::V3f eye =
		    eyePosition(m_sceneRotationX, m_sceneRotationY, m_sceneDistance);

		const float dx = -(float)(event->x() - m_mouseX) / (float)(width());
		const float dy = (float)(event->y() - m_mouseY) / (float)(height());

		Imath::V3f right = Imath::V3f(0, 1, 0).cross(eye).normalized();
		Imath::V3f up = eye.cross(right).normalized();

		right = right * m_sceneDistance * dx;
		up = up * m_sceneDistance * dy;

		m_origin += right + up;

		update();
	}

	m_mouseX = event->x();
	m_mouseY = event->y();
}

const Imath::M44f& Viewport::projection() const {
	return m_projection;
}

const Imath::M44f& Viewport::modelview() const {
	return m_modelview;
}

