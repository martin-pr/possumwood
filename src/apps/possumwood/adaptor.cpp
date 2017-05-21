#include "adaptor.h"

#include <QHBoxLayout>
#include <QMessageBox>

#include "node.inl"

#include "node_data.h"
#include "connected_edge.h"

Adaptor::Adaptor(dependency_graph::Graph* graph) : m_graph(graph) {
	// register callbacks
	m_signals.push_back(graph->onAddNode(
		[&](dependency_graph::Node& node) { onAddNode(node); }
	));

	m_signals.push_back(graph->onRemoveNode(
		[&](dependency_graph::Node& node) { onRemoveNode(node); }
	));

	m_signals.push_back(graph->onConnect(
		[&](dependency_graph::Port& p1, dependency_graph::Port& p2) { onConnect(p1, p2); }
	));

	m_signals.push_back(graph->onDisconnect(
		[&](dependency_graph::Port& p1, dependency_graph::Port& p2) { onDisconnect(p1, p2); }
	));

	m_signals.push_back(graph->onBlindDataChanged(
		[&](dependency_graph::Node& n) { onBlindDataChanged(n); }
	));

	// instantiate the graph widget
	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0,0,0,0);
	setLayout(layout);

	m_graphWidget = new node_editor::GraphWidget();
	layout->addWidget(m_graphWidget);

	m_graphWidget->scene().setMouseConnectionCallback([&](node_editor::Port& p1, node_editor::Port& p2) {
		// find the two nodes that were connected
		auto n1 = m_nodes.right.find(&p1.parentNode());
		assert(n1 != m_nodes.right.end());
		auto n2 = m_nodes.right.find(&p2.parentNode());
		assert(n2 != m_nodes.right.end());

		// and connect them in the graph as well
		try {
			n1->second->port(p1.index()).connect(n2->second->port(p2.index()));
		}
		catch(std::runtime_error& err) {
			// something went wrong during connecting, undo it
			m_graphWidget->scene().disconnect(p1, p2);
			// and make an error dialog with the problem
			QMessageBox::critical(m_graphWidget, "Error connecting nodes", err.what());
		}
	});

	m_graphWidget->scene().setNodeMoveCallback([&](node_editor::Node& node) {
		auto n = m_nodes.right.find(&node);
		assert(n != m_nodes.right.end());

		n->second->setBlindData<NodeData>(NodeData{n->first->pos()});
	});

	// and instantiate the current graph state
	for(auto& n : m_graph->nodes())
		onAddNode(n);

	for(auto& c : m_graph->connections())
		onConnect(c.first, c.second);
}

Adaptor::~Adaptor() {
	for(auto& c : m_signals)
		c.disconnect();
}

void Adaptor::onAddNode(dependency_graph::Node& node) {
	// get the blind data, containing the node's position
	const NodeData& data = node.blindData<NodeData>();

	// instantiate new graphical item
	node_editor::Node& newNode = m_graphWidget->scene().addNode(
		node.name().c_str(), data.position);

	// add all ports, based on the node's metadata
	for(size_t a = 0; a<node.metadata().attributeCount(); ++a) {
		const dependency_graph::Attr& attr = node.metadata().attr(a);

		newNode.addPort(node_editor::Node::PortDefinition{
			attr.name().c_str(),
			attr.category() == dependency_graph::Attr::kInput ? node_editor::Port::kInput : node_editor::Port::kOutput,
			QColor(255,0,0)
		});
	}

	// and register the node in the internal map
	m_nodes.left.insert(std::make_pair(&node, &newNode));
}

void Adaptor::onRemoveNode(dependency_graph::Node& node) {
	// find the item to be deleted
	auto it = m_nodes.left.find(&node);
	assert(it != m_nodes.left.end());

	// and remove it
	m_graphWidget->scene().removeNode(*(it->second));
}

void Adaptor::onConnect(dependency_graph::Port& p1, dependency_graph::Port& p2) {
	// find the two nodes to be connected
	auto n1 = m_nodes.left.find(&p1.node());
	assert(n1 != m_nodes.left.end());
	auto n2 = m_nodes.left.find(&p2.node());
	assert(n2 != m_nodes.left.end());

	// instantiate the connection
	m_graphWidget->scene().connect(n1->second->port(p1.index()), n2->second->port(p2.index()));
}

void Adaptor::onDisconnect(dependency_graph::Port& p1, dependency_graph::Port& p2) {
	// find the two nodes to be disconnected
	auto n1 = m_nodes.left.find(&p1.node());
	assert(n1 != m_nodes.left.end());
	auto n2 = m_nodes.left.find(&p2.node());
	assert(n2 != m_nodes.left.end());

	// remove the connection
	m_graphWidget->scene().disconnect(n1->second->port(p1.index()), n2->second->port(p2.index()));
}

void Adaptor::onBlindDataChanged(dependency_graph::Node& node) {
	auto n = m_nodes.left.find(&node);
	assert(n != m_nodes.left.end());

	const NodeData& data = node.blindData<NodeData>();

	n->second->setPos(data.position);
}

QPointF Adaptor::mapToScene(QPoint pos) const {
	return m_graphWidget->mapToScene(pos);
}

void Adaptor::deleteSelected() {
	unsigned ei = 0;
	while(ei < m_graphWidget->scene().edgeCount()) {
		node_editor::ConnectedEdge& e = m_graphWidget->scene().edge(ei);
		if(e.isSelected()) {
			auto n1 = m_nodes.right.find(&(e.fromPort().parentNode()));
			assert(n1 != m_nodes.right.end());

			auto n2 = m_nodes.right.find(&(e.toPort().parentNode()));
			assert(n2 != m_nodes.right.end());

			n1->second->port(e.fromPort().index()).disconnect(n2->second->port(e.toPort().index()));
		}
		else
			++ei;
	}

	unsigned ni = 0;
	while(ni < m_graphWidget->scene().nodeCount()) {
		node_editor::Node& n = m_graphWidget->scene().node(ni);
		if(n.isSelected()) {
			auto node = m_nodes.right.find(&n);
			assert(node != m_nodes.right.end());

			auto toErase = std::find_if(m_graph->nodes().begin(), m_graph->nodes().end(),
				[&](const dependency_graph::Node& n) {
					return &n == node->second;
				}
			);
			assert(toErase != m_graph->nodes().end());

			m_graph->nodes().erase(toErase);
		}
		else
			++ni;
	}
}

std::vector<std::reference_wrapper<dependency_graph::Node>> Adaptor::selectedNodes() {
	std::vector<std::reference_wrapper<dependency_graph::Node>> result;

	for(unsigned ni=0; ni<m_graphWidget->scene().nodeCount(); ++ni) {
		node_editor::Node& n = m_graphWidget->scene().node(ni);
		if(n.isSelected()) {
			auto node = m_nodes.right.find(&n);
			assert(node != m_nodes.right.end());

			auto nodeRef = std::find_if(m_graph->nodes().begin(), m_graph->nodes().end(),
				[&](const dependency_graph::Node& n) {
					return &n == node->second;
				}
			);

			// deselection happens during the Qt object destruction -> the original
			//   node might not exist
			if(nodeRef != m_graph->nodes().end())
				result.push_back(*nodeRef);
		}
	}

	return result;
}

node_editor::GraphScene& Adaptor::scene() {
	return m_graphWidget->scene();
}

const node_editor::GraphScene& Adaptor::scene() const {
	return m_graphWidget->scene();
}
