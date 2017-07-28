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
			g.m_data.data(pi).toJson(j["ports"][p.name()]);
	}

	if(g.m_blindData == nullptr)
		j["blind_data"] = nullptr;
	else {
		g.m_blindData->toJson(j["blind_data"]["value"]);
		j["blind_data"]["type"] = g.m_blindData->type();
	}
}

void adl_serializer<Node>::from_json(const json& j, ::dependency_graph::Node& n) {
	if(j.find("ports") != j.end()) {
		for(json::const_iterator p = j["ports"].begin(); p != j["ports"].end(); ++p) {
			if(!p.value().is_null()) {
				int pi = -1;
				for(unsigned a=0;a<n.portCount();++a)
					if(n.port(a).name() == p.key())
						pi = a;
				if(pi >= 0) {
					n.m_data.data(pi).fromJson(p.value());
					n.markAsDirty(pi);
				}
				else
					std::cerr << "Found unused property '" << p.key() << "' while loading a file. Ignoring its value." << std::endl;
			}
		}
	}
}

} }
