#include "node.h"

#include "graph.h"

namespace dependency_graph { namespace io {

void adl_serializer<Node>::to_json(json& j, const ::dependency_graph::Node& g) {
	j["name"] = g.name();
	j["type"] = g.metadata().type();

	for(size_t pi=0; pi<g.portCount(); ++pi) {
		const Port& p = g.port(pi);

		// only serialize unconnected inputs
		if(p.category() == Attr::kInput && !g.graph().connections().connectedFrom(p))
			g.m_data.data(pi).toJson(j["ports"][pi]);
		else
			j["ports"][pi] = nullptr;
	}

	if(g.m_blindData == nullptr)
		j["blind_data"] = nullptr;
	else {
		g.m_blindData->toJson(j["blind_data"]["value"]);
		j["blind_data"]["type"] = g.m_blindData->type();
	}
}

void adl_serializer<Node>::from_json(const json& j, ::dependency_graph::Node& n) {
	// type and name are already read from graph - only need to read ports and blind data

	unsigned pi = 0;
	for(auto& p : j["ports"]) {
		if(!p.is_null())
			n.m_data.data(pi).fromJson(p);
		++pi;
	}

	if(!j["blind_data"].is_null()) {
		n.m_blindData = BaseData::create(j["blind_data"]["type"].get<std::string>());
		n.m_blindData->fromJson(j["blind_data"]["value"]);
	}
	else
		n.m_blindData.reset();
}

} }
