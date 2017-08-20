#pragma once

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

#include "datatypes/animation.h"

namespace anim {

class MotionMap : public QGraphicsScene {
	Q_OBJECT

	public:
		MotionMap();

		void init(const anim::Animation& ax, const anim::Animation& ay);

		std::size_t width() const;
		std::size_t height() const;

	signals:
		void mousePress(QGraphicsSceneMouseEvent* mouseEvent);
		void mouseRelease(QGraphicsSceneMouseEvent* mouseEvent);
		void mouseMove(QGraphicsSceneMouseEvent* mouseEvent);

	protected:
		virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent);
		virtual void mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent);
		virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent);

	private:
		QGraphicsPixmapItem* m_pixmap;
};

}
