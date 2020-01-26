#include "actions.h"

#include <functional>
#include <set>

#include <dependency_graph/node_base.inl>
#include <dependency_graph/nodes.inl>
#include <dependency_graph/attr_map.h>
#include <dependency_graph/values.h>
#include <dependency_graph/detail.h>
#include <dependency_graph/metadata_register.h>
#include <dependency_graph/network.h>

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
	std::unique_ptr<dependency_graph::Data> nodeData(
		new dependency_graph::Data(_data));

	auto action = detail::createNodeAction(current, meta, name, std::move(nodeData), id);

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

namespace {

template<typename T>
const T& dereference(const std::reference_wrapper<T>& n) {
	return n.get();
}

template<typename T>
const T& dereference(const T& n) {
	return n;
}

possumwood::io::json writeNode(const dependency_graph::NodeBase& n) {
	possumwood::io::json j;

	j["name"] = n.name();
	j["type"] = n.metadata()->type();

	for(size_t pi=0; pi<n.portCount(); ++pi) {
		const dependency_graph::Port& p = n.port(pi);

		// only serialize unconnected inputs
		if(p.category() == dependency_graph::Attr::kInput && !n.network().connections().connectedFrom(p))
			if(!n.datablock().isNull(pi) && dependency_graph::io::isSaveable(n.datablock().data(pi)))
				io::toJson(j["ports"][p.name()], n.datablock().data(pi));
	}

	if(not n.hasBlindData())
		j["blind_data"] = nullptr;
	else {
		assert(dependency_graph::io::isSaveable(n.blindData()));
		io::toJson(j["blind_data"]["value"], n.blindData());
		j["blind_data"]["type"] = n.blindDataType();
	}

	return j;
}


possumwood::io::json writeNetwork(const ::dependency_graph::Network& net, const dependency_graph::Selection& selection = dependency_graph::Selection());

/// a wrapper of node writing, with CONTAINER either a Nodes instance or a Selection
template<typename CONTAINER>
void writeNodes(possumwood::io::json& j, const CONTAINER& nodes, std::map<std::string, unsigned>& uniqueIds, std::map<const ::dependency_graph::NodeBase*, std::string>& nodeIds) {
	for(auto& ni : nodes) {
		const dependency_graph::NodeBase& n = dereference(ni);

		// figure out a unique name - type with a number appended
		std::string name = n.metadata()->type();
		auto slash = name.rfind('/');
		if(slash != std::string::npos)
			name = name.substr(slash+1);
		name += "_" + std::to_string(uniqueIds[name]++);

		// and use this to save the node
		j[name] = writeNode(n);

		if(n.is<dependency_graph::Network>()) {
			auto tmp = writeNetwork(n.as<dependency_graph::Network>());
			for(possumwood::io::json::const_iterator i = tmp.begin(); i != tmp.end(); ++i)
				j[name][i.key()] = i.value();
		}

		// remember the assigned ID for connection saving
		nodeIds[&n] = name;
	}
}

possumwood::io::json writeNetwork(const dependency_graph::Network& net, const dependency_graph::Selection& selection) {
	possumwood::io::json j;

	std::map<std::string, unsigned> uniqueIds;
	std::map<const ::dependency_graph::NodeBase*, std::string> nodeIds;

	j["nodes"] = "{}"_json;
	if(selection.empty())
		writeNodes(j["nodes"], net.nodes(), uniqueIds, nodeIds);
	else
		writeNodes(j["nodes"], selection.nodes(), uniqueIds, nodeIds);

	j["connections"] = "[]"_json;
	for(auto& c : net.connections()) {
		auto itOut = nodeIds.find(&c.first.node());
		auto itIn = nodeIds.find(&c.second.node());
		if(itOut != nodeIds.end() && itIn != nodeIds.end()) {
			j["connections"].push_back("{}"_json);

			auto& connection = j["connections"].back();

			connection["out_node"] = itOut->second;
			connection["out_port"] = c.first.name();
			connection["in_node"] = itIn->second;
			connection["in_port"] = c.second.name();
		}
	}

	return j;
}

}

possumwood::io::json toJson(const dependency_graph::Selection& selection) {
	dependency_graph::Network* net = &possumwood::AppCore::instance().graph();
	if(!selection.empty() && selection.nodes().begin()->get().hasParentNetwork())
		net = &selection.nodes().begin()->get().network();

	// convert the selection to JSON string
	possumwood::io::json json = writeNetwork(*net, selection);

	return json;
}

void copy(const dependency_graph::Selection& selection) {
	auto json = toJson(selection);

	std::stringstream ss;
	ss << std::setw(4) << json;

	// and put it to the clipboard
	Clipboard::instance().setClipboardContent(ss.str().c_str());
}

