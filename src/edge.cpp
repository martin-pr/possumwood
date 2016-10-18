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

	const float x1 = bb1.x() + bb1.width();
	const float y1 = bb1.y() + bb1.height() / 2;
	const float x2 = bb2.x();
	const float y2 = bb2.y() + bb2.height() / 2;

	m_origin = QPointF(x1, y1);
	m_target = QPointF(x2, y2);

	if(scene())
		scene()->update(boundingRect().united(origBbox));
}

QRectF Edge::boundingRect() const {
	const float xMin = min(m_origin.x(), m_target.x(), m_origin.x() + s_curvature, m_target.x() - s_curvature);
	const float xMax = max(m_origin.x(), m_target.x(), m_origin.x() + s_curvature, m_target.x() - s_curvature);
	const float yMin = std::min(m_origin.y(), m_target.y());
	const float yMax = std::max(m_origin.y(), m_target.y());

	const QRectF box(xMin - 1, yMin - 1, xMax - xMin + 2, yMax - yMin + 2);

	return box;
}

void Edge::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	QPainterPath path;

	path.moveTo(m_origin);
	for(unsigned a = 1; a < 20; ++a) {
		const float t = (float)a / 20.0f;
		path.lineTo(
		    powf(1.0f - t, 3) * m_origin +
		    3.0 * (1.0 - t) * (1.0 - t) * t * (m_origin + QPointF(s_curvature, 0)) +
		    3.0 * (1.0 - t) * t * t * (m_target - QPointF(s_curvature, 0)) +
		    t * t * t * m_target
		);
	}
	path.lineTo(m_target);

	painter->drawPath(path);


}
