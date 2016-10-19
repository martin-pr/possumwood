#include "edge.h"

#include <cassert>
#include <iostream>

#include <QPainter>
#include <QGraphicsScene>
#include <QStyleOptionGraphicsItem>
#include <QPainterPathStroker>

#include "port.h"

namespace {

const float s_curvature = 50.0f;

float min(float a, float b, float c, float d) {
	return std::min(std::min(a, b), std::min(c, d));
}

float max(float a, float b, float c, float d) {
	return std::max(std::max(a, b), std::max(c, d));
}

QPointF evalBezier(float t, const QPointF& p0, const QPointF& p1, const QPointF& p2, const QPointF& p3) {
	return QPointF(
	           powf(1.0f - t, 3) * p0 +
	           3.0 * (1.0 - t) * (1.0 - t) * t * p1 +
	           3.0 * (1.0 - t) * t * t * p2 +
	           t * t * t * p3
	       );
}

float length(const QPointF& p) {
	return sqrt(p.x() * p.x() + p.y() * p.y());
}

}


Edge::Edge(Port& p1, Port& p2) : m_p1(&p1), m_p2(&p2) {
	setFlags(ItemIsSelectable);

	assert(p1.portType() & Port::kOutput);
	assert(p2.portType() & Port::kInput);

	p1.m_edges.insert(this);
	p2.m_edges.insert(this);

	adjust();
}

void Edge::adjust() {
	const QRectF origBbox = boundingRect();

	const QRectF bb1 = m_p1->mapRectToScene(m_p1->boundingRect());
	const QRectF bb2 = m_p2->mapRectToScene(m_p2->boundingRect());

	const float x1 = bb1.x() + bb1.width() - bb1.height() / 2;
	const float y1 = bb1.y() + bb1.height() / 2;
	const float x2 = bb2.x() + bb2.height() / 2;
	const float y2 = bb2.y() + bb2.height() / 2;

	m_origin = QPointF(x1, y1);
	m_target = QPointF(x2, y2);

	if(scene())
		scene()->update(boundingRect().united(origBbox));

	prepareGeometryChange();
}

QRectF Edge::boundingRect() const {
	const QPointF p1 = m_origin;
	const QPointF p2 = m_target;

	const float xMin = min(p1.x(), p2.x(), p1.x() + s_curvature, p2.x() - s_curvature);
	const float xMax = max(p1.x(), p2.x(), p1.x() + s_curvature, p2.x() - s_curvature);
	const float yMin = std::min(p1.y(), p2.y());
	const float yMax = std::max(p1.y(), p2.y());

	const QRectF box(xMin - 2, yMin - 2, xMax - xMin + 4, yMax - yMin + 4);

	return box;
}

QPointF Edge::bezierPoint(float t) const {
	const QPointF tangent = QPointF(s_curvature, 0) * pow(length(m_origin - m_target) / 20.0f, 0.25);

	return evalBezier(
	           t,
	           m_origin,
	           m_origin + tangent,
	           m_target - tangent,
	           m_target
	       );
}

QPainterPath Edge::makePath() const {
	QPainterPath path;

	path.moveTo(m_origin);
	for(unsigned a = 1; a < 50; ++a) {
		const float t = (float)a / 50.0f;
		path.lineTo(bezierPoint(t));
	}
	path.lineTo(m_target);

	return path;
}

void Edge::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {

	QColor color;
	color.setRed(m_p1->color().red() / 2 + m_p2->color().red() / 2);
	color.setGreen(m_p1->color().green() / 2 + m_p2->color().green() / 2);
	color.setBlue(m_p1->color().blue() / 2 + m_p2->color().blue() / 2);

	const QPainterPath path = makePath();

	if(option->state & QStyle::State_Selected) {
		painter->setPen(QPen(Qt::white, 4));
		painter->drawPath(path);
	}

	painter->setPen(QPen(color, 2));
	painter->drawPath(path);
}

QPainterPath Edge::shape() const {
	QPainterPathStroker stroker;
	stroker.setWidth(8);

	return stroker.createStroke(makePath());
}
