#pragma once

#include <dependency_graph/network.h>

#include <boost/optional.hpp>

#include "../node_data.h"
#include "../undo_stack.h"

namespace possumwood {
namespace actions {
namespace detail {

possumwood::UndoStack::Action changeMetadataAction(dependency_graph::NodeBase& node,
                                                   const dependency_graph::MetadataHandle& handle);

void buildNetwork(dependency_graph::Network& net);

}  // namespace detail
}  // namespace actions
}  // namespace possumwood
