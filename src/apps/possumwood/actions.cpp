#include "actions.h"

#include <functional>

#include <QApplication>
#include <QClipboard>
#include <QMainWindow>

#include <dependency_graph/io/graph.h>
#include <dependency_graph/node.inl>
#include <dependency_graph/nodes.inl>

#include <possumwood_sdk/app.h>

#include "main_window.h"

namespace {

dependency_graph::NodeBase& findNode(const possumwood::UniqueId& id) {
	// we will need the Index instance to map between node IDs and their pointers
	possumwood::Index& index = possumwood::App::instance().index();

	// and get the node reference
	return *index[id].graphNode;
}

void doCreateNode(const dependency_graph::Metadata& meta, const std::string& name, const possumwood::NodeData& blindData, boost::optional<const dependency_graph::Datablock&> data = boost::optional<const dependency_graph::Datablock&>()) {
	if(data)
		assert(&data->meta() == &meta);

	possumwood::App::instance().graph().nodes().add(meta, name, blindData, data);
}

void doRemoveNode(const possumwood::UniqueId& id) {
	auto& graph = possumwood::App::instance().graph();
	auto it = std::find_if(graph.nodes().begin(), graph.nodes().end(), [&](const dependency_graph::NodeBase & i) {
		return i.blindData<possumwood::NodeData>().id() == id;
	});

	assert(it != graph.nodes().end());

	graph.nodes().erase(it);
}

void doConnect(const possumwood::UniqueId& fromNode, std::size_t fromPort, const possumwood::UniqueId& toNode, std::size_t toPort) {
	dependency_graph::NodeBase& from = findNode(fromNode);
	dependency_graph::NodeBase& to = findNode(toNode);

	from.port(fromPort).connect(to.port(toPort));
}

void doDisconnect(const possumwood::UniqueId& fromNode, std::size_t fromPort, const possumwood::UniqueId& toNode, std::size_t toPort) {
	dependency_graph::NodeBase& from = findNode(fromNode);
	dependency_graph::NodeBase& to = findNode(toNode);

	from.port(fromPort).disconnect(to.port(toPort));
}

void doSetBlindData(const possumwood::UniqueId& node, const possumwood::NodeData& blindData) {
	dependency_graph::NodeBase& n = findNode(node);
	n.setBlindData(blindData);
}

} // anonymous namespace

/////////////////////////////////////////////////////////////////////

void Actions::createNode(const dependency_graph::Metadata& meta, const std::string& name, const possumwood::NodeData& _data) {
	possumwood::NodeData data;
	data.setPosition(_data.position());

	possumwood::UndoStack::Action action;
	action.addCommand(
		std::bind(&doCreateNode, std::ref(meta), name, data, boost::optional<const dependency_graph::Datablock&>()),
		std::bind(&doRemoveNode, data.id())
	);

	possumwood::App::instance().undoStack().execute(action);
}

namespace {

possumwood::UndoStack::Action removeNodeAction(dependency_graph::NodeBase& node) {
	possumwood::UndoStack::Action action;
	const dependency_graph::NodeBase& cnode = node;

	action.addCommand(
		std::bind(&doRemoveNode, node.blindData<possumwood::NodeData>().id()),
		std::bind(&doCreateNode, std::ref(node.metadata()), node.name(),
			node.blindData<possumwood::NodeData>(), cnode.datablock())
	);

	return action;
}

}

void Actions::removeNode(dependency_graph::NodeBase& node) {
	auto action =  removeNodeAction(node);

	possumwood::App::instance().undoStack().execute(action);
}

void Actions::connect(dependency_graph::Port& p1, dependency_graph::Port& p2) {
	possumwood::UndoStack::Action action;

	action.addCommand(
		std::bind(&doConnect,
			p1.node().blindData<possumwood::NodeData>().id(), p1.index(),
			p2.node().blindData<possumwood::NodeData>().id(), p2.index()),
		std::bind(&doDisconnect,
			p1.node().blindData<possumwood::NodeData>().id(), p1.index(),
			p2.node().blindData<possumwood::NodeData>().id(), p2.index())
	);

	possumwood::App::instance().undoStack().execute(action);
}

