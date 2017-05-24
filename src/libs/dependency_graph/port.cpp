#include "port.inl"

#include "graph.h"

namespace dependency_graph {

Port::Port(const std::string& name, unsigned id, Node* parent) :
	m_name(name), m_id(id), m_parent(parent) {

	m_dirty = category() == Attr::kOutput;

}

Port::Port(Port&& p) : m_name(std::move(p.m_name)), m_id(p.m_id), m_dirty(p.m_dirty), m_parent(p.m_parent) {

}

const std::string& Port::name() const {
	return m_name;
}

const Attr::Category Port::category() const {
	return m_parent->m_meta->attr(m_id).category();
}

const unsigned Port::index() const {
	return m_id;
}

const std::string Port::type() const {
	return m_parent->m_meta->attr(m_id).type().name();
}

bool Port::isDirty() const {
	return m_dirty;
}

Node& Port::node() {
	assert(m_parent != NULL);
	return *m_parent;
}

const Node& Port::node() const {
	assert(m_parent != NULL);
	return *m_parent;
}

void Port::setDirty(bool d) {
	// change in dirtiness flag
	if(m_dirty != d) {
		m_dirty = d;

		// call all flags change callbacks (intended to update UIs accordingly)
		m_flagsCallbacks();
	}
}

void Port::connect(Port& p) {
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
	p.node().markAsDirty(p.index());
	// connect / disconnect - might change UI's appearance
	m_flagsCallbacks();
	p.m_flagsCallbacks();
}

void Port::disconnect(Port& p) {
	p.m_parent->m_parent->connections().remove(*this, p);
	// connect / disconnect - might change UI's appearance
	m_flagsCallbacks();
	p.m_flagsCallbacks();
}

boost::signals2::connection Port::valueCallback(const std::function<void()>& fn) {
	return m_flagsCallbacks.connect(fn);
}

boost::signals2::connection Port::flagsCallback(const std::function<void()>& fn) {
	return m_flagsCallbacks.connect(fn);
}

}
