#include "motion_map.h"

#include <QGraphicsSceneWheelEvent>
#include <QGraphicsWidget>
#include <QLayout>
#include <QScrollBar>
#include <QWidget>

namespace anim {
namespace ui {

MotionMap::MotionMap(QWidget* parent)
    : QGraphicsView(parent), m_pixmap(new QGraphicsPixmapItem()), m_imageChanged(false) {
	m_scene = new QGraphicsScene(this);
	setScene(m_scene);

	m_scene->addItem(m_pixmap);
}

void MotionMap::init(const ::anim::MotionMap& mmap) {
	m_image = QImage(mmap.width(), mmap.height(), QImage::Format_RGB888);
	m_imageChanged = true;

	for(unsigned y = 0; y < mmap.height(); ++y)
		for(unsigned x = 0; x < mmap.width(); ++x) {
			const float val = ((mmap(x, y) - mmap.min()) / (mmap.max() - mmap.min())) * 255.0f;
			m_image.setPixel(x, y, qRgb(val, val, val));
		}
}

void MotionMap::setPixel(std::size_t x, std::size_t y, const QColor& color) {
	m_image.setPixel(x, y, color.rgba());

	m_imageChanged = true;
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
	scrollBar->setValue(int(factor * scrollBar->value() + ((factor - 1) * scrollBar->pageStep() * pos)));
}

}  // namespace

void MotionMap::wheelEvent(QWheelEvent* mouseEvent) {
	const float sc = powf(10.0f, (float)mouseEvent->angleDelta().y() / 480.0f);

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
	QPointF pos = mouseEvent->position();
#else
	QPointF pos = mouseEvent->pos();
#endif

	setTransformationAnchor(QGraphicsView::NoAnchor);
	scale(sc, sc);

	adjustScrollBar(horizontalScrollBar(), sc, pos.x() / (float)QGraphicsView::width());
	adjustScrollBar(verticalScrollBar(), sc, pos.y() / (float)QGraphicsView::height());
}

void MotionMap::paintEvent(QPaintEvent* event) {
	if(m_imageChanged) {
		m_pixmap->setPixmap(QPixmap::fromImage(m_image));
		m_imageChanged = false;
	}

	QGraphicsView::paintEvent(event);
}

}  // namespace ui
}  // namespace anim
