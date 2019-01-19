#include "viewport.h"

#include <cassert>
#include <iostream>
#include <cmath>

#include <QtGui/QMouseEvent>

#include <possumwood_sdk/gl.h>

#include "gl_init.h"

Viewport::Viewport(QWidget* parent)
    : QOpenGLWidget(parent), m_sceneDistance(10), m_sceneRotationX(30),
      m_sceneRotationY(30), m_origin(0, 0, 0), m_mouseX(0), m_mouseY(0) {
	setMouseTracking(true);

	GL_CHECK_ERR;
}

Viewport::~Viewport() {
}

void Viewport::initializeGL() {
	GL_CHECK_ERR;

	// initialize GLEW
	doInitGlew();

	GL_CHECK_ERR;

	// setup the viewport basics
	glClearColor(0, 0, 0, 0);
	resizeGL(width(), height());

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// store current FPS timer value
	m_timer = boost::posix_time::microsec_clock::universal_time();

	GL_CHECK_ERR;
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

}

void Viewport::paintGL() {
	GL_CHECK_ERR;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// update the matrices
	m_viewportState.resize(width(), height());

	m_viewportState.perspective(45, m_sceneDistance * 0.1f,
	                std::max(m_sceneDistance * 2.0f, 1000.0f));

	m_viewportState.lookAt(
	    eyePosition(m_sceneRotationX, m_sceneRotationY, m_sceneDistance) + m_origin,
	    m_origin, Imath::V3f(0, 1, 0));

	// record the time difference between frames, to determine current FPS
	const boost::posix_time::ptime t(boost::posix_time::microsec_clock::universal_time());
	const float dt = (float)(t - m_timer).total_microseconds() / 1e6f;
	m_timer = t;

	// emit the render signal, to render everything in the scene (implemented in Adaptor)
	emit render(dt);

	GL_CHECK_ERR;
}

void Viewport::resizeGL(int w, int h) {
	GL_CHECK_ERR;
	QOpenGLWidget::resizeGL(w, h);

	glViewport(0, 0, w, h);
	GL_CHECK_ERR;
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

const possumwood::ViewportState& Viewport::viewportState() const {
	return m_viewportState;
}
