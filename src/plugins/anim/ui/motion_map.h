#pragma once

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

#include "datatypes/animation.h"

namespace anim {

class MotionMap : public QGraphicsScene {
	public:
		MotionMap();

		void init(const anim::Animation& ax, const anim::Animation& ay);

		std::size_t width() const;
		std::size_t height() const;

	private:
		QGraphicsPixmapItem* m_pixmap;
};

}
