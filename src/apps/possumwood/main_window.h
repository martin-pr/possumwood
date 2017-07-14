#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QTreeWidget>

#include <possumwood_sdk/app.h>

#include "adaptor.h"
#include "properties.h"
#include "viewport.h"
#include "log.h"

class MainWindow : public QMainWindow {
	Q_OBJECT

	public:
		MainWindow();
		~MainWindow();

	private slots:
		void draw(float dt);

	private:
		Viewport* m_viewport;
		Adaptor* m_adaptor;
		Properties* m_properties;
		Log* m_log;
};
