#include "actions.h"

#include <functional>

#include <QApplication>
#include <QClipboard>
#include <QMainWindow>

#include <dependency_graph/io/graph.h>
#include <dependency_graph/node.inl>

#include <possumwood_sdk/app.h>

#include "main_window.h"

namespace {

dependency_graph::Node& findNode(const possumwood::UniqueId& id) {
	// we will need the Index instance to map between node IDs and their pointers
	Index& index = dynamic_cast<MainWindow&>(*possumwood::App::instance().mainWindow()).adaptor().index();

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
	auto it = std::find_if(graph.nodes().begin(), graph.nodes().end(), [&](const dependency_graph::Node & i) {
		return i.blindData<possumwood::NodeData>().id() == id;
	});

	assert(it != graph.nodes().end());

	graph.nodes().erase(it);
}

void doConnect(const possumwood::UniqueId& fromNode, std::size_t fromPort, const possumwood::UniqueId& toNode, std::size_t toPort) {
	dependency_graph::Node& from = findNode(fromNode);
	dependency_graph::Node& to = findNode(toNode);

	from.port(fromPort).connect(to.port(toPort));
}

void doDisconnect(const possumwood::UniqueId& fromNode, std::size_t fromPort, const possumwood::UniqueId& toNode, std::size_t toPort) {
	dependency_graph::Node& from = findNode(fromNode);
	dependency_graph::Node& to = findNode(toNode);

	from.port(fromPort).disconnect(to.port(toPort));
}

} // anonymous namespace

/////////////////////////////////////////////////////////////////////

possumwood::UndoStack::Action Actions::createNode(const dependency_graph::Metadata& meta, const std::string& name, const possumwood::NodeData& _data) {
	possumwood::NodeData data;
	data.setPosition(_data.position());

	possumwood::UndoStack::Action action;
	action.addCommand(
		std::bind(&doCreateNode, std::ref(meta), name, data, boost::optional<const dependency_graph::Datablock&>()),
		std::bind(&doRemoveNode, data.id())
	);

	return action;
}

possumwood::UndoStack::Action Actions::removeNode(dependency_graph::Node& node) {
	possumwood::UndoStack::Action action;

	action.addCommand(
		std::bind(&doRemoveNode, node.blindData<possumwood::NodeData>().id()),
		std::bind(&doCreateNode, std::ref(node.metadata()), node.name(),
			node.blindData<possumwood::NodeData>(), node.datablock())
	);

	return action;
}

possumwood::UndoStack::Action Actions::connect(dependency_graph::Port& p1, dependency_graph::Port& p2) {
	possumwood::UndoStack::Action action;

	action.addCommand(
		std::bind(&doConnect,
			p1.node().blindData<possumwood::NodeData>().id(), p1.index(),
			p2.node().blindData<possumwood::NodeData>().id(), p2.index()),
		std::bind(&doDisconnect,
			p1.node().blindData<possumwood::NodeData>().id(), p1.index(),
			p2.node().blindData<possumwood::NodeData>().id(), p2.index())
	);

	return action;
}

possumwood::UndoStack::Action Actions::disconnect(dependency_graph::Port& p1, dependency_graph::Port& p2) {
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

possumwood::UndoStack::Action Actions::cut(const dependency_graph::Selection& selection) {
	// trigger the copy action first
	copy(selection);

	// and delete selection
	return remove(selection);
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

possumwood::UndoStack::Action Actions::paste(dependency_graph::Selection& selection) {
	possumwood::UndoStack::Action action;

	try {
		// we will need the Index instance to map between node IDs and their pointers
		// Index& index = dynamic_cast<MainWindow&>(*possumwood::App::instance().mainWindow()).adaptor().index();

		// convert the selection to JSON object
		auto json = dependency_graph::io::json::parse(QApplication::clipboard()->text().toStdString());

		// import the clipboard
		dependency_graph::Graph graph;
		dependency_graph::io::from_json(json, graph);

		// add all the nodes to the main graph
		//  - each node has a unique ID (unique between all graphs), store that
		for(auto& n : graph.nodes()) {
			possumwood::NodeData d = n.blindData<possumwood::NodeData>();
			d.setPosition(QPointF(20, 20) + d.position());

			action.addCommand(
				std::bind(&doCreateNode, std::ref(n.metadata()), n.name(), d, n.datablock()),
				std::bind(&doRemoveNode, n.blindData<possumwood::NodeData>().id())
			);

			// selection.addNode(*index[n.blindData<possumwood::NodeData>().id()].graphNode);
		}

		// add all connetions, based on "unique" IDs
		for(auto& c : graph.connections()) {
			possumwood::UniqueId id1 = c.first.node().blindData<possumwood::NodeData>().id();
			possumwood::UniqueId id2 = c.second.node().blindData<possumwood::NodeData>().id();

			action.addCommand(
				std::bind(&doConnect, id1, c.first.index(), id2, c.second.index()),
				std::bind(&doDisconnect, id1, c.first.index(), id2, c.second.index())
			);

			// dependency_graph::Node& n1 = *index[id1].graphNode;
			// dependency_graph::Node& n2 = *index[id2].graphNode;

			// dependency_graph::Port& p1 = n1.port(c.first.index());
			// dependency_graph::Port& p2 = n2.port(c.second.index());

			// selection.addConnection(p1, p2);

			std::cout << "add connection" << std::endl;
		}
	}
	catch(std::exception& e) {
		// do nothing
		// std::cout << e.what() << std::endl;
	}

	return action;
}

possumwood::UndoStack::Action Actions::remove(const dependency_graph::Selection& _selection) {
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
		action.merge(disconnect(e.from, e.to));

	/// and all nodes
	for(auto& n : selection.nodes())
		action.merge(removeNode(n));

	return action;
}

void Actions::move(dependency_graph::Node& n, const QPointF& pos) {
	possumwood::NodeData data = n.blindData<possumwood::NodeData>();
	data.setPosition(pos);
	n.setBlindData(data);
}
