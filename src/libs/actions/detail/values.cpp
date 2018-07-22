#include "values.h"

#include "../app.h"

namespace possumwood { namespace actions { namespace detail {

namespace {

void doSetValueFromJson(const dependency_graph::UniqueId& nodeId, const std::string& portName, const possumwood::io::json& value, std::shared_ptr<std::unique_ptr<dependency_graph::BaseData>> original) {
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
		if(*original == nullptr)
			*original = nodeIt->port(portId).getData().clone();
		assert(*original != nullptr);

		// parse the new value
		assert(dependency_graph::io::isSaveable(**original));

		std::unique_ptr<dependency_graph::BaseData> data((*original)->clone());
		io::fromJson(value, *data);

		// and set the new data
		nodeIt->port(portId).setData(*data);
	}
	else
		std::cerr << "Found unused property '" << portName << "' of node type '" << nodeIt->metadata()->type() << "' while loading a file. Ignoring its value." << std::endl;
}

void doSetValue(const dependency_graph::UniqueId& nodeId, unsigned portId, std::shared_ptr<const dependency_graph::BaseData> value, std::shared_ptr<std::unique_ptr<dependency_graph::BaseData>> original) {
	auto it = AppCore::instance().graph().nodes().find(nodeId, dependency_graph::Nodes::kRecursive);
	assert(it != AppCore::instance().graph().nodes().end());

	assert(original != nullptr);
	if(*original == nullptr)
		*original = it->port(portId).getData().clone();
	assert(*original != nullptr);

	it->port(portId).setData(*value);
}

void doResetValue(const dependency_graph::UniqueId& nodeId, unsigned portId, std::shared_ptr<std::unique_ptr<dependency_graph::BaseData>> value) {
	assert(value != nullptr);
	if(*value != nullptr) {
		auto it = AppCore::instance().graph().nodes().find(nodeId, dependency_graph::Nodes::kRecursive);
		assert(it != AppCore::instance().graph().nodes().end());

		it->port(portId).setData(**value);
	}
}

void doResetValueFromJson(const dependency_graph::UniqueId& nodeId, const std::string& portName, std::shared_ptr<std::unique_ptr<dependency_graph::BaseData>> value) {
	assert(value != nullptr);
	if(*value != nullptr) {
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

		nodeIt->port(portId).setData(**value);
	}
}

}

possumwood::UndoStack::Action setValueAction(dependency_graph::Port& port, const dependency_graph::BaseData& value) {
	return setValueAction(port.node().index(), port.index(), value);
}

possumwood::UndoStack::Action setValueAction(const dependency_graph::UniqueId& nodeId, std::size_t portId, const dependency_graph::BaseData& value) {
	UndoStack::Action action;

	std::shared_ptr<std::unique_ptr<dependency_graph::BaseData>> original(new std::unique_ptr<dependency_graph::BaseData>());

	// std::shared_ptr<const dependency_graph::BaseData> original = node.port(portId).getData().clone();
	std::shared_ptr<const dependency_graph::BaseData> target = value.clone();

	action.addCommand(
		std::bind(&doSetValue, nodeId, portId, std::move(target), original),
		std::bind(&doResetValue, nodeId, portId, original)
	);

	return action;
}

possumwood::UndoStack::Action setValueAction(const dependency_graph::UniqueId& nodeId, const std::string& portName, const possumwood::io::json& value) {
	UndoStack::Action action;

	std::shared_ptr<std::unique_ptr<dependency_graph::BaseData>> original(new std::unique_ptr<dependency_graph::BaseData>());

	action.addCommand(
		std::bind(&doSetValueFromJson, nodeId, portName, value, original),
		std::bind(&doResetValueFromJson, nodeId, portName, original)
	);

	return action;
}

} } }
