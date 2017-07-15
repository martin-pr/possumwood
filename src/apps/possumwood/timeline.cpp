#include "timeline.h"

#include <iostream>
#include <cassert>
#include <sstream>
#include <cmath>

#include <QPainter>
#include <QMouseEvent>

using std::cout;
using std::endl;

Timeline::Timeline(QWidget* parent) : QWidget(parent), m_range(0, 100), m_value(0), m_tickDistance(8), m_labelDistance(80) {
	setMinimumHeight(40);
}

Timeline::~Timeline() {
}

int Timeline::positionFromValue(float value) const {
	return std::round((value - m_range.first) / (m_range.second - m_range.first)  * (float)width());
}

float Timeline::valueFromPosition(int pos) const {
	const float val = (float)pos / (float)width() * (m_range.second - m_range.first) + m_range.first;
	return std::max(std::min(val, m_range.second), m_range.first);
}

void Timeline::paintEvent(QPaintEvent* event) {
	QPainter painter(this);

	// fill the background
	painter.setBrush(QColor(20, 20, 20));
	painter.setPen(QColor(20, 20, 20));
	painter.drawRect(rect());

	// get the font properties
	const QFontMetrics metrics(font());

	// draw the ticks
	{
		painter.setPen(QColor(64, 64, 64));

		unsigned end = std::floor(m_range.second);
		unsigned step = tickSkip(m_tickDistance);
		for(unsigned a = std::ceil(m_range.first); a < end; a += step) {
			const int pos = positionFromValue(a);
			painter.drawLine(pos, metrics.height() + 6, pos, height());
		}
	}

	// draw the units
	{
		painter.setPen(QColor(128, 128, 128));

		unsigned end = std::floor(m_range.second);
		unsigned step = tickSkip(m_labelDistance);
		for(unsigned a = std::ceil(m_range.first); a < end; a += step) {
			const int pos = positionFromValue(a);
			painter.drawLine(pos, metrics.height() + 2, pos, height());

			std::stringstream label;
			label << a;
			painter.drawText(pos, metrics.height(), label.str().c_str());
		}
	}

	// draw the current value
	QPen pen(QColor(255, 128, 0), 3);
	painter.setPen(pen);

	const int pos = positionFromValue(m_value);
	painter.drawLine(pos, 0, pos, height());

	// painter.setFont(font());
}

void Timeline::setRange(std::pair<float, float> range) {
	m_range = range;

	m_value = std::min(std::max(m_value, m_range.first), m_range.second);

	repaint();
}

const std::pair<float, float>& Timeline::range() const {
	return m_range;
}

void Timeline::setValue(float val) {
	m_value = std::min(std::max(val, m_range.first), m_range.second);

	repaint();
}

float Timeline::value() const {
	return m_value;
}

void Timeline::mousePressEvent(QMouseEvent* event) {
	if(event->button() == Qt::LeftButton) {
		const float value = valueFromPosition(event->x());
		setValue(value);

		emit valueChanged(value);
	}
	else
		QWidget::mousePressEvent(event);
}

void Timeline::mouseMoveEvent(QMouseEvent* event) {
	if(event->buttons() & Qt::LeftButton) {
		const float value = valueFromPosition(event->x());
		setValue(value);

		emit valueChanged(value);
	}
	else
		QWidget::mouseMoveEvent(event);
}

void Timeline::setTickDistance(unsigned dist) {
	assert(dist > 0);
	m_tickDistance = dist;
}

unsigned Timeline::tickDistance() const {
	return m_tickDistance;
}

void Timeline::setLabelDistance(unsigned dist) {
	assert(dist > 0);
	m_labelDistance = dist;
}

unsigned Timeline::labelDistance() const {
	return m_labelDistance;
}

unsigned Timeline::tickSkip(unsigned dist) const {
	// crazy cases
	if(m_range.second - m_range.first < 1.0f)
		return 1;

	// compute the mantissa and exponent
	const float singleTickDist = (m_range.second - m_range.first) * (float)dist / (float)width();
	const int exponent = std::floor(log10(singleTickDist));
	const float mantissa = singleTickDist / std::pow(10.0f, exponent);

	// enough space = just draw all sticks
	if(exponent < 0)
		return 1;

	const float d1 = std::fabs(1.0f - mantissa);
	const float d2 = std::fabs(2.0f - mantissa);
	const float d5 = std::fabs(5.0f - mantissa);
	const float d10 = std::fabs(10.0f - mantissa);

	unsigned m;
	if((d1 <= d2) && (d1 <= d5) && (d1 <= d10))
		m = 1;
	else if((d2 <= d1) && (d2 <= d5) && (d2 <= d10))
		m = 2;
	else if((d5 <= d1) && (d5 <= d2) && (d5 <= d10))
		m = 5;
	else
		m = 10;

	return m * std::pow(10, exponent);
}