namespace {
	possumwood::UndoStack::Action pasteNetwork(const dependency_graph::UniqueId& targetIndex, const possumwood::io::json& source, std::set<dependency_graph::UniqueId>* ids = nullptr) {
		possumwood::UndoStack::Action action;

		// indices of newly loaded nodes
		std::map<std::string, dependency_graph::UniqueId> nodeIds;

		// add all the nodes to the parent network
		//  - each node has a unique ID (unique between all graphs), store that
		if(source.find("nodes") != source.end()) {
			for(possumwood::io::json::const_iterator ni = source["nodes"].begin(); ni != source["nodes"].end(); ++ni) {
				const possumwood::io::json& n = ni.value();

				// extract the blind data via factory mechanism
				std::unique_ptr<dependency_graph::Data> blindData;
				if(n.find("blind_data") != n.end() && !n["blind_data"].is_null()) {
					blindData = dependency_graph::Data::create(n["blind_data"]["type"].get<std::string>());
					assert(blindData != nullptr);
					assert(dependency_graph::io::isSaveable(*blindData));
					io::fromJson(n["blind_data"]["value"], *blindData);
				}

				// find the metadata instance
				const dependency_graph::MetadataHandle& meta = dependency_graph::MetadataRegister::singleton()[n["type"].get<std::string>()];

				// generate a new unique index for the node
				assert(nodeIds.find(ni.key()) == nodeIds.end());
				const dependency_graph::UniqueId nodeId;
				nodeIds.insert(std::make_pair(ni.key(), nodeId));

				if(ids)
					ids->insert(nodeId);

				// add the action to create the node itself
				action.append(detail::createNodeAction(targetIndex, meta, n["name"].get<std::string>(),
					std::move(blindData), nodeId));

				// recurse to add nested networks
				//   -> this will also construct the internals of the network, and instantiate
				//      its inputs and outputs
				if(n["type"] == "network")
					action.append(pasteNetwork(nodeId, n));

				// and another action to set all port values based on the json content
				//   -> as the node doesn't exist yet, we can't interpret the types
				if(n.find("ports") != n.end())
					for(possumwood::io::json::const_iterator pi = n["ports"].begin(); pi != n["ports"].end(); ++pi)
						action.append(detail::setValueAction(nodeId, pi.key(), pi.value()));
			}
		}

		// add all connections, based on "unique" IDs
		if(source.find("connections") != source.end()) {
			for(auto& c : source["connections"]) {
				auto id1 = nodeIds.find(c["out_node"].get<std::string>());
				assert(id1 != nodeIds.end());
				auto port1 = c["out_port"].get<std::string>();

				auto id2 = nodeIds.find(c["in_node"].get<std::string>());
				assert(id2 != nodeIds.end());
				auto port2 = c["in_port"].get<std::string>();

				action.append(detail::connectAction(id1->second, port1, id2->second, port2));
			}
		}

		return action;
	}
}

void paste(dependency_graph::Network& current, dependency_graph::Selection& selection) {
	paste(current, selection, Clipboard::instance().clipboardContent());
}

dependency_graph::State paste(dependency_graph::Network& current, dependency_graph::Selection& selection, const std::string& content, bool haltOnError) {
	dependency_graph::State state;

	try {
		// convert the clipboard content to a json object
		auto json = possumwood::io::json::parse(content);

		// and pass it to the paste() implementation
		state = fromJson(current, selection, json, haltOnError);
	}
	catch(std::exception& e) {
		// do nothing
		#ifndef NDEBUG
		std::cout << e.what() << std::endl;
		#endif
	}

	return state;
}

dependency_graph::State fromJson(dependency_graph::Network& current, dependency_graph::Selection& selection, const possumwood::io::json& json, bool haltOnError) {
	possumwood::UndoStack::Action action;

	std::set<dependency_graph::UniqueId> pastedNodeIds;

	// paste the network extracted from the JSON
	action.append(pasteNetwork(current.index(), json, &pastedNodeIds));

	// execute the action (will actually make the nodes and connections)
	dependency_graph::State state = possumwood::AppCore::instance().undoStack().execute(action, haltOnError);

	// and make the selection based on added nodes
	for(auto& n : pastedNodeIds)
		selection.addNode(detail::findNode(n));

	return state;
}

void move(const std::map<dependency_graph::NodeBase*, possumwood::NodeData::Point>& nodes) {
	possumwood::UndoStack::Action action;

	for(auto& n : nodes) {
		const possumwood::NodeData originalData = n.first->blindData<possumwood::NodeData>();

		if(originalData.position() != n.second) {
			possumwood::NodeData data = originalData;
			data.setPosition(n.second);

			action.addCommand(
				"Move " + n.first->name(),
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

void setValue(dependency_graph::Port& port, const dependency_graph::Data& value) {
	possumwood::UndoStack::Action action = detail::setValueAction(port, value);

	AppCore::instance().undoStack().execute(action);
}

void renameNode(dependency_graph::NodeBase& node, const std::string& name) {
	possumwood::UndoStack::Action action = detail::renameNodeAction(node.index(), name);

	AppCore::instance().undoStack().execute(action);
}

} }
