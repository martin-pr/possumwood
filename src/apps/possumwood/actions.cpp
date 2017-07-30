#include "actions.h"

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

void Actions::createNode(const dependency_graph::Metadata& meta, const std::string& name, const possumwood::NodeData& data) {
	doCreateNode(meta, name, data);
}

void Actions::removeNode(dependency_graph::Node& node) {
	doRemoveNode(node.blindData<possumwood::NodeData>().id());
}

void Actions::connect(dependency_graph::Port& p1, dependency_graph::Port& p2) {
	doConnect(p1.node().blindData<possumwood::NodeData>().id(), p1.index(),
		p2.node().blindData<possumwood::NodeData>().id(), p2.index());
}

void Actions::disconnect(dependency_graph::Port& p1, dependency_graph::Port& p2) {
	doDisconnect(p1.node().blindData<possumwood::NodeData>().id(), p1.index(),
		p2.node().blindData<possumwood::NodeData>().id(), p2.index());
}

void Actions::cut(const dependency_graph::Selection& selection) {
	// trigger the copy action first
	copy(selection);

	// and delete selection
	remove(selection);
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

dependency_graph::Selection Actions::paste() {
	dependency_graph::Selection selection;

	try {
		// we will need the Index instance to map between node IDs and their pointers
		Index& index = dynamic_cast<MainWindow&>(*possumwood::App::instance().mainWindow()).adaptor().index();

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

			doCreateNode(n.metadata(), n.name(), d, n.datablock());

			selection.addNode(*index[n.blindData<possumwood::NodeData>().id()].graphNode);
		}

		// add all connetions, based on "unique" IDs
		for(auto& c : graph.connections()) {
			possumwood::UniqueId id1 = c.first.node().blindData<possumwood::NodeData>().id();
			possumwood::UniqueId id2 = c.second.node().blindData<possumwood::NodeData>().id();

			doConnect(id1, c.first.index(), id2, c.second.index());

			dependency_graph::Node& n1 = *index[id1].graphNode;
			dependency_graph::Node& n2 = *index[id2].graphNode;

			dependency_graph::Port& p1 = n1.port(c.first.index());
			dependency_graph::Port& p2 = n2.port(c.second.index());

			selection.addConnection(p1, p2);
		}
	}
	catch(std::exception& e) {
		// do nothing
		// std::cout << e.what() << std::endl;
	}

	return selection;
}

void Actions::remove(const dependency_graph::Selection& selection) {
	for(auto& e : selection.connections()) {
		auto& n1 = e.from.get().node();
		auto& n2 = e.to.get().node();

		doDisconnect(n1.blindData<possumwood::NodeData>().id(), e.from.get().index(),
			n2.blindData<possumwood::NodeData>().id(), e.to.get().index());
	}

	for(auto& n : selection.nodes())
		doRemoveNode(n.get().blindData<possumwood::NodeData>().id());
}

void Actions::move(dependency_graph::Node& n, const QPointF& pos) {
	possumwood::NodeData data = n.blindData<possumwood::NodeData>();
	data.setPosition(pos);
	n.setBlindData(data);
}
