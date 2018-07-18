#pragma once

#include <dependency_graph/node_base.h>

namespace possumwood { namespace actions { namespace detail {

dependency_graph::NodeBase& findNode(const dependency_graph::UniqueId& id);

} } }
