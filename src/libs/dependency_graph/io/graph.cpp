#include "graph.h"

#include "node.h"
#include "node_base.inl"
#include "io.h"
#include "../metadata_register.h"

namespace dependency_graph { namespace io {

namespace {

template<typename T>
const T& dereference(const std::reference_wrapper<T>& n) {
	return n.get();
}

template<typename T>
const T& dereference(const T& n) {
	return n;
}

void writeNetwork(json& j, const ::dependency_graph::Network& net, const Selection& selection = Selection());

/// a wrapper of node writing, with CONTAINER either a Nodes instance or a Selection
template<typename CONTAINER>
void writeNodes(json& j, const CONTAINER& nodes, std::map<std::string, unsigned>& uniqueIds, std::map<const ::dependency_graph::NodeBase*, std::string>& nodeIds) {
	for(auto& ni : nodes) {
		const NodeBase& n = dereference(ni);

		// figure out a unique name - type with a number appended
		std::string name = n.metadata().type();
		auto slash = name.rfind('/');
		if(slash != std::string::npos)
			name = name.substr(slash+1);
		name += "_" + std::to_string(uniqueIds[name]++);

		// and use this to save the node
		j[name] = n;

		if(n.is<Network>())
			writeNetwork(j[name], n.as<Network>());

		// remember the assigned ID for connection saving
		nodeIds[&n] = name;
	}
}

void writeNetwork(json& j, const ::dependency_graph::Network& net, const Selection& selection) {
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
}

}

void to_json(json& j, const ::dependency_graph::Graph& g, const Selection& selection) {
	writeNetwork(j, g, selection);
}

void adl_serializer<Graph>::to_json(json& j, const ::dependency_graph::Graph& g) {
	::dependency_graph::io::to_json(j, g);
}

//////////////

namespace {

void loadNetwork(const json& j, Network& net) {
	std::map<std::string, dependency_graph::UniqueId> nodeIds;

	for(json::const_iterator ni = j["nodes"].begin(); ni != j["nodes"].end(); ++ni) {
		const json& n = ni.value();

		// extract the blind data via factory mechanism
		std::unique_ptr<BaseData> blindData;
		if(!n["blind_data"].is_null()) {
			blindData = BaseData::create(n["blind_data"]["type"].get<std::string>());
			assert(blindData != nullptr);
			assert(io::isSaveable(*blindData));
			io::fromJson(n["blind_data"]["value"], *blindData);
		}

		// find the metadata instance
		const MetadataHandle& meta = MetadataRegister::singleton()[n["type"].get<std::string>()];
		NodeBase& node = net.nodes().add(meta, n["name"].get<std::string>(), std::move(blindData));

		// read the node data
		adl_serializer<NodeBase>::from_json(n, node);

		// special handling for networks - load the network on top of base node data
		if(node.is<Network>())
			loadNetwork(ni.value(), node.as<Network>());

		// and record the new node instance, in index-to-uniqueid translation
		assert(nodeIds.find(ni.key()) == nodeIds.end());
		nodeIds[ni.key()] = node.index();
	}

	for(auto& c : j["connections"]) {
		NodeBase& n1 = net.nodes()[nodeIds[c["out_node"].get<std::string>()]];
		NodeBase& n2 = net.nodes()[nodeIds[c["in_node"].get<std::string>()]];

		int p1 = -1;
		for(unsigned p=0;p<n1.portCount();++p)
			if(n1.port(p).name() == c["out_port"].get<std::string>())
				p1 = p;

		int p2 = -1;
		for(unsigned p=0;p<n2.portCount();++p)
			if(n2.port(p).name() == c["in_port"].get<std::string>())
				p2 = p;

		if(p1 >= 0 && p2 >= 0)
			n1.port(p1).connect(n2.port(p2));
	}
}

}

void from_json(const json& j, Graph& g) {
	loadNetwork(j, g);
}

void adl_serializer<Graph>::from_json(const json& j, ::dependency_graph::Graph& g) {
	::dependency_graph::io::from_json(j, g);
}

} }
