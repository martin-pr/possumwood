#include "selection.h"

namespace dependency_graph {

bool Selection::Connection::operator < (const Connection& c) const {
	Port* f1 = &(from.get());
	Port* f2 = &(c.from.get());

	if(f1 != f2)
		return f1 < f2;

	Port* t1 = &(to.get());
	Port* t2 = &(to.get());

	return t1 < t2;
}

bool Selection::NodeComparator::operator()(const std::reference_wrapper<NodeBase>& n1, const std::reference_wrapper<NodeBase>& n2) const {
	return &(n1.get()) < &(n2.get());
}


///////

void Selection::addNode(NodeBase& n) {
	m_nodes.insert(std::ref(n));
}

void Selection::addConnection(Port& from, Port& to) {
	m_connections.insert(Connection{std::ref(from), std::ref(to)});
}

const std::set<std::reference_wrapper<NodeBase>, Selection::NodeComparator>& Selection::nodes() const {
	return m_nodes;
}

const std::set<Selection::Connection>& Selection::connections() const {
	return m_connections;
}

bool Selection::empty() const {
	return m_nodes.empty() && m_connections.empty();
}

}
