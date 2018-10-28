#pragma once

#include <set>
#include <iostream>

#include "node.h"

namespace dependency_graph {

class Selection {
	private:
		struct NodeComparator {
			bool operator()(const std::reference_wrapper<NodeBase>& n1, const std::reference_wrapper<NodeBase>& n2) const;
		};

	public:
		struct Connection {
			std::reference_wrapper<Port> from, to;

			bool operator < (const Connection& c) const;
		};

		void addNode(NodeBase& n);
		void addConnection(Port& from, Port& to);

		bool empty() const;

		const std::set<std::reference_wrapper<NodeBase>, NodeComparator>& nodes() const;
		const std::set<Connection>& connections() const;

	protected:
		std::set<std::reference_wrapper<NodeBase>, NodeComparator> m_nodes;
		std::set<Connection> m_connections;
};

std::ostream& operator << (std::ostream& out, const Selection& s);

};
