#include "metadata.h"

#include <dependency_graph/attr_map.h>

#include "tools.h"
#include "connections.h"

namespace possumwood { namespace actions { namespace detail {

namespace {

void doSetMetadata(const dependency_graph::UniqueId& node, const dependency_graph::MetadataHandle& meta, const dependency_graph::Datablock& datablock) {
	dependency_graph::NodeBase& n = detail::findNode(node);
	n.setMetadata(meta);

	n.setDatablock(datablock);
}

struct ConnectionItem {
	dependency_graph::UniqueId thatNode;
	std::size_t thisPort, thatPort;
};

}

possumwood::UndoStack::Action changeMetadataAction(dependency_graph::NodeBase& node, const dependency_graph::MetadataHandle& handle) {
	assert(node.hasParentNetwork());

	possumwood::UndoStack::Action action;

	// first, store all connections that relate to the node that is to be changed
	std::vector<ConnectionItem> inConnections, outConnections;

	for(std::size_t pi=0; pi<node.portCount(); ++pi) {
		const dependency_graph::Port& port = node.port(pi);
		if(port.category() == dependency_graph::Attr::kOutput) {
			auto conns = node.network().connections().connectedTo(port);

			for(auto& c : conns)
				outConnections.push_back(ConnectionItem{
					c.get().node().index(),
					pi,
					c.get().index()
				});
		}
		else {
			assert(port.category() == dependency_graph::Attr::kInput);

			auto conn = node.network().connections().connectedFrom(port);
			if(conn)
				inConnections.push_back(ConnectionItem{
					conn->node().index(),
					pi,
					conn->index()
				});
		}
	}

	// disconnect everything using actions
	for(auto& c : inConnections)
		action.append(detail::disconnectAction(c.thatNode, c.thatPort, node.index(), c.thisPort));

	for(auto& c : outConnections)
		action.append(detail::disconnectAction(node.index(), c.thisPort, c.thatNode, c.thatPort));

	// create a new datablock by mapping values using attr_map
	const dependency_graph::AttrMap map(node.metadata(), handle);

	const dependency_graph::Datablock& srcDatablock = ((const dependency_graph::NodeBase&)node).datablock();
	dependency_graph::Datablock destDatablock(handle);

	for(auto& i : map)
		if(!srcDatablock.isNull(i.first))
			destDatablock.setData(i.second, srcDatablock.data(i.first));

	action.addCommand(
		std::bind(&doSetMetadata, node.index(), handle, destDatablock),
		std::bind(&doSetMetadata, node.index(), node.metadata(), srcDatablock)
	);

	// reconnect everything using actions and map
	for(auto& c : inConnections) {
		auto it = map.find(c.thisPort);
		if(it != map.end())
			action.append(detail::connectAction(c.thatNode, c.thatPort, node.index(), it->second));
	}

	for(auto& c : outConnections) {
		auto it = map.find(c.thisPort);
		if(it != map.end())
			action.append(detail::connectAction(node.index(), it->second, c.thatNode, c.thatPort));
	}

	return action;
}

} } }
