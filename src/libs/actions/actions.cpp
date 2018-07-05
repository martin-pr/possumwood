#include "actions.h"

#include <functional>

#include <dependency_graph/node_base.inl>
#include <dependency_graph/nodes.inl>
#include <dependency_graph/attr_map.h>

#include <possumwood_sdk/metadata.h>

#include <actions/io/graph.h>

#include "app.h"
#include "clipboard.h"

namespace possumwood {

namespace {

dependency_graph::NodeBase& findNode(const dependency_graph::UniqueId& id) {
	if(id == possumwood::AppCore::instance().graph().index())
		return possumwood::AppCore::instance().graph();

	auto it = possumwood::AppCore::instance().graph().nodes().find(id, dependency_graph::Nodes::kRecursive);
	assert(it != possumwood::AppCore::instance().graph().nodes().end());

	// and get the node reference
	return *it;
}

dependency_graph::NodeBase& doCreateNode(const dependency_graph::UniqueId& currentNetworkIndex, const dependency_graph::MetadataHandle& meta, const std::string& name, const dependency_graph::UniqueId& id, const possumwood::NodeData& blindData, boost::optional<const dependency_graph::Datablock&> data = boost::optional<const dependency_graph::Datablock&>()) {
	if(data)
		assert(data->meta() == meta);

	std::unique_ptr<dependency_graph::BaseData> bd(new dependency_graph::Data<possumwood::NodeData>(blindData));

	dependency_graph::NodeBase& netBase = findNode(currentNetworkIndex);
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

void doConnect(const dependency_graph::UniqueId& fromNode, std::size_t fromPort, const dependency_graph::UniqueId& toNode, std::size_t toPort) {
	dependency_graph::NodeBase& from = findNode(fromNode);
	dependency_graph::NodeBase& to = findNode(toNode);

	from.port(fromPort).connect(to.port(toPort));
}

void doDisconnect(const dependency_graph::UniqueId& fromNode, std::size_t fromPort, const dependency_graph::UniqueId& toNode, std::size_t toPort) {
	dependency_graph::NodeBase& from = findNode(fromNode);
	dependency_graph::NodeBase& to = findNode(toNode);

	from.port(fromPort).disconnect(to.port(toPort));
}

void doSetBlindData(const dependency_graph::UniqueId& node, const possumwood::NodeData& blindData) {
	dependency_graph::NodeBase& n = findNode(node);
	n.setBlindData(blindData);
}

} // anonymous namespace

/////////////////////////////////////////////////////////////////////

namespace {

possumwood::UndoStack::Action createNodeAction(dependency_graph::Network& current, const dependency_graph::MetadataHandle& meta, const std::string& name, const possumwood::NodeData& _data, const dependency_graph::UniqueId& id) {
	possumwood::NodeData data;
	data.setPosition(_data.position());

	possumwood::UndoStack::Action action;
	action.addCommand(
		std::bind(&doCreateNode, current.index(), std::ref(meta), name, id, data, boost::optional<const dependency_graph::Datablock&>()),
		std::bind(&doRemoveNode, id)
	);

	return action;
}

}

void Actions::createNode(dependency_graph::Network& current, const dependency_graph::MetadataHandle& meta, const std::string& name, const possumwood::NodeData& _data, const dependency_graph::UniqueId& id) {
	auto action = createNodeAction(current, meta, name, _data, id);

	possumwood::AppCore::instance().undoStack().execute(action);
}

namespace {
	struct Link {
		dependency_graph::UniqueId fromNode;
		std::size_t fromPort;
		dependency_graph::UniqueId toNode;
		std::size_t toPort;
	};

	void doAddLink(const Link& l) {
		dependency_graph::NodeBase& fromNode = findNode(l.fromNode);
		dependency_graph::NodeBase& toNode = findNode(l.toNode);

		assert(!fromNode.port(l.fromPort).isLinked());

		fromNode.port(l.fromPort).linkTo(toNode.port(l.toPort));

		assert(fromNode.port(l.fromPort).isLinked());
		assert(fromNode.port(l.fromPort).linkedTo().index() == l.toPort);
		assert(fromNode.port(l.fromPort).linkedTo().node().index() == l.toNode);
	}

