#include "values.h"

namespace dependency_graph {

Values::Values(NodeBase& n) : m_node(&n) {
}

Values::Values(Values&& vals) : m_node(vals.m_node) {
}

Values& Values::operator=(Values&& vals) {
	m_node = vals.m_node;

	return *this;
}

void Values::copy(const InAttr<void>& inAttr, const OutAttr<void>& outAttr) {
	m_node->port(outAttr.offset()).setData(m_node->port(inAttr.offset()).getData());
}

const Data& Values::data(std::size_t index) const {
	return m_node->port(index).getData();
}

void Values::setData(std::size_t index, const Data& data) {
	m_node->port(index).setData(data);
}

}  // namespace dependency_graph
