#include "connections.h"

#include <dependency_graph/detail.h>
#include <dependency_graph/values.h>
#include <dependency_graph/nodes.inl>

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

	if(fromNode.hasParentNetwork() && toNode.hasParentNetwork() && fromNode.network().index() == toNode.network().index()) {
		dependency_graph::Network& network = fromNode.network();

		if((fromNode.metadata()->type() == "input" || toNode.metadata()->type() == "output")) {
			for(std::size_t pi=0; pi<network.portCount(); ++pi)
				if(network.port(pi).isLinked() && network.port(pi).linkedTo().node().network().index() == network.index())
					network.port(pi).unlink();

			for(auto& n : network.nodes())
				for(std::size_t pi=0; pi<n.portCount(); ++pi)
					if(n.port(pi).isLinked() && n.port(pi).linkedTo().node().index() == network.index())
						n.port(pi).unlink();
		}
	}
}

void buildNetwork(const dependency_graph::UniqueId& fromNodeId, const dependency_graph::UniqueId& toNodeId) {
	// special handling for "input" and "output" types
	dependency_graph::NodeBase& fromNode = detail::findNode(fromNodeId);
	dependency_graph::NodeBase& toNode = detail::findNode(toNodeId);
	if((fromNode.metadata()->type() == "input" || toNode.metadata()->type() == "output")) {
		if(fromNode.hasParentNetwork() && toNode.hasParentNetwork() && fromNode.network().index() == toNode.network().index()) {
			dependency_graph::Network& network = fromNode.network();

			::possumwood::actions::detail::buildNetwork(network);
		}
	}
}

void doConnectByRefs(dependency_graph::NodeBase& fromNode, std::size_t fromPort, dependency_graph::NodeBase& toNode, std::size_t toPort) {
	unlinkAll(fromNode.index(), toNode.index());

	assert(fromNode.portCount() > fromPort);
	assert(toNode.portCount() > toPort);

	fromNode.port(fromPort).connect(toNode.port(toPort));

	buildNetwork(fromNode.index(), toNode.index());
}

void doConnectByIds(const dependency_graph::UniqueId& fromNode, std::size_t fromPort, const dependency_graph::UniqueId& toNode, std::size_t toPort) {
	dependency_graph::NodeBase& from = detail::findNode(fromNode);
	dependency_graph::NodeBase& to = detail::findNode(toNode);

	doConnectByRefs(from, fromPort, to, toPort);
}

void doConnectByNames(const dependency_graph::UniqueId& fromNode, const std::string& fromPort, const dependency_graph::UniqueId& toNode, const std::string& toPort) {
	dependency_graph::NodeBase& from = detail::findNode(fromNode);
	dependency_graph::NodeBase& to = detail::findNode(toNode);

	int fromPortId = -1;
	for(std::size_t p = 0; p < from.portCount(); ++p)
		if(from.port(p).name() == fromPort) {
			fromPortId = p;
			break;
		}

	int toPortId = -1;
	for(std::size_t p = 0; p < to.portCount(); ++p)
		if(to.port(p).name() == toPort) {
			toPortId = p;
			break;
		}

	if(fromPortId < 0)
		throw std::runtime_error("Cannot connect "+from.name()+":"+fromPort+" - port doesn't exist on the node.");

	if(toPortId < 0)
		throw std::runtime_error("Cannot connect "+to.name()+":"+toPort+" - port doesn't exist on the node.");

	doConnectByRefs(from, fromPortId, to, toPortId);
}

void doDisconnectByRefs(dependency_graph::NodeBase& fromNode, std::size_t fromPort, dependency_graph::NodeBase& toNode, std::size_t toPort) {
	unlinkAll(fromNode.index(), toNode.index());

	assert(fromNode.portCount() > fromPort);
	assert(toNode.portCount() > toPort);

	fromNode.port(fromPort).disconnect(toNode.port(toPort));

	buildNetwork(fromNode.index(), toNode.index());
}

void doDisconnectByIds(const dependency_graph::UniqueId& fromNode, std::size_t fromPort, const dependency_graph::UniqueId& toNode, std::size_t toPort) {
	dependency_graph::NodeBase& from = detail::findNode(fromNode);
	dependency_graph::NodeBase& to = detail::findNode(toNode);

	doDisconnectByRefs(from, fromPort, to, toPort);
}

void doDisconnectByNames(const dependency_graph::UniqueId& fromNode, const std::string& fromPort, const dependency_graph::UniqueId& toNode, const std::string& toPort) {
	dependency_graph::NodeBase& from = detail::findNode(fromNode);
	dependency_graph::NodeBase& to = detail::findNode(toNode);

	int fromPortId = -1;
	for(std::size_t p = 0; p < from.portCount(); ++p)
		if(from.port(p).name() == fromPort) {
			fromPortId = p;
			break;
		}

	int toPortId = -1;
	for(std::size_t p = 0; p < to.portCount(); ++p)
		if(to.port(p).name() == toPort) {
			toPortId = p;
			break;
		}

	doDisconnectByRefs(from, fromPortId, to, toPortId);
}

}


