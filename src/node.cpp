#include "node.h"

#include "graph.h"

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

bool Node::Port::isDirty() const {
	return m_dirty;
}

Node& Node::Port::node() {
	assert(m_parent != NULL);
	return *m_parent;
}

const Node& Node::Port::node() const {
	assert(m_parent != NULL);
	return *m_parent;
}

void Node::Port::markAsDirty() {
	m_dirty = true;
}

/////////////

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

void Node::markAsDirty(size_t index) {
	Node::Port& p = port(index);

	// mark the port itself as dirty
	p.markAsDirty();

	// recurse + handle each port type slightly differently
	if(p.category() == Attr::kInput) {
		// all outputs influenced by this input are marked dirty
		for(const Attr& i : m_meta.influences(p.m_id))
			markAsDirty(i.offset());
	}
	else {
		// all inputs connected to this output are marked dirty
		for(Port& o : m_parent->connections().connectedTo(port(index)))
			o.markAsDirty();
	}
}

bool Node::inputIsNotConnected(const Port& p) const {
	assert(p.category() == Attr::kInput);

	// return true if there are no connections leading to this input port
	return !m_parent->connections().connectedFrom(p);
}

void Node::computeInput(size_t index) {

}

void Node::computeOutput(size_t index) {

}
