#pragma once

#include <boost/optional.hpp>

#include <dependency_graph/network.h>

#include "../undo_stack.h"
#include "../node_data.h"

namespace possumwood { namespace actions { namespace detail {

possumwood::UndoStack::Action setValueAction(dependency_graph::Port& port, const dependency_graph::BaseData& value);
possumwood::UndoStack::Action setValueAction(const dependency_graph::UniqueId& nodeId, std::size_t portId, const dependency_graph::BaseData& value);

} } }
