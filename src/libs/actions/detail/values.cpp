#include "values.h"

#include "../app.h"

namespace possumwood { namespace actions { namespace detail {

namespace {

void doSetValue(const dependency_graph::UniqueId& id, unsigned portId, std::shared_ptr<const dependency_graph::BaseData> value, std::shared_ptr<std::unique_ptr<dependency_graph::BaseData>> original) {
	auto it = AppCore::instance().graph().nodes().find(id, dependency_graph::Nodes::kRecursive);
	assert(it != AppCore::instance().graph().nodes().end());

	assert(original != nullptr);
	if(*original == nullptr)
		*original = it->port(portId).getData().clone();

	assert(*original != nullptr);

	it->port(portId).setData(*value);
}

void doResetValue(const dependency_graph::UniqueId& id, unsigned portId, std::shared_ptr<std::unique_ptr<dependency_graph::BaseData>> value) {
	auto it = AppCore::instance().graph().nodes().find(id, dependency_graph::Nodes::kRecursive);
	assert(it != AppCore::instance().graph().nodes().end());

	assert(value != nullptr);
	assert(*value != nullptr);

	it->port(portId).setData(**value);
}

}

possumwood::UndoStack::Action setValueAction(dependency_graph::Port& port, const dependency_graph::BaseData& value) {
	return setValueAction(port.node().index(), port.index(), value);
}

possumwood::UndoStack::Action setValueAction(const dependency_graph::UniqueId& nodeId, std::size_t portId, const dependency_graph::BaseData& value) {
	UndoStack::Action action;

	// dependency_graph::NodeBase& node = detail::findNode(nodeId);

	std::shared_ptr<std::unique_ptr<dependency_graph::BaseData>> original(new std::unique_ptr<dependency_graph::BaseData>());

	// std::shared_ptr<const dependency_graph::BaseData> original = node.port(portId).getData().clone();
	std::shared_ptr<const dependency_graph::BaseData> target = value.clone();

	action.addCommand(
		std::bind(&doSetValue, nodeId, portId, std::move(target), original),
		std::bind(&doResetValue, nodeId, portId, original)
	);

	return action;
}

} } }
