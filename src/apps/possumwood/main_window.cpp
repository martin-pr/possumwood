#include "main_window.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <QMenuBar>
#include <QAction>
#include <QMessageBox>
#include <QFileDialog>
#include <QDockWidget>
#include <QApplication>
#include <QDesktopWidget>
#include <QVBoxLayout>

#include <qt_node_editor/bind.h>
#include <dependency_graph/values.inl>

#include <possumwood_sdk/metadata.h>
#include <possumwood_sdk/node_data.h>
#include <possumwood_sdk/node_data.h>
#include <possumwood_sdk/app.h>

#include "adaptor.h"

namespace {

QAction* makeAction(QString title, std::function<void()> fn, QWidget* parent) {
	QAction* result = new QAction(title, parent);
	node_editor::qt_bind(result, SIGNAL(triggered(bool)), fn);
	return result;
}

}

MainWindow::MainWindow() : QMainWindow() {
	possumwood::App::instance().setMainWindow(this);

	QWidget* centralWidget = new QWidget();
	QVBoxLayout* centralLayout = new QVBoxLayout(centralWidget);
	centralLayout->setContentsMargins(0, 0, 0, 0);
	centralLayout->setSpacing(0);
	setCentralWidget(centralWidget);

	m_viewport = new Viewport();
	centralLayout->addWidget(m_viewport, 1);

	m_timeline = new TimelineWidget();
	centralLayout->addWidget(m_timeline, 0);

	m_properties = new Properties();
	QDockWidget* propDock = new QDockWidget("Properties", this);
	propDock->setWidget(m_properties);
	m_properties->setMinimumWidth(300);
	addDockWidget(Qt::RightDockWidgetArea, propDock);

	m_adaptor = new Adaptor(&possumwood::App::instance().graph());
	auto geom = m_adaptor->sizeHint();
	geom.setWidth(QApplication::desktop()->screenGeometry().width() / 2 - 200);
	m_adaptor->setSizeHint(geom);

	m_log = new Log();
	QDockWidget* logDock = new QDockWidget("Log", this);
	logDock->setWidget(m_log);
	m_log->setMinimumWidth(300);
	addDockWidget(Qt::RightDockWidgetArea, logDock);
	connect(m_adaptor, &Adaptor::logged, [this](QIcon icon, const QString & msg) {
		m_log->addMessage(icon, msg);
	});

	QDockWidget* graphDock = new QDockWidget("Graph", this);
	graphDock->setWidget(m_adaptor);
	addDockWidget(Qt::LeftDockWidgetArea, graphDock);

	// connect the selection signal
	m_adaptor->scene().setNodeSelectionCallback(
	[&](const node_editor::GraphScene::Selection & selection) {
		m_properties->show(m_adaptor->selection());
	}
	);

	// create the context click menu
	m_adaptor->setContextMenuPolicy(Qt::CustomContextMenu);

	{
		QMenu* contextMenu = new QMenu(m_adaptor);
		connect(m_adaptor, &Adaptor::customContextMenuRequested, [contextMenu, this](QPoint pos) {
			contextMenu->popup(m_adaptor->mapToGlobal(pos));
		});

		{
			std::map<std::string, QMenu*> groups;

			for(auto& m : possumwood::Metadata::instances()) {
				std::vector<std::string> pieces;
				boost::split(pieces, m.type(), boost::algorithm::is_any_of("/"));

				QMenu* current = contextMenu;
				for(unsigned a = 0; a < pieces.size() - 1; ++a) {
					if(groups[pieces[a]] == nullptr)
						groups[pieces[a]] = current->addMenu(pieces[a].c_str());

					current = groups[pieces[a]];
				}

				const std::string name = pieces.back();
				current->addAction(makeAction(name.c_str(), [&m, name, contextMenu, this]() {
					QPointF pos = m_adaptor->mapToScene(m_adaptor->mapFromGlobal(contextMenu->pos()));
					possumwood::App::instance().graph().nodes().add(m, name, possumwood::NodeData{pos});
				}, m_adaptor));
			}
		}

		QAction* separator = new QAction(m_adaptor);
		separator->setSeparator(true);
		contextMenu->addAction(separator);

		QAction* deleteAction = new QAction("Delete selected items", m_adaptor);
		deleteAction->setShortcut(QKeySequence::Delete);
		contextMenu->addAction(deleteAction);
		m_adaptor->addAction(deleteAction);
		node_editor::qt_bind(deleteAction, SIGNAL(triggered()), [this]() {
			m_adaptor->deleteSelected();
		});
	}

	// drawing callback
	connect(m_viewport, SIGNAL(render(float)), this, SLOT(draw(float)));
	possumwood::App::instance().graph().onDirty([this]() {
		m_viewport->update();
	});

	////////////////////
	// window actions
	QAction* newAct = new QAction("&New...", this);
	newAct->setShortcuts(QKeySequence::New);
	connect(newAct, &QAction::triggered, [&](bool) {
		QMessageBox::StandardButton res = QMessageBox::question(this, "New file...", "Do you want to clear the scene?");
		if(res == QMessageBox::Yes) {
			m_properties->show({});
			possumwood::App::instance().newFile();
		}
	});

	QAction* openAct = new QAction("&Open...", this);
	openAct->setShortcuts(QKeySequence::Open);
	connect(openAct, &QAction::triggered, [this](bool) {
		QString filename = QFileDialog::getOpenFileName(this, tr("Open File"),
		                   possumwood::App::instance().filename().string().c_str(),
		                   tr("Possumwood files (*.psw)"));

		if(!filename.isEmpty()) {
			try {
				m_properties->show({});
				possumwood::App::instance().loadFile(filename.toStdString());
			}
			catch(std::exception& err) {
				QMessageBox::critical(this, "Error loading file...", "Error loading " + filename + ":\n" + err.what());
			}
			catch(...) {
				QMessageBox::critical(this, "Error loading file...", "Error loading " + filename + ":\nUnhandled exception thrown during loading.");
			}
		}
	});

	QAction* saveAct = new QAction("&Save...", this);
	saveAct->setShortcuts(QKeySequence::Save);
	QAction* saveAsAct = new QAction("Save &As...", this);
	saveAsAct->setShortcuts(QKeySequence::SaveAs);

	connect(saveAct, &QAction::triggered, [saveAsAct, this](bool) {
		if(possumwood::App::instance().filename().empty())
			saveAsAct->triggered(true);

		else {
			try {
				possumwood::App::instance().saveFile();
			}
			catch(std::exception& err) {
				QMessageBox::critical(this, "Error saving file...", "Error saving " + QString(possumwood::App::instance().filename().string().c_str()) + ":\n" + err.what());
			}
			catch(...) {
				QMessageBox::critical(this, "Error saving file...", "Error saving " + QString(possumwood::App::instance().filename().string().c_str()) + ":\nUnhandled exception thrown during saving.");
			}
		}
	});

	connect(saveAsAct, &QAction::triggered, [saveAct, this](bool) {
		QString filename = QFileDialog::getSaveFileName(this, tr("Save File"),
		                   possumwood::App::instance().filename().string().c_str(),
		                   tr("Possumwood files (*.psw)"));

		if(!filename.isEmpty()) {
			try {
				possumwood::App::instance().saveFile(filename.toStdString());
			}
			catch(std::exception& err) {
				QMessageBox::critical(this, "Error saving file...", "Error saving " + QString(possumwood::App::instance().filename().string().c_str()) + ":\n" + err.what());
			}
			catch(...) {
				QMessageBox::critical(this, "Error saving file...", "Error saving " + QString(possumwood::App::instance().filename().string().c_str()) + ":\nUnhandled exception thrown during saving.");
			}
		}
	});


	////////////////////
	// file menu

	{
		QMenu* fileMenu = menuBar()->addMenu("&File");

		fileMenu->addAction(newAct);
		fileMenu->addAction(openAct);
		fileMenu->addAction(saveAct);
		fileMenu->addAction(saveAsAct);
	}

	///////////////////////////////
	// copy + paste functionality

	{
		QMenu* editMenu = menuBar()->addMenu("&Edit");

		editMenu->addAction(m_adaptor->copyAction());
		editMenu->addAction(m_adaptor->pasteAction());
	}

	/////////////////////
	// view menu

	{
		QMenu* viewMenu = menuBar()->addMenu("&View");

		viewMenu->addAction(propDock->toggleViewAction());
		viewMenu->addAction(graphDock->toggleViewAction());
	}
}

MainWindow::~MainWindow() {
	possumwood::App::instance().graph().clear();
}

void MainWindow::draw(float dt) {
	// draw the floor grid
	glDisable(GL_LIGHTING);
	glBegin(GL_LINES);

	for(int a = -10; a <= 10; ++a) {
		const float c = 0.3f + (float)(a % 10 == 0) * 0.2f;
		glColor3f(c, c, c);

		glVertex3f(a, 0, -10);
		glVertex3f(a, 0, 10);
	}

	for(int a = -10; a <= 10; ++a) {
		const float c = 0.3f + (float)(a % 10 == 0) * 0.2f;
		glColor3f(c, c, c);

		glVertex3f(-10, 0, a);
		glVertex3f(10, 0, a);
	}

	glEnd();

	// draw everything else
	glEnable(GL_LIGHTING);
	for(auto& n : m_adaptor->graph().nodes()) {
		glPushAttrib(GL_ALL_ATTRIB_BITS);

		const possumwood::Metadata& meta = dynamic_cast<const possumwood::Metadata&>(n.metadata());
		meta.draw(dependency_graph::Values(n));

		glPopAttrib();
	}

}
