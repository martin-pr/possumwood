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


Edge::Edge(QPointF origin, QPointF target) : m_origin(origin), m_target(target) {
	setFlags(ItemIsSelectable);

	setPoints(origin, target);
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
	const QPainterPath path = makePath();

	if(option->state & QStyle::State_Selected) {
		QPen tmp = painter->pen();

		painter->setPen(QPen(Qt::white, 4));
		painter->drawPath(path);

		painter->setPen(tmp);
	}

	painter->drawPath(path);
}

QPainterPath Edge::shape() const {
	QPainterPathStroker stroker;
	stroker.setWidth(8);

	return stroker.createStroke(makePath());
}

void Edge::setPoints(QPointF origin, QPointF target) {
	const QRectF origBbox = boundingRect();

	m_origin = origin;
	m_target = target;

	if(scene())
		scene()->update(boundingRect().united(origBbox));

	prepareGeometryChange();
}

const QPointF& Edge::origin() const {
	return m_origin;
}

const QPointF& Edge::target() const {
	return m_target;
}
