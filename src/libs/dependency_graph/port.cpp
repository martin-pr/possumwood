#include "port.inl"

#include "graph.h"
#include "io.h"

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
	// test if the input is not connected already
	if(p.node().graph().network().connections().connectedFrom(p)) {
		std::stringstream msg;
		msg << "Port " << node().name() << "/" << name() << " is already connected";

		throw std::runtime_error(msg.str());
	}

	// test the recursivity
	{
		std::set<Port*> addedPorts;
		addedPorts.insert(&p);

		while(!addedPorts.empty()) {
			// collect all connected ports
			std::set<Port*> newAddedPorts;
			for(auto& current : addedPorts) {
				if(current->category() == Attr::kInput)
					for(const Attr& i : current->node().metadata().influences(current->m_id))
						newAddedPorts.insert(&current->node().port(i.offset()));
				else {
					for(Port& i : current->node().graph().network().connections().connectedTo(*current))
						newAddedPorts.insert(&i);
				}
			}

			if(newAddedPorts.find(this) != newAddedPorts.end()) {
				std::stringstream msg;
				msg << "A connection between " << node().name() << "/" << name() << " and " << p.node().name() << "/" << p.name() << " would cause a cyclical dependency";

				throw(std::runtime_error(msg.str()));
			}

			addedPorts = newAddedPorts;
		}
	}

	// test for datatype
	if(type() != p.type()) {
		std::stringstream msg;
		msg << "A connection between " << node().name() << "/" << name() << " and " << p.node().name() << "/" << p.name() << " does not connect the same datatype";

		throw(std::runtime_error(msg.str()));
	}

	// add the connection
 	p.m_parent->graph().network().connections().add(*this, p);
	// and mark the "connected to" as dirty - will most likely need recomputation
	// TODO: compare values, before marking it dirty wholesale?
	p.node().markAsDirty(p.index());
	// connect / disconnect - might change UI's appearance
	m_flagsCallbacks();
	p.m_flagsCallbacks();
}

void Port::disconnect(Port& p) {
	// remove the connection
	p.m_parent->graph().network().connections().remove(*this, p);

	// disconnecting a non-saveable port should reset the data, otherwise
	//   if the scene is saved, this data will not be in the file
	if(!io::isSaveable(p.m_parent->m_data.data(p.m_id))) {
		// set to default (i.e., reset)
		p.m_parent->m_data.reset(p.m_id);

		// explicitly setting a value makes it not dirty, but makes everything that
		//   depends on it dirty
		p.m_parent->markAsDirty(p.m_id);
		setDirty(false);
	}


	// connect / disconnect - might change UI's appearance
	m_flagsCallbacks();
	p.m_flagsCallbacks();
}

bool Port::isConnected() const {
	if(category() == Attr::kInput)
		// return true if there are no connections leading to this input port
		return static_cast<bool>(m_parent->graph().network().connections().connectedFrom(*this));
	else
		return not m_parent->graph().network().connections().connectedTo(*this).empty();
}

boost::signals2::connection Port::valueCallback(const std::function<void()>& fn) {
	return m_valueCallbacks.connect(fn);
}

boost::signals2::connection Port::flagsCallback(const std::function<void()>& fn) {
	return m_flagsCallbacks.connect(fn);
}

}
