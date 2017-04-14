#include "graph_widget.h"

#include <cassert>

#include <QHBoxLayout>
#include <QGLWidget>
#include <QResizeEvent>

#include "connected_edge.h"

namespace node_editor {

GraphWidget::GraphWidget(QWidget* parent) : QGraphicsView(parent), m_scene(this) {
	m_scene.setSceneRect(-width() / 2, -height() / 2, width(), height());

	setScene(&m_scene);
	setDragMode(QGraphicsView::RubberBandDrag);
	setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
	setBackgroundBrush(Qt::black);
}

GraphScene& GraphWidget::scene() {
	return m_scene;
}

void GraphWidget::resizeEvent(QResizeEvent* event) {
	const QPointF mid = m_scene.sceneRect().center();

	m_scene.setSceneRect(mid.x() - event->size().width() / 2,
	                      mid.y() - event->size().height() / 2,
	                      event->size().width(),
	                      event->size().height());

	QGraphicsView::resizeEvent(event);
}

void GraphWidget::mouseMoveEvent(QMouseEvent* event) {
	if(m_scene.isEdgeEditInProgress()) {
		QMouseEvent tmp(event->type(), event->pos(), Qt::NoButton,
			event->buttons() & ~Qt::LeftButton, event->modifiers());
		QGraphicsView::mouseMoveEvent(&tmp);
	}
	else
		QGraphicsView::mouseMoveEvent(event);
}

}
