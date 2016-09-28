#include "viewport.h"

#include <cassert>
#include <iostream>
#include <cmath>

#include <QtGui/QMouseEvent>

#include <GL/glu.h>

namespace {
	std::vector<viewport*> s_instances;
}

viewport::viewport(QWidget* parent) : QGLWidget(parent, (!s_instances.empty() ? s_instances[0] : NULL)), m_mouseX(0), m_mouseY(0) {
	s_instances.push_back(this);

	setMouseTracking(true);

	setMinimumWidth(400);
	setMinimumHeight(300);
}

viewport::~viewport() {
	s_instances.erase(std::find(s_instances.begin(), s_instances.end(), this));
}

void viewport::initializeGL() {
	glClearColor(0,0,0,0);
	resizeGL(width(), height());

	// glEnable(GL_DEPTH_TEST);
	// glDepthFunc(GL_LESS);

	update();
}

void viewport::paintGL() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, m_width, m_height);
	gluOrtho2D(0, m_width, m_height, 0);
	glMatrixMode(GL_MODELVIEW);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	emit render();
}

void viewport::repaint() {
	update();
}

void viewport::resizeGL(int w, int h) {
	QGLWidget::resizeGL(w,h);

	m_width = w;
	m_height = h;
}

void viewport::mouseMoveEvent(QMouseEvent* event) {
	m_mouseX = event->x();
	m_mouseY = event->y();

	repaint();
}

int viewport::width() const {
	return m_width;
}

int viewport::height() const {
	return m_height;
}
