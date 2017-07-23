#include "timeline_widget.h"

#include <QHBoxLayout>
#include <QAction>

#include <possumwood_sdk/app.h>

namespace {
	static const float minTime = 0.0f;
	static const float maxTime = 10.0f;
}

TimelineWidget::TimelineWidget(QWidget* parent) : QWidget(parent), m_playbackTimer(new QTimer(this)) {
	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);

	m_timeline = new Timeline();
	m_timeline->setRange(std::make_pair(minTime, maxTime));

	layout->addWidget(m_timeline, 1);

	// signal for time changing in App
	m_timeChangedConnection = possumwood::App::instance().onTimeChanged([this](float t) {
		m_timeline->setValue(t);
	});

	// response to time changed by clicking in the timeline widget
	connect(m_timeline, &Timeline::valueChanged, [this](float t) {
		possumwood::App::instance().setTime(t);
	});

	// assemble the context menu
	setContextMenuPolicy(Qt::ActionsContextMenu);

	m_playAction = new QAction("Play/stop", this);
	m_playAction->setShortcut(Qt::Key_Space);
	connect(m_playAction, SIGNAL(triggered()), this, SLOT(onPlayAction()));

	addAction(m_playAction);

	m_playbackTimer->setInterval(10);
	connect(m_playbackTimer, SIGNAL(timeout()), this, SLOT(onTimer()));
	m_playbackTimer->stop();
}

TimelineWidget::~TimelineWidget() {
	m_timeChangedConnection.disconnect();
}

QAction* TimelineWidget::playAction() {
	return m_playAction;
}

void TimelineWidget::onPlayAction() {
	if(m_playbackTimer->isActive()) {
		m_playbackTimer->stop();
		assert(!m_playbackTimer->isActive());
	}
	else {
		m_playbackTimer->start();
		assert(m_playbackTimer->isActive());
		m_currentTime = std::chrono::system_clock::now();
	}
}

void TimelineWidget::onTimer() {
	auto now = std::chrono::system_clock::now();
	std::chrono::duration<float> elapsed = now - m_currentTime;

	float t = possumwood::App::instance().time() + elapsed.count();
	while(t > maxTime)
		t -= maxTime - minTime;

	possumwood::App::instance().setTime(t);

	m_currentTime = now;
}

void TimelineWidget::paintEvent(QPaintEvent* event) {
		QWidget::paintEvent(event);
}
