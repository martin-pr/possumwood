#pragma once

#include <QGraphicsView>
#include <QGraphicsPixmapItem>

#include "datatypes/animation.h"

namespace anim {

class MotionMap : public QGraphicsView {
	Q_OBJECT

	public:
		MotionMap();

		void init(const anim::Animation& ax, const anim::Animation& ay);

		std::size_t width() const;
		std::size_t height() const;

	signals:
		void mousePress(QMouseEvent* mouseEvent);
		void mouseRelease(QMouseEvent* mouseEvent);
		void mouseMove(QMouseEvent* mouseEvent);

	protected:
		virtual void mouseMoveEvent(QMouseEvent* mouseEvent) override;
		virtual void mousePressEvent(QMouseEvent* mouseEvent) override;
		virtual void mouseReleaseEvent(QMouseEvent* mouseEvent) override;
		virtual void wheelEvent(QWheelEvent* mouseEvent) override;

	private:
		QGraphicsScene* m_scene;
		QGraphicsPixmapItem* m_pixmap;

		QPoint m_mousePos;
};

}