namespace {

possumwood::UndoStack::Action disconnectAction(dependency_graph::Port& p1, dependency_graph::Port& p2) {
	possumwood::UndoStack::Action action;

	action.addCommand(
		std::bind(&doDisconnect,
			p1.node().blindData<possumwood::NodeData>().id(), p1.index(),
			p2.node().blindData<possumwood::NodeData>().id(), p2.index()),
		std::bind(&doConnect,
			p1.node().blindData<possumwood::NodeData>().id(), p1.index(),
			p2.node().blindData<possumwood::NodeData>().id(), p2.index())
	);

	return action;
}

}

void Actions::disconnect(dependency_graph::Port& p1, dependency_graph::Port& p2) {
	auto action = disconnectAction(p1, p2);

	possumwood::App::instance().undoStack().execute(action);
}

namespace {

possumwood::UndoStack::Action removeAction(const dependency_graph::Selection& _selection) {
	// add all connections to selected nodes - they'll be removed as well as the selected connections
	//   with the removed nodes
	dependency_graph::Selection selection = _selection;
	for(auto& c : possumwood::App::instance().graph().connections()) {
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

void Actions::remove(const dependency_graph::Selection& selection) {
	auto action = removeAction(selection);

	possumwood::App::instance().undoStack().execute(action);
}

void Actions::cut(const dependency_graph::Selection& selection) {
	// trigger the copy action first
	copy(selection);

	// and delete selection
	auto action = removeAction(selection);

	possumwood::App::instance().undoStack().execute(action);
}

void Actions::copy(const dependency_graph::Selection& selection) {
	// convert the selection to JSON string
	dependency_graph::io::json json;
	dependency_graph::io::to_json(json, possumwood::App::instance().graph(), selection);

	std::stringstream ss;
	ss << std::setw(4) << json;

	// and put it to the clipboard
	QApplication::clipboard()->setText(ss.str().c_str());
}

void Actions::paste(dependency_graph::Selection& selection) {
	possumwood::UndoStack::Action action;

	dependency_graph::Graph graph;

	try {
		// convert the selection to JSON object
		auto json = dependency_graph::io::json::parse(QApplication::clipboard()->text().toStdString());

		// import the clipboard
		dependency_graph::io::from_json(json, graph);

		// add all the nodes to the main graph
		//  - each node has a unique ID (unique between all graphs), store that
		for(auto& n : graph.nodes()) {
			possumwood::NodeData d = n.blindData<possumwood::NodeData>();
			d.setPosition(QPointF(20, 20) + d.position());

			const dependency_graph::NodeBase& cn = n;
			action.addCommand(
				std::bind(&doCreateNode, std::ref(n.metadata()), n.name(), d, cn.datablock()),
				std::bind(&doRemoveNode, n.blindData<possumwood::NodeData>().id())
			);
		}

		// add all connetions, based on "unique" IDs
		for(auto& c : graph.connections()) {
			possumwood::UniqueId id1 = c.first.node().blindData<possumwood::NodeData>().id();
			possumwood::UniqueId id2 = c.second.node().blindData<possumwood::NodeData>().id();

			action.addCommand(
				std::bind(&doConnect, id1, c.first.index(), id2, c.second.index()),
				std::bind(&doDisconnect, id1, c.first.index(), id2, c.second.index())
			);
		}
	}
	catch(std::exception& e) {
		// do nothing
		// std::cout << e.what() << std::endl;
	}

	// execute the action (will actually make the nodes and connections)
	possumwood::App::instance().undoStack().execute(action);

	// and make the selection based on added nodes
	{
		// we will need the Index instance to map between node IDs and their pointers
		possumwood::Index& index = possumwood::App::instance().index();

		for(auto& n : graph.nodes())
			selection.addNode(*index[n.blindData<possumwood::NodeData>().id()].graphNode);

		for(auto& c : graph.connections()) {
			possumwood::UniqueId id1 = c.first.node().blindData<possumwood::NodeData>().id();
			possumwood::UniqueId id2 = c.second.node().blindData<possumwood::NodeData>().id();

			dependency_graph::NodeBase& n1 = *index[id1].graphNode;
			dependency_graph::NodeBase& n2 = *index[id2].graphNode;

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
				std::bind(&doSetBlindData, data.id(), data),
				std::bind(&doSetBlindData, data.id(), n.first->blindData<possumwood::NodeData>())
			);
		}
	}

	possumwood::App::instance().undoStack().execute(action);
}
