#include "viewport.h"

#include <cassert>
#include <iostream>
#include <cmath>

#include <QtGui/QMouseEvent>

#include <GL/glu.h>

namespace {
	std::vector<Viewport*> s_instances;
}

Viewport::Viewport(QWidget* parent) : QGLWidget(parent, (s_instances.size()>0 ? s_instances[0] : NULL)), m_sceneDistance(10), m_sceneRotationX(30), m_sceneRotationY(30), m_mouseX(0), m_mouseY(0) {
	s_instances.push_back(this);

	setMouseTracking(true);
}

Viewport::~Viewport() {
	int index = -1;
	for(unsigned a=0;a<s_instances.size();a++)
		if(s_instances[a] == this)
			index = a;
	assert(index >= 0);
	s_instances.erase(s_instances.begin()+index);
}

void Viewport::initializeGL() {
	glClearColor(0,0,0,0);
	resizeGL(width(), height());

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	m_timer = boost::posix_time::microsec_clock::universal_time();
}

void Viewport::paintGL() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(0,0,m_sceneDistance, 0,0,0, 0,1,0);

	glRotatef(m_sceneRotationY, 1, 0, 0);
	glRotatef(m_sceneRotationX, 0, 1, 0);

	const boost::posix_time::ptime t(boost::posix_time::microsec_clock::universal_time());
	const float dt = (float)(t - m_timer).total_microseconds() / 1e6f;
	m_timer = t;

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	emit render(dt);
	glPopAttrib();
}

void Viewport::resizeGL(int w, int h) {
	QGLWidget::resizeGL(w,h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, w, h);
	gluPerspective(45, (float)w / (float)h, 0.01, 1000);
	glMatrixMode(GL_MODELVIEW);
}

void Viewport::mouseMoveEvent(QMouseEvent* event) {
	if(event->buttons() & Qt::LeftButton) {
		m_sceneRotationX += (float)(event->x() - m_mouseX) / (float)width() * 360.0;
		m_sceneRotationY += (float)(event->y() - m_mouseY) / (float)height() * 360.0;

		update();
	}
	m_sceneRotationY = std::max(-90.0f, m_sceneRotationY);
	m_sceneRotationY = std::min(90.0f, m_sceneRotationY);

	if(event->buttons() & Qt::RightButton) {
		m_sceneDistance /= pow(10, (float)(event->y() - m_mouseY) / (float)height());

		update();
	}

	m_mouseX = event->x();
	m_mouseY = event->y();
}
