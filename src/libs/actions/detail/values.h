#pragma once

#include <actions/io/json.h>
#include <dependency_graph/network.h>

#include <boost/optional.hpp>

#include "../node_data.h"
#include "../undo_stack.h"

namespace possumwood {
namespace actions {
namespace detail {

possumwood::UndoStack::Action setValueAction(dependency_graph::Port& port, const dependency_graph::Data& value);
possumwood::UndoStack::Action setValueAction(const dependency_graph::UniqueId& nodeId, std::size_t portId,
                                             const dependency_graph::Data& value);

possumwood::UndoStack::Action setValueAction(const dependency_graph::UniqueId& nodeId, const std::string& portName,
                                             const possumwood::io::json& value);

}  // namespace detail
}  // namespace actions
}  // namespace possumwood
