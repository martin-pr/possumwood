#include "timeline_widget.h"

#include <QHBoxLayout>

#include <possumwood_sdk/app.h>

TimelineWidget::TimelineWidget(QWidget* parent) : QWidget(parent) {
	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);

	m_timeline = new Timeline();

	layout->addWidget(m_timeline, 1);

	// signal for time changing in App
	m_timeChangedConnection = possumwood::App::instance().onTimeChanged([this](float t) {
		m_timeline->setValue(t);
	});

	// response to time changed by clicking in the timeline widget
	connect(m_timeline, &Timeline::valueChanged, [this](float t) {
		possumwood::App::instance().setTime(t);
	});
}

TimelineWidget::~TimelineWidget() {
	m_timeChangedConnection.disconnect();
}
