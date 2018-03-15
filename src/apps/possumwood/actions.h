#pragma once

#include <dependency_graph/metadata.h>
#include <dependency_graph/selection.h>
#include <possumwood_sdk/node_data.h>
#include <possumwood_sdk/undo_stack.h>

struct Actions {
	static void createNode(const dependency_graph::Metadata& meta, const std::string& name, const possumwood::NodeData& data);
	static void removeNode(dependency_graph::NodeBase& node);

	static void connect(dependency_graph::Port& p1, dependency_graph::Port& p2);
	static void disconnect(dependency_graph::Port& p1, dependency_graph::Port& p2);

	static void cut(const dependency_graph::Selection& selection);
	static void copy(const dependency_graph::Selection& selection);
	static void paste(dependency_graph::Selection& selection);
	static void remove(const dependency_graph::Selection& selection);

	static void move(const std::map<dependency_graph::NodeBase*, QPointF>& nodes);
};
