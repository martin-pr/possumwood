#pragma once

#include <boost/optional.hpp>

#include <dependency_graph/network.h>
#include <dependency_graph/selection.h>

#include "../undo_stack.h"
#include "../node_data.h"

namespace possumwood { namespace actions { namespace detail {

possumwood::UndoStack::Action createNodeAction(dependency_graph::Network& current, const dependency_graph::MetadataHandle& meta, const std::string& name,
	const dependency_graph::Data& _data, const dependency_graph::UniqueId& id,
	boost::optional<const dependency_graph::Datablock> data = boost::optional<const dependency_graph::Datablock>());

possumwood::UndoStack::Action createNodeAction(const dependency_graph::UniqueId& currentNetworkId, const dependency_graph::MetadataHandle& meta, const std::string& name,
	const dependency_graph::Data& _data, const dependency_graph::UniqueId& id,
	boost::optional<const dependency_graph::Datablock> data = boost::optional<const dependency_graph::Datablock>());

possumwood::UndoStack::Action removeAction(const dependency_graph::Selection& _selection);
possumwood::UndoStack::Action removeNodeAction(dependency_graph::NodeBase& node);
possumwood::UndoStack::Action removeNetworkAction(dependency_graph::Network& net);

possumwood::UndoStack::Action renameNodeAction(const dependency_graph::UniqueId& nodeId, const std::string& name);

} } }
