#include "values.h"

namespace dependency_graph {

Values::Values(NodeBase& n) : m_node(&n) {
}

Values::Values(Values&& vals) : m_node(vals.m_node) {
}

Values& Values::operator =(Values&& vals) {
	m_node = vals.m_node;

	return *this;
}

void Values::transfer(std::size_t index, Port& p) {
	m_node->port(index).setData(p.getData());
}

const BaseData& Values::data(std::size_t index) const {
	return m_node->port(index).getData();
}

}
