#include <GL/glew.h>

#include "viewport.h"

#include <cassert>
#include <iostream>
#include <cmath>

#include <QtGui/QMouseEvent>

#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/gl.h>

namespace {

QGLFormat getGLFormat() {
	QGLFormat format = QGLFormat::defaultFormat();
	// way higher than currently supported - will fall back to highest
	format.setVersion(6, 0);
	format.setProfile(QGLFormat::CoreProfile);
	return format;
}
}

Viewport::Viewport(QWidget* parent)
    : QGLWidget(getGLFormat(), parent), m_sceneDistance(10), m_sceneRotationX(30),
      m_sceneRotationY(30), m_origin(0, 0, 0), m_mouseX(0), m_mouseY(0) {
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
}

void Viewport::paintGL() {
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
	const float dt = (float)(t - m_timer).total_microseconds() / 1e6f;
	m_timer = t;

	// glPushAttrib(GL_ALL_ATTRIB_BITS);
	emit render(dt);

	if(m_grid == nullptr)
		m_grid = std::unique_ptr<possumwood::Grid>(new possumwood::Grid());
	m_grid->draw(m_projection, m_modelview);

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
