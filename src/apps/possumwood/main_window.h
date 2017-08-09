#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QTreeWidget>

#include <possumwood_sdk/app.h>
#include <possumwood_sdk/editor.h>

#include "adaptor.h"
#include "properties.h"
#include "viewport.h"
#include "log.h"
#include "timeline_widget.h"

class MainWindow : public QMainWindow {
	Q_OBJECT

	public:
		MainWindow();
		~MainWindow();

		/// returns the graph adaptor instance (a QWidget) - used by actions
		Adaptor& adaptor();

	private slots:
		void draw(float dt);

	private:
		Viewport* m_viewport;
		Adaptor* m_adaptor;
		Properties* m_properties;
		Log* m_log;
		TimelineWidget* m_timeline;
		std::unique_ptr<possumwood::Editor> m_editor;
};
