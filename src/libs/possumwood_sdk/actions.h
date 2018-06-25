#pragma once

#include <dependency_graph/metadata.h>
#include <dependency_graph/selection.h>
#include <possumwood_sdk/node_data.h>
#include <possumwood_sdk/undo_stack.h>

namespace possumwood {

struct Actions {
		static void createNode(dependency_graph::Network& current, const dependency_graph::MetadataHandle& meta,
		                       const std::string& name, const possumwood::NodeData& data,
		                       const dependency_graph::UniqueId& id = dependency_graph::UniqueId());
		static void removeNode(dependency_graph::NodeBase& node);

		static void connect(dependency_graph::Port& p1, dependency_graph::Port& p2);
		static void disconnect(dependency_graph::Port& p1, dependency_graph::Port& p2);

		template<typename T>
		static void setValue(dependency_graph::Port& p, const T& value);
		static void setValue(dependency_graph::Port& p, const dependency_graph::BaseData& value);

		static void changeMetadata(dependency_graph::NodeBase& node, const dependency_graph::MetadataHandle& handle);

		static void cut(const dependency_graph::Selection& selection);
		static void copy(const dependency_graph::Selection& selection);
		static void paste(dependency_graph::Network& current, dependency_graph::Selection& selection);
		static void remove(const dependency_graph::Selection& selection);

		static void move(const std::map<dependency_graph::NodeBase*, QPointF>& nodes);

	private:
		static possumwood::UndoStack::Action removeAction(const dependency_graph::Selection& _selection);
		static possumwood::UndoStack::Action removeNodeAction(dependency_graph::NodeBase& node);
		static possumwood::UndoStack::Action removeNetworkAction(dependency_graph::Network& net);

		static possumwood::UndoStack::Action connectAction(const dependency_graph::Port& p1, const dependency_graph::Port& p2);
		static possumwood::UndoStack::Action connectAction(const dependency_graph::UniqueId& fromNodeId, std::size_t fromPort, const dependency_graph::UniqueId& toNodeId, std::size_t toPort);
		static possumwood::UndoStack::Action disconnectAction(dependency_graph::Port& p1, dependency_graph::Port& p2);
		static possumwood::UndoStack::Action disconnectAction(const dependency_graph::UniqueId& fromNodeId, std::size_t fromPort, const dependency_graph::UniqueId& toNodeId, std::size_t toPort);

		static possumwood::UndoStack::Action changeMetadataAction(dependency_graph::NodeBase& node, const dependency_graph::MetadataHandle& handle);

		static possumwood::UndoStack::Action setValueAction(const dependency_graph::UniqueId& nodeId, std::size_t portId, const dependency_graph::BaseData& value);
		static possumwood::UndoStack::Action setValueAction(dependency_graph::Port& p, const dependency_graph::BaseData& value);
};

template<typename T>
void Actions::setValue(dependency_graph::Port& p, const T& value) {
	const std::unique_ptr<const dependency_graph::BaseData> data(new dependency_graph::Data<T>(value));
	setValue(p, *data);
}

}
