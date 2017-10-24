#pragma once

#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <QtOpenGL/QGLWidget>

class Viewport : public QGLWidget, public boost::noncopyable {
	Q_OBJECT

  signals:
	void render(float dt);

  public:
	Viewport(QWidget* parent = NULL);
	virtual ~Viewport();

  protected:
	virtual void initializeGL();
	virtual void paintGL();
	virtual void resizeGL(int w, int h);
	virtual void mouseMoveEvent(QMouseEvent* event);

  private:
	float m_sceneDistance, m_sceneRotationX, m_sceneRotationY;
	float m_originX, m_originY, m_originZ;
	int m_mouseX, m_mouseY;

	boost::posix_time::ptime m_timer;
};
