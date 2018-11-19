#include "adaptor.h"

#include <map>
#include <array>
#include <functional>
#include <cassert>

#include <GL/gl.h>

#include <QVBoxLayout>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QStyle>

#include <dependency_graph/nodes.inl>
#include <dependency_graph/node_base.inl>
#include <dependency_graph/network.h>
#include <dependency_graph/unique_id.h>

#include <qt_node_editor/connected_edge.h>
#include <possumwood_sdk/metadata.h>
#include <possumwood_sdk/app.h>
#include <possumwood_sdk/gl.h>
#include <possumwood_sdk/colours.h>

#include <actions/node_data.h>
#include <actions/actions.h>

Adaptor::Adaptor(dependency_graph::Graph* graph) : m_graph(graph), m_currentNetwork(NULL), m_sizeHint(400,400) {
	// register callbacks
	m_signals.push_back(graph->onAddNode(
		[this](dependency_graph::NodeBase& node) { onAddNode(node); }
	));

	m_signals.push_back(graph->onRemoveNode(
		[this](dependency_graph::NodeBase& node) { onRemoveNode(node); }
	));

	m_signals.push_back(graph->onConnect(
		[this](dependency_graph::Port& p1, dependency_graph::Port& p2) { onConnect(p1, p2); }
	));

	m_signals.push_back(graph->onDisconnect(
		[this](dependency_graph::Port& p1, dependency_graph::Port& p2) { onDisconnect(p1, p2); }
	));

	m_signals.push_back(graph->onBlindDataChanged(
		[this](dependency_graph::NodeBase& n) { onBlindDataChanged(n); }
	));

	m_signals.push_back(graph->onNameChanged(
		[this](dependency_graph::NodeBase& n) { onNameChanged(n); }
	));

	m_signals.push_back(graph->onStateChanged(
		[this](const dependency_graph::NodeBase& n) { onStateChanged(n); }
	));

	m_signals.push_back(graph->onMetadataChanged(
		[this](dependency_graph::NodeBase& n) { onMetadataChanged(n); }
	));

	// instantiate the graph widget
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(0,0,0,0);
	layout->setSpacing(0);
	setLayout(layout);

	m_pathWidget = new PathWidget(this);
	layout->addWidget(m_pathWidget);

	connect(m_pathWidget, &PathWidget::changeCurrentNetwork, [this](PathWidget::Path path) {
		assert(path.size() >= 1);
		assert(path[0] == possumwood::App::instance().graph().index());

		bool done = false;
		while(path.size() > 1 && !done) {
			auto it = possumwood::App::instance().graph().nodes().find(path.back(), dependency_graph::Nodes::kRecursive);
			if(it != possumwood::App::instance().graph().nodes().end()) {
				dependency_graph::Network& net = dynamic_cast<dependency_graph::Network&>(*it);

				setCurrentNetwork(net, false);
				done = true;
			}
			else
				path.pop_back();
		}

		if(!done)
			setCurrentNetwork(possumwood::App::instance().graph(), false);
	});

	m_graphWidget = new node_editor::GraphWidget();
	layout->addWidget(m_graphWidget);

	connect(&m_graphWidget->scene(), &node_editor::GraphScene::portsConnected, [&](node_editor::Port& p1, node_editor::Port& p2) {
		// find the two nodes that were connected
		auto& n1 = m_index[&p1.parentNode()];
		auto& n2 = m_index[&p2.parentNode()];

		// and connect them in the graph as well
		try {
			if(!n2.graphNode->port(p2.index()).isConnected())
				possumwood::actions::connect(n1.graphNode->port(p1.index()), n2.graphNode->port(p2.index()));
			else
				m_graphWidget->scene().disconnect(p1, p2);
		}
		catch(std::runtime_error& err) {
			// something went wrong during connecting, undo it
			m_graphWidget->scene().disconnect(p1, p2);
			// and make an error dialog with the problem
			QMessageBox::critical(m_graphWidget, "Error connecting nodes", err.what());
		}
	});

	connect(&m_graphWidget->scene(), &node_editor::GraphScene::nodesMoved, [&](const std::set<node_editor::Node*>& nodes) {
		std::map<dependency_graph::NodeBase*, possumwood::NodeData::Point> positions;
		for(auto& nptr : nodes) {
			auto& n = m_index[nptr];

			positions[n.graphNode] = possumwood::NodeData::Point {
				(float)n.editorNode->pos().x(), (float)n.editorNode->pos().y()
			};
		}

		possumwood::actions::move(positions);
	});

	connect(&m_graphWidget->scene(), &node_editor::GraphScene::doubleClicked, [this](node_editor::Node* node) {
		// moving "up"
		if(node == nullptr) {
			if(m_currentNetwork->hasParentNetwork())
				setCurrentNetwork(m_currentNetwork->network());
		}

		// moving "into"
		else {
			dependency_graph::NodeBase& nb = *m_index[node].graphNode;
			if(nb.is<dependency_graph::Network>())
				setCurrentNetwork(nb.as<dependency_graph::Network>());
		}
	});

	// setup copy+paste action
	m_copy = new QAction(QIcon(":icons/edit-copy.png"), "C&opy", this);
	m_copy->setShortcut(QKeySequence::Copy);
	m_copy->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(m_copy, &QAction::triggered, [this](bool) {
		possumwood::actions::copy(selection());
	});
	addAction(m_copy);

	m_cut = new QAction(QIcon(":icons/edit-cut.png"), "&Cut", this);
	m_cut->setShortcut(QKeySequence::Cut);
	m_cut->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(m_cut, &QAction::triggered, [this](bool) {
		possumwood::actions::cut(selection());
	});
	addAction(m_cut);

	m_paste = new QAction(QIcon(":icons/edit-paste.png"), "&Paste", this);
	m_paste->setShortcut(QKeySequence::Paste);
	m_paste->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(m_paste, &QAction::triggered, [this](bool) {
		dependency_graph::Selection sel;
		possumwood::actions::paste(currentNetwork(), sel);
		setSelection(sel);
	});
	addAction(m_paste);

	m_delete = new QAction(QIcon(":icons/edit-delete.png"), "&Delete", this);
	m_delete->setShortcut(QKeySequence::Delete);
	m_delete->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(m_delete, &QAction::triggered, [this](bool) {
		possumwood::actions::remove(selection());
	});
	addAction(m_delete);

	m_undo = new QAction(QIcon(":icons/edit-undo.png"), "&Undo", this);
	m_undo->setShortcut(QKeySequence::Undo);
	m_undo->setShortcutContext(Qt::ApplicationShortcut);
	connect(m_undo, &QAction::triggered, [](bool) {
		possumwood::App::instance().undoStack().undo();
	});
	addAction(m_undo);

	m_redo = new QAction(QIcon(":icons/edit-redo.png"), "&Redo", this);
	// m_redo->setShortcut(QKeySequence::Redo);
	m_redo->setShortcut(Qt::CTRL + Qt::Key_Y);
	m_redo->setShortcutContext(Qt::ApplicationShortcut);
	connect(m_redo, &QAction::triggered, [](bool) {
		possumwood::App::instance().undoStack().redo();
	});
	addAction(m_redo);

	// set the m_currentNetwork to point to the graph
	setCurrentNetwork(*graph);
}

