#pragma once

#include <set>

#include "node.h"

namespace dependency_graph {

class Selection {
	private:
		struct NodeComparator {
			bool operator()(const std::reference_wrapper<Node>& n1, const std::reference_wrapper<Node>& n2) const;
		};

	public:
		struct Connection {
			std::reference_wrapper<Port> from, to;

			bool operator < (const Connection& c) const;
		};

		void addNode(Node& n);
		void addConnection(Port& from, Port& to);

		bool empty() const;

		const std::set<std::reference_wrapper<Node>, NodeComparator>& nodes() const;
		const std::set<Connection>& connections() const;

	protected:
		std::set<std::reference_wrapper<Node>, NodeComparator> m_nodes;
		std::set<Connection> m_connections;

};

};
