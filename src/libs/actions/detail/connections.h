#pragma once

#include <dependency_graph/network.h>

#include <boost/optional.hpp>

#include "../node_data.h"
#include "../undo_stack.h"

namespace possumwood {
namespace actions {
namespace detail {

possumwood::UndoStack::Action connectAction(const dependency_graph::Port& p1, const dependency_graph::Port& p2);
possumwood::UndoStack::Action connectAction(const dependency_graph::UniqueId& fromNodeId, std::size_t fromPort,
                                            const dependency_graph::UniqueId& toNodeId, std::size_t toPort);
possumwood::UndoStack::Action connectAction(const dependency_graph::UniqueId& fromNodeId,
                                            const std::string& fromPortName, const dependency_graph::UniqueId& toNodeId,
                                            const std::string& toPortName);
possumwood::UndoStack::Action disconnectAction(dependency_graph::Port& p1, dependency_graph::Port& p2);
possumwood::UndoStack::Action disconnectAction(const dependency_graph::UniqueId& fromNodeId, std::size_t fromPort,
                                               const dependency_graph::UniqueId& toNodeId, std::size_t toPort);

}  // namespace detail
}  // namespace actions
}  // namespace possumwood
