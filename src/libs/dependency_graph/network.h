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
		Network(const std::string& name, const UniqueId& id, const MetadataHandle& md, Network* parent);

		static const MetadataHandle& defaultMetadata();

	private:
		Nodes m_nodes;
		Connections m_connections;

	friend class Nodes;
	friend class Metadata;
};

}
