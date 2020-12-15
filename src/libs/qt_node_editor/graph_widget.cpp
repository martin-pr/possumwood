#include "graph_widget.h"

#include <QHBoxLayout>
#include <QResizeEvent>
#include <QScrollBar>
#include <cassert>
#include <cmath>
#include <iostream>

#include "connected_edge.h"

namespace node_editor {

GraphWidget::GraphWidget(QWidget* parent) : QGraphicsView(parent), m_scene(this), m_mouseX(0), m_mouseY(0) {
	setDragMode(QGraphicsView::NoDrag);
	setSizeAdjustPolicy(AdjustIgnored);

	m_scene.setSceneRect(-10000, -10000, 20000, 20000);
	translate(-width() / 2, -height() / 2);

	setScene(&m_scene);
	setDragMode(QGraphicsView::RubberBandDrag);
	setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

	setTransformationAnchor(QGraphicsView::NoAnchor);

	setInteractive(true);

	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	verticalScrollBar()->setEnabled(false);
	horizontalScrollBar()->setEnabled(false);
}

GraphScene& GraphWidget::scene() {
	return m_scene;
}

void GraphWidget::mouseMoveEvent(QMouseEvent* event) {
	// "eat" the left mouse moves while edge editing is in progress
	if(m_scene.isEdgeEditInProgress()) {
		QMouseEvent tmp(event->type(), event->pos(), Qt::NoButton, event->buttons() & ~Qt::LeftButton,
		                event->modifiers());
		QGraphicsView::mouseMoveEvent(&tmp);

		event->accept();
	}
	// translation when mid button is pressed
	else if(event->buttons() & Qt::MiddleButton) {
		const QPointF translation = QPointF(event->x(), event->y()) - QPointF(m_mouseX, m_mouseY);

		const QTransform m = transform();
		const float scale = sqrt(m.m11() * m.m22());

		translate(translation.x() / scale, translation.y() / scale);

		event->accept();
	}
	// default handling
	else
		QGraphicsView::mouseMoveEvent(event);

	// store current mouse position
	m_mouseX = event->x();
	m_mouseY = event->y();
}

void GraphWidget::wheelEvent(QWheelEvent* event) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
	const QPointF orig = mapToScene(event->position().x(), event->position().y());
#else
	const QPointF orig = mapToScene(event->x(), event->y());
#endif
	const float delta = pow(1.002, (float)event->angleDelta().y());

	QTransform tr = transform();
	tr.scale(delta, delta);

	const float det = tr.determinant();
	if(det > 1e-3 && det < 1e2)
		setTransform(tr);

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
	const QPointF current = mapToScene(event->position().x(), event->position().y());
#else
	const QPointF current = mapToScene(event->x(), event->y());
#endif

	translate((current - orig).x(), (current - orig).y());
}

}  // namespace node_editor