Adaptor::~Adaptor() {
	for(auto& c : m_signals)
		c.disconnect();
}

namespace {
	void addToIndex(possumwood::Index& index, dependency_graph::NodeBase* node, node_editor::Node* uiNode) {
		const possumwood::Metadata* meta = dynamic_cast<const possumwood::Metadata*>(&node->metadata().metadata());

		// create a drawable (if factory returns anything)
		std::unique_ptr<possumwood::Drawable> drawable;
		if(meta != nullptr)
			drawable = meta->createDrawable(dependency_graph::Values(*node));

		// add the new item to index (uiNode and drawable can be nullptr)
		index.add(possumwood::Index::Item(
			node,
			uiNode,
			std::move(drawable)
		));

		// continue recursively
		if(node->is<dependency_graph::Network>()) {
			dependency_graph::Network& net = node->as<dependency_graph::Network>();
			for(auto& child : net.nodes())
				addToIndex(index, &child, nullptr);
		}
	}
}

void Adaptor::onAddNode(dependency_graph::NodeBase& node) {
	// get the possumwood::Metadata pointer from the metadata's blind data
	const possumwood::Metadata* meta = dynamic_cast<const possumwood::Metadata*>(&node.metadata().metadata());
	node_editor::Node* uiNode = nullptr;

	// create visual representation of the node only if in current network
	if(&node.network() == m_currentNetwork) {

		// get the blind data, containing the node's position
		const possumwood::NodeData& data = node.blindData<possumwood::NodeData>();

		// instantiate new graphical item
		std::string typeString = node.metadata()->type();
		{
			auto pos = typeString.rfind("/");
			if(pos != std::string::npos)
				typeString = typeString.substr(pos+1);
		}

		// node colour - hardcoded different colour for networks (for now)
		QColor nodeColor(64,64,64);
		if(node.metadata()->type() == "network")
			nodeColor = QColor(32,64,96);
		if(node.metadata()->type() == "input")
			nodeColor = QColor(96,64,32);
		if(node.metadata()->type() == "output")
			nodeColor = QColor(64,96,32);

		// make the new node
		node_editor::Node& newNode = m_graphWidget->scene().addNode(
			node.name().c_str(), typeString.c_str(), QPointF(data.position().x, data.position().y), nodeColor);
		uiNode = &newNode;

		// add all ports, based on the node's metadata
		for(size_t a = 0; a < node.metadata()->attributeCount(); ++a) {
			const dependency_graph::Attr& attr = node.metadata()->attr(a);

			std::array<float, 3> colour{{1,1,1}};
			if(meta)
				colour = possumwood::Colours::get(attr.type());

			newNode.addPort(node_editor::Node::PortDefinition {
				attr.name().c_str(),
				attr.category() == dependency_graph::Attr::kInput ? node_editor::Port::kInput : node_editor::Port::kOutput,
				QColor(colour[0]*255, colour[1]*255, colour[2]*255)
			});
		}
	}

	addToIndex(m_index, &node, uiNode);
}

