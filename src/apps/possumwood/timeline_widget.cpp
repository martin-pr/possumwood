#include "timeline_widget.h"

#include <QHBoxLayout>
#include <QAction>

#include <possumwood_sdk/app.h>
#include <possumwood_sdk/config.inl>

TimelineWidget::TimelineWidget(QWidget* parent) : QWidget(parent), m_playbackTimer(new QTimer(this)) {
	possumwood::Config& config = possumwood::App::instance().sceneConfig();

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);

	// make the timeline
	m_timeline = new Timeline();
	m_timeline->setRange(std::make_pair(config["start_time"].as<float>(), config["end_time"].as<float>()));
	layout->addWidget(m_timeline, 1);

	// start/end time observers
	m_connections.push_back(config["start_time"].onChanged([this](possumwood::Config::Item& item) {
		m_timeline->setRange(std::make_pair(item.as<float>(), m_timeline->range().second));
	}));

	m_connections.push_back(config["end_time"].onChanged([this](possumwood::Config::Item& item) {
		m_timeline->setRange(std::make_pair(m_timeline->range().first, item.as<float>()));
	}));

	// signal for time changing in App
	m_connections.push_back(possumwood::App::instance().onTimeChanged([this](float t) {
		m_timeline->setValue(t);
	}));

	// response to time changed by clicking in the timeline widget
	connect(m_timeline, &Timeline::valueChanged, [this](float t) {
		possumwood::App::instance().setTime(t);
	});

	// assemble the context menu
	setContextMenuPolicy(Qt::ActionsContextMenu);

	m_playAction = new QAction(QIcon(":icons/player_play.png"), "Play/stop", this);
	m_playAction->setShortcut(Qt::Key_Space);
	m_playAction->setShortcutContext(Qt::ApplicationShortcut);
	connect(m_playAction, SIGNAL(triggered()), this, SLOT(onPlayAction()));
	addAction(m_playAction);

	m_frameFwdAction = new QAction(QIcon(":icons/player_fwd.png"), "One frame forward", this);
	m_frameFwdAction->setShortcut(Qt::Key_Right);
	m_frameFwdAction->setShortcutContext(Qt::ApplicationShortcut);
	connect(m_frameFwdAction, &QAction::triggered, [this]() {
		auto& app = possumwood::App::instance();

		float t = app.time() + 1.0f/app.sceneConfig()["fps"].as<float>();
		t = std::min(t, app.sceneConfig()["end_time"].as<float>());

		app.setTime(t);
	});
	addAction(m_frameFwdAction);

	m_frameBwdAction = new QAction(QIcon(":icons/player_rew.png"), "One frame backward", this);
	m_frameBwdAction->setShortcut(Qt::Key_Left);
	m_frameBwdAction->setShortcutContext(Qt::ApplicationShortcut);
	connect(m_frameBwdAction, &QAction::triggered, [this]() {
		auto& app = possumwood::App::instance();

		float t = app.time() - 1.0f/app.sceneConfig()["fps"].as<float>();
		t = std::max(t, app.sceneConfig()["start_time"].as<float>());

		app.setTime(t);
	});
	addAction(m_frameBwdAction);

	m_playbackTimer->setInterval(10);
	connect(m_playbackTimer, SIGNAL(timeout()), this, SLOT(onTimer()));
	m_playbackTimer->stop();
}

TimelineWidget::~TimelineWidget() {
	for(auto& c : m_connections)
		c.disconnect();
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

	const possumwood::Config& config = possumwood::App::instance().sceneConfig();
	const float startTime = config["start_time"].as<float>();
	const float endTime = config["end_time"].as<float>();

	float t = possumwood::App::instance().time() + elapsed.count();
	while(t > endTime)
		t -= endTime - startTime;

	possumwood::App::instance().setTime(t);

	m_currentTime = now;
}

void TimelineWidget::paintEvent(QPaintEvent* event) {
		QWidget::paintEvent(event);
}
