#include "actions.h"

#include <functional>

#include <QApplication>
#include <QClipboard>
#include <QMainWindow>

#include <dependency_graph/io/graph.h>
#include <dependency_graph/node_base.inl>
#include <dependency_graph/nodes.inl>

#include <possumwood_sdk/app.h>

#include "main_window.h"

namespace {

dependency_graph::NodeBase& findNode(const dependency_graph::UniqueId& id) {
	if(id == possumwood::AppCore::instance().graph().index())
		return possumwood::AppCore::instance().graph();

	auto it = possumwood::AppCore::instance().graph().nodes().find(id, dependency_graph::Nodes::kRecursive);
	assert(it != possumwood::AppCore::instance().graph().nodes().end());

	// and get the node reference
	return *it;
}

dependency_graph::NodeBase& doCreateNode(const dependency_graph::UniqueId& currentNetworkIndex, const dependency_graph::MetadataHandle& meta, const std::string& name, const dependency_graph::UniqueId& id, const possumwood::NodeData& blindData, boost::optional<const dependency_graph::Datablock&> data = boost::optional<const dependency_graph::Datablock&>()) {
	if(data)
		assert(data->meta() == meta);

	std::unique_ptr<dependency_graph::BaseData> bd(new dependency_graph::Data<possumwood::NodeData>(blindData));

	dependency_graph::NodeBase& netBase = findNode(currentNetworkIndex);
	assert(netBase.is<dependency_graph::Network>());

	dependency_graph::NodeBase& n = netBase.as<dependency_graph::Network>().nodes().add(meta, name, std::move(bd), data, id);
	assert(n.index() == id);
	return n;
}

void doRemoveNode(const dependency_graph::UniqueId& id) {
	auto& graph = possumwood::AppCore::instance().graph();
	auto it = std::find_if(graph.nodes().begin(dependency_graph::Nodes::kRecursive), graph.nodes().end(), [&](const dependency_graph::NodeBase & i) {
		return i.index() == id;
	});

	assert(it != graph.nodes().end());

	it->network().nodes().erase(it);
}

void doConnect(const dependency_graph::UniqueId& fromNode, std::size_t fromPort, const dependency_graph::UniqueId& toNode, std::size_t toPort) {
	dependency_graph::NodeBase& from = findNode(fromNode);
	dependency_graph::NodeBase& to = findNode(toNode);

	from.port(fromPort).connect(to.port(toPort));
}

void doDisconnect(const dependency_graph::UniqueId& fromNode, std::size_t fromPort, const dependency_graph::UniqueId& toNode, std::size_t toPort) {
	dependency_graph::NodeBase& from = findNode(fromNode);
	dependency_graph::NodeBase& to = findNode(toNode);

	from.port(fromPort).disconnect(to.port(toPort));
}

void doSetBlindData(const dependency_graph::UniqueId& node, const possumwood::NodeData& blindData) {
	dependency_graph::NodeBase& n = findNode(node);
	n.setBlindData(blindData);
}

} // anonymous namespace

/////////////////////////////////////////////////////////////////////

namespace {

possumwood::UndoStack::Action createNodeAction(dependency_graph::Network& current, const dependency_graph::MetadataHandle& meta, const std::string& name, const possumwood::NodeData& _data) {
	possumwood::NodeData data;
	data.setPosition(_data.position());

	dependency_graph::UniqueId id;

	possumwood::UndoStack::Action action;
	action.addCommand(
		std::bind(&doCreateNode, current.index(), std::ref(meta), name, id, data, boost::optional<const dependency_graph::Datablock&>()),
		std::bind(&doRemoveNode, id)
	);

	return action;
}

}

void Actions::createNode(dependency_graph::Network& current, const dependency_graph::MetadataHandle& meta, const std::string& name, const possumwood::NodeData& _data) {
	auto action = createNodeAction(current, meta, name, _data);

	possumwood::AppCore::instance().undoStack().execute(action);
}

