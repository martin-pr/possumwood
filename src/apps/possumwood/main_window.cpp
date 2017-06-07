#include "main_window.h"

#include <fstream>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QAction>
#include <QHeaderView>
#include <QMenuBar>
#include <QAction>
#include <QMessageBox>
#include <QFileDialog>
#include <QDockWidget>
#include <QApplication>
#include <QDesktopWidget>

#include <bind.h>
#include <dependency_graph/attr.inl>
#include <dependency_graph/metadata.inl>
#include <dependency_graph/values.inl>
#include <dependency_graph/io/graph.h>

#include "adaptor.h"
#include "node_data.h"
#include "metadata.h"
#include "io/io.h"

namespace {

QAction* makeAction(QString title, std::function<void()> fn, QWidget* parent) {
	QAction* result = new QAction(title, parent);
	node_editor::qt_bind(result, SIGNAL(triggered(bool)), fn);
	return result;
}

}

MainWindow::MainWindow() : QMainWindow(), m_nodeCounter(0) {
	// initialise the graph with some nodes (to be removed)
	{
		auto& add = m_graph.nodes().add(Metadata::instance("addition"), "add1");
		add.setBlindData(NodeData{QPointF(100, 20)});

		auto& mult = m_graph.nodes().add(Metadata::instance("multiplication"), "mult1");
		mult.setBlindData(NodeData{QPointF(300, 20)});

		add.port(2).connect(mult.port(0));
	}

	m_viewport = new Viewport();
	setCentralWidget(m_viewport);

	m_properties = new Properties();
	QDockWidget* propDock = new QDockWidget("Properties", this);
	propDock->setWidget(m_properties);
	addDockWidget(Qt::RightDockWidgetArea, propDock);

	m_adaptor = new Adaptor(&m_graph);
	auto geom = m_adaptor->sizeHint();
	geom.setWidth(QApplication::desktop()->screenGeometry().width() / 2 - 200);
	m_adaptor->setSizeHint(geom);

	QDockWidget* graphDock = new QDockWidget("Graph", this);
	graphDock->setWidget(m_adaptor);
	addDockWidget(Qt::LeftDockWidgetArea, graphDock);

	// connect the selection signal
	m_adaptor->scene().setNodeSelectionCallback(
	[&](std::set<std::reference_wrapper<node_editor::Node>, node_editor::GraphScene::NodeRefComparator> selection) {
		m_properties->show(m_adaptor->selectedNodes());
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

			for(auto& m : Metadata::instances()) {
				std::vector<std::string> pieces;
				boost::split(pieces, m.type(), boost::algorithm::is_any_of("/"));

				QMenu* current = contextMenu;
				for(unsigned a=0;a<pieces.size()-1;++a) {
					if(groups[pieces[a]] == nullptr)
						groups[pieces[a]] = current->addMenu(pieces[a].c_str());

					current = groups[pieces[a]];
				}

				current->addAction(makeAction(pieces.back().c_str(), [&m, &pieces, this]() {
					QPointF pos = m_adaptor->mapToScene(m_adaptor->mapFromGlobal(QCursor::pos()));
					m_graph.nodes().add(m, pieces.back() + "_" + std::to_string(m_nodeCounter++), NodeData{pos});
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
	m_graph.onDirty([this]() {
		m_viewport->update();
	});

	////////////////////
	// window actions
	QAction* newAct = new QAction("&New...", this);
	newAct->setShortcuts(QKeySequence::New);
	connect(newAct, &QAction::triggered, [&](bool) {
		QMessageBox::StandardButton res = QMessageBox::question(this, "New file...", "Do you want to clear the scene?");
		if(res == QMessageBox::Yes) {
			m_adaptor->graph().nodes().clear();
			m_filename = "";
		}
	});

	QAction* openAct = new QAction("&Open...", this);
	openAct->setShortcuts(QKeySequence::Open);
	connect(openAct, &QAction::triggered, [this](bool) {
		QString filename = QFileDialog::getOpenFileName(this, tr("Open File"),
		                   m_filename.string().c_str(),
		                   tr("Possumwood files (*.psw)"));

		if(!filename.isEmpty()) {
			try {
				std::ifstream in(filename.toStdString());

				dependency_graph::io::json json;
				in >> json;

				dependency_graph::io::adl_serializer<dependency_graph::Graph>::from_json(json, m_adaptor->graph());

				m_filename = filename.toStdString();
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
		if(m_filename.empty())
			saveAsAct->triggered(true);

		else {
			try {
				std::ofstream out(m_filename.string());

				dependency_graph::io::json json;
				json = m_adaptor->graph();

				out << std::setw(4) << json;
			}
			catch(std::exception& err) {
				QMessageBox::critical(this, "Error saving file...", "Error saving " + QString(m_filename.string().c_str()) + ":\n" + err.what());
			}
			catch(...) {
				QMessageBox::critical(this, "Error saving file...", "Error saving " + QString(m_filename.string().c_str()) + ":\nUnhandled exception thrown during saving.");
			}
		}
	});

	connect(saveAsAct, &QAction::triggered, [saveAct, this](bool) {
		QString filename = QFileDialog::getSaveFileName(this, tr("Save File"),
		                   m_filename.string().c_str(),
		                   tr("Possumwood files (*.psw)"));

		if(!filename.isEmpty()) {
			m_filename = filename.toStdString();

			saveAct->triggered(true);
		}
	});


	////////////////////
	// create menu

	QMenu* fileMenu = menuBar()->addMenu("&File");

	fileMenu->addAction(newAct);
	fileMenu->addAction(openAct);
	fileMenu->addAction(saveAct);

	QMenu* viewMenu = menuBar()->addMenu("&View");
	fileMenu->addAction(saveAsAct);

	viewMenu->addAction(propDock->toggleViewAction());
	viewMenu->addAction(graphDock->toggleViewAction());
}

void MainWindow::draw(float dt) {
	// draw the floor grid
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
	for(auto& n : m_adaptor->graph().nodes()) {
		glPushAttrib(GL_ALL_ATTRIB_BITS);

		const Metadata& meta = dynamic_cast<const Metadata&>(n.metadata());
		meta.draw(dependency_graph::Values(n));

		glPopAttrib();
	}

}
