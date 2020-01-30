#include "values.h"

#include <dependency_graph/nodes.inl>

#include "../app.h"

namespace possumwood { namespace actions { namespace detail {

namespace {

void doSetValueFromJson(const dependency_graph::UniqueId& nodeId, const std::string& portName, const possumwood::io::json& value, std::shared_ptr<dependency_graph::Data> original) {
	// get the node
	auto nodeIt = AppCore::instance().graph().nodes().find(nodeId, dependency_graph::Nodes::kRecursive);
	assert(nodeIt != AppCore::instance().graph().nodes().end());

	// get the port
	int portId = -1;
	for(std::size_t pi=0; pi<nodeIt->portCount(); ++pi)
		if(nodeIt->port(pi).name() == portName) {
			portId = pi;
			break;
		}

	if(portId >= 0) {
		// store the original value
		assert(original != nullptr);
		if(original->empty())
			*original = nodeIt->port(portId).getData();
		assert(!original->empty());

		// parse the new value
		assert(dependency_graph::io::isSaveable(*original));

		dependency_graph::Data data(*original);
		io::fromJson(value, data);

		// and set the new data
		nodeIt->port(portId).setData(data);
	}
	else
		std::cerr << "Found unused property '" << portName << "' of node type '" << nodeIt->metadata()->type() << "' while loading a file. Ignoring its value." << std::endl;
}

void doSetValue(const dependency_graph::UniqueId& nodeId, unsigned portId, std::shared_ptr<const dependency_graph::Data> value, std::shared_ptr<dependency_graph::Data> original) {
	auto it = AppCore::instance().graph().nodes().find(nodeId, dependency_graph::Nodes::kRecursive);
	assert(it != AppCore::instance().graph().nodes().end());

	if(it->port(portId).category() == dependency_graph::Attr::kOutput || !it->port(portId).isConnected()) {
		assert(original != nullptr);
		if(original->empty())
			*original = it->port(portId).getData();
		assert(!original->empty());

		it->port(portId).setData(*value);
	}
}

void doResetValue(const dependency_graph::UniqueId& nodeId, unsigned portId, std::shared_ptr<dependency_graph::Data> value) {
	assert(value != nullptr);
	if(!value->empty()) {
		auto it = AppCore::instance().graph().nodes().find(nodeId, dependency_graph::Nodes::kRecursive);
		assert(it != AppCore::instance().graph().nodes().end());

		if(it->port(portId).category() == dependency_graph::Attr::kOutput || !it->port(portId).isConnected())
			it->port(portId).setData(*value);
	}
}

void doResetValueFromJson(const dependency_graph::UniqueId& nodeId, const std::string& portName, std::shared_ptr<dependency_graph::Data> value) {
	assert(value != nullptr);
	if(!value->empty()) {
		auto nodeIt = AppCore::instance().graph().nodes().find(nodeId, dependency_graph::Nodes::kRecursive);
		assert(nodeIt != AppCore::instance().graph().nodes().end());

		// get the port
		int portId = -1;
		for(std::size_t pi=0; pi<nodeIt->portCount(); ++pi)
			if(nodeIt->port(pi).name() == portName) {
				portId = pi;
				break;
			}

		assert(portId >= 0);

		nodeIt->port(portId).setData(*value);
	}
}

}

possumwood::UndoStack::Action setValueAction(dependency_graph::Port& port, const dependency_graph::Data& value) {
	return setValueAction(port.node().index(), port.index(), value);
}

possumwood::UndoStack::Action setValueAction(const dependency_graph::UniqueId& nodeId, std::size_t portId, const dependency_graph::Data& value) {
	UndoStack::Action action;

	std::shared_ptr<dependency_graph::Data> original(new dependency_graph::Data());
	std::shared_ptr<dependency_graph::Data> target(new dependency_graph::Data(value));

	std::stringstream ss;
	ss << "Setting value of " << nodeId << "/" << portId << " to " << *target;

	action.addCommand(
		ss.str(),
		[nodeId, portId, target, original]() { doSetValue(nodeId, portId, target, original); },
		[nodeId, portId, original]() { doResetValue(nodeId, portId, original); }
	);

	return action;
}

possumwood::UndoStack::Action setValueAction(const dependency_graph::UniqueId& nodeId, const std::string& portName, const possumwood::io::json& value) {
	UndoStack::Action action;

	std::shared_ptr<dependency_graph::Data> original(new dependency_graph::Data());

	std::stringstream ss;
	ss << "Setting value of " << nodeId << "/" << portName << " to " << value << " from JSON";

	action.addCommand(
		ss.str(),
		std::bind(&doSetValueFromJson, nodeId, portName, value, original),
		std::bind(&doResetValueFromJson, nodeId, portName, original)
	);

	return action;
}

} } }