namespace {
	void removeFromIndex(possumwood::Index& index, const dependency_graph::UniqueId& id) {
		auto it = index.find(id);

		if(it != index.end()) {
			possumwood::Index::Item& item = it->second;

			if(item.graphNode->is<dependency_graph::Network>()) {
				dependency_graph::Network& net = item.graphNode->as<dependency_graph::Network>();
				for(auto& node : net.nodes())
					removeFromIndex(index, node.index());
			}

			index.remove(id);
		}
	}
}

void Adaptor::onRemoveNode(dependency_graph::NodeBase& node) {
	const auto id = node.index();

	if(&node.network() == m_currentNetwork) {
		// find the item to be deleted
		auto& it = m_index[id];

		// and remove it
		m_graphWidget->scene().removeNode(*(it.editorNode));
	}

	// and delete it from the list of nodes
	removeFromIndex(m_index, id);
}

void Adaptor::onConnect(dependency_graph::Port& p1, dependency_graph::Port& p2) {
	assert(&p1.node().network() == &p2.node().network());
	if(&p1.node().network() == m_currentNetwork) {
		// find the two nodes to be connected
		auto& n1 = m_index[p1.node().index()];
		auto& n2 = m_index[p2.node().index()];

		assert(n1.editorNode->portCount() > p1.index());
		assert(n2.editorNode->portCount() > p2.index());

		// instantiate the connection
		m_graphWidget->scene().connect(n1.editorNode->port(p1.index()), n2.editorNode->port(p2.index()));
	}
}

