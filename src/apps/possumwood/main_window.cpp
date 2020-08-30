#include "main_window.h"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>

#include <QAction>
#include <QApplication>
#include <QDesktopWidget>
#include <QDialogButtonBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QInputDialog>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QScreen>
#include <QTextBrowser>
#include <QToolBar>
#include <QVBoxLayout>

#include <actions/actions.h>
#include <actions/node_data.h>
#include <dependency_graph/metadata_register.h>
#include <possumwood_sdk/app.h>
#include <possumwood_sdk/gl.h>
#include <qt_node_editor/connected_edge.h>
#include <dependency_graph/values.inl>
#include <possumwood_sdk/metadata.inl>

#include "adaptor.h"
#include "config_dialog.h"
#include "error_dialog.h"
#include "grid.h"
#include "node_menu.h"
#include "searchable_menu.h"
#include "toolbar.h"

class MainWindow::TabFilter : public QObject {
  public:
	TabFilter(MainWindow* win) : QObject(win), m_win(win) {
	}

	bool eventFilter(QObject* o, QEvent* e) {
		QKeyEvent* key = dynamic_cast<QKeyEvent*>(e);
		if(key && key->key() == Qt::Key_Tab && m_win->m_adaptor->graphWidget()->underMouse())
			m_win->m_adaptor->graphWidget()->setFocus(Qt::ShortcutFocusReason);

		return false;
	}

  private:
	MainWindow* m_win;
};

