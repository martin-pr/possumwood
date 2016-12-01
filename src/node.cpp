#include "node.h"

Node::Port::Port(const std::string& name, unsigned id, Node* parent) :
	m_name(name), m_id(id), m_parent(parent) {

}

Node::Port::Port(Port&& p) : m_name(std::move(p.m_name)), m_id(p.m_id), m_parent(p.m_parent) {

}

const std::string& Node::Port::name() const {
	return m_name;
}

const Attr::Category Node::Port::category() const {
	return m_parent->m_meta.attr(m_id).category();
}

Node::Node(const std::string& name, const Metadata& def) : m_name(name), m_meta(def) {
	for(std::size_t a = 0; a < def.attributeCount(); ++a) {
		auto& meta = def.attr(a);
		assert(meta.offset() == a);

		m_ports.push_back(std::move(Port(meta.name(), meta.offset(), this)));
	}
}

Node::Port& Node::port(const std::string& name) {
	auto it = m_ports.end();
	for(auto i = m_ports.begin(); i != m_ports.end(); ++i)
		if(i->name() == name) {
			it = i;
			break;
		}

	assert(it != m_ports.end());
	return *it;
}

const Node::Port& Node::port(const std::string& name) const {
	auto it = m_ports.end();
	for(auto i = m_ports.begin(); i != m_ports.end(); ++i)
		if(i->name() == name) {
			it = i;
			break;
		}

	assert(it != m_ports.end());
	return *it;
}