void Adaptor::onDisconnect(dependency_graph::Port& p1, dependency_graph::Port& p2) {
	assert(&p1.node().network() == &p2.node().network());
	if(&p1.node().network() == m_currentNetwork) {
		// find the two nodes to be disconnected
		auto& n1 = m_index[p1.node().index()];
		auto& n2 = m_index[p2.node().index()];

		assert(n1.editorNode->portCount() > p1.index());
		assert(n2.editorNode->portCount() > p2.index());

		// remove the connection
		m_graphWidget->scene().disconnect(n1.editorNode->port(p1.index()), n2.editorNode->port(p2.index()));
	}
}

void Adaptor::onBlindDataChanged(dependency_graph::NodeBase& node) {
	if(&node.network() == m_currentNetwork) {
		const possumwood::NodeData& data = node.blindData<possumwood::NodeData>();

		auto& n = m_index[node.index()];

		n.editorNode->setPos(QPointF(data.position().x, data.position().y));
	}
}

void Adaptor::onNameChanged(dependency_graph::NodeBase& node) {
	if(&node.network() == m_currentNetwork) {
		auto& n = m_index[node.index()];

		n.editorNode->setName(node.name().c_str());
	}
}

void Adaptor::onStateChanged(const dependency_graph::NodeBase& node) {
	if(&node.network() == m_currentNetwork) {
		auto& n = m_index[node.index()];

		// count the different error messages
		unsigned info = 0, warn = 0, err = 0;

		for(auto& m : node.state()) {
			switch(m.first) {
				case dependency_graph::State::kInfo: ++info; break;
				case dependency_graph::State::kWarning: ++warn; break;
				case dependency_graph::State::kError: ++err; break;
			}
		}

		possumwood::Drawable* drw = m_index[&node].drawable.get();
		if(drw)
			for(auto& m : drw->drawState()) {
				switch(m.first) {
					case dependency_graph::State::kInfo: ++info; break;
					case dependency_graph::State::kWarning: ++warn; break;
					case dependency_graph::State::kError: ++err; break;
				}
			}

		if(err > 0)
			n.editorNode->setState(node_editor::Node::kError);
		else if(warn > 0)
			n.editorNode->setState(node_editor::Node::kWarning);
		else if(info > 0)
			n.editorNode->setState(node_editor::Node::kInfo);
		else
			n.editorNode->setState(node_editor::Node::kOk);
	}
}

void Adaptor::onMetadataChanged(dependency_graph::NodeBase& node) {
	if(&node.network() == m_currentNetwork) {
		auto& n = m_index[node.index()];

#ifndef NDEBUG
		// make sure there are no connections
		for(unsigned ei=0;ei<m_graphWidget->scene().edgeCount();++ei) {
			auto& e = m_graphWidget->scene().edge(ei);
			assert(&e.fromPort().parentNode() != n.editorNode);
			assert(&e.toPort().parentNode() != n.editorNode);
		}
#endif
		// remove and add the node
		onRemoveNode(node);
		onAddNode(node);
	}
}

QPointF Adaptor::mapToScene(QPoint pos) const {
	return m_graphWidget->mapToScene(pos);
}

dependency_graph::Selection Adaptor::selection() const {
	dependency_graph::Selection result;

	for(unsigned ni=0; ni<m_graphWidget->scene().nodeCount(); ++ni) {
		node_editor::Node& n = m_graphWidget->scene().node(ni);
		if(n.isSelected()) {
			auto& node = m_index[&n];

			auto nodeRef = std::find_if(m_currentNetwork->nodes().begin(dependency_graph::Nodes::kRecursive), m_currentNetwork->nodes().end(),
				[&](const dependency_graph::NodeBase& n) {
					return &n == node.graphNode;
				}
			);

			// deselection happens during the Qt object destruction -> the original
			//   node might not exist
			if(nodeRef != m_currentNetwork->nodes().end())
				result.addNode(*nodeRef);
		}
	}

	for(unsigned ei=0; ei<m_graphWidget->scene().edgeCount(); ++ei) {
		node_editor::ConnectedEdge& e = m_graphWidget->scene().edge(ei);
		if(e.isSelected()) {
			auto& n1 = m_index[&e.fromPort().parentNode()];
			auto& n2 = m_index[&e.toPort().parentNode()];

			result.addConnection(n1.graphNode->port(e.fromPort().index()), n2.graphNode->port(e.toPort().index()));
		}
	}

	return result;
}

