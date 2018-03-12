#pragma once

#include "nodes.h"
#include "connections.h"

namespace dependency_graph {

class Network : public boost::noncopyable {
	public:
		bool empty() const;
		void clear();

		Nodes& nodes();
		const Nodes& nodes() const;

		Connections& connections();
		const Connections& connections() const;

		const Graph& graph() const;
		Graph& graph();

	private:
		Network(Graph* parent);

		Graph* m_graph;

		Nodes m_nodes;
		Connections m_connections;

	friend class Graph;
};

}
