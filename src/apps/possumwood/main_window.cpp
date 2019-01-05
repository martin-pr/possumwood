#include "main_window.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/filesystem.hpp>

#include <QMenuBar>
#include <QAction>
#include <QMessageBox>
#include <QFileDialog>
#include <QDockWidget>
#include <QApplication>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include <QToolBar>
#include <QLabel>
#include <QInputDialog>

#include <dependency_graph/values.inl>
#include <dependency_graph/metadata_register.h>

#include <qt_node_editor/connected_edge.h>

#include <possumwood_sdk/metadata.inl>
#include <possumwood_sdk/app.h>
#include <possumwood_sdk/gl.h>

#include <actions/actions.h>
#include <actions/node_data.h>

#include "adaptor.h"
#include "config_dialog.h"
#include "grid.h"
#include "toolbar.h"

namespace {

QAction* makeAction(QString title, std::function<void()> fn, QWidget* parent) {
	QAction* result = new QAction(title, parent);
	QObject::connect(result, &QAction::triggered, [fn](bool) { fn(); });
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
	propDock->setObjectName("properties");
	propDock->setWidget(m_properties);
	propDock->toggleViewAction()->setIcon(QIcon(":icons/dock_properties.png"));
	m_properties->setMinimumWidth(420);
	addDockWidget(Qt::RightDockWidgetArea, propDock);

	m_adaptor = new Adaptor(&possumwood::App::instance().graph());
	auto geom = m_adaptor->sizeHint();
	geom.setWidth(QApplication::desktop()->screenGeometry().width() / 2 - 200);
	m_adaptor->setSizeHint(geom);

	QDockWidget* graphDock = new QDockWidget("Graph", this);
	graphDock->setObjectName("graph");
	graphDock->setWidget(m_adaptor);
	graphDock->toggleViewAction()->setIcon(QIcon(":icons/dock_graph.png"));
	addDockWidget(Qt::LeftDockWidgetArea, graphDock);

	QDockWidget* editorDock = new QDockWidget("Editor", this);
	editorDock->setObjectName("editor");
	QLabel* editorWidget = new QLabel("No editable node selected");
	editorWidget->setAlignment(Qt::AlignCenter);
	editorWidget->setMinimumHeight(100);
	editorDock->setWidget(editorWidget);
	editorDock->toggleViewAction()->setIcon(QIcon(":icons/dock_editor.png"));
	addDockWidget(Qt::RightDockWidgetArea, editorDock);

	// connect the selection signal
	connect(m_adaptor, &Adaptor::selectionChanged,
			[editorDock, this](dependency_graph::Selection selection) {

				// show the selection in the properties dock
				m_properties->show(selection);

				// instantiate the editor
				{
					unsigned editorCounter = 0;
					for(auto& n : selection.nodes()) {
						const possumwood::Metadata* meta = dynamic_cast<const possumwood::Metadata*>(&n.get().metadata().metadata());
						if(meta != nullptr && meta->hasEditor())
							++editorCounter;
					}

					if(editorCounter == 0) {
						QLabel* editorWidget = new QLabel("No editable node selected");
						editorWidget->setAlignment(Qt::AlignCenter);
						editorWidget->setMinimumHeight(100);
						editorDock->setWidget(editorWidget);

						m_editor.reset();
					}
					else if(editorCounter > 1) {
						QLabel* editorWidget = new QLabel("More than one editable node selected");
						editorWidget->setAlignment(Qt::AlignCenter);
						editorWidget->setMinimumHeight(100);
						editorDock->setWidget(editorWidget);

						m_editor.reset();
					}
					else {
						for(auto& n : selection.nodes()) {
							const possumwood::Metadata* meta = dynamic_cast<const possumwood::Metadata*>(&n.get().metadata().metadata());
							if(meta != nullptr && meta->hasEditor()) {
								m_editor = meta->createEditor(n);
								editorDock->setWidget(m_editor->widget());
							}
						}
					}
				}

				// set the status bar
				updateStatusBar();
			});

	// connect status signal
	m_adaptor->graph().onStateChanged([this](const dependency_graph::NodeBase& node) {
		updateStatusBar();
	});

	// create the context click menu
	m_adaptor->graphWidget()->setContextMenuPolicy(Qt::CustomContextMenu);

	{
		QMenu* contextMenu = new QMenu(m_adaptor);

		{
			m_newNodeMenu = new SearchableMenu("Create");
			m_newNodeMenu->menuAction()->setShortcut(Qt::Key_Tab);
			contextMenu->addMenu(m_newNodeMenu);

			{
				QAction* newNodeAction = new QAction("Create node", m_adaptor);
				newNodeAction->setShortcut(Qt::Key_Tab);
				m_adaptor->addAction(newNodeAction);

				connect(newNodeAction, &QAction::triggered, [this]() {
					if(m_adaptor->underMouse()) {
						m_newNodeMenu->move(QCursor::pos());
						m_newNodeMenu->popup(QCursor::pos());
					}
				});
			}

			// first, create all "folders"
			std::map<std::string, QMenu*> groups;
			for(auto& m : dependency_graph::MetadataRegister::singleton()) {
				std::vector<std::string> pieces;
				boost::split(pieces, m.metadata().type(), boost::algorithm::is_any_of("/"));
				assert(pieces.size() >= 1);

				for(unsigned a = 1; a < pieces.size(); ++a) {
					const std::string current = boost::join(std::make_pair(pieces.begin(), pieces.begin() + a), "/");

					if(groups.find(current) == groups.end()) {
						QMenu* menu = new QMenu(pieces[a - 1].c_str());
						menu->setIcon(QIcon(":icons/package.png"));

						groups.insert(std::make_pair(current, menu));
					}
				}
			}

			// then create all items
			std::map<std::string, QAction*> items;
			for(auto& m : dependency_graph::MetadataRegister::singleton()) {
				std::string itemName = m.metadata().type();

				auto it = m.metadata().type().rfind('/');
				if(it != std::string::npos)
					itemName = m.metadata().type().substr(it + 1);

				QAction* addNode = makeAction(
					itemName.c_str(),
					[&m, itemName, this]() {
						auto qp = m_adaptor->graphWidget()->mapToScene(m_adaptor->graphWidget()->mapFromGlobal(m_newNodeMenu->pos()));
						const possumwood::NodeData::Point p {(float)qp.x(), (float)qp.y()};

						possumwood::actions::createNode(m_adaptor->currentNetwork(), m, itemName, p);
					},
					m_adaptor);
				addNode->setIcon(QIcon(":icons/add-node.png"));

				items.insert(std::make_pair(m.metadata().type(), addNode));
			}

			// then, assemble the menu
			for(auto& m : groups) {
				auto it = m.first.rfind('/');
				if(it != std::string::npos) {
					assert(it > 0);
					std::string parentName = m.first.substr(0, it);

					auto menu = groups.find(parentName);
					assert(menu != groups.end());
					menu->second->addMenu(m.second);
				}
			}

			for(auto& i : items) {
				auto it = i.first.rfind('/');
				if(it != std::string::npos) {
					std::string parentName = i.first.substr(0, it);

					auto menu = groups.find(parentName);
					assert(menu != groups.end());
					menu->second->addAction(i.second);
				}
			}

			// finally, populate the m_newNodeMenu
			for(auto& m : groups)
				if(m.first.find('/') == std::string::npos)
					m_newNodeMenu->addMenu(m.second);
			for(auto& i : items)
				if(i.first.find('/') == std::string::npos)
					m_newNodeMenu->addAction(i.second);
		}

		QAction* separator = new QAction(m_adaptor);
		separator->setSeparator(true);
		contextMenu->addAction(separator);

		contextMenu->addAction(m_adaptor->copyAction());
		contextMenu->addAction(m_adaptor->cutAction());
		contextMenu->addAction(m_adaptor->pasteAction());
		contextMenu->addAction(m_adaptor->deleteAction());

		QAction* separator2 = new QAction(m_adaptor);
		separator2->setSeparator(true);
		contextMenu->addAction(separator2);

		contextMenu->addAction(m_adaptor->undoAction());
		contextMenu->addAction(m_adaptor->redoAction());

		// node-specific items - communicate via currentNode shared ptr
		std::shared_ptr<node_editor::Node*> currentNode(new node_editor::Node*());

		QAction* nodeSeparator = new QAction(m_adaptor);
		nodeSeparator->setSeparator(true);
		contextMenu->addAction(nodeSeparator);

		QAction* renameNodeAction = new QAction(QIcon(":icons/rename.png"), "&Rename node...", this);
		connect(renameNodeAction, &QAction::triggered, [currentNode, this](bool) {
			QString name = QInputDialog::getText(this, "Rename...", "Node name:", QLineEdit::Normal, (*currentNode)->name());

			if(name != (*currentNode)->name()) {
				dependency_graph::NodeBase& depNode = *(m_adaptor->index()[*currentNode].graphNode);
				possumwood::actions::renameNode(depNode, name.toStdString());
			}
		});
		contextMenu->addAction(renameNodeAction);

		QAction* openNetworkAction = new QAction(QIcon(":icons/open-network.png"), "&Open network", this);
		connect(openNetworkAction, &QAction::triggered, [currentNode, this](bool) {
			dependency_graph::NodeBase& depNode = *(m_adaptor->index()[*currentNode].graphNode);
			assert(depNode.is<dependency_graph::Network>());

			if(depNode.is<dependency_graph::Network>())
				m_adaptor->setCurrentNetwork(depNode.as<dependency_graph::Network>());
		});
		contextMenu->addAction(openNetworkAction);

		QAction* exitNetworkAction = new QAction(QIcon(":icons/exit-network.png"), "&Exit network", this);
		connect(exitNetworkAction, &QAction::triggered, [currentNode, this](bool) {
			if(m_adaptor->currentNetwork().hasParentNetwork())
				m_adaptor->setCurrentNetwork(m_adaptor->currentNetwork().network());
		});
		contextMenu->addAction(exitNetworkAction);

		connect(m_adaptor->graphWidget(), &node_editor::GraphWidget::customContextMenuRequested, [=](QPoint pos) {
			contextMenu->move(m_adaptor->graphWidget()->mapToGlobal(pos));  // to make sure pos() is right, which is
															 // used for placing the new node

			// node-specific menu
			node_editor::Node* node = nullptr;
			{
				QGraphicsItem* item = m_adaptor->scene().itemAt(m_adaptor->graphWidget()->transform().inverted().map(pos) , QTransform());
				while(item != nullptr && node == nullptr) {
					node = dynamic_cast<node_editor::Node*>(item);

					item = item->parentItem();
				}
			}
			*currentNode = node;

			nodeSeparator->setVisible(node != nullptr || m_adaptor->currentNetwork().hasParentNetwork());
			renameNodeAction->setVisible(node != nullptr);

			if(*currentNode) {
				dependency_graph::NodeBase& depNode = *(m_adaptor->index()[*currentNode].graphNode);
				openNetworkAction->setVisible(depNode.is<dependency_graph::Network>());
			}
			else
				openNetworkAction->setVisible(false);

			exitNetworkAction->setVisible(node == nullptr && m_adaptor->currentNetwork().hasParentNetwork());

			// show the menu
			contextMenu->popup(m_adaptor->graphWidget()->mapToGlobal(pos));
		});
	}

	// drawing callback
	connect(m_viewport, SIGNAL(render(float)), this, SLOT(draw(float)));
	possumwood::App::instance().graph().onDirty([this]() { m_viewport->update(); });

	// drawable refresh callback
	possumwood::Drawable::onRefreshQueued([this]() { m_viewport->update(); });

	////////////////////
	// window actions
	QAction* newAct = new QAction(QIcon(":icons/filenew.png"), "&New...", this);
	newAct->setShortcuts(QKeySequence::New);
	connect(newAct, &QAction::triggered, [&](bool) {
		QMessageBox::StandardButton res = QMessageBox::question(this, "New file...", "Do you want to clear the scene?");
		if(res == QMessageBox::Yes) {
			m_properties->show({});
			possumwood::App::instance().newFile();
		}
	});

	QAction* openAct = new QAction(QIcon(":icons/fileopen.png"), "&Open...", this);
	openAct->setShortcuts(QKeySequence::Open);
	connect(openAct, &QAction::triggered, [this](bool) {
		QString filename =
			QFileDialog::getOpenFileName(this, tr("Open File"), possumwood::App::instance().filename().string().c_str(),
										 tr("Possumwood files (*.psw)"));

		if(!filename.isEmpty()) {
			try {
				m_properties->show({});
				possumwood::App::instance().loadFile(boost::filesystem::path(filename.toStdString()));
			}
			catch(std::exception& err) {
				QMessageBox::critical(this, "Error loading file...", "Error loading " + filename + ":\n" + err.what());
			}
			catch(...) {
				QMessageBox::critical(this, "Error loading file...",
									  "Error loading " + filename + ":\nUnhandled exception thrown during loading.");
			}
		}
	});

	QAction* saveAct = new QAction(QIcon(":icons/filesave.png"), "&Save...", this);
	saveAct->setShortcuts(QKeySequence::Save);
	QAction* saveAsAct = new QAction(QIcon(":icons/filesaveas.png"), "Save &As...", this);
	saveAsAct->setShortcuts(QKeySequence::SaveAs);

	connect(saveAct, &QAction::triggered, [saveAsAct, this](bool) {
		if(possumwood::App::instance().filename().empty())
			saveAsAct->triggered(true);

		else {
			try {
				possumwood::App::instance().saveFile();
			}
			catch(std::exception& err) {
				QMessageBox::critical(this, "Error saving file...",
									  "Error saving " +
										  QString(possumwood::App::instance().filename().string().c_str()) + ":\n" +
										  err.what());
			}
			catch(...) {
				QMessageBox::critical(this, "Error saving file...",
									  "Error saving " +
										  QString(possumwood::App::instance().filename().string().c_str()) +
										  ":\nUnhandled exception thrown during saving.");
			}
		}
	});

	connect(saveAsAct, &QAction::triggered, [this](bool) {
		QString filename =
			QFileDialog::getSaveFileName(this, tr("Save File"), possumwood::App::instance().filename().string().c_str(),
										 tr("Possumwood files (*.psw)"));

		if(!filename.isEmpty()) {
			try {
				boost::filesystem::path path = filename.toStdString();
				if(!path.has_extension())
					path.replace_extension(".psw");

				possumwood::App::instance().saveFile(path);
			}
			catch(std::exception& err) {
				QMessageBox::critical(this, "Error saving file...",
									  "Error saving " +
										  QString(possumwood::App::instance().filename().string().c_str()) + ":\n" +
										  err.what());
			}
			catch(...) {
				QMessageBox::critical(this, "Error saving file...",
									  "Error saving " +
										  QString(possumwood::App::instance().filename().string().c_str()) +
										  ":\nUnhandled exception thrown during saving.");
			}
		}
	});

	QAction* sceneConfigAct = new QAction(QIcon(":icons/settings-scene.png"), "Scene &configuration...", this);
	connect(sceneConfigAct, &QAction::triggered, [this](bool) {
		ConfigDialog dialog(this, possumwood::App::instance().sceneConfig());
		dialog.setWindowTitle("Scene configuration...");
		dialog.exec();
	});

	QAction* quitAct = new QAction(QIcon(":icons/exit.png"), "&Quit", this);
	connect(quitAct, &QAction::triggered, [this](bool) { close(); });

	//////////////

	QWidget* menuWidget = new QWidget();

	QHBoxLayout* topLayout = new QHBoxLayout(menuWidget);
	topLayout->setContentsMargins(0,0,0,0);

	QVBoxLayout* menuLayout = new QVBoxLayout();
	menuLayout->setContentsMargins(0,0,0,0);
	menuLayout->setSpacing(0);
	topLayout->addLayout(menuLayout, 0);

	QWidget* tabs = new Toolbar();
	topLayout->addWidget(tabs, 1);

	QMenuBar* mainMenu = new QMenuBar();
	menuLayout->addWidget(mainMenu);

	QToolBar* docksToolbar = new QToolBar();
	docksToolbar->setObjectName("docks_toolbar");
	docksToolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	docksToolbar->setIconSize(QSize(32, 32));
	menuLayout->addWidget(docksToolbar);

	setMenuWidget(menuWidget);

	/////////////////////
	// toolbar
	docksToolbar->addAction(graphDock->toggleViewAction());
	docksToolbar->addAction(propDock->toggleViewAction());
	docksToolbar->addAction(editorDock->toggleViewAction());

	////////////////////
	// file menu

	{
		QMenu* fileMenu = mainMenu->addMenu("&File");

		fileMenu->addAction(newAct);
		fileMenu->addAction(openAct);
		fileMenu->addAction(saveAct);
		fileMenu->addAction(saveAsAct);
		fileMenu->addSeparator();
		fileMenu->addAction(sceneConfigAct);
		fileMenu->addSeparator();
		fileMenu->addAction(quitAct);
	}

	///////////////////////////////
	// copy + paste functionality

	{
		QMenu* editMenu = mainMenu->addMenu("&Edit");

		editMenu->addAction(m_adaptor->undoAction());
		editMenu->addAction(m_adaptor->redoAction());

		editMenu->addSeparator();

		editMenu->addAction(m_adaptor->copyAction());
		editMenu->addAction(m_adaptor->cutAction());
		editMenu->addAction(m_adaptor->pasteAction());
		editMenu->addAction(m_adaptor->deleteAction());
	}

	/////////////////////
	// view menu

	{
		QMenu* viewMenu = mainMenu->addMenu("&View");

		viewMenu->addAction(propDock->toggleViewAction());
		viewMenu->addAction(graphDock->toggleViewAction());
		viewMenu->addAction(editorDock->toggleViewAction());
		viewMenu->addSeparator();
		viewMenu->addAction(docksToolbar->toggleViewAction());
	}

	/////////////////////
	// playback menu
	{
		QMenu* playbackMenu = mainMenu->addMenu("&Playback");

		playbackMenu->addAction(m_timeline->playAction());
	}

	/////////////////////
	// status bar
	m_statusBar = new QStatusBar();
	setStatusBar(m_statusBar);

	m_statusIcon = new QLabel();
	m_statusBar->addPermanentWidget(m_statusIcon, 0);

	m_statusText = new QLabel();
	m_statusText->setWordWrap(true);
	m_statusBar->addPermanentWidget(m_statusText, 1);
}

MainWindow::~MainWindow() {
	// first, clear selection to avoid destruction order problems
	m_adaptor->setSelection(dependency_graph::Selection());

	// then clear the graph
	possumwood::App::instance().graph().clear();
}

void MainWindow::draw(float dt) {
	GL_CHECK_ERR;

	// draw the floor grid
	static std::unique_ptr<possumwood::Grid> s_grid(new possumwood::Grid());
	s_grid->draw(m_viewport->projection(), m_viewport->modelview());

	// draw everything else
	possumwood::ViewportState viewport;
	viewport.width = m_viewport->width();
	viewport.height = m_viewport->height();
	viewport.modelview = m_viewport->modelview();
	viewport.projection = m_viewport->projection();

	GL_CHECK_ERR;

	m_adaptor->draw(viewport);

	GL_CHECK_ERR;
}

Adaptor& MainWindow::adaptor() {
	assert(m_adaptor != nullptr);
	return *m_adaptor;
}

void MainWindow::updateStatusBar() {
	auto& index = m_adaptor->index();

	std::string errors, warnings, infos;

	dependency_graph::Selection selection = m_adaptor->selection();
	for(auto& n : selection.nodes()) {
		for(auto& msg : index[n.get().index()].graphNode->state()) {
			if(msg.first == dependency_graph::State::kInfo)
				infos += msg.second;
			if(msg.first == dependency_graph::State::kWarning)
				warnings += msg.second;
			if(msg.first == dependency_graph::State::kError)
				errors += msg.second;
		}
	}

	std::replace(errors.begin(), errors.end(), '\n', '\t');
	std::replace(warnings.begin(), warnings.end(), '\n', '\t');
	std::replace(infos.begin(), infos.end(), '\n', '\t');

	if(!errors.empty()) {
		m_statusIcon->setPixmap(QIcon::fromTheme("dialog-error").pixmap(QSize(16,16)));
		m_statusText->setText(errors.c_str());
	}
	else if(!warnings.empty()) {
		m_statusIcon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(QSize(16,16)));
		m_statusText->setText(warnings.c_str());
	}
	else if(!infos.empty()) {
		m_statusIcon->setPixmap(QIcon::fromTheme("dialog-information").pixmap(QSize(16,16)));
		m_statusText->setText(infos.c_str());
	}
	else {
		m_statusIcon->setPixmap(QPixmap());
		m_statusText->setText("");
	}
}