MainWindow::MainWindow() : QMainWindow(), m_editor(nullptr) {
	possumwood::App::instance().setMainWindow(this);

	installEventFilter(new TabFilter(this));

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

	m_editorDock = new QDockWidget("Editor", this);
	m_editorDock->setObjectName("editor");
	m_editorDock->toggleViewAction()->setIcon(QIcon(":icons/dock_editor.png"));
	addDockWidget(Qt::RightDockWidgetArea, m_editorDock);

	// connect the selection signal - changes the editor and updates the status bar
	connect(m_adaptor, &Adaptor::selectionChanged, this, &MainWindow::selectionChanged);

	// connect status signal
	m_adaptor->graph().onStateChanged([this](const dependency_graph::NodeBase& node) { updateStatusBar(); });

	// create the context click menu
	m_adaptor->graphWidget()->setContextMenuPolicy(Qt::CustomContextMenu);

	{
		// create the context menu to be popped (parented to graph widget, where it will
		// appear)
		QMenu* contextMenu = new QMenu(m_adaptor->graphWidget());

		{
			NodeMenu builder(m_adaptor);
			builder.addFromNodeRegister(dependency_graph::MetadataRegister::singleton());
			builder.addFromDirectory(possumwood::Filepath::fromString("$NODES").toPath());
			std::unique_ptr<QMenu> newNodeMenu = builder.build();

			newNodeMenu->setIcon(QIcon(":icons/edit-add.png"));
			contextMenu->addMenu(newNodeMenu.release());
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
			QString name =
			    QInputDialog::getText(this, "Rename...", "Node name:", QLineEdit::Normal, (*currentNode)->name());

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
				QGraphicsItem* item =
				    m_adaptor->scene().itemAt(m_adaptor->graphWidget()->transform().inverted().map(pos), QTransform());
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

			m_adaptor->setCurrentNetwork(possumwood::App::instance().graph());

			selectionChanged(dependency_graph::Selection());
		}
	});

	QAction* openAct = new QAction(QIcon(":icons/fileopen.png"), "&Open...", this);
	openAct->setShortcuts(QKeySequence::Open);
	connect(openAct, &QAction::triggered, [this](bool) {
		QString filename = QFileDialog::getOpenFileName(
		    this, tr("Open File"), possumwood::App::instance().filename().toPath().string().c_str(),
		    tr("Possumwood files (*.psw)"));

		if(!filename.isEmpty())
			loadFile(filename.toStdString());
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
				                          QString(possumwood::App::instance().filename().toString().c_str()) + ":\n" +
				                          err.what());
			}
			catch(...) {
				QMessageBox::critical(this, "Error saving file...",
				                      "Error saving " +
				                          QString(possumwood::App::instance().filename().toString().c_str()) +
				                          ":\nUnhandled exception thrown during saving.");
			}
		}
	});

	connect(saveAsAct, &QAction::triggered, [this](bool) {
		QString filename = QFileDialog::getSaveFileName(
		    this, tr("Save File"), possumwood::App::instance().filename().toPath().string().c_str(),
		    tr("Possumwood files (*.psw)"));

		if(!filename.isEmpty()) {
			try {
				boost::filesystem::path path = filename.toStdString();
				if(!path.has_extension())
					path.replace_extension(".psw");

				possumwood::App::instance().saveFile(possumwood::Filepath::fromPath(path));
			}
			catch(std::exception& err) {
				QMessageBox::critical(this, "Error saving file...",
				                      "Error saving " +
				                          QString(possumwood::App::instance().filename().toString().c_str()) + ":\n" +
				                          err.what());
			}
			catch(...) {
				QMessageBox::critical(this, "Error saving file...",
				                      "Error saving " +
				                          QString(possumwood::App::instance().filename().toString().c_str()) +
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
	topLayout->setContentsMargins(0, 0, 0, 0);

	QVBoxLayout* menuLayout = new QVBoxLayout();
	menuLayout->setContentsMargins(0, 0, 0, 0);
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
	docksToolbar->addAction(m_editorDock->toggleViewAction());

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
		viewMenu->addAction(m_editorDock->toggleViewAction());
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

	m_statusDetail = new QPushButton();
	m_statusDetail->setText("more...");
	m_statusDetail->setStyleSheet("padding: 0 3px;");
	m_statusDetail->setFlat(true);
	m_statusDetail->setVisible(false);
	m_statusBar->addPermanentWidget(m_statusDetail, 0);

	m_statusDialog = new QDialog(this);
	m_statusDialog->resize(qApp->primaryScreen()->size().width() * 3 / 4,
	                       qApp->primaryScreen()->size().height() * 3 / 4);
	QVBoxLayout* diaLayout = new QVBoxLayout(m_statusDialog);
	// diaLayout->setContentsMargins(0, 0, 0, 0);
	m_statusContent = new QTextBrowser();
	diaLayout->addWidget(m_statusContent, 1);
	QDialogButtonBox* diaButtons = new QDialogButtonBox(QDialogButtonBox::Ok);
	diaLayout->addWidget(diaButtons, 0);

	connect(diaButtons, &QDialogButtonBox::accepted, m_statusDialog, &QDialog::reject);
	connect(m_statusDetail, &QPushButton::pressed, m_statusDialog, &QDialog::show);

	// update selection and status bar
	selectionChanged(dependency_graph::Selection());
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
	s_grid->draw(m_viewport->viewportState().projection(), m_viewport->viewportState().modelview());

	GL_CHECK_ERR;

	// draw everything else
	m_adaptor->draw(m_viewport->viewportState());

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
				infos += msg.second + "\n";
			if(msg.first == dependency_graph::State::kWarning)
				warnings += msg.second + "\n";
			if(msg.first == dependency_graph::State::kError)
				errors += msg.second + "\n";
		}

		boost::optional<possumwood::Drawable&> drw = possumwood::Metadata::getDrawable(n);
		if(drw)
			for(auto& msg : drw->drawState()) {
				if(msg.first == dependency_graph::State::kInfo)
					infos += msg.second + "\n";
				if(msg.first == dependency_graph::State::kWarning)
					warnings += msg.second + "\n";
				if(msg.first == dependency_graph::State::kError)
					errors += msg.second + "\n";
			}
	}

	errors = boost::trim_copy(errors);
	warnings = boost::trim_copy(warnings);
	infos = boost::trim_copy(infos);

	if(!errors.empty()) {
		m_statusIcon->setPixmap(QIcon::fromTheme("dialog-error").pixmap(QSize(16, 16)));
		m_statusContent->setText(errors.c_str());

		auto lineend = errors.find('\n');
		if(lineend != std::string::npos)
			errors = errors.substr(0, lineend) + " ...";

		m_statusText->setText(errors.c_str());
		m_statusDetail->setVisible(lineend != std::string::npos);
	}
	else if(!warnings.empty()) {
		m_statusIcon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(QSize(16, 16)));
		m_statusContent->setText(warnings.c_str());

		auto lineend = warnings.find('\n');
		if(lineend != std::string::npos)
			warnings = warnings.substr(0, lineend) + " ...";

		m_statusText->setText(warnings.c_str());
		m_statusDetail->setVisible(lineend != std::string::npos);
	}
	else if(!infos.empty()) {
		m_statusIcon->setPixmap(QIcon::fromTheme("dialog-information").pixmap(QSize(16, 16)));
		m_statusContent->setText(infos.c_str());

		auto lineend = infos.find('\n');
		if(lineend != std::string::npos)
			infos = infos.substr(0, lineend) + " ...";

		m_statusText->setText(infos.c_str());
		m_statusDetail->setVisible(lineend != std::string::npos);
	}
	else {
		m_statusIcon->setPixmap(QPixmap());
		m_statusText->setText("");
		m_statusDetail->setVisible(false);
	}
}

void MainWindow::selectionChanged(const dependency_graph::Selection& selection) {
	// show the selection in the properties dock
	m_properties->show(selection);

	// instantiate the editor
	{
		if(m_editor != nullptr) {
			m_editor->deleteLater();
			m_editor = nullptr;
		}

		unsigned editorCounter = 0;
		for(auto& n : selection.nodes()) {
			const possumwood::Metadata* meta =
			    dynamic_cast<const possumwood::Metadata*>(&n.get().metadata().metadata());
			if(meta != nullptr && meta->hasEditor())
				++editorCounter;
		}

		if(editorCounter == 0) {
			QWidget* descriptionWidget = new QWidget();
			QVBoxLayout* layout = new QVBoxLayout(descriptionWidget);

			QTextBrowser* tb = new QTextBrowser();
			tb->setOpenExternalLinks(true);
			tb->setHtml(possumwood::App::instance().sceneDescription().html().c_str());
			layout->addWidget(tb, 1);

			QDialogButtonBox* buttons = new QDialogButtonBox();
			QPushButton* edit = buttons->addButton("Edit scene description...", QDialogButtonBox::ActionRole);
			layout->addWidget(buttons, 0);

			connect(edit, &QPushButton::pressed, [this, tb]() {
				QDialog* editorDialog = new QDialog(this);
				editorDialog->setMinimumSize(QSize(width() / 2, height() / 2));

				QVBoxLayout* layout = new QVBoxLayout(editorDialog);

				QTextEdit* editor = new QTextEdit();
				editor->setText(possumwood::App::instance().sceneDescription().markdown().c_str());
				layout->addWidget(editor, 1);

				QDialogButtonBox* buttons = new QDialogButtonBox();
				buttons->addButton(QDialogButtonBox::Ok);
				buttons->addButton(QDialogButtonBox::Cancel);
				layout->addWidget(buttons, 0);

				connect(buttons, SIGNAL(accepted()), editorDialog, SLOT(accept()));
				connect(buttons, SIGNAL(rejected()), editorDialog, SLOT(reject()));

				int result = editorDialog->exec();
				if(result == QDialog::Accepted) {
					possumwood::App::instance().sceneDescription().setMarkdown(
					    editor->document()->toPlainText().toStdString());
					tb->setHtml(possumwood::App::instance().sceneDescription().html().c_str());
				}

				editorDialog->deleteLater();
			});

			m_editorDock->setWidget(descriptionWidget);
		}
		else if(editorCounter > 1) {
			QLabel* editorWidget = new QLabel("More than one editable node selected");
			editorWidget->setAlignment(Qt::AlignCenter);
			editorWidget->setMinimumHeight(100);
			m_editorDock->setWidget(editorWidget);
		}
		else {
			for(auto& n : selection.nodes()) {
				const possumwood::Metadata* meta =
				    dynamic_cast<const possumwood::Metadata*>(&n.get().metadata().metadata());
				if(meta != nullptr && meta->hasEditor()) {
					m_editor = meta->createEditor(n).release();
					m_editorDock->setWidget(m_editor);
				}
			}
		}
	}

	// set the status bar
	updateStatusBar();
}

void MainWindow::loadFile(const boost::filesystem::path& filename, bool updateFilename) {
	m_properties->show({});
	const dependency_graph::State state =
	    possumwood::App::instance().loadFile(possumwood::Filepath::fromPath(filename), updateFilename);

	m_adaptor->setCurrentNetwork(possumwood::App::instance().graph());
	selectionChanged(dependency_graph::Selection());

	if(state.errored()) {
		std::stringstream ss;
		ss << "Error loading " << filename << "...";

		ErrorDialog* err = new ErrorDialog(state, possumwood::App::instance().mainWindow(), ss.str().c_str());
		err->show();
	}
}
