#include "actions.h"

#include <functional>

#include <dependency_graph/node_base.inl>
#include <dependency_graph/nodes.inl>
#include <dependency_graph/attr_map.h>
#include <dependency_graph/values.h>
#include <dependency_graph/detail.h>

#include "io/graph.h"

#include "detail/tools.h"
#include "detail/nodes.h"
#include "detail/connections.h"
#include "detail/metadata.h"
#include "detail/values.h"

#include "app.h"
#include "clipboard.h"

namespace possumwood { namespace actions {

namespace {

void doSetBlindData(const dependency_graph::UniqueId& node, const possumwood::NodeData& blindData) {
	dependency_graph::NodeBase& n = detail::findNode(node);
	n.setBlindData(blindData);
}

} // anonymous namespace

/////////////////////////////////////////////////////////////////////

void createNode(dependency_graph::Network& current, const dependency_graph::MetadataHandle& meta, const std::string& name, const possumwood::NodeData& _data, const dependency_graph::UniqueId& id) {
	auto action = detail::createNodeAction(current, meta, name, _data, id);

	possumwood::AppCore::instance().undoStack().execute(action);
}

void removeNode(dependency_graph::NodeBase& node) {
	dependency_graph::Selection selection;
	selection.addNode(node);

	auto action = detail::removeAction(selection);

	possumwood::AppCore::instance().undoStack().execute(action);
}

void connect(dependency_graph::Port& p1, dependency_graph::Port& p2) {
	auto action = detail::connectAction(p1, p2);

	possumwood::AppCore::instance().undoStack().execute(action);
}

void disconnect(dependency_graph::Port& p1, dependency_graph::Port& p2) {
	auto action = detail::disconnectAction(p1, p2);

	possumwood::AppCore::instance().undoStack().execute(action);
}

void remove(const dependency_graph::Selection& selection) {
	auto action = detail::removeAction(selection);

	possumwood::AppCore::instance().undoStack().execute(action);
}

void cut(const dependency_graph::Selection& selection) {
	// trigger the copy action first
	copy(selection);

	// and delete selection
	auto action = detail::removeAction(selection);

	possumwood::AppCore::instance().undoStack().execute(action);
}

void copy(const dependency_graph::Selection& selection) {
	dependency_graph::Network* net = &possumwood::AppCore::instance().graph();
	if(!selection.empty() && selection.nodes().begin()->get().hasParentNetwork())
		net = &selection.nodes().begin()->get().network();

	// convert the selection to JSON string
	possumwood::io::json json;
	possumwood::io::to_json(json, *net, selection);

	std::stringstream ss;
	ss << std::setw(4) << json;

	// and put it to the clipboard
	Clipboard::instance().setClipboardContent(ss.str().c_str());
}

namespace {
	possumwood::UndoStack::Action pasteNetwork(const dependency_graph::UniqueId& targetIndex, const dependency_graph::Network& source) {
		possumwood::UndoStack::Action action;

		// add all the nodes to the parent network
		//  - each node has a unique ID (unique between all graphs), store that
		for(auto& n : source.nodes()) {
			possumwood::NodeData d = n.blindData<possumwood::NodeData>();
			d.setPosition(possumwood::NodeData::Point{20, 20} + d.position());

			const dependency_graph::NodeBase& cn = n;

			action.append(detail::createNodeAction(targetIndex, n.metadata(), n.name(), d, n.index(), cn.datablock()));

			// recurse to add nested networks
			if(cn.is<dependency_graph::Network>())
				action.append(pasteNetwork(n.index(), n.as<dependency_graph::Network>()));
		}

		// add all connections, based on "unique" IDs
		for(auto& c : source.connections()) {
			dependency_graph::UniqueId id1 = c.first.node().index();
			dependency_graph::UniqueId id2 = c.second.node().index();

			action.append(detail::connectAction(id1, c.first.index(), id2, c.second.index()));
		}

		return action;
	}
}

void paste(dependency_graph::Network& current, dependency_graph::Selection& selection) {
	try {
		// convert the clipboard content to a json object
		auto json = possumwood::io::json::parse(Clipboard::instance().clipboardContent());

		// and pass it to the paste() implementation
		paste(current, selection, json);
	}
	catch(std::exception& e) {
		// do nothing
		#ifndef NDEBUG
		std::cout << e.what() << std::endl;
		#endif
	}
}

void paste(dependency_graph::Network& current, dependency_graph::Selection& selection, const possumwood::io::json& json) {
	possumwood::UndoStack::Action action;

	dependency_graph::Graph pastedGraph;

	try {
		// import the clipboard
		possumwood::io::from_json(json, pastedGraph);
	}
	catch(std::exception& e) {
		// do nothing
		#ifndef NDEBUG
		std::cout << e.what() << std::endl;
		#endif
	}

	// paste the network extracted from the JSON
	action.append(pasteNetwork(current.index(), pastedGraph));

	// execute the action (will actually make the nodes and connections)
	possumwood::AppCore::instance().undoStack().execute(action);

	// and make the selection based on added nodes
	{
		for(auto& n : pastedGraph.nodes())
			selection.addNode(detail::findNode(n.index()));

		for(auto& c : pastedGraph.connections()) {
			dependency_graph::UniqueId id1 = c.first.node().index();
			dependency_graph::UniqueId id2 = c.second.node().index();

			dependency_graph::NodeBase& n1 = detail::findNode(id1);
			dependency_graph::NodeBase& n2 = detail::findNode(id2);

			dependency_graph::Port& p1 = n1.port(c.first.index());
			dependency_graph::Port& p2 = n2.port(c.second.index());

			selection.addConnection(p1, p2);
		}
	}
}

void move(const std::map<dependency_graph::NodeBase*, possumwood::NodeData::Point>& nodes) {
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

void changeMetadata(dependency_graph::NodeBase& node, const dependency_graph::MetadataHandle& handle) {
	possumwood::UndoStack::Action action = detail::changeMetadataAction(node, handle);

	possumwood::AppCore::instance().undoStack().execute(action);
}

void setValue(dependency_graph::Port& port, const dependency_graph::BaseData& value) {
	possumwood::UndoStack::Action action = detail::setValueAction(port, value);

	AppCore::instance().undoStack().execute(action);
}

} }
