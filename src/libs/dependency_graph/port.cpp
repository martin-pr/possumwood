#include "port.inl"

#include "graph.h"
#include "io.h"
#include "rtti.h"

namespace dependency_graph {

Port::Port(unsigned id, NodeBase* parent) : m_id(id),
	m_dirty(parent->metadata().attr(id).category() == Attr::kOutput), m_parent(parent) {
}

Port::Port(Port&& p) : m_id(p.m_id), m_dirty(p.m_dirty), m_parent(p.m_parent) {
}

const std::string& Port::name() const {
	return m_parent->metadata().attr(m_id).name();
}

const Attr::Category Port::category() const {
	return m_parent->metadata().attr(m_id).category();
}

const unsigned Port::index() const {
	return m_id;
}

const std::string Port::type() const {
	std::string t = unmangledName(m_parent->metadata().attr(m_id).type().name());

	// void port "type" can be determined by any connected other ports
	if(t == unmangledTypeId<void>()) {
		if(category() == Attr::kInput) {
			auto out = node().network().connections().connectedFrom(*this);
			if(out)
				t = out->type();
		}
		else if(category() == Attr::kOutput) {
			auto in = node().network().connections().connectedTo(*this);
			if(!in.empty())
				t = in[0].get().type();
		}
	}

	return t;
}

bool Port::isDirty() const {
	return m_dirty;
}

NodeBase& Port::node() {
	assert(m_parent != NULL);
	return *m_parent;
}

const NodeBase& Port::node() const {
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
	if(p.node().network().connections().connectedFrom(p)) {
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
					for(std::size_t i : current->node().metadata().influences(current->m_id))
						newAddedPorts.insert(&current->node().port(i));
				else {
					for(Port& i : current->node().network().connections().connectedTo(*current))
						newAddedPorts.insert(&i);
				}
			}

			if(newAddedPorts.find(this) != newAddedPorts.end()) {
				std::stringstream msg;
				msg << "A connection between " << node().name() << "/" << name() << " and " << p.node().name() << "/" << p.name() <<
				    " would cause a cyclical dependency";

				throw(std::runtime_error(msg.str()));
			}

			addedPorts = newAddedPorts;
		}
	}

	// test for datatype
	{
		const unsigned voidCount = (type() == unmangledTypeId<void>()) + (p.type() == unmangledTypeId<void>());
		if((type() != p.type() && (voidCount != 1)) || voidCount == 2) {
			std::stringstream msg;
			msg << "A connection between " << node().name() << "/" << name() << " and " << p.node().name() << "/" << p.name() <<
			    " does not connect the same datatype";

			throw(std::runtime_error(msg.str()));
		}
	}


	// add the connection
	p.m_parent->network().connections().add(*this, p);
	assert(p.type() == type() && "at this stage, the types should match");

	// datablock initialisation handling for untyped ports
	{
		// at least one side of the connection has to be not null
		assert(not node().datablock().isNull(index()) || not p.node().datablock().isNull(p.index()));

		// if the "input" is void acc to the metadata, we need to initialise its datablock
		if(p.node().metadata().attr(p.index()).type() == typeid(void))
			p.node().datablock().set(p.index(), *this);
		assert(not p.node().datablock().isNull(p.index()));

		// or, if the "output" is void, we need to initialise its datablock
		if(node().metadata().attr(index()).type() == typeid(void))
			node().datablock().set(index(), p);
		assert(not node().datablock().isNull(index()));
	}

	// and mark the "connected to" as dirty - will most likely need recomputation
	// TODO: compare values, before marking it dirty wholesale?

	// If the state of this node is error, try to re-evaluate the errored output.
	// Adding a connection might have changed something, and the error might
	// go away.

	// This would be making a lot of assumptions about how the node is implemented.
	// Not good. Lets just dirty the whole thing, and force reevaluation independently
	// of the current state.
	node().markAsDirty(index());
	p.node().markAsDirty(p.index());

	// connect / disconnect - might change UI's appearance
	m_flagsCallbacks();
	p.m_flagsCallbacks();
}

void Port::disconnect(Port& p) {
	// remove the connection
	p.m_parent->network().connections().remove(*this, p);

	// disconnecting a non-saveable or untyped input port should reset the data, otherwise
	//   if the scene is saved, this data will not be in the file
	if(p.type() == unmangledTypeId<void>() || !io::isSaveable(p.m_parent->datablock().data(p.m_id))) {
		// set to default (i.e., reset)
		p.m_parent->datablock().reset(p.m_id);

		// explicitly setting a value makes it not dirty, but makes everything that
		//   depends on it dirty
		p.m_parent->markAsDirty(p.m_id);
		setDirty(false);
	}

	// disconnecting an untyped output port should reset the data, to keep the behaviour consistent
	if(type() == unmangledTypeId<void>())
		// set to default (i.e., reset)
		m_parent->datablock().reset(m_id);

	// connect / disconnect - might change UI's appearance
	m_flagsCallbacks();
	p.m_flagsCallbacks();
}

bool Port::isConnected() const {
	if(category() == Attr::kInput)
		// return true if there are no connections leading to this input port
		return static_cast<bool>(m_parent->network().connections().connectedFrom(*this));
	else
		return not m_parent->network().connections().connectedTo(*this).empty();
}

boost::signals2::connection Port::valueCallback(const std::function<void()>& fn) {
	return m_valueCallbacks.connect(fn);
}

boost::signals2::connection Port::flagsCallback(const std::function<void()>& fn) {
	return m_flagsCallbacks.connect(fn);
}

}
