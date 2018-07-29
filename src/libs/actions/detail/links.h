#pragma once

#include <boost/optional.hpp>

#include <dependency_graph/network.h>
#include <dependency_graph/selection.h>

#include "../undo_stack.h"
#include "../node_data.h"

namespace possumwood { namespace actions { namespace detail {

struct Link {
	dependency_graph::UniqueId fromNode;
	std::size_t fromPort;
	dependency_graph::UniqueId toNode;
	std::size_t toPort;
};

possumwood::UndoStack::Action linkAction(const Link& l);
possumwood::UndoStack::Action unlinkAction(const dependency_graph::Port& p);

} } }
