#include "port.inl"

#include "graph.h"
#include "io.h"
#include "rtti.h"

namespace dependency_graph {

Port::Port(unsigned id, NodeBase* parent) : m_parent(parent), m_id(id),
	m_dirty(parent->metadata()->attr(id).category() == Attr::kOutput), m_linkedToPort(nullptr), m_linkedFromPort(nullptr) {
}

Port::Port(Port&& p) : m_parent(p.m_parent), m_id(p.m_id), m_dirty(p.m_dirty), m_linkedToPort(nullptr), m_linkedFromPort(nullptr) {
	if(p.m_linkedFromPort)
		p.m_linkedFromPort->unlink();
	if(p.m_linkedToPort)
		p.unlink();

	assert(p.m_linkedToPort == nullptr);
	assert(p.m_linkedFromPort == nullptr);
	p.m_parent = nullptr;
}

Port::~Port() {
	if(m_parent) {
		// this port should not be connected
		assert(!isConnected());

		if(m_linkedFromPort)
			m_linkedFromPort->unlink();

		if(m_linkedToPort)
			unlink();
	}
}

const std::string& Port::name() const {
	return m_parent->metadata()->attr(m_id).name();
}

std::string Port::fullName() const {
	std::string result = node().name() + "/" + name();

	const NodeBase* ptr = &node();
	while(ptr->hasParentNetwork()) {
		ptr = &(ptr->network());
		result = ptr->name() + "/" + result;
	}

	return result;
}

Attr::Category Port::category() const {
	return m_parent->metadata()->attr(m_id).category();
}

unsigned Port::index() const {
	return m_id;
}

std::type_index Port::type() const {
	std::type_index t = m_parent->metadata()->attr(m_id).type();

	// void port "type" can be determined by any connected other ports
	if(t == typeid(void)) {
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

const Data& Port::getData() {
	// do the computation if needed, to get rid of the dirty flag
	if(m_dirty) {
		if(category() == Attr::kInput) {
			if(isConnected())
				m_parent->computeInput(m_id);
			else if(m_linkedFromPort)
				setData(m_linkedFromPort->getData());
			else
				setDirty(false);
		}
		else if(category() == Attr::kOutput) {
			if(!m_linkedFromPort)
				m_parent->computeOutput(m_id);
			else
				setData(m_linkedFromPort->getData());
		}
	}

	// when the computation is done, the port should not be dirty
	assert(!m_dirty);

	// and return the value
	return m_parent->get(m_id);
}

void Port::setData(const Data& val) {
	// setting a value in the middle of the graph might do
	//   weird things, so lets assert it
	assert(category() == Attr::kOutput || !isConnected());

	// set the value in the data block
	const bool valueWasSet = (m_parent->get(m_id).type() != val.type()) ||
		(m_parent->get(m_id) != val);
	m_parent->set(m_id, val);

	// explicitly setting a value makes it not dirty, but makes everything that
	//   depends on it dirty
	setDirty(false);
	m_parent->markAsDirty(m_id, true);
	assert(!isDirty());

	// call the values callback
	if(valueWasSet)
		m_valueCallbacks();

	// and make linked port dirty, to allow it to pull on next evaluation
	if(isLinked())
		m_linkedToPort->node().markAsDirty(m_linkedToPort->index());
}

void Port::setDirty(bool d) {
	// change in dirtiness flag
	if(m_dirty != d) {
		m_dirty = d;

		// call all flags change callbacks (intended to update UIs accordingly)
		m_flagsCallbacks();

		// if linked, mark the linked network as dirty as well
		if(isLinked() && d)
			m_linkedToPort->node().markAsDirty(m_linkedToPort->index());
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
					for(std::size_t i : current->node().metadata()->influences(current->m_id))
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
		const unsigned voidCount = (type() == typeid(void)) + (p.type() == typeid(void));
		if((type() != p.type() && (voidCount != 1)) || voidCount == 2) {
			std::stringstream msg;
			msg << "A connection between " << node().name() << "/" << name() << " and " << p.node().name() << "/" << p.name() <<
			    " does not connect the same datatype (" << unmangledTypeId(type()) << " vs " << unmangledTypeId(p.type()) << ")";

			throw(std::runtime_error(msg.str()));
		}
	}

	// test that both nodes are inside one network
	assert(p.node().hasParentNetwork() && node().hasParentNetwork());
	if(&p.node().network() != &node().network()) {
			std::stringstream msg;
			msg << "Ports " << node().name() << "/" << name() << " and " << p.node().name() << "/" << p.name() <<
			    " don't belong to nodes within the same network";

			throw(std::runtime_error(msg.str()));
	}

	// add the connection
	try {
		p.m_parent->network().connections().add(*this, p);
	}
	catch(std::runtime_error& err) {
		std::stringstream msg;
		msg << "Ports " << node().name() << "/" << name() << " and " << p.node().name() << "/" << p.name() <<
		    " - " << err.what();

		throw(std::runtime_error(msg.str()));
	}
	assert(p.type() == type() && "at this stage, the types should match");

	// datablock initialisation handling for untyped ports
	{
		// at least one side of the connection has to be not null
		assert(not node().datablock().isNull(index()) || not p.node().datablock().isNull(p.index()));

		// if the "input" is void acc to the metadata, we need to initialise its datablock
		if(p.node().metadata()->attr(p.index()).type() == typeid(void))
			p.node().datablock().set(p.index(), *this);
		assert(not p.node().datablock().isNull(p.index()));

		// or, if the "output" is void, we need to initialise its datablock
		if(node().metadata()->attr(index()).type() == typeid(void))
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
	if(isLinked())
		m_linkedToPort->node().markAsDirty(m_linkedToPort->index());

	p.node().markAsDirty(p.index());
	if(p.isLinked())
		p.m_linkedToPort->node().markAsDirty(p.m_linkedToPort->index());

	// connect / disconnect - might change UI's appearance
	m_flagsCallbacks();
	p.m_flagsCallbacks();
}

void Port::disconnect(Port& p) {
	// remove the connection
	p.m_parent->network().connections().remove(*this, p);

	// disconnecting a non-saveable or untyped input port should reset the data, otherwise
	//   if the scene is saved, this data will not be in the file
	if(p.type() == typeid(void) || !io::isSaveable(p.m_parent->datablock().data(p.m_id))) {
		// set to default (i.e., reset)
		p.m_parent->datablock().reset(p.m_id);

		// explicitly setting a value makes it not dirty, but makes everything that
		//   depends on it dirty
		p.m_parent->markAsDirty(p.m_id, true /* dependants only */ );
	}

	// disconnecting an untyped output port should reset the data, to keep the behaviour consistent
	if(type() == typeid(void))
		// set to default (i.e., reset)
		m_parent->datablock().reset(m_id);

	// this port is going to be dirty after disconnect - might change value and require recomputation
	node().markAsDirty(index());
	if(isLinked())
		m_linkedToPort->node().markAsDirty(m_linkedToPort->index());

	p.node().markAsDirty(p.index());
	if(p.isLinked())
		p.m_linkedToPort->node().markAsDirty(p.m_linkedToPort->index());

	// connect / disconnect - might change UI's appearance
	m_flagsCallbacks();
	p.m_flagsCallbacks();
}

bool Port::isConnected() const {
	// return true if there are no connections leading to/from this input/output port
	return static_cast<bool>(m_parent->network().connections().connectedFrom(*this)) ||
		not m_parent->network().connections().connectedTo(*this).empty();
}

void Port::linkTo(Port& targetPort) {
	assert(m_linkedToPort == nullptr);
	assert(targetPort.m_linkedFromPort == nullptr);

	m_linkedToPort = &targetPort;
	targetPort.m_linkedFromPort = this;

	targetPort.node().markAsDirty(targetPort.index());
}

bool Port::isLinked() const {
	return m_linkedToPort != nullptr;
}

void Port::unlink() {
	assert(m_linkedToPort != nullptr);
	assert(m_linkedToPort->m_linkedFromPort != nullptr);

	m_linkedToPort->node().markAsDirty(m_linkedToPort->index());

	m_linkedToPort->m_linkedFromPort = nullptr;
	m_linkedToPort = nullptr;

	node().markAsDirty(index());
}

const Port& Port::linkedTo() const {
	assert(isLinked());
	return *m_linkedToPort;
}

Port& Port::linkedTo() {
	assert(isLinked());
	return *m_linkedToPort;
}

boost::signals2::connection Port::valueCallback(const std::function<void()>& fn) {
	return m_valueCallbacks.connect(fn);
}

boost::signals2::connection Port::flagsCallback(const std::function<void()>& fn) {
	return m_flagsCallbacks.connect(fn);
}

}
