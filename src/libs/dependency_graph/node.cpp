#include "node.h"

#include <cassert>

#include "graph.h"
#include "values.h"

namespace dependency_graph {

Node::Node(const std::string& name, const MetadataHandle& def, Network* parent) : NodeBase(name, def, parent), m_data(def) {
	for(std::size_t a = 0; a < def.metadata().attributeCount(); ++a) {
		auto& meta = def.metadata().attr(a);
		assert(meta.offset() == a);

		m_ports.push_back(Port(meta.offset(), this));
	}
}

const Datablock& Node::datablock() const {
	return m_data;
}

Datablock& Node::datablock() {
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

void Node::computeInput(size_t index) {
	assert(port(index).category() == Attr::kInput && "computeInput can be only called on inputs");
	assert(port(index).isDirty() && "input should be dirty for recomputation");
	assert(port(index).isConnected() && "input has to be connected to be computed");

	// pull on the single connected output if needed
	boost::optional<Port&> out = network().connections().connectedFrom(port(index));
	assert(out);
	if(out->isDirty())
		out->node().computeOutput(out->index());
	assert(not out->isDirty());

	// assign the value directly
	m_data.data(index).assign(out->node().datablock().data(out->index()));
	assert(m_data.data(index).isEqual(out->node().datablock().data(out->index())));

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
	std::vector<std::size_t> inputs = metadata().influencedBy(index);

	// pull on all inputs
	for(std::size_t& i : inputs) {
		if(port(i).isDirty()) {
			if(port(i).isConnected())
				computeInput(i);
			else
				port(i).setDirty(false);
		}

		assert(!port(i).isDirty());
	}

	// now run compute, as all inputs are fine
	//  -> this will change the output value (if the compute method works)
	State result;
	try {
		Values vals(*this);
		result = metadata().m_compute(vals);
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

		network().graph().stateChanged(*this);
	}
}

const State& Node::state() const {
	return m_state;
}

void Node::setDatablock(const Datablock& data) {
	m_data = data;
}

}
