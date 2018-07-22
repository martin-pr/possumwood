#include "nodes.h"

#include "../app.h"

#include "tools.h"
#include "connections.h"

namespace possumwood { namespace actions { namespace detail {

namespace {

dependency_graph::NodeBase& doCreateNode(const dependency_graph::UniqueId& currentNetworkIndex, const dependency_graph::MetadataHandle& meta, const std::string& name, const dependency_graph::UniqueId& id,
	std::shared_ptr<const dependency_graph::BaseData> blindData, boost::optional<const dependency_graph::Datablock> data = boost::optional<const dependency_graph::Datablock>()) {

#ifndef NDEBUG
	if(data) {
		// assert(data->meta() == meta);
		assert(data->meta()->attributeCount() == meta->attributeCount());

		for(std::size_t i=0; i<meta->attributeCount(); ++i) {
			assert(data->meta()->attr(i).type() == meta->attr(i).type());
			assert(data->meta()->attr(i).category() == meta->attr(i).category());
		}
	}
#endif

	dependency_graph::NodeBase& netBase = detail::findNode(currentNetworkIndex);
	assert(netBase.is<dependency_graph::Network>());

	boost::optional<const dependency_graph::Datablock&> dataRef;
	if(data)
		dataRef = boost::optional<const dependency_graph::Datablock&>(*data);

	dependency_graph::NodeBase& n = netBase.as<dependency_graph::Network>().nodes().add(meta, name, blindData->clone(), dataRef, id);
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

}

possumwood::UndoStack::Action createNodeAction(const dependency_graph::UniqueId& currentNetworkId, const dependency_graph::MetadataHandle& meta, const std::string& name,
	const dependency_graph::BaseData& data, const dependency_graph::UniqueId& id,
	boost::optional<const dependency_graph::Datablock> datablock) {

	// make a copy of blind data for binding
	std::shared_ptr<const dependency_graph::BaseData> blindData(data.clone());

	possumwood::UndoStack::Action action;
	action.addCommand(
		std::bind(&doCreateNode, currentNetworkId, meta, name, id, blindData, boost::optional<const dependency_graph::Datablock>(datablock)),
		std::bind(&doRemoveNode, id)
	);

	return action;
}


possumwood::UndoStack::Action createNodeAction(dependency_graph::Network& current, const dependency_graph::MetadataHandle& meta, const std::string& name, const dependency_graph::BaseData& data, const dependency_graph::UniqueId& id,
	boost::optional<const dependency_graph::Datablock> datablock) {

	return createNodeAction(current.index(), meta, name, data, id, datablock);
}

possumwood::UndoStack::Action removeNodeAction(dependency_graph::NodeBase& node) {
	possumwood::UndoStack::Action action;
	const dependency_graph::NodeBase& cnode = node;

	// recusively remove all sub-nodes
	if(node.is<dependency_graph::Network>())
		action.append(removeNetworkAction(node.as<dependency_graph::Network>()));

	// store the original blind data for undo
	std::shared_ptr<const dependency_graph::BaseData> blindData(node.blindData().clone());

	// and remove current node
	action.addCommand(
		std::bind(&doRemoveNode, node.index()),
		std::bind(&doCreateNode, node.network().index(), node.metadata(), node.name(), node.index(),
			blindData, cnode.datablock())
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

} } }
