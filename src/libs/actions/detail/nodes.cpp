#include "nodes.h"

#include "../app.h"

#include "tools.h"
#include "connections.h"

namespace possumwood { namespace actions { namespace detail {

namespace {

dependency_graph::NodeBase& doCreateNode(const dependency_graph::UniqueId& currentNetworkIndex, const dependency_graph::MetadataHandle& meta, const std::string& name, const dependency_graph::UniqueId& id, const possumwood::NodeData& blindData, boost::optional<const dependency_graph::Datablock&> data = boost::optional<const dependency_graph::Datablock&>()) {
	if(data)
		assert(data->meta() == meta);

	std::unique_ptr<dependency_graph::BaseData> bd(new dependency_graph::Data<possumwood::NodeData>(blindData));

	dependency_graph::NodeBase& netBase = detail::findNode(currentNetworkIndex);
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

}

possumwood::UndoStack::Action createNodeAction(const dependency_graph::UniqueId& currentNetworkId, const dependency_graph::MetadataHandle& meta, const std::string& name,
	const possumwood::NodeData& _data, const dependency_graph::UniqueId& id,
	boost::optional<const dependency_graph::Datablock&> datablock) {

	possumwood::NodeData data;
	data.setPosition(_data.position());

	possumwood::UndoStack::Action action;
	action.addCommand(
		std::bind(&doCreateNode, currentNetworkId, std::ref(meta), name, id, data, boost::optional<const dependency_graph::Datablock&>(datablock)),
		std::bind(&doRemoveNode, id)
	);

	return action;
}


possumwood::UndoStack::Action createNodeAction(dependency_graph::Network& current, const dependency_graph::MetadataHandle& meta, const std::string& name, const possumwood::NodeData& _data, const dependency_graph::UniqueId& id,
	boost::optional<const dependency_graph::Datablock&> datablock) {

	return createNodeAction(current.index(), meta, name, _data, id, datablock);
}

possumwood::UndoStack::Action removeNodeAction(dependency_graph::NodeBase& node) {
	possumwood::UndoStack::Action action;
	const dependency_graph::NodeBase& cnode = node;

	// recusively remove all sub-nodes
	if(node.is<dependency_graph::Network>())
		action.append(removeNetworkAction(node.as<dependency_graph::Network>()));

	// and remove current node
	action.addCommand(
		std::bind(&doRemoveNode, node.index()),
		std::bind(&doCreateNode, node.network().index(), std::ref(node.metadata().metadata()), node.name(), node.index(),
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

} } }
