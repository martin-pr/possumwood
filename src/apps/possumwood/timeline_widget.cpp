#include "timeline_widget.h"

#include <QHBoxLayout>

TimelineWidget::TimelineWidget(QWidget* parent) : QWidget(parent) {
	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);

	m_timeline = new Timeline();

	layout->addWidget(m_timeline, 1);
}
