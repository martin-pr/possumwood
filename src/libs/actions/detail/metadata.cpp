#include "metadata.h"

#include <dependency_graph/attr_map.h>
#include <dependency_graph/detail.h>
#include <dependency_graph/values.h>
#include <dependency_graph/nodes_iterator.inl>

#include "tools.h"
#include "connections.h"
#include "links.h"
#include "values.h"

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

	std::stringstream ss;
	ss << "Setting metadata to " << node.name();

	action.addCommand(
		ss.str(),
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

void buildNetwork(dependency_graph::Network& network) {
	// find all input and output nodes of the network with connected outputs
	//   and build metadata that correspond to them
	std::unique_ptr<dependency_graph::Metadata> meta = dependency_graph::instantiateMetadata("network");

	std::vector<Link> links;
	std::vector<dependency_graph::Data> values;

	for(auto& n : network.nodes()) {
		if(n.metadata()->type() == "input" && n.port(0).isConnected()) {
			// link the two ports - will just transfer values blindly
			links.push_back(Link {
				network.index(), meta->attributeCount(),
				n.index(), 0
			});

			// get the attribute to add to metadata from the other side of the connection
			auto conns = n.network().connections().connectedTo(n.port(0));
			assert(!conns.empty());

			dependency_graph::Attr in = conns.front().get().node().metadata()->attr(conns.front().get().index());
			dependency_graph::detail::MetadataAccess::addAttribute(*meta, n.name(), in.category(), in.createData(), in.flags());

			// also the port value
			values.push_back(n.port(0).getData());
		}

		if(n.metadata()->type() == "output" && n.port(0).isConnected()) {
			// link the two ports - this will in practice just transfer the dirtiness
			links.push_back(Link {
				n.index(), 0,
				network.index(), meta->attributeCount()
			});

			// get the attribute to add to metadata from the other side of the connection
			auto conn = network.connections().connectedFrom(n.port(0));
			assert(conn);

			dependency_graph::Attr out = conn->node().metadata()->attr(conn->index());;
			dependency_graph::detail::MetadataAccess::addAttribute(*meta, n.name(), out.category(), out.createData(), out.flags());

			// also port the value
			values.push_back(conn->node().port(conn->index()).getData());
		}
	}

	// build the compute method from all the output-to-output links
	//   -> links between outputs guarantee that dirtiness is passed correctly.
	//      Compute method provides the pull mechanism for data transfer.
	{
		std::vector<std::function<void(dependency_graph::Values&)>> assignments;

		for(auto& l : links)
			if(l.toNode == network.index()) {
				dependency_graph::NodeBase& fromNode = detail::findNode(l.fromNode);

				assignments.push_back([&fromNode, l](dependency_graph::Values& vals) {
					vals.transfer(l.toPort, fromNode.port(l.fromPort));
				});
			}

		meta->setCompute([assignments](dependency_graph::Values& vals) {
			for(auto& a : assignments)
				a(vals);

			return dependency_graph::State();
		});
	}

	possumwood::UndoStack::Action action;

	// unlink everything currently linked
	for(std::size_t pi=0; pi<network.portCount(); ++pi)
		if(network.port(pi).isLinked())
			action.append(detail::unlinkAction(network.port(pi)));

	for(auto& n : network.nodes())
		if((n.metadata()->type() == "output") && n.port(0).isLinked())
			action.append(detail::unlinkAction(n.port(0)));

	// change metadata of the node, using an action (unless root - root handling to be addressed at some point)
	if(network.hasParentNetwork()) {
		dependency_graph::MetadataHandle handle(std::move(meta));
		action.append(changeMetadataAction(network, handle));

		// transfer all the values
		for(std::size_t pi=0; pi<values.size(); ++pi)
			action.append(detail::setValueAction(network.index(), pi, values[pi]));

		// link all what needs to be linked
		for(auto& l : links)
			action.append(linkAction(l));
	}

	possumwood::UndoStack tmpStack;
	tmpStack.execute(action);
}

} } }
