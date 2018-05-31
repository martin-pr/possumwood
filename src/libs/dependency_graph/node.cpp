#include "node.h"

#include <cassert>

#include "graph.h"
#include "values.h"

namespace dependency_graph {

Node::Node(const std::string& name, const UniqueId& id, const MetadataHandle& def, Network* parent) : NodeBase(name, id, def, parent) {
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
	const NodeBase& srcNode = out->node();
	const Datablock& srcData = srcNode.datablock();
	datablock().setData(index, srcData.data(out->index()));
	assert(datablock().data(index).isEqual(srcData.data(out->index())));

	// run the watcher callbacks
	port(index).m_valueCallbacks();

	// and mark as not dirty
	port(index).setDirty(false);
	assert(not port(index).isDirty());
}

void Node::computeOutput(size_t index) {
	assert(port(index).category() == Attr::kOutput && "computeOutput can be only called on outputs");
	assert(port(index).isDirty() && "output should be dirty for recomputation");

	// first, figure out which inputs need pulling, if any
	std::vector<std::size_t> inputs = metadata()->influencedBy(index);

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

const State& Node::state() const {
	return m_state;
}

}