possumwood::UndoStack::Action disconnectAction(const dependency_graph::UniqueId& fromNodeId, std::size_t fromPort, const dependency_graph::UniqueId& toNodeId, std::size_t toPort) {
	possumwood::UndoStack::Action action;

	std::stringstream ss;
	ss << "Disconnecting " << fromNodeId << "/" << fromPort << " and " << toNodeId << "/" << toPort;

	// the initial connect / disconnect action
	action.addCommand(
		ss.str(),
		std::bind(&doDisconnectByIds,
			fromNodeId, fromPort,
			toNodeId, toPort),
		std::bind(&doConnectByIds,
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

	// value reset after disconnect
	//   -> we want the value as it was when undoing a connect action
	std::shared_ptr<dependency_graph::Data> data(
		new dependency_graph::Data());

	{
		std::stringstream ss;
		ss << "Cloning data for a new connection between " << fromNodeId << "/" << fromPort << " and " << toNodeId << "/" << toPort;

		action.addCommand(
			ss.str(),
			// on connect, save the value
			[toNodeId, toPort, data]() {
				dependency_graph::NodeBase& node = detail::findNode(toNodeId);

				// only on non-void ports, though
				if(data->empty() && node.port(toPort).type() != typeid(void))
					*data = node.port(toPort).getData();
			},

			// and on disconnect, put it back
			[toNodeId, toPort, data]() {
				dependency_graph::NodeBase& node = detail::findNode(toNodeId);

				if(!data->empty()) {
					assert(!node.port(toPort).isConnected());
					node.port(toPort).setData(*data);
				}
			}
		);
	}

	{
		std::stringstream ss;
		ss << "Creating a connection between " << fromNodeId << "/" << fromPort << " and " << toNodeId << "/" << toPort;

		action.addCommand(
			ss.str(),
			std::bind(&doConnectByIds,
				fromNodeId, fromPort,
				toNodeId, toPort),
			std::bind(&doDisconnectByIds,
				fromNodeId, fromPort,
				toNodeId, toPort)
		);
	}

	return action;
}

possumwood::UndoStack::Action connectAction(const dependency_graph::UniqueId& fromNodeId, const std::string& fromPortName, const dependency_graph::UniqueId& toNodeId, const std::string& toPortName) {
	possumwood::UndoStack::Action action;

	// value reset after disconnect
	//   -> we want the value as it was when undoing a connect action
	std::shared_ptr<dependency_graph::Data> data(new dependency_graph::Data());

	{
		std::stringstream ss;
		ss << "Cloning data for a new connection between " << fromNodeId << "/" << fromPortName << " and " << toNodeId << "/" << toPortName;

		action.addCommand(
			ss.str(),
			// on connect, save the value
			[toNodeId, toPortName, data]() {
				dependency_graph::NodeBase& node = detail::findNode(toNodeId);

				int toPort = -1;
				for(std::size_t p = 0; p < node.portCount(); ++p)
					if(node.port(p).name() == toPortName) {
						toPort = p;
						break;
					}
				if(toPort < 0)
					throw std::runtime_error("Port '" + toPortName + "' not found on node '" + node.name() + "'.");

				// only on non-void ports, though
				if(data->empty() && node.port(toPort).type() != typeid(void))
					*data = node.port(toPort).getData();
			},

			// and on disconnect, put it back
			[toNodeId, toPortName, data]() {
				dependency_graph::NodeBase& node = detail::findNode(toNodeId);

				int toPort = -1;
				for(std::size_t p = 0; p < node.portCount(); ++p)
					if(node.port(p).name() == toPortName) {
						toPort = p;
						break;
					}
				if(toPort < 0)
					throw std::runtime_error("Port '" + toPortName + "' not found on node '" + node.name() + "'.");

				if(!data->empty()) {
					assert(!node.port(toPort).isConnected());
					node.port(toPort).setData(*data);
				}
			}
		);
	}

	{
		std::stringstream ss;
		ss << "Creating a connection between " << fromNodeId << "/" << fromPortName << " and " << toNodeId << "/" << toPortName;

		action.addCommand(
			ss.str(),
			std::bind(&doConnectByNames,
				fromNodeId, fromPortName,
				toNodeId, toPortName),
			std::bind(&doDisconnectByNames,
				fromNodeId, fromPortName,
				toNodeId, toPortName)
		);
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
