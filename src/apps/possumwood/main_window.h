#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextBrowser>

#include <possumwood_sdk/app.h>
#include <possumwood_sdk/editor.h>

#include "adaptor.h"
#include "properties.h"
#include "viewport.h"
#include "timeline_widget.h"
#include "searchable_menu.h"

class MainWindow : public QMainWindow {
	Q_OBJECT

	public:
		MainWindow();
		~MainWindow();

		/// returns the graph adaptor instance (a QWidget) - used by actions
		Adaptor& adaptor();

		void loadFile(const boost::filesystem::path& filename);

	private slots:
		void draw(float dt);

	private:
		void updateStatusBar();
		void selectionChanged(const dependency_graph::Selection& selection);

		Viewport* m_viewport;
		Adaptor* m_adaptor;
		Properties* m_properties;
		TimelineWidget* m_timeline;
		possumwood::Editor* m_editor;

		QStatusBar* m_statusBar;
		QLabel* m_statusIcon;
		QLabel* m_statusText;
		QPushButton* m_statusDetail;

		QDialog* m_statusDialog;
		QTextBrowser* m_statusContent;

		SearchableMenu* m_newNodeMenu;

		QDockWidget* m_editorDock;
};
