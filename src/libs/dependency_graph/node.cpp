#include "node.h"

#include <cassert>

#include "graph.h"
#include "values.h"

namespace dependency_graph {

Node::Node(const std::string& name, const Metadata* def, Graph* parent) : NodeBase(name, parent), m_meta(def), m_data(*m_meta) {
	for(std::size_t a = 0; a < m_meta->attributeCount(); ++a) {
		auto& meta = m_meta->attr(a);
		assert(meta.offset() == a);

		m_ports.push_back(Port(meta.name(), meta.offset(), this));
	}
}

const Metadata& Node::metadata() const {
	return *m_meta;
}

const Datablock& Node::datablock() const {
	return m_data;
}

Port& Node::port(size_t index) {
	assert(index < m_ports.size());
	return m_ports[index];
}

const Port& Node::port(size_t index) const {
	assert(index < m_ports.size());
	return m_ports[index];
}

const size_t Node::portCount() const {
	return m_ports.size();
}

void Node::markAsDirty(size_t index) {
	Port& p = port(index);

	// mark the port itself as dirty
	if(!p.isDirty()) {
		p.setDirty(true);

		graph().dirtyChanged();

		// recurse + handle each port type slightly differently
		if(p.category() == Attr::kInput) {
			// all outputs influenced by this input are marked dirty
			for(const Attr& i : m_meta->influences(p.m_id))
				markAsDirty(i.offset());
		}
		else {
			// all inputs connected to this output are marked dirty
			for(Port& o : graph().network().connections().connectedTo(port(index)))
				o.node().markAsDirty(o.m_id);
		}
	}
}

void Node::computeInput(size_t index) {
	assert(port(index).category() == Attr::kInput && "computeInput can be only called on inputs");
	assert(port(index).isDirty() && "input should be dirty for recomputation");
	assert(port(index).isConnected() && "input has to be connected to be computed");

	// pull on the single connected output if needed
	boost::optional<Port&> out = graph().network().connections().connectedFrom(port(index));
	assert(out);
	if(out->isDirty())
		out->node().computeOutput(out->m_id);
	assert(not out->isDirty());

	// assign the value directly
	m_data.data(index).assign(out->node().m_data.data(out->m_id));
	assert(m_data.data(index).isEqual(out->node().m_data.data(out->m_id)));

	// run the watcher callbacks
	m_ports[index].m_valueCallbacks();

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
	for(const Attr& i : inputs) {
		if(port(i.offset()).isDirty()) {
			if(port(i.offset()).isConnected())
				computeInput(i.offset());
			else
				port(i.offset()).setDirty(false);
		}

		assert(!port(i.offset()).isDirty());
	}

	// now run compute, as all inputs are fine
	//  -> this will change the output value (if the compute method works)
	State result;
	try {
		Values vals(*this);
		result = m_meta->m_compute(vals);
	}
	catch(std::exception& e) {
		result.addError(e.what());
	}

	// mark as not dirty
	port(index).setDirty(false);
	assert(not port(index).isDirty());

	// errored - reset the output to default value
	if(result.errored())
		m_data.reset(index);

	// and run the watcher callbacks
	m_ports[index].m_valueCallbacks();

	// if the state changed, run state changed callback
	if(result != m_state) {
		m_state = result;

		graph().stateChanged(*this);
	}
}

const State& Node::state() const {
	return m_state;
}

}
