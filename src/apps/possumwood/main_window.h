#pragma once

#include <GL/glew.h>

#include <possumwood_sdk/app.h>
#include <possumwood_sdk/editor.h>

#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QTreeWidget>

#include "adaptor.h"
#include "properties.h"
#include "timeline_widget.h"
#include "viewport.h"

class MainWindow : public QMainWindow {
	Q_OBJECT

  public:
	class TabFilter;

	MainWindow();
	~MainWindow();

	/// returns the graph adaptor instance (a QWidget) - used by actions
	Adaptor& adaptor();

	void loadFile(const boost::filesystem::path& filename, bool updateFilename = true);

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

	QDockWidget* m_editorDock;

	friend class TabFilter;
};
