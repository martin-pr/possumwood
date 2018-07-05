#include "node.h"

#include "../io.h"
#include "graph.h"

namespace possumwood { namespace io {

void adl_serializer<dependency_graph::NodeBase>::to_json(json& j, const ::dependency_graph::NodeBase& g) {
	j["name"] = g.name();
	j["type"] = g.metadata()->type();

	for(size_t pi=0; pi<g.portCount(); ++pi) {
		const dependency_graph::Port& p = g.port(pi);

		// only serialize unconnected inputs
		if(p.category() == dependency_graph::Attr::kInput && !g.network().connections().connectedFrom(p))
			if(dependency_graph::io::isSaveable(g.datablock().data(pi)))
				io::toJson(j["ports"][p.name()], g.datablock().data(pi));
	}

	if(not g.hasBlindData())
		j["blind_data"] = nullptr;
	else {
		assert(dependency_graph::io::isSaveable(g.blindData()));
		io::toJson(j["blind_data"]["value"], g.blindData());
		j["blind_data"]["type"] = g.blindDataType();
	}
}

void adl_serializer<dependency_graph::NodeBase>::from_json(const json& j, ::dependency_graph::NodeBase& n) {
	if(j.find("ports") != j.end()) {
		std::vector<bool> dirty;
		dependency_graph::Datablock datablock = ((const dependency_graph::Node&)n).datablock();

		for(json::const_iterator p = j["ports"].begin(); p != j["ports"].end(); ++p) {
			if(!p.value().is_null()) {
				int pi = -1;
				for(unsigned a=0;a<n.portCount();++a)
					if(n.port(a).name() == p.key())
						pi = a;
				if(pi >= 0) {
					assert(dependency_graph::io::isSaveable(datablock.data(pi)));

					std::unique_ptr<dependency_graph::BaseData> data(datablock.data(pi).clone());
					io::fromJson(p.value(), *data);
					datablock.setData(pi, *data);

					// n.markAsDirty(pi);
					dirty.push_back(pi);
				}
				else
					std::cerr << "Found unused property '" << p.key() << "' while loading a file. Ignoring its value." << std::endl;
			}
		}

		n.setDatablock(datablock);
	}
}

} }
