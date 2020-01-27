#include "nodes.h"

#include <dependency_graph/node_base.inl>
#include <dependency_graph/nodes_iterator.inl>
#include <dependency_graph/network.h>

#include "../app.h"

#include "tools.h"
#include "connections.h"
#include "metadata.h"

namespace possumwood { namespace actions { namespace detail {

namespace {

bool isCompatible(const dependency_graph::Metadata& m1, const dependency_graph::Metadata& m2) {
	if(m1.attributeCount() != m2.attributeCount())
		return false;

	for(std::size_t i=0; i<m1.attributeCount(); ++i) {
		const dependency_graph::Attr& a1 = m1.attr(i);
		const dependency_graph::Attr& a2 = m2.attr(i);

		if(a1.name() != a2.name())
			return false;

		if(a1.category() != a2.category())
			return false;

		if(a1.type() != a2.type())
			return false;
	}

	return true;
}


dependency_graph::NodeBase& doCreateNode(const dependency_graph::UniqueId& currentNetworkIndex, const dependency_graph::MetadataHandle& meta, const std::string& name, const dependency_graph::UniqueId& id,
	const dependency_graph::Data& blindData, boost::optional<const dependency_graph::Datablock> data = boost::optional<const dependency_graph::Datablock>()) {

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
	dependency_graph::Network& network = netBase.as<dependency_graph::Network>();

	boost::optional<const dependency_graph::Datablock&> dataRef;
	if(data) {
		assert(isCompatible(meta.metadata(), data->meta().metadata()));
		dataRef = boost::optional<const dependency_graph::Datablock&>(*data);
	}

	dependency_graph::NodeBase& n = network.nodes().add(meta, name, blindData, dataRef, id);
	assert(n.index() == id);
	return n;
}

void doRemoveNode(const dependency_graph::UniqueId& id) {
	auto& graph = possumwood::AppCore::instance().graph();
	auto it = std::find_if(graph.nodes().begin(dependency_graph::Nodes::kRecursive), graph.nodes().end(), [&](const dependency_graph::NodeBase & i) {
		return i.index() == id;
	});

	assert(it != graph.nodes().end());

	// removing a network - the network should be empty at this stage, as all its
	// nodes and connections are handled already
	#ifndef NDEBUG
	dependency_graph::Network* net = dynamic_cast<dependency_graph::Network*>(&(*it));
	if(net)
		assert(net->empty());
	#endif

	it->network().nodes().erase(it);
}

}

possumwood::UndoStack::Action createNodeAction(const dependency_graph::UniqueId& currentNetworkId, const dependency_graph::MetadataHandle& meta, const std::string& name,
	const dependency_graph::Data& blindData, const dependency_graph::UniqueId& id,
	boost::optional<const dependency_graph::Datablock> datablock) {

	std::stringstream ss;
	ss << "Creating node " << name << " of type " << meta->type();

	possumwood::UndoStack::Action action;
	action.addCommand(
		ss.str(),
		std::bind(&doCreateNode, currentNetworkId, meta, name, id, blindData, boost::optional<const dependency_graph::Datablock>(datablock)),
		std::bind(&doRemoveNode, id)
	);

	return action;
}


possumwood::UndoStack::Action createNodeAction(dependency_graph::Network& current, const dependency_graph::MetadataHandle& meta, const std::string& name,
	const dependency_graph::Data& blindData, const dependency_graph::UniqueId& id,
	boost::optional<const dependency_graph::Datablock> datablock) {

	assert(!blindData.empty());

	return createNodeAction(current.index(), meta, name, blindData, id, datablock);
}

possumwood::UndoStack::Action removeNodeAction(dependency_graph::NodeBase& node) {
	possumwood::UndoStack::Action action;
	const dependency_graph::NodeBase& cnode = node;

	// recusively remove all sub-nodes
	if(node.is<dependency_graph::Network>())
		action.append(removeNetworkAction(node.as<dependency_graph::Network>()));

	// store the original blind data for undo
	std::shared_ptr<const dependency_graph::Data> blindData(new dependency_graph::Data(node.blindData()));

	std::stringstream ss;
	ss << "Removing node " << node.name() << " of type " << node.metadata()->type();

	// and remove current node
	action.addCommand(
		ss.str(),
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
	// collect all "parent" networks for all selected nodes
	std::set<dependency_graph::Network*> networks;
	networks.insert(&possumwood::AppCore::instance().graph());

	for(auto& n : _selection.nodes())
		if(n.get().hasParentNetwork())
			networks.insert(&n.get().network());

	// add all connections to selected nodes - they'll be removed as well as the selected connections
	//   with the removed nodes
	dependency_graph::Selection selection = _selection;
	for(auto& net : networks)
		for(auto& c : net->connections()) {
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

namespace {

void doRenameNode(const dependency_graph::UniqueId& id, std::shared_ptr<std::string> newName, std::shared_ptr<std::string> originalName) {
	auto& graph = possumwood::AppCore::instance().graph();
	auto it = std::find_if(graph.nodes().begin(dependency_graph::Nodes::kRecursive), graph.nodes().end(), [&](const dependency_graph::NodeBase & i) {
		return i.index() == id;
	});

	assert(it != graph.nodes().end());

	*originalName = it->name();
	it->setName(*newName);

	if(it->hasParentNetwork() && (it->metadata()->type() == "input" || it->metadata()->type() == "output"))
		buildNetwork(it->network());
}

}

possumwood::UndoStack::Action renameNodeAction(const dependency_graph::UniqueId& nodeId, const std::string& name) {
	possumwood::UndoStack::Action action;

	std::shared_ptr<std::string> newName(new std::string(name));
	std::shared_ptr<std::string> originalName(new std::string());

	std::stringstream ss;
	ss << "Renaming node " << nodeId << " to " << name;

	action.addCommand(
		ss.str(),
		std::bind(&doRenameNode, nodeId, newName, originalName),
		std::bind(&doRenameNode, nodeId, originalName, newName)
	);

	return action;
}

} } }
