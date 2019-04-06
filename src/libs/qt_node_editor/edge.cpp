#include "edge.h"

#include <cassert>
#include <iostream>
#include <cmath>

#include <QPainter>
#include <QGraphicsScene>
#include <QStyleOptionGraphicsItem>
#include <QPainterPathStroker>

#include "port.h"
#include "graph_scene.h"

namespace node_editor {

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

	setZValue(0);
}

Edge::~Edge() {
	GraphScene* s = dynamic_cast<GraphScene*>(scene());
	assert(s);

	// s->removeItem(this);
	s->remove(this);
}

QRectF Edge::boundingRect() const {
	const QPointF p1 = m_origin;
	const QPointF p2 = m_target;

	const float tangent = s_curvature * pow(length(m_origin - m_target) / 20.0f, 0.25);

	const float xMin = min(p1.x(), p2.x(),
	                       p1.x() + tangent * (m_originDirection == Port::Orientation::kHorizontal),
	                       p2.x() - tangent * (m_targetDirection == Port::Orientation::kHorizontal));
	const float xMax = max(p1.x(), p2.x(),
	                       p1.x() + tangent * (m_originDirection == Port::Orientation::kHorizontal),
	                       p2.x() - tangent * (m_targetDirection == Port::Orientation::kHorizontal));
	const float yMin = min(p1.y(), p2.y(),
	                       p1.y() + tangent * (m_originDirection == Port::Orientation::kVertical),
	                       p2.y() - tangent * (m_targetDirection == Port::Orientation::kVertical));
	const float yMax = max(p1.y(), p2.y(),
	                       p1.y() + tangent * (m_originDirection == Port::Orientation::kVertical),
	                       p2.y() - tangent * (m_targetDirection == Port::Orientation::kVertical));

	const QRectF box(xMin - 3, yMin - 3, xMax - xMin + 6, yMax - yMin + 6);

	return box;
}

QPointF Edge::bezierPoint(float t) const {
	const float tangent = s_curvature * pow(length(m_origin - m_target) / 20.0f, 0.25);

	QPointF p1_tangent;
	if(m_originDirection == Port::Orientation::kHorizontal)
		p1_tangent = QPointF(tangent, 0);
	else
		p1_tangent = QPointF(0, tangent);

	QPointF p2_tangent;
	if(m_targetDirection == Port::Orientation::kHorizontal)
		p2_tangent = QPointF(tangent, 0);
	else
		p2_tangent = QPointF(0, tangent);


	return evalBezier(
	           t,
	           m_origin,
	           m_origin + p1_tangent,
	           m_target - p2_tangent,
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
		painter->setPen(QPen(Qt::white, 4));
		painter->drawPath(path);
	}

	painter->setPen(m_pen);
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

	prepareGeometryChange();

	if(scene())
		scene()->update(boundingRect().united(origBbox));
}

const QPointF& Edge::origin() const {
	return m_origin;
}

const QPointF& Edge::target() const {
	return m_target;
}

void Edge::setPen(QPen pen) {
	m_pen = pen;
}

const QPen& Edge::pen() const {
	return m_pen;
}

void Edge::setDirection(Port::Orientation ori, Port::Orientation tgt) {
	m_originDirection = ori;
	m_targetDirection = tgt;

	setPoints(m_origin, m_target);
}

}
