#pragma once

#include <chrono>

#include <boost/signals2.hpp>

#include <QtGui/QPaintEvent>
#include <QtCore/QTimer>

#include "timeline.h"

/// A simple stand-alone widget, allowing to change the current time and star/end of the timelne.
/// Directly changes the current time in App singleton, which in turn fires off a signal
/// to all registered listeners.
class TimelineWidget : public QWidget {
	Q_OBJECT

	public:
		TimelineWidget(QWidget* parent = NULL);
		virtual ~TimelineWidget();

		QAction* playAction();

	protected:
		virtual void paintEvent(QPaintEvent* event) override;

	private slots:
		void onPlayAction();
		void onTimer();

	private:
		Timeline* m_timeline;

		QAction *m_playAction, *m_frameFwdAction, *m_frameBwdAction;
		QTimer* m_playbackTimer;

		std::vector<boost::signals2::connection> m_connections;

		std::chrono::time_point<std::chrono::system_clock> m_currentTime;
};
