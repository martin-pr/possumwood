#include "connections.h"

#include <dependency_graph/detail.h>
#include <dependency_graph/values.h>

#include "tools.h"
#include "links.h"
#include "metadata.h"
#include "values.h"

namespace possumwood { namespace actions { namespace detail {

namespace {

void unlinkAll(const dependency_graph::UniqueId& fromNodeId, const dependency_graph::UniqueId& toNodeId) {
	// special handling for "input" and "output" types
	dependency_graph::NodeBase& fromNode = detail::findNode(fromNodeId);
	dependency_graph::NodeBase& toNode = detail::findNode(toNodeId);
	if((fromNode.metadata()->type() == "input" || toNode.metadata()->type() == "output") &&
		fromNode.hasParentNetwork() && toNode.hasParentNetwork()) {

		// unlink any linked ports first
		dependency_graph::Network& network = fromNode.network();
		for(std::size_t pi=0; pi<network.portCount(); ++pi)
			if(network.port(pi).isLinked())
				network.port(pi).unlink();

		for(auto& n : network.nodes())
			for(std::size_t pi=0; pi<n.portCount(); ++pi)
				if(n.port(pi).isLinked())
					n.port(pi).unlink();
	}
}

possumwood::UndoStack::Action buildNetwork(const dependency_graph::UniqueId& fromNodeId, const dependency_graph::UniqueId& toNodeId) {
	possumwood::UndoStack::Action action;

	// special handling for "input" and "output" types
	dependency_graph::NodeBase& fromNode = detail::findNode(fromNodeId);
	dependency_graph::NodeBase& toNode = detail::findNode(toNodeId);
	if((fromNode.metadata()->type() == "input" || toNode.metadata()->type() == "output") &&
		fromNode.hasParentNetwork() && toNode.hasParentNetwork()) {

		dependency_graph::Network& network = fromNode.network();

		// find all input and output nodes of the network with connected outputs
		//   and build metadata that correspond to them
		std::unique_ptr<dependency_graph::Metadata> meta = dependency_graph::instantiateMetadata("network");

		std::vector<Link> links;
		std::vector<std::unique_ptr<dependency_graph::BaseData>> values;

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

				dependency_graph::Attr in = conns.front().get().node().metadata()->attr(conns.front().get().index());;
				dependency_graph::detail::MetadataAccess::addAttribute(*meta, in);

				// also the port value
				values.push_back(n.port(0).getData().clone());
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
				dependency_graph::detail::MetadataAccess::addAttribute(*meta, out);

				// also port the value
				values.push_back(conn->node().port(conn->index()).getData().clone());
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

		// change metadata of the node, using an action
		{
			dependency_graph::MetadataHandle handle(std::move(meta));
			action.append(changeMetadataAction(fromNode.network(), handle));
		}

		// transfer all the values
		for(std::size_t pi=0; pi<values.size(); ++pi)
			action.append(detail::setValueAction(fromNode.network().index(), pi, *values[pi]->clone()));

		// link all what needs to be linked
		for(auto& l : links)
			action.append(linkAction(l));
	}

	return action;
}

void doConnect(const dependency_graph::UniqueId& fromNode, std::size_t fromPort, const dependency_graph::UniqueId& toNode, std::size_t toPort) {
	dependency_graph::NodeBase& from = detail::findNode(fromNode);
	dependency_graph::NodeBase& to = detail::findNode(toNode);

	unlinkAll(fromNode, toNode);

	assert(from.portCount() > fromPort);
	assert(to.portCount() > toPort);

	from.port(fromPort).connect(to.port(toPort));

	possumwood::UndoStack tmpStack;
	tmpStack.execute(buildNetwork(fromNode, toNode));
}

void doDisconnect(const dependency_graph::UniqueId& fromNode, std::size_t fromPort, const dependency_graph::UniqueId& toNode, std::size_t toPort) {
	dependency_graph::NodeBase& from = detail::findNode(fromNode);
	dependency_graph::NodeBase& to = detail::findNode(toNode);

	unlinkAll(fromNode, toNode);

	from.port(fromPort).disconnect(to.port(toPort));

	possumwood::UndoStack tmpStack;
	tmpStack.execute(buildNetwork(fromNode, toNode));
}

}


possumwood::UndoStack::Action disconnectAction(const dependency_graph::UniqueId& fromNodeId, std::size_t fromPort, const dependency_graph::UniqueId& toNodeId, std::size_t toPort) {
	possumwood::UndoStack::Action action;

	// the initial connect / disconnect action
	action.addCommand(
		std::bind(&doDisconnect,
			fromNodeId, fromPort,
			toNodeId, toPort),
		std::bind(&doConnect,
			fromNodeId, fromPort,
			toNodeId, toPort)
	);

	return action;
}

possumwood::UndoStack::Action disconnectAction(dependency_graph::Port& p1, dependency_graph::Port& p2) {
	return disconnectAction(
		p1.node().index(), p1.index(),
		p2.node().index(), p2.index()
	);
}

possumwood::UndoStack::Action connectAction(const dependency_graph::UniqueId& fromNodeId, std::size_t fromPort, const dependency_graph::UniqueId& toNodeId, std::size_t toPort) {
	possumwood::UndoStack::Action action;

	// the connect action
	action.addCommand(
		std::bind(&doConnect,
			fromNodeId, fromPort,
			toNodeId, toPort),
		std::bind(&doDisconnect,
			fromNodeId, fromPort,
			toNodeId, toPort)
	);

	return action;
}

possumwood::UndoStack::Action connectAction(const dependency_graph::Port& p1, const dependency_graph::Port& p2) {
	return connectAction(
		p1.node().index(), p1.index(),
		p2.node().index(), p2.index()
	);
}

} } }