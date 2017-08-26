#include "motion_map.h"

#include <QGraphicsSceneWheelEvent>
#include <QGraphicsWidget>
#include <QWidget>
#include <QScrollBar>

namespace anim {

namespace {

float compare(const anim::Skeleton& s1, const anim::Skeleton& s2) {
	if(s1.size() != s2.size() || s1.size() == 0)
		return 0.0f;

	float result = 0.0f;
	for(unsigned bi = 1; bi < s1.size(); ++bi) {
		const auto& b1 = s1[bi];
		const auto& b2 = s2[bi];

		if((b1.tr().rotation ^ b2.tr().rotation) > 0.0f)
			result += (b1.tr().rotation * b2.tr().rotation.inverse()).angle();
		else
			result += (b1.tr().rotation * (-b2.tr().rotation).inverse()).angle();
	}

	return result;
}

}

MotionMap::MotionMap() : m_pixmap(new QGraphicsPixmapItem()) {
	m_scene = new QGraphicsScene(this);
	setScene(m_scene);

	m_scene->addItem(m_pixmap);
}

void MotionMap::init(const anim::Animation& anim) {
	std::vector<float> matrix(anim.frames.size() * anim.frames.size());
	float maxVal = 0.0f;
	float minVal = compare(anim.frames[0], anim.frames[0]);

	for(unsigned a = 0; a < anim.frames.size(); ++a)
		for(unsigned b = a; b < anim.frames.size(); ++b) {
			auto& f1 = anim.frames[a];
			auto& f2 = anim.frames[b];

			const float res = compare(f1, f2);
			maxVal = std::max(res, maxVal);
			minVal = std::min(res, minVal);

			matrix[a + b * anim.frames.size()] = res;
			matrix[b + a * anim.frames.size()] = res;
		}

	if(maxVal > 0.0f)
		for(auto& f : matrix)
			f = (f - minVal) / (maxVal - minVal) * 255.0f;

	QImage img = QImage(anim.frames.size(), anim.frames.size(), QImage::Format_RGB32);

	for(unsigned a = 0; a < anim.frames.size(); ++a)
		for(unsigned b = 0; b < anim.frames.size(); ++b) {
			const float val = matrix[a + b * anim.frames.size()];
			img.setPixel(a, b, qRgb(val, val, val));
		}

	m_pixmap->setPixmap(QPixmap::fromImage(img));
}

void MotionMap::init(const anim::Animation& ax, const anim::Animation& ay) {
	std::vector<float> matrix(ax.frames.size() * ay.frames.size());
	float maxVal = 0.0f;
	float minVal = compare(ax.frames[0], ay.frames[0]);

	for(unsigned a = 0; a < ax.frames.size(); ++a)
		for(unsigned b = 0; b < ay.frames.size(); ++b) {
			auto& f1 = ax.frames[a];
			auto& f2 = ay.frames[b];

			const float res = compare(f1, f2);
			maxVal = std::max(res, maxVal);
			minVal = std::min(res, minVal);

			matrix[a + b * ax.frames.size()] = res;
		}

	if(maxVal > 0.0f)
		for(auto& f : matrix)
			f = (f - minVal) / (maxVal - minVal) * 255.0f;

	QImage img = QImage(ax.frames.size(), ay.frames.size(), QImage::Format_RGB32);

	for(unsigned a = 0; a < ax.frames.size(); ++a)
		for(unsigned b = 0; b < ay.frames.size(); ++b) {
			const float val = matrix[a + b * ax.frames.size()];
			img.setPixel(a, b, qRgb(val, val, val));
		}

	m_pixmap->setPixmap(QPixmap::fromImage(img));
}

std::size_t MotionMap::width() const {
	return m_pixmap->pixmap().width();
}

std::size_t MotionMap::height() const {
	return m_pixmap->pixmap().height();
}

void MotionMap::mouseMoveEvent(QMouseEvent* mouseEvent) {
	if(mouseEvent->buttons() & Qt::MiddleButton) {
		const QPoint d = mouseEvent->pos() - m_mousePos;
		horizontalScrollBar()->setValue(horizontalScrollBar()->value() - d.x());
		verticalScrollBar()->setValue(verticalScrollBar()->value() - d.y());
	}

	emit mouseMove(mouseEvent);

	m_mousePos = mouseEvent->pos();
}

void MotionMap::mousePressEvent(QMouseEvent* mouseEvent) {
	emit mousePress(mouseEvent);

	m_mousePos = mouseEvent->pos();
}

void MotionMap::mouseReleaseEvent(QMouseEvent* mouseEvent) {
	emit mouseRelease(mouseEvent);

	m_mousePos = mouseEvent->pos();
}

namespace {

void adjustScrollBar(QScrollBar* scrollBar, float factor, float pos) {


	scrollBar->setValue(int(factor * scrollBar->value()
	                        + ((factor - 1) * scrollBar->pageStep() * pos)));
}

}

void MotionMap::wheelEvent(QWheelEvent* mouseEvent) {
	const float sc = powf(10.0f, (float)mouseEvent->delta() / 480.0f);

	QPointF pos = mouseEvent->pos();

	setTransformationAnchor(QGraphicsView::NoAnchor);
	scale(sc, sc);

	adjustScrollBar(horizontalScrollBar(), sc, pos.x() / (float)QGraphicsView::width());
	adjustScrollBar(verticalScrollBar(), sc, pos.y() / (float)QGraphicsView::height());
}

}
