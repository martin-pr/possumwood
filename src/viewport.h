#pragma once

#include <vector>
#include <boost/noncopyable.hpp>
#include <QtOpenGL/QGLWidget>

class viewport : public QGLWidget, public boost::noncopyable {
	Q_OBJECT

	signals:
		void render();

	public slots:
		void repaint();

	public:
		viewport(QWidget* parent);
		virtual ~viewport();

		int width() const;
		int height() const;

	protected:
		virtual void initializeGL();
		virtual void paintGL();
		virtual void resizeGL(int w, int h);
		virtual void mouseMoveEvent(QMouseEvent* event);

	private:
		int m_mouseX, m_mouseY;
		int m_width, m_height;
};
