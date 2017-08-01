#include "adaptor.h"

#include <map>
#include <array>
#include <functional>

#include <GL/gl.h>

#include <QHBoxLayout>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QStyle>

#include <dependency_graph/node.inl>
#include <dependency_graph/io/graph.h>

#include <qt_node_editor/connected_edge.h>
#include <possumwood_sdk/node_data.h>
#include <possumwood_sdk/metadata.h>
#include <possumwood_sdk/app.h>

#include "actions.h"

namespace {

/// creates a unique colour for a datatype
QColor colour(const std::string& datatype) {
	static std::map<std::string, QColor> s_colours;
	auto it = s_colours.find(datatype);
	if(it != s_colours.end())
		return it->second;

	const unsigned hash = std::hash<std::string>()(datatype);
	QColor result(
		(unsigned char)hash,
		(unsigned char)(hash >> 8),
		(unsigned char)(hash >> 16)
	);

	s_colours.insert(std::make_pair(datatype, result));

	return result;
}

}

Adaptor::Adaptor(dependency_graph::Graph* graph) : m_graph(graph), m_sizeHint(400,400) {
	// register callbacks
	m_signals.push_back(graph->onAddNode(
		[this](dependency_graph::Node& node) { onAddNode(node); }
	));

	m_signals.push_back(graph->onRemoveNode(
		[this](dependency_graph::Node& node) { onRemoveNode(node); }
	));

	m_signals.push_back(graph->onConnect(
		[this](dependency_graph::Port& p1, dependency_graph::Port& p2) { onConnect(p1, p2); }
	));

	m_signals.push_back(graph->onDisconnect(
		[this](dependency_graph::Port& p1, dependency_graph::Port& p2) { onDisconnect(p1, p2); }
	));

	m_signals.push_back(graph->onBlindDataChanged(
		[this](dependency_graph::Node& n) { onBlindDataChanged(n); }
	));

	m_signals.push_back(graph->onNameChanged(
		[this](dependency_graph::Node& n) { onNameChanged(n); }
	));

	m_signals.push_back(graph->onStateChanged(
		[this](const dependency_graph::Node& n) { onStateChanged(n); }
	));

	m_signals.push_back(graph->onLog(
		[this](dependency_graph::State::MessageType t, const std::string& msg) { onLog(t, msg); }
	));

	// instantiate the graph widget
	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0,0,0,0);
	setLayout(layout);

	m_graphWidget = new node_editor::GraphWidget();
	layout->addWidget(m_graphWidget);

	m_graphWidget->scene().setMouseConnectionCallback([&](node_editor::Port& p1, node_editor::Port& p2) {
		// find the two nodes that were connected
		auto& n1 = m_index[&p1.parentNode()];
		auto& n2 = m_index[&p2.parentNode()];

		// and connect them in the graph as well
		try {
			auto action = Actions::connect(n1.graphNode->port(p1.index()), n2.graphNode->port(p2.index()));

			possumwood::App::instance().undoStack().execute(action);
		}
		catch(std::runtime_error& err) {
			// something went wrong during connecting, undo it
			m_graphWidget->scene().disconnect(p1, p2);
			// and make an error dialog with the problem
			QMessageBox::critical(m_graphWidget, "Error connecting nodes", err.what());
		}
	});

	m_graphWidget->scene().setNodeMoveCallback([&](node_editor::Node& node) {
		auto& n = m_index[&node];

		Actions::move(*n.graphNode, n.editorNode->pos());
	});

	m_graphWidget->scene().setNodeInfoCallback([&](const node_editor::Node& node) {
		auto& n = m_index[const_cast<node_editor::Node*>(&node)]; // hack for bimap

		std::stringstream ss;
		ss << "<span style=\"color:#fff;\">" << n.graphNode->name() << " (" << n.graphNode->metadata().type() << ")" << "</p>";
		ss << "<br />" << std::endl;
		for(auto& i : n.graphNode->state()) {
			switch(i.first) {
				case dependency_graph::State::kInfo:
					ss << "<br /><span style=\"color:#aaa\">";
					break;
				case dependency_graph::State::kWarning:
					ss << "<br /><span style=\"color:#ff0\">";
					break;
				case dependency_graph::State::kError:
					ss << "<br /><span style=\"color:#f00\">";
					break;
			}

			ss << i.second << "</span>" << std::endl;
		}

		return ss.str();
	});

	// and instantiate the current graph state
	for(auto& n : m_graph->nodes())
		onAddNode(n);

	for(auto& c : m_graph->connections())
		onConnect(c.first, c.second);

	// setup copy+paste action
	m_copy = new QAction(QIcon(":icons/edit-copy.png"), "C&opy", this);
	m_copy->setShortcut(QKeySequence::Copy);
	m_copy->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(m_copy, &QAction::triggered, [this](bool) {
		Actions::copy(selection());
	});
	addAction(m_copy);

	m_cut = new QAction(QIcon(":icons/edit-cut.png"), "&Cut", this);
	m_cut->setShortcut(QKeySequence::Cut);
	m_cut->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(m_cut, &QAction::triggered, [this](bool) {
		auto action = Actions::cut(selection());

		possumwood::App::instance().undoStack().execute(action);
	});
	addAction(m_cut);

	m_paste = new QAction(QIcon(":icons/edit-paste.png"), "&Paste", this);
	m_paste->setShortcut(QKeySequence::Paste);
	m_paste->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(m_paste, &QAction::triggered, [this](bool) {
		dependency_graph::Selection sel;
		auto action = Actions::paste(sel);

		possumwood::App::instance().undoStack().execute(action);
		setSelection(sel);
	});
	addAction(m_paste);

	m_delete = new QAction(QIcon(":icons/edit-delete.png"), "&Delete", this);
	m_delete->setShortcut(QKeySequence::Delete);
	m_delete->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(m_delete, &QAction::triggered, [this](bool) {
		auto action = Actions::remove(selection());

		possumwood::App::instance().undoStack().execute(action);
	});
	addAction(m_delete);

	m_undo = new QAction(QIcon(":icons/edit-undo.png"), "&Undo", this);
	m_undo->setShortcut(QKeySequence::Undo);
	m_undo->setShortcutContext(Qt::ApplicationShortcut);
	connect(m_undo, &QAction::triggered, [this](bool) {
		possumwood::App::instance().undoStack().undo();
	});
	addAction(m_undo);

	m_redo = new QAction(QIcon(":icons/edit-redo.png"), "&Redo", this);
	// m_redo->setShortcut(QKeySequence::Redo);
	m_redo->setShortcut(Qt::CTRL + Qt::Key_Y);
	m_redo->setShortcutContext(Qt::ApplicationShortcut);
	connect(m_redo, &QAction::triggered, [this](bool) {
		possumwood::App::instance().undoStack().redo();
	});
	addAction(m_redo);
}

