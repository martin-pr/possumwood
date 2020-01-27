#include "node_base.h"

#include "graph.h"
#include "values.h"

namespace dependency_graph {

NodeBase::NodeBase(const std::string& name, const UniqueId& id, const MetadataHandle& metadata, Network* parent) : m_name(name), m_network(parent), m_index(id), m_metadata(metadata), m_data(metadata) {
	for(std::size_t a = 0; a < metadata.metadata().attributeCount(); ++a) {
		auto& meta = metadata.metadata().attr(a);
		assert(meta.offset() == a);

		m_ports.push_back(Port(meta.offset(), this));
	}
}

NodeBase::~NodeBase() {
	for(auto& p : m_ports) {
		if(p.isLinked())
			p.unlink();
		assert(!p.isConnected());
	}
}

void NodeBase::disconnectAll() {
	if(hasParentNetwork()) {
		auto it = network().connections().begin();
		while(it != network().connections().end())
			if(&it->first.node() == this || &it->second.node() == this) {
				it->first.disconnect(it->second);
				it = network().connections().begin(); // inefficient!
			}
			else
				++it;
	}
}

const std::string& NodeBase::name() const {
	return m_name;
}

void NodeBase::setName(const std::string& name) {
	m_name = name;
	network().graph().nameChanged(*this);
}

const Network& NodeBase::network() const {
	assert(m_network != nullptr);
	return *m_network;
}

Network& NodeBase::network() {
	assert(m_network != nullptr);
	return *m_network;
}

bool NodeBase::hasParentNetwork() const {
	return m_network != nullptr;
}

const Graph& NodeBase::graph() const {
	if(hasParentNetwork())
		return network().graph();

	// TERRIBLE hacking, to be replaced with something more sensible at some point
	const Graph& g = dynamic_cast<const Graph&>(*this);
	return g;
}

Graph& NodeBase::graph() {
	if(hasParentNetwork())
		return network().graph();

	// TERRIBLE hacking, to be replaced with something more sensible at some point
	Graph& g = dynamic_cast<Graph&>(*this);
	return g;
}

UniqueId NodeBase::index() const {
	return m_index;
}

void NodeBase::markAsDirty(size_t portIndex, bool dependantsOnly) {
	Port& p = port(portIndex);

	// mark the port itself as dirty
	if(!p.isDirty()) {
		if(!dependantsOnly) {
			p.setDirty(true);

			graph().dirtyChanged();
		}

		// recurse + handle each port type slightly differently
		if(p.category() == Attr::kInput) {
			// all outputs influenced by this input are marked dirty
			for(std::size_t i : metadata()->influences(p.index()))
				markAsDirty(i);
		}
		else {
			// all inputs connected to this output are marked dirty
			for(Port& o : network().connections().connectedTo(port(portIndex)))
				o.node().markAsDirty(o.index());
		}

		// propagate to linked ports
		if(p.isLinked())
			p.linkedTo().node().markAsDirty(p.linkedTo().m_id, dependantsOnly);
	}
}

const MetadataHandle& NodeBase::metadata() const {
	return m_metadata;
}

void NodeBase::setMetadata(const MetadataHandle& handle) {
	for(const Port& p : m_ports)
		if(p.isConnected())
			throw std::runtime_error("Can only change metadata for nodes without connections.");

	// set the new metadata
	m_metadata = handle;

	// create new datablock - this will initialise all values to default.
	// Setting the actual values should be done in Actions.
	m_data = Datablock(handle);

	// redo the ports based on the new metadata
	m_ports.clear();
	for(std::size_t a = 0; a < handle->attributeCount(); ++a) {
		auto& meta = handle->attr(a);
		m_ports.push_back(Port(meta.offset(), this));
	}

	// fire the callback
	graph().metadataChanged(*this);

	// mark everything as dirty
	for(std::size_t p = 0; p < m_ports.size(); ++p)
		markAsDirty(p);
}

const Datablock& NodeBase::datablock() const {
	return m_data;
}

Datablock& NodeBase::datablock() {
	return m_data;
}

void NodeBase::setDatablock(const Datablock& data) {
	assert(data.meta() == metadata());
	m_data = data;
}

Port& NodeBase::port(size_t index) {
	assert(index < m_ports.size());
	return m_ports[index];
}

const Port& NodeBase::port(size_t index) const {
	assert(index < m_ports.size());
	return m_ports[index];
}

size_t NodeBase::portCount() const {
	return m_ports.size();
}

const Data& NodeBase::get(size_t index) const {
	return datablock().data(index);
}

void NodeBase::set(size_t index, const Data& value) {
	assert(port(index).category() == Attr::kOutput || !port(index).isConnected());
	return datablock().setData(index, value);
}

void NodeBase::computeInput(size_t index) {
	assert(port(index).category() == Attr::kInput && "computeInput can be only called on inputs");
	assert(port(index).isDirty() && "input should be dirty for recomputation");
	assert(port(index).isConnected() && "input has to be connected to be computed");

	// pull on the single connected output if needed
	boost::optional<Port&> out = network().connections().connectedFrom(port(index));
	assert(out);
	if(out->isDirty()) {
		out->getData(); // throw away (bad)
	}
	assert(not out->isDirty());

	// assign the value directly
	const NodeBase& srcNode = out->node();
	const Datablock& srcData = srcNode.datablock();
	datablock().setData(index, srcData.data(out->index()));
	assert(datablock().data(index) == srcData.data(out->index()));

	// and mark as not dirty
	port(index).setDirty(false);
	assert(not port(index).isDirty());

	// run the watcher callbacks
	port(index).m_valueCallbacks();
}

void NodeBase::computeOutput(size_t index) {
	assert(port(index).category() == Attr::kOutput && "computeOutput can be only called on outputs");
	assert(port(index).isDirty() && "output should be dirty for recomputation");

	// first, figure out which inputs need pulling, if any
	std::vector<std::size_t> inputs = metadata()->influencedBy(index);

	// main computation
	State result;
	try {
		// pull on all inputs - triggers their recomputation
		for(std::size_t& i : inputs) {
			if(port(i).isDirty())
				port(i).getData(); // throw away (bad! should eventually start using shader_ptr to hold data to avoid wasting resources)

			assert(!port(i).isDirty());
		}

		// now run compute, as all inputs are fine
		//  -> this will change the output value (if the compute method works)
		Values vals(*this);
		result = metadata()->m_compute(vals);
	}
	catch(std::exception& e) {
		result.addError(e.what());
	}

	// mark as not dirty
	port(index).setDirty(false);
	assert(not port(index).isDirty());

	// errored - reset the output to default value
	std::string error_to_throw;
	if(result.errored()) {
		// non-void - default value comes from metadata
		if(metadata()->attr(index).type() != typeid(void))
			datablock().reset(index);
		// void - default comes from the default of the connected port
		else {
			auto conn = network().connections().connectedTo(port(index));
			if(conn.empty()) {
				std::stringstream err;
				err << "Error evaluating " << name() << "/" << port(index).name() << " - untyped port without a connection cannot be evaluated." << std::endl;

				// throw an exception later, after all other state handling is finished
				error_to_throw = err.str();

				// WARNING - as no default value can be set at this stage, the ORIGINAL value
				// on the port is kept. As this exception should not be thrown during normal
				// runtime (it is used in tests, and can be triggered with bad graph handling),
				// this should not pose a problem. Famous last words.
			}

			// initialise using default of the FIRST connected port (arbitrary choice, but whatever)
			else
				datablock().set(index, conn.begin()->get());
		}
	}

	// and run the watcher callbacks
	port(index).m_valueCallbacks();

	// if the state changed, run state changed callback
	if(result != m_state) {
		m_state = result;

		network().graph().stateChanged(*this);
	}

	// throw an exception if errored and no reset could be done
	if(!error_to_throw.empty())
		throw std::runtime_error(error_to_throw);
}

const State& NodeBase::state() const {
	return m_state;
}

void NodeBase::setBlindData(const Data& data) {
	m_blindData = data;
}

bool NodeBase::hasBlindData() const {
	return !m_blindData.empty();
}

std::string NodeBase::blindDataType() const {
	assert(hasBlindData());
	return m_blindData.type();
}

const Data& NodeBase::blindData() const {
	assert(hasBlindData());
	return m_blindData;
}

}
