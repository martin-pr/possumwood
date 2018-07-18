#include "connections.h"

#include <dependency_graph/detail.h>
#include <dependency_graph/values.h>

#include "tools.h"
#include "links.h"
#include "metadata.h"
#include "values.h"

namespace possumwood { namespace actions { namespace detail {

namespace {

void doConnect(const dependency_graph::UniqueId& fromNode, std::size_t fromPort, const dependency_graph::UniqueId& toNode, std::size_t toPort) {
	dependency_graph::NodeBase& from = detail::findNode(fromNode);
	dependency_graph::NodeBase& to = detail::findNode(toNode);

	from.port(fromPort).connect(to.port(toPort));
}

void doDisconnect(const dependency_graph::UniqueId& fromNode, std::size_t fromPort, const dependency_graph::UniqueId& toNode, std::size_t toPort) {
	dependency_graph::NodeBase& from = detail::findNode(fromNode);
	dependency_graph::NodeBase& to = detail::findNode(toNode);

	from.port(fromPort).disconnect(to.port(toPort));
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

	// special handling for "input" and "output" types
	dependency_graph::NodeBase& fromNode = detail::findNode(fromNodeId);
	dependency_graph::NodeBase& toNode = detail::findNode(toNodeId);
	if((fromNode.metadata()->type() == "input" || toNode.metadata()->type() == "output") &&
		fromNode.hasParentNetwork() && toNode.hasParentNetwork()) {

		// unlink any linked ports first
		dependency_graph::Network& network = fromNode.network();
		for(std::size_t pi=0; pi<network.portCount(); ++pi)
			if(network.port(pi).isLinked())
				action.append(unlinkAction(network.port(pi)));

		// find all input and output nodes of the network with connected outputs
		//   and build metadata that correspond to them
		std::unique_ptr<dependency_graph::Metadata> meta = dependency_graph::instantiateMetadata("network");

		std::vector<Link> links;
		std::vector<std::unique_ptr<dependency_graph::BaseData>> values;

		for(auto& n : fromNode.network().nodes()) {
			if(n.metadata()->type() == "input" && (n.port(0).isConnected() && n.index() != fromNodeId)) {
				// link the two ports - will just transfer values blindly
				links.push_back(Link {
					fromNode.network().index(), meta->attributeCount(),
					n.index(), 0
				});

				// also the port value
				values.push_back(n.port(0).getData().clone());

				// get the attribute to add to metadata from the other side of the connection
				auto conns = n.network().connections().connectedTo(n.port(0));
				assert(!conns.empty());

				dependency_graph::Attr in = conns.front().get().node().metadata()->attr(conns.front().get().index());;
				dependency_graph::detail::MetadataAccess::addAttribute(*meta, in);
			}

			if(n.metadata()->type() == "output" && (n.port(0).isConnected() && n.index() != toNodeId)) {
				// link the two ports - this will in practice just transfer the dirtiness
				links.push_back(Link {
					n.index(), 0,
					fromNode.network().index(), meta->attributeCount()
				});

				// but first unlink it if it is linked already
				if(n.port(0).isLinked())
					action.append(unlinkAction(n.port(0)));

				// also port the value
				values.push_back(n.port(0).getData().clone());

				// get the attribute to add to metadata from the other side of the connection
				auto conns = n.network().connections().connectedFrom(n.port(0));
				assert(conns);

				dependency_graph::Attr out = conns->node().metadata()->attr(conns->index());;
				dependency_graph::detail::MetadataAccess::addAttribute(*meta, out);
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

	// special handling for "input" and "output" types
	dependency_graph::NodeBase& fromNode = detail::findNode(fromNodeId);
	dependency_graph::NodeBase& toNode = detail::findNode(toNodeId);
	if((fromNode.metadata()->type() == "input" || toNode.metadata()->type() == "output") &&
		fromNode.hasParentNetwork() && toNode.hasParentNetwork()) {

		// unlink any linked ports first
		dependency_graph::Network& network = fromNode.network();
		for(std::size_t pi=0; pi<network.portCount(); ++pi)
			if(network.port(pi).isLinked())
				action.append(unlinkAction(network.port(pi)));

		// find all input and output nodes of the network with connected outputs
		//   and build metadata that correspond to them
		std::unique_ptr<dependency_graph::Metadata> meta = dependency_graph::instantiateMetadata("network");

		std::vector<Link> links;
		std::vector<std::unique_ptr<dependency_graph::BaseData>> values;

		for(auto& n : fromNode.network().nodes()) {
			if(n.metadata()->type() == "input") {
				if(n.port(0).isConnected() || n.index() == fromNodeId) {
					// link the two ports - will just transfer values blindly
					links.push_back(Link {
						fromNode.network().index(), meta->attributeCount(),
						n.index(), 0
					});

					// also port the value
					if(n.port(0).isConnected())
						values.push_back(n.port(0).getData().clone());
					else
						values.push_back(toNode.port(toPort).getData().clone());

					// get the attribute to add to metadata from the other side of the connection
					std::unique_ptr<dependency_graph::Attr> in;

					if(n.port(0).isConnected()) {
						auto conns = n.network().connections().connectedTo(n.port(0));
						assert(!conns.empty());

						in = std::unique_ptr<dependency_graph::Attr>(
							new dependency_graph::Attr(conns.front().get().node().metadata()->attr(conns.front().get().index())));
					}
					else {
						assert(n.index() == fromNodeId);

						in = std::unique_ptr<dependency_graph::Attr>(
							new dependency_graph::Attr(toNode.metadata()->attr(toPort)));
					}

					dependency_graph::detail::MetadataAccess::addAttribute(*meta, *in);
				}
			}

			if(n.metadata()->type() == "output") {
				if(n.port(0).isConnected() || n.index() == toNodeId) {
					// link the two ports - this will in practice just transfer the dirtiness
					links.push_back(Link {
						n.index(), 0,
						fromNode.network().index(), meta->attributeCount()
					});

					// but first unlink it if it is linked already
					if(n.port(0).isLinked())
						action.append(unlinkAction(n.port(0)));

					// also port the value
					if(n.port(0).isConnected())
						values.push_back(n.port(0).getData().clone());
					else
						values.push_back(fromNode.port(fromPort).getData().clone());

					// get the attribute to add to metadata from the other side of the connection
					std::unique_ptr<dependency_graph::Attr> out;

					if(n.port(0).isConnected()) {
						auto conns = n.network().connections().connectedFrom(n.port(0));
						assert(conns);

						out = std::unique_ptr<dependency_graph::Attr>(
							new dependency_graph::Attr(conns->node().metadata()->attr(conns->index())));
					}
					else {
						assert(n.index() == toNodeId);

						out = std::unique_ptr<dependency_graph::Attr>(
							new dependency_graph::Attr(fromNode.metadata()->attr(fromPort)));
					}

					dependency_graph::detail::MetadataAccess::addAttribute(*meta, *out);
				}
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

possumwood::UndoStack::Action connectAction(const dependency_graph::Port& p1, const dependency_graph::Port& p2) {
	return connectAction(
		p1.node().index(), p1.index(),
		p2.node().index(), p2.index()
	);
}

} } }
