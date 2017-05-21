#include "node.h"

#include <cassert>

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

const std::string Node::Port::type() const {
	return m_parent->m_meta->attr(m_id).type().name();
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
	// change in dirtiness flag
	if(m_dirty != d) {
		m_dirty = d;

		// call all flags change callbacks (intended to update UIs accordingly)
		m_flagsCallbacks();
	}
}

void Node::Port::connect(Port& p) {
	// test the recursivity
	{
		std::set<Port*> usedPorts;
		usedPorts.insert(&p);
		usedPorts.insert(this);

		std::set<Port*> addedPorts;
		addedPorts.insert(&p);
		addedPorts.insert(this);

		while(!addedPorts.empty()) {
			// collect all connected ports
			std::set<Port*> newAddedPorts;
			for(auto& current : addedPorts) {
				if(current->category() == Attr::kInput)
					for(const Attr& i : current->node().metadata().influences(current->m_id))
						newAddedPorts.insert(&current->node().port(i.offset()));
				else {
					for(Port& i : current->node().graph().connections().connectedTo(*current))
						newAddedPorts.insert(&i);
				}
			}

			// and insert all, or throw an exception if recursion is detected
			for(auto a : newAddedPorts) {
				const bool insertedNew = usedPorts.insert(a).second;
				if(!insertedNew) {
					std::stringstream msg;
					msg << "a connection between " << node().name() << "/" << name() << " and " << p.node().name() << "/" << p.name() << " would cause a cyclical dependency";

					throw(std::runtime_error(msg.str()));
				}
			}
			addedPorts = newAddedPorts;
		}
	}

	// add the connection
 	p.m_parent->m_parent->connections().add(*this, p);
	// and mark the "connected to" as dirty - will most likely need recomputation
	// TODO: compare values, before marking it dirty wholesale?
	node().markAsDirty(p.index());
	// connect / disconnect - might change UI's appearance
	m_flagsCallbacks();
	p.m_flagsCallbacks();
}

void Node::Port::disconnect(Port& p) {
	p.m_parent->m_parent->connections().remove(*this, p);
	// connect / disconnect - might change UI's appearance
	m_flagsCallbacks();
	p.m_flagsCallbacks();
}

boost::signals2::connection Node::Port::valueCallback(const std::function<void()>& fn) {
	return m_flagsCallbacks.connect(fn);
}

boost::signals2::connection Node::Port::flagsCallback(const std::function<void()>& fn) {
	return m_flagsCallbacks.connect(fn);
}


/////////////

Node::Node(const std::string& name, const Metadata* def, Graph* parent) : m_name(name), m_parent(parent), m_meta(def), m_data(*m_meta) {
	for(std::size_t a = 0; a < m_meta->attributeCount(); ++a) {
		auto& meta = m_meta->attr(a);
		assert(meta.offset() == a);

		m_ports.push_back(Port(meta.name(), meta.offset(), this));
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
	if(!p.isDirty()) {
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
}

bool Node::inputIsConnected(const Port& p) const {
	assert(p.category() == Attr::kInput);

	// return true if there are no connections leading to this input port
	return m_parent->connections().connectedFrom(p);
}

void Node::computeInput(size_t index) {
	assert(port(index).category() == Attr::kInput && "computeInput can be only called on inputs");
	assert(port(index).isDirty() && "input should be dirty for recomputation");
	assert(inputIsConnected(port(index)) && "input has to be connected to be computed");

	// pull on the single connected output if needed
	boost::optional<Node::Port&> out = m_parent->connections().connectedFrom(port(index));
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
			if(inputIsConnected(port(i.offset())))
				computeInput(i.offset());
			else
				port(i.offset()).setDirty(false);
		}

		assert(!port(i.offset()).isDirty());
	}

	// now run compute, as all inputs are fine
	//  -> this will change the output value (if the compute method works)
	m_meta->m_compute(m_data);

	// mark as not dirty
	port(index).setDirty(false);
	assert(not port(index).isDirty());

	// and run the watcher callbacks
	m_ports[index].m_valueCallbacks();
}

const Graph& Node::graph() const {
	assert(m_parent);
	return *m_parent;
}

Graph& Node::graph() {
	assert(m_parent);
	return *m_parent;
}

}