Adaptor::~Adaptor() {
	for(auto& c : m_signals)
		c.disconnect();
}

void Adaptor::onAddNode(dependency_graph::Node& node) {
	// get the blind data, containing the node's position
	const possumwood::NodeData& data = node.blindData<possumwood::NodeData>();

	// instantiate new graphical item
	node_editor::Node& newNode = m_graphWidget->scene().addNode(
		node.name().c_str(), data.position());

	// add all ports, based on the node's metadata
	for(size_t a = 0; a < node.metadata().attributeCount(); ++a) {
		const dependency_graph::Attr& attr = node.metadata().attr(a);

		newNode.addPort(node_editor::Node::PortDefinition {
			attr.name().c_str(),
			attr.category() == dependency_graph::Attr::kInput ? node_editor::Port::kInput : node_editor::Port::kOutput,
			colour(attr.type().name())
		});
	}

	// create a drawable (if factory returns anything)
	const possumwood::Metadata& meta = dynamic_cast<const possumwood::Metadata&>(node.metadata());
	std::unique_ptr<possumwood::Drawable> drawable = meta.createDrawable(dependency_graph::Values(node));

	// and register the node in the internal index
	m_index.add(Index::Item{
		&node,
		&newNode,
		std::move(drawable)
	});
}

void Adaptor::onRemoveNode(dependency_graph::Node& node) {
	// find the item to be deleted
	const auto id = node.blindData<possumwood::NodeData>().id();
	auto& it = m_index[id];

	// and remove it
	m_graphWidget->scene().removeNode(*(it.editorNode));

	// and delete it from the list of nodes
	m_index.remove(id);
}

