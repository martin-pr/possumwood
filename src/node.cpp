#include "node.h"

Node::Port::Port(const std::string& name, unsigned id, Node* parent) :
	m_name(name), m_id(id), m_parent(parent) {

}

Node::Port::Port(Port&& p) : m_name(std::move(p.m_name)), m_id(p.m_id), m_dirty(true), m_parent(p.m_parent) {

}

const std::string& Node::Port::name() const {
	return m_name;
}

const Attr::Category Node::Port::category() const {
	return m_parent->m_meta.attr(m_id).category();
}

Node::Node(const std::string& name, const Metadata& def, Graph* parent) : m_name(name), m_parent(parent), m_meta(def), m_data(def) {
	for(std::size_t a = 0; a < def.attributeCount(); ++a) {
		auto& meta = def.attr(a);
		assert(meta.offset() == a);

		m_ports.push_back(std::move(Port(meta.name(), meta.offset(), this)));
	}
}

Node::Port& Node::port(size_t index) {
	assert(index < m_ports.size());
	return m_ports[index];
}

const Node::Port& Node::port(size_t index) const {
	assert(index < m_ports.size());
	return m_ports[index];
}
