#include "motion_map.h"

#include <QGraphicsSceneWheelEvent>
#include <QGraphicsWidget>
#include <QWidget>
#include <QScrollBar>

namespace anim { namespace ui {

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
	assert(!anim.empty());

	std::vector<float> matrix(anim.size() * anim.size());
	float maxVal = 0.0f;
	float minVal = compare(anim.frame(0), anim.frame(0));

	for(unsigned a = 0; a < anim.size(); ++a)
		for(unsigned b = a; b < anim.size(); ++b) {
			auto& f1 = anim.frame(a);
			auto& f2 = anim.frame(b);

			const float res = compare(f1, f2);
			maxVal = std::max(res, maxVal);
			minVal = std::min(res, minVal);

			matrix[a + b * anim.size()] = res;
			matrix[b + a * anim.size()] = res;
		}

	if(maxVal > 0.0f)
		for(auto& f : matrix)
			f = (f - minVal) / (maxVal - minVal) * 255.0f;

	QImage img = QImage(anim.size(), anim.size(), QImage::Format_RGB32);

	for(unsigned a = 0; a < anim.size(); ++a)
		for(unsigned b = 0; b < anim.size(); ++b) {
			const float val = matrix[a + b * anim.size()];
			img.setPixel(a, b, qRgb(val, val, val));
		}

	m_pixmap->setPixmap(QPixmap::fromImage(img));
}

void MotionMap::init(const anim::Animation& ax, const anim::Animation& ay) {
	std::vector<float> matrix(ax.size() * ay.size());
	float maxVal = 0.0f;
	float minVal = compare(ax.frame(0), ay.frame(0));

	for(unsigned a = 0; a < ax.size(); ++a)
		for(unsigned b = 0; b < ay.size(); ++b) {
			auto& f1 = ax.frame(a);
			auto& f2 = ay.frame(b);

			const float res = compare(f1, f2);
			maxVal = std::max(res, maxVal);
			minVal = std::min(res, minVal);

			matrix[a + b * ax.size()] = res;
		}

	if(maxVal > 0.0f)
		for(auto& f : matrix)
			f = (f - minVal) / (maxVal - minVal) * 255.0f;

	QImage img = QImage(ax.size(), ay.size(), QImage::Format_RGB32);

	for(unsigned a = 0; a < ax.size(); ++a)
		for(unsigned b = 0; b < ay.size(); ++b) {
			const float val = matrix[a + b * ax.size()];
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

} }