	void doRemoveLink(const Link& l) {
		dependency_graph::NodeBase& fromNode = findNode(l.fromNode);
		// dependency_graph::NodeBase& toNode = findNode(l.toNode);

		assert(fromNode.port(l.fromPort).isLinked());
		assert(fromNode.port(l.fromPort).linkedTo().index() == l.toPort);
		assert(fromNode.port(l.fromPort).linkedTo().node().index() == l.toNode);

		fromNode.port(l.fromPort).unlink();

		assert(!fromNode.port(l.fromPort).isLinked());
	}

	possumwood::UndoStack::Action linkAction(const Link& l) {
		possumwood::UndoStack::Action action;

		action.addCommand(
			std::bind(&doAddLink, l),
			std::bind(&doRemoveLink,l)
		);

		return action;
	};

	possumwood::UndoStack::Action unlinkAction(const dependency_graph::Port& p) {
		Link l{
			p.node().index(), p.index(),
			p.linkedTo().node().index(), p.linkedTo().index()
		};

		possumwood::UndoStack::Action action;

		action.addCommand(
			std::bind(&doRemoveLink,l),
			std::bind(&doAddLink, l)
		);

		return action;
	}
}

possumwood::UndoStack::Action Actions::disconnectAction(const dependency_graph::UniqueId& fromNodeId, std::size_t fromPort, const dependency_graph::UniqueId& toNodeId, std::size_t toPort) {
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
	dependency_graph::NodeBase& fromNode = findNode(fromNodeId);
	dependency_graph::NodeBase& toNode = findNode(toNodeId);
	if((fromNode.metadata()->type() == "input" || toNode.metadata()->type() == "output") &&
		fromNode.hasParentNetwork() && toNode.hasParentNetwork()) {

		// unlink any linked ports first
		dependency_graph::Network& network = fromNode.network();
		for(std::size_t pi=0; pi<network.portCount(); ++pi)
			if(network.port(pi).isLinked())
				action.append(unlinkAction(network.port(pi)));

		// find all input and output nodes of the network with connected outputs
		//   and build metadata that correspond to them
		std::unique_ptr<possumwood::Metadata> meta(new possumwood::Metadata("network"));

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
				meta->doAddAttribute(in);
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
				meta->doAddAttribute(out);
			}
		}

