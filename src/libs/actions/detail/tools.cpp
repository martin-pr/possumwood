#include "tools.h"

#include <dependency_graph/nodes_iterator.inl>

#include "../app.h"

namespace possumwood {
namespace actions {
namespace detail {

dependency_graph::NodeBase& findNode(const dependency_graph::UniqueId& id) {
	if(id == possumwood::AppCore::instance().graph().index())
		return possumwood::AppCore::instance().graph();

	auto it = possumwood::AppCore::instance().graph().nodes().find(id, dependency_graph::Nodes::kRecursive);
	assert(it != possumwood::AppCore::instance().graph().nodes().end());

	// and get the node reference
	return *it;
}

}  // namespace detail
}  // namespace actions
}  // namespace possumwood