possumwood::UndoStack::Action disconnectAction(dependency_graph::Port& p1, dependency_graph::Port& p2) {
	possumwood::UndoStack::Action action;

	action.addCommand(
		std::bind(&doDisconnect,
			p1.node().index(), p1.index(),
			p2.node().index(), p2.index()),
		std::bind(&doConnect,
			p1.node().index(), p1.index(),
			p2.node().index(), p2.index())
	);

	return action;
}

namespace {

possumwood::UndoStack::Action removeNetworkAction(dependency_graph::Network& net);

possumwood::UndoStack::Action removeNodeAction(dependency_graph::NodeBase& node) {
	possumwood::UndoStack::Action action;
	const dependency_graph::NodeBase& cnode = node;

	// recusively remove all sub-nodes
	if(node.is<dependency_graph::Network>())
		action.append(removeNetworkAction(node.as<dependency_graph::Network>()));

	// and remove current node
	action.addCommand(
		std::bind(&doRemoveNode, node.index()),
		std::bind(&doCreateNode, node.network().index(), std::ref(node.metadata()), node.name(), node.index(),
			node.blindData<possumwood::NodeData>(), cnode.datablock())
	);

	return action;
}

possumwood::UndoStack::Action removeNetworkAction(dependency_graph::Network& net) {
	possumwood::UndoStack::Action action;

	// remove all connections
	for(auto& e : net.connections())
		action.append(disconnectAction(e.first, e.second));

	/// and all nodes
	for(auto& n : net.nodes())
		action.append(removeNodeAction(n));

	return action;
}

possumwood::UndoStack::Action removeAction(const dependency_graph::Selection& _selection) {
	// add all connections to selected nodes - they'll be removed as well as the selected connections
	//   with the removed nodes
	dependency_graph::Selection selection = _selection;
	for(auto& c : possumwood::AppCore::instance().graph().connections()) {
		auto& n1 = c.first.node();
		auto& n2 = c.second.node();

		if(selection.nodes().find(n1) != selection.nodes().end())
			selection.addConnection(c.first, c.second);
		if(selection.nodes().find(n2) != selection.nodes().end())
			selection.addConnection(c.first, c.second);
	}

	// this will be the resulting action
	possumwood::UndoStack::Action action;

	// remove all connections
	for(auto& e : selection.connections())
		action.append(disconnectAction(e.from, e.to));

	/// and all nodes
	for(auto& n : selection.nodes())
		action.append(removeNodeAction(n));

	return action;
}

}

void Actions::removeNode(dependency_graph::NodeBase& node) {
	dependency_graph::Selection selection;
	selection.addNode(node);

	auto action = removeAction(selection);

	possumwood::AppCore::instance().undoStack().execute(action);
}

void Actions::connect(dependency_graph::Port& p1, dependency_graph::Port& p2) {
	possumwood::UndoStack::Action action;

	action.addCommand(
		std::bind(&doConnect,
			p1.node().index(), p1.index(),
			p2.node().index(), p2.index()),
		std::bind(&doDisconnect,
			p1.node().index(), p1.index(),
			p2.node().index(), p2.index())
	);

	possumwood::AppCore::instance().undoStack().execute(action);
}

void Actions::disconnect(dependency_graph::Port& p1, dependency_graph::Port& p2) {
	auto action = disconnectAction(p1, p2);

	possumwood::AppCore::instance().undoStack().execute(action);
}

void Actions::remove(const dependency_graph::Selection& selection) {
	auto action = removeAction(selection);

	possumwood::AppCore::instance().undoStack().execute(action);
}

void Actions::cut(const dependency_graph::Selection& selection) {
	// trigger the copy action first
	copy(selection);

	// and delete selection
	auto action = removeAction(selection);

	possumwood::AppCore::instance().undoStack().execute(action);
}