void Adaptor::onConnect(dependency_graph::Port& p1, dependency_graph::Port& p2) {
	// find the two nodes to be connected
	auto& n1 = m_index[p1.node().blindData<possumwood::NodeData>().id()];
	auto& n2 = m_index[p2.node().blindData<possumwood::NodeData>().id()];

	assert(n1.editorNode->portCount() > p1.index());
	assert(n2.editorNode->portCount() > p2.index());

	// instantiate the connection
	m_graphWidget->scene().connect(n1.editorNode->port(p1.index()), n2.editorNode->port(p2.index()));
}

void Adaptor::onDisconnect(dependency_graph::Port& p1, dependency_graph::Port& p2) {
	// find the two nodes to be disconnected
	auto& n1 = m_index[p1.node().blindData<possumwood::NodeData>().id()];
	auto& n2 = m_index[p2.node().blindData<possumwood::NodeData>().id()];

	assert(n1.editorNode->portCount() > p1.index());
	assert(n2.editorNode->portCount() > p2.index());

	// remove the connection
	m_graphWidget->scene().disconnect(n1.editorNode->port(p1.index()), n2.editorNode->port(p2.index()));
}

void Adaptor::onBlindDataChanged(dependency_graph::Node& node) {
	const possumwood::NodeData& data = node.blindData<possumwood::NodeData>();

	auto& n = m_index[data.id()];

	n.editorNode->setPos(data.position());
}

void Adaptor::onNameChanged(dependency_graph::Node& node) {
	auto& n = m_index[node.blindData<possumwood::NodeData>().id()];

	n.editorNode->setName(node.name().c_str());
}

void Adaptor::onStateChanged(const dependency_graph::Node& node) {
	auto& n = m_index[node.blindData<possumwood::NodeData>().id()];

	// count the different error messages
	unsigned info = 0, warn = 0, err = 0;
	for(auto& m : node.state()) {
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

void Adaptor::onLog(dependency_graph::State::MessageType t, const std::string& msg) {
	switch(t) {
		case dependency_graph::State::kInfo:
			emit logged(style()->standardIcon(QStyle::SP_MessageBoxInformation), msg.c_str());
			break;
		case dependency_graph::State::kWarning:
			emit logged(style()->standardIcon(QStyle::SP_MessageBoxWarning), msg.c_str());
			break;
		case dependency_graph::State::kError:
			emit logged(style()->standardIcon(QStyle::SP_MessageBoxCritical), msg.c_str());
			break;
		default:
			emit logged(style()->standardIcon(QStyle::SP_MessageBoxQuestion), msg.c_str());
			break;
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

			auto nodeRef = std::find_if(m_graph->nodes().begin(), m_graph->nodes().end(),
				[&](const dependency_graph::Node& n) {
					return &n == node.graphNode;
				}
			);

			// deselection happens during the Qt object destruction -> the original
			//   node might not exist
			if(nodeRef != m_graph->nodes().end())
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

void Adaptor::draw() {
	for(auto& n : m_index) {
		glPushAttrib(GL_ALL_ATTRIB_BITS);

		if(n.second.drawable != nullptr)
			n.second.drawable->draw();

		glPopAttrib();
	}

}

Index& Adaptor::index() {
	return m_index;
}