		// build the compute method from all the output-to-output links
		//   -> links between outputs guarantee that dirtiness is passed correctly.
		//      Compute method provides the pull mechanism for data transfer.
		{
			std::vector<std::function<void(dependency_graph::Values&)>> assignments;

			for(auto& l : links)
				if(l.toNode == network.index()) {
					dependency_graph::NodeBase& fromNode = findNode(l.fromNode);

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
			action.append(setValueAction(fromNode.network().index(), pi, *values[pi]->clone()));

		// link all what needs to be linked
		for(auto& l : links)
			action.append(linkAction(l));
	}

	return action;
}

possumwood::UndoStack::Action Actions::disconnectAction(dependency_graph::Port& p1, dependency_graph::Port& p2) {
	return disconnectAction(
		p1.node().index(), p1.index(),
		p2.node().index(), p2.index()
	);
}

possumwood::UndoStack::Action Actions::removeNodeAction(dependency_graph::NodeBase& node) {
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

possumwood::UndoStack::Action Actions::removeNetworkAction(dependency_graph::Network& net) {
	possumwood::UndoStack::Action action;

	// remove all connections
	for(auto& e : net.connections())
		action.append(disconnectAction(e.first, e.second));

	/// and all nodes
	for(auto& n : net.nodes())
		action.append(removeNodeAction(n));

	return action;
}

possumwood::UndoStack::Action Actions::removeAction(const dependency_graph::Selection& _selection) {
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

void Actions::removeNode(dependency_graph::NodeBase& node) {
	dependency_graph::Selection selection;
	selection.addNode(node);

	auto action = removeAction(selection);

	possumwood::AppCore::instance().undoStack().execute(action);
}

possumwood::UndoStack::Action Actions::connectAction(const dependency_graph::UniqueId& fromNodeId, std::size_t fromPort, const dependency_graph::UniqueId& toNodeId, std::size_t toPort) {
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
	dependency_graph::NodeBase& fromNode = findNode(fromNodeId);
	dependency_graph::NodeBase& toNode = findNode(toNodeId);
	if((fromNode.metadata()->type() == "input" || toNode.metadata()->type() == "output") &&
		fromNode.hasParentNetwork() && toNode.hasParentNetwork()) {

		// unlink any linked ports first
		dependency_graph::Network& network = fromNode.network();
		for(std::size_t pi=0; pi<network.portCount(); ++pi)
			if(network.port(pi).isLinked())
				action.append(unlinkAction(network.port(pi)));

		// find all input and output nodes of the network with connected outputs
		//   and build metadata that correspond to them
		std::unique_ptr<possumwood::Metadata> meta(new possumwood::Metadata("network"));

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

					meta->doAddAttribute(*in);
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

					meta->doAddAttribute(*out);
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
					dependency_graph::NodeBase& fromNode = findNode(l.fromNode);

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
			action.append(setValueAction(fromNode.network().index(), pi, *values[pi]->clone()));

		// link all what needs to be linked
		for(auto& l : links)
			action.append(linkAction(l));
	}

	return action;
}

possumwood::UndoStack::Action Actions::connectAction(const dependency_graph::Port& p1, const dependency_graph::Port& p2) {
	return connectAction(
		p1.node().index(), p1.index(),
		p2.node().index(), p2.index()
	);
}

void Actions::connect(dependency_graph::Port& p1, dependency_graph::Port& p2) {
	auto action = connectAction(p1, p2);

	possumwood::AppCore::instance().undoStack().execute(action);
}

void Actions::disconnect(dependency_graph::Port& p1, dependency_graph::Port& p2) {
	auto action = disconnectAction(p1, p2);

	possumwood::AppCore::instance().undoStack().execute(action);
}

void Actions::remove(const dependency_graph::Selection& selection) {
	auto action = removeAction(selection);

	possumwood::AppCore::instance().undoStack().execute(action);
}

void Actions::cut(const dependency_graph::Selection& selection) {
	// trigger the copy action first
	copy(selection);

	// and delete selection
	auto action = removeAction(selection);

	possumwood::AppCore::instance().undoStack().execute(action);
}

void Actions::copy(const dependency_graph::Selection& selection) {
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
			action.addCommand(
				std::bind(&doCreateNode, targetIndex, dependency_graph::MetadataHandle(n.metadata()), n.name(), n.index(), d, cn.datablock()),
				std::bind(&doRemoveNode, n.index())
			);

			// recurse to add nested networks
			if(cn.is<dependency_graph::Network>())
				action.append(pasteNetwork(n.index(), n.as<dependency_graph::Network>()));
		}

		// add all connections, based on "unique" IDs
		for(auto& c : source.connections()) {
			dependency_graph::UniqueId id1 = c.first.node().index();
			dependency_graph::UniqueId id2 = c.second.node().index();

			action.addCommand(
				std::bind(&doConnect, id1, c.first.index(), id2, c.second.index()),
				std::bind(&doDisconnect, id1, c.first.index(), id2, c.second.index())
			);
		}

		return action;
	}
}

void Actions::paste(dependency_graph::Network& current, dependency_graph::Selection& selection) {
	possumwood::UndoStack::Action action;

	dependency_graph::Graph pastedGraph;

	try {
		// convert the selection to JSON object
		auto json = possumwood::io::json::parse(Clipboard::instance().clipboardContent());

		// import the clipboard
		possumwood::io::from_json(json, pastedGraph);

		// THIS WILL ALSO NEED TO WORK RECURSIVELY
		action.append(pasteNetwork(current.index(), pastedGraph));
	}
	catch(std::exception& e) {
		// do nothing
		#ifndef NDEBUG
		std::cout << e.what() << std::endl;
		#endif
	}

	// execute the action (will actually make the nodes and connections)
	possumwood::AppCore::instance().undoStack().execute(action);

	// and make the selection based on added nodes
	{
		for(auto& n : pastedGraph.nodes())
			selection.addNode(findNode(n.index()));

		for(auto& c : pastedGraph.connections()) {
			dependency_graph::UniqueId id1 = c.first.node().index();
			dependency_graph::UniqueId id2 = c.second.node().index();

			dependency_graph::NodeBase& n1 = findNode(id1);
			dependency_graph::NodeBase& n2 = findNode(id2);

			dependency_graph::Port& p1 = n1.port(c.first.index());
			dependency_graph::Port& p2 = n2.port(c.second.index());

			selection.addConnection(p1, p2);
		}
	}
}

void Actions::move(const std::map<dependency_graph::NodeBase*, possumwood::NodeData::Point>& nodes) {
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

namespace {

void doSetMetadata(const dependency_graph::UniqueId& node, const dependency_graph::MetadataHandle& meta, const dependency_graph::Datablock& datablock) {
	dependency_graph::NodeBase& n = findNode(node);
	n.setMetadata(meta);

	n.setDatablock(datablock);
}

struct ConnectionItem {
	dependency_graph::UniqueId thatNode;
	std::size_t thisPort, thatPort;
};

}

possumwood::UndoStack::Action Actions::changeMetadataAction(dependency_graph::NodeBase& node, const dependency_graph::MetadataHandle& handle) {
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
		action.append(disconnectAction(c.thatNode, c.thatPort, node.index(), c.thisPort));

	for(auto& c : outConnections)
		action.append(disconnectAction(node.index(), c.thisPort, c.thatNode, c.thatPort));

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
			action.append(connectAction(c.thatNode, c.thatPort, node.index(), it->second));
	}

	for(auto& c : outConnections) {
		auto it = map.find(c.thisPort);
		if(it != map.end())
			action.append(connectAction(node.index(), it->second, c.thatNode, c.thatPort));
	}

	return action;
}

void Actions::changeMetadata(dependency_graph::NodeBase& node, const dependency_graph::MetadataHandle& handle) {
	possumwood::UndoStack::Action action = changeMetadataAction(node, handle);

	possumwood::AppCore::instance().undoStack().execute(action);
}

namespace {

void doSetValue(const dependency_graph::UniqueId& id, unsigned portId, std::shared_ptr<const dependency_graph::BaseData> value, std::shared_ptr<std::unique_ptr<dependency_graph::BaseData>> original) {
	auto it = AppCore::instance().graph().nodes().find(id, dependency_graph::Nodes::kRecursive);
	assert(it != AppCore::instance().graph().nodes().end());

	assert(original != nullptr);
	if(*original == nullptr)
		*original = it->port(portId).getData().clone();

	assert(*original != nullptr);

	it->port(portId).setData(*value);
}

void doResetValue(const dependency_graph::UniqueId& id, unsigned portId, std::shared_ptr<std::unique_ptr<dependency_graph::BaseData>> value) {
	auto it = AppCore::instance().graph().nodes().find(id, dependency_graph::Nodes::kRecursive);
	assert(it != AppCore::instance().graph().nodes().end());

	assert(value != nullptr);
	assert(*value != nullptr);

	it->port(portId).setData(**value);
}

}

possumwood::UndoStack::Action Actions::setValueAction(dependency_graph::Port& port, const dependency_graph::BaseData& value) {
	return setValueAction(port.node().index(), port.index(), value);
}

possumwood::UndoStack::Action Actions::setValueAction(const dependency_graph::UniqueId& nodeId, std::size_t portId, const dependency_graph::BaseData& value) {
	UndoStack::Action action;

	// dependency_graph::NodeBase& node = findNode(nodeId);

	std::shared_ptr<std::unique_ptr<dependency_graph::BaseData>> original(new std::unique_ptr<dependency_graph::BaseData>());

	// std::shared_ptr<const dependency_graph::BaseData> original = node.port(portId).getData().clone();
	std::shared_ptr<const dependency_graph::BaseData> target = value.clone();

	action.addCommand(
		std::bind(&doSetValue, nodeId, portId, std::move(target), original),
		std::bind(&doResetValue, nodeId, portId, original)
	);

	return action;
}

void Actions::setValue(dependency_graph::Port& port, const dependency_graph::BaseData& value) {
	possumwood::UndoStack::Action action = setValueAction(port, value);

	AppCore::instance().undoStack().execute(action);
}

}