void Adaptor::setSelection(const dependency_graph::Selection& selection) {
	for(auto& n : m_index)
		if(n.second.editorNode != nullptr)
			n.second.editorNode->setSelected(selection.nodes().find(std::ref(*n.second.graphNode)) != selection.nodes().end());

	for(unsigned ei=0; ei<m_graphWidget->scene().edgeCount(); ++ei) {
		node_editor::ConnectedEdge& e = m_graphWidget->scene().edge(ei);

		dependency_graph::Port& p1 = m_index[&e.fromPort().parentNode()].graphNode->port(e.fromPort().index());
		dependency_graph::Port& p2 = m_index[&e.toPort().parentNode()].graphNode->port(e.toPort().index());

		auto it = selection.connections().find(dependency_graph::Selection::Connection{std::ref(p1), std::ref(p2)});

		e.setSelected(it != selection.connections().end());
	}
}

node_editor::GraphScene& Adaptor::scene() {
	return m_graphWidget->scene();
}

const node_editor::GraphScene& Adaptor::scene() const {
	return m_graphWidget->scene();
}

dependency_graph::Graph& Adaptor::graph() {
	return *m_graph;
}

node_editor::GraphWidget* Adaptor::graphWidget() {
	return m_graphWidget;
}

void Adaptor::setSizeHint(const QSize& sh) {
	m_sizeHint = sh;
}

QSize Adaptor::sizeHint() const {
	return m_sizeHint;
}

QAction* Adaptor::copyAction() const {
	return m_copy;
}

QAction* Adaptor::cutAction() const {
	return m_cut;
}

QAction* Adaptor::pasteAction() const {
	return m_paste;
}

QAction* Adaptor::deleteAction() const {
	return m_delete;
}

QAction* Adaptor::undoAction() const {
	return m_undo;
}

QAction* Adaptor::redoAction() const {
	return m_redo;
}

void Adaptor::draw(const possumwood::Drawable::ViewportState& viewport) {
	GL_CHECK_ERR;

	for(auto& n : m_index) {
		GL_CHECK_ERR;

		if(n.second.drawable != nullptr) {
			const auto currentDrawState = n.second.drawable->drawState();

			GL_CHECK_ERR;
			n.second.drawable->doDraw(viewport);
			GL_CHECK_ERR;

			if(n.second.drawable->drawState() != currentDrawState)
				onStateChanged(*n.second.graphNode);
		}

		GL_CHECK_ERR;
	}

	GL_CHECK_ERR;
}

const possumwood::Index& Adaptor::index() const {
	return m_index;
}

void Adaptor::setCurrentNetwork(dependency_graph::Network& n, bool recordHistory) {
	// clear current view
	while(m_graphWidget->scene().edgeCount() > 0)
		// onDisconnect()
		m_graphWidget->scene().disconnect(m_graphWidget->scene().edge(0));

	while(m_graphWidget->scene().nodeCount() > 0) {
		dependency_graph::NodeBase* node = m_index[&m_graphWidget->scene().node(0)].graphNode;
		assert(node != nullptr);

		onRemoveNode(*node);
	}

	// set the m_currentNetwork, which will represent the network everywhere in this class
	m_currentNetwork = &n;

	// and fill the widget
	for(auto& node : n.nodes())
		onAddNode(node);

	for(auto& c : n.connections())
		onConnect(c.first, c.second);

	// refresh the viewport
	possumwood::Drawable::refresh();

	// and change the path widget
	if(recordHistory)
		m_pathWidget->setPath(n);
}

dependency_graph::Network& Adaptor::currentNetwork() {
	assert(m_currentNetwork != nullptr);
	return *m_currentNetwork;
}

// ACTIONS SHOULD BE PERFOMED ON CURRENT NETWORK
