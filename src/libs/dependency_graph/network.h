#pragma once

#include "nodes.h"
#include "connections.h"

namespace dependency_graph {

class Network : public NodeBase {
	public:
		virtual ~Network();

		bool empty() const;
		void clear();

		Nodes& nodes();
		const Nodes& nodes() const;

		Connections& connections();
		const Connections& connections() const;

	protected:
		std::unique_ptr<NodeBase> makeNode(const std::string& name, const MetadataHandle& md, const UniqueId& id = UniqueId());

		static const MetadataHandle& defaultMetadata();

	private:
		Network(const std::string& name, const UniqueId& id, const MetadataHandle& md, Network* parent);

		Nodes m_nodes;
		Connections m_connections;

	friend class Graph;
	friend class Nodes;
};

}
