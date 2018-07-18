#pragma once

#include <boost/optional.hpp>

#include <dependency_graph/network.h>

#include "../undo_stack.h"
#include "../node_data.h"

namespace possumwood { namespace actions { namespace detail {

possumwood::UndoStack::Action changeMetadataAction(dependency_graph::NodeBase& node, const dependency_graph::MetadataHandle& handle);

} } }
