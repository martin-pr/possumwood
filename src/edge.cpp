#include "edge.h"

#include <cassert>
#include <iostream>

#include <QPainter>
#include <QGraphicsScene>

#include "port.h"

namespace {

const float s_curvature = 50.0f;

float min(float a, float b, float c, float d) {
	return std::min(std::min(a, b), std::min(c, d));
}

float max(float a, float b, float c, float d) {
	return std::max(std::max(a, b), std::max(c, d));
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
	const float xMin = min(m_origin.x(), m_target.x(), m_origin.x() + s_curvature, m_target.x() - s_curvature);
	const float xMax = max(m_origin.x(), m_target.x(), m_origin.x() + s_curvature, m_target.x() - s_curvature);
	const float yMin = std::min(m_origin.y(), m_target.y());
	const float yMax = std::max(m_origin.y(), m_target.y());

	const QRectF box(xMin - 1, yMin - 1, xMax - xMin + 2, yMax - yMin + 2);

	return box;
}

namespace {
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

void Edge::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	QPainterPath path;

	path.moveTo(m_origin);
	for(unsigned a = 1; a < 50; ++a) {
		const float t = (float)a / 50.0f;
		path.lineTo(bezierPoint(t));
	}
	path.lineTo(m_target);

	QColor color;
	color.setRed(m_p1->color().red() / 2 + m_p2->color().red() / 2);
	color.setGreen(m_p1->color().green() / 2 + m_p2->color().green() / 2);
	color.setBlue(m_p1->color().blue() / 2 + m_p2->color().blue() / 2);
	painter->setPen(QPen(color, 2));

	painter->drawPath(path);
}
