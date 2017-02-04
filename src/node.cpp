#include "node.h"

#include "graph.h"

namespace dependency_graph {

Node::Port::Port(const std::string& name, unsigned id, Node* parent) :
	m_name(name), m_id(id), m_dirty(true), m_parent(parent) {

}

Node::Port::Port(Port&& p) : m_name(std::move(p.m_name)), m_id(p.m_id), m_dirty(true), m_parent(p.m_parent) {

}

const std::string& Node::Port::name() const {
	return m_name;
}

const Attr::Category Node::Port::category() const {
	return m_parent->m_meta->attr(m_id).category();
}

const unsigned Node::Port::index() const {
	return m_id;
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

void Node::Port::setDirty(bool d) {
	m_dirty = d;
}

void Node::Port::connect(Port& p) {
	p.m_parent->m_parent->connections().add(*this, p);
}

void Node::Port::disconnect(Port& p) {
	p.m_parent->m_parent->connections().remove(*this, p);
}

/////////////

Node::Node(const std::string& name, const Metadata* def, Graph* parent) : m_name(name), m_parent(parent), m_meta(def), m_data(*m_meta) {
	for(std::size_t a = 0; a < m_meta->attributeCount(); ++a) {
		auto& meta = m_meta->attr(a);
		assert(meta.offset() == a);

		m_ports.push_back(std::move(Port(meta.name(), meta.offset(), this)));
	}
}

const std::string& Node::name() const {
	return m_name;
}

const Metadata& Node::metadata() const {
	return *m_meta;
}

Node::Port& Node::port(size_t index) {
	assert(index < m_ports.size());
	return m_ports[index];
}

const Node::Port& Node::port(size_t index) const {
	assert(index < m_ports.size());
	return m_ports[index];
}

const size_t Node::portCount() const {
	return m_ports.size();
}

void Node::markAsDirty(size_t index) {
	Node::Port& p = port(index);

	// mark the port itself as dirty
	p.setDirty(true);

	// recurse + handle each port type slightly differently
	if(p.category() == Attr::kInput) {
		// all outputs influenced by this input are marked dirty
		for(const Attr& i : m_meta->influences(p.m_id))
			markAsDirty(i.offset());
	}
	else {
		// all inputs connected to this output are marked dirty
		for(Port& o : m_parent->connections().connectedTo(port(index)))
			o.node().markAsDirty(o.m_id);
	}
}

bool Node::inputIsNotConnected(const Port& p) const {
	assert(p.category() == Attr::kInput);

	// return true if there are no connections leading to this input port
	return !m_parent->connections().connectedFrom(p);
}

void Node::computeInput(size_t index) {
	assert(port(index).category() == Attr::kInput && "computeInput can be only called on inputs");
	assert(port(index).isDirty() && "input should be dirty for recomputation");
	assert(not inputIsNotConnected(port(index)) && "input has to be connected to be computed");

	// pull on the single connected output if needed
	boost::optional<Node::Port&> out = m_parent->connections().connectedFrom(port(index));
	assert(out);
	if(out->isDirty())
		out->node().computeOutput(out->m_id);
	assert(not out->isDirty());

	// assign the value directly
	m_data.data(index).assign(out->node().m_data.data(out->m_id));
	assert(m_data.data(index).isEqual(out->node().m_data.data(out->m_id)));

	// and mark as not dirty
	port(index).setDirty(false);
	assert(not port(index).isDirty());
}

void Node::computeOutput(size_t index) {
	assert(port(index).category() == Attr::kOutput && "computeOutput can be only called on outputs");
	assert(port(index).isDirty() && "output should be dirty for recomputation");

	// first, figure out which inputs need pulling, if any
	std::vector<std::reference_wrapper<const Attr>> inputs = m_meta->influencedBy(index);

	// pull on all inputs
	for(const Attr& i : inputs)
		if(port(i.offset()).isDirty())
			computeInput(i.offset());

	// now run compute, as all inputs are fine
	//  -> this will change the output value (if the compute method works)
	m_meta->m_compute(m_data);

	// and mark as not dirty
	port(index).setDirty(false);
	assert(not port(index).isDirty());
}

}