void Actions::copy(const dependency_graph::Selection& selection) {
	dependency_graph::Network* net = &possumwood::AppCore::instance().graph();
	if(!selection.empty() && selection.nodes().begin()->get().hasParentNetwork())
		net = &selection.nodes().begin()->get().network();

	// convert the selection to JSON string
	dependency_graph::io::json json;
	dependency_graph::io::to_json(json, *net, selection);

	std::stringstream ss;
	ss << std::setw(4) << json;

	// and put it to the clipboard
	QApplication::clipboard()->setText(ss.str().c_str());
}

namespace {
	possumwood::UndoStack::Action pasteNetwork(const dependency_graph::UniqueId& targetIndex, const dependency_graph::Network& source) {
		possumwood::UndoStack::Action action;

		// add all the nodes to the parent network
		//  - each node has a unique ID (unique between all graphs), store that
		for(auto& n : source.nodes()) {
			possumwood::NodeData d = n.blindData<possumwood::NodeData>();
			d.setPosition(QPointF(20, 20) + d.position());

			const dependency_graph::NodeBase& cn = n;
			action.addCommand(
				std::bind(&doCreateNode, targetIndex, dependency_graph::MetadataHandle(n.metadata()), n.name(), n.index(), d, cn.datablock()),
				std::bind(&doRemoveNode, n.index())
			);

			// recurse to add nested networks
			if(cn.is<dependency_graph::Network>())
				action.append(pasteNetwork(n.index(), n.as<dependency_graph::Network>()));
		}

		// add all connections, based on "unique" IDs
		for(auto& c : source.connections()) {
			dependency_graph::UniqueId id1 = c.first.node().index();
			dependency_graph::UniqueId id2 = c.second.node().index();

			action.addCommand(
				std::bind(&doConnect, id1, c.first.index(), id2, c.second.index()),
				std::bind(&doDisconnect, id1, c.first.index(), id2, c.second.index())
			);
		}

		return action;
	}
}

void Actions::paste(dependency_graph::Network& current, dependency_graph::Selection& selection) {
	possumwood::UndoStack::Action action;

	dependency_graph::Graph pastedGraph;

	try {
		// convert the selection to JSON object
		auto json = dependency_graph::io::json::parse(QApplication::clipboard()->text().toStdString());

		// import the clipboard
		dependency_graph::io::from_json(json, pastedGraph);

		// THIS WILL ALSO NEED TO WORK RECURSIVELY
		action.append(pasteNetwork(current.index(), pastedGraph));
	}
	catch(std::exception& e) {
		// do nothing
		#ifndef NDEBUG
		std::cout << e.what() << std::endl;
		#endif
	}

	// execute the action (will actually make the nodes and connections)
	possumwood::AppCore::instance().undoStack().execute(action);

	// and make the selection based on added nodes
	{
		for(auto& n : pastedGraph.nodes())
			selection.addNode(findNode(n.index()));

		for(auto& c : pastedGraph.connections()) {
			dependency_graph::UniqueId id1 = c.first.node().index();
			dependency_graph::UniqueId id2 = c.second.node().index();

			dependency_graph::NodeBase& n1 = findNode(id1);
			dependency_graph::NodeBase& n2 = findNode(id2);

			dependency_graph::Port& p1 = n1.port(c.first.index());
			dependency_graph::Port& p2 = n2.port(c.second.index());

			selection.addConnection(p1, p2);
		}
	}
}

void Actions::move(const std::map<dependency_graph::NodeBase*, QPointF>& nodes) {
	possumwood::UndoStack::Action action;

	for(auto& n : nodes) {
		const possumwood::NodeData originalData = n.first->blindData<possumwood::NodeData>();

		if(originalData.position() != n.second) {
			possumwood::NodeData data = originalData;
			data.setPosition(n.second);

			action.addCommand(
				std::bind(&doSetBlindData, n.first->index(), data),
				std::bind(&doSetBlindData, n.first->index(), n.first->blindData<possumwood::NodeData>())
			);
		}
	}

	possumwood::AppCore::instance().undoStack().execute(action);
}
