#pragma once

#include <dependency_graph/metadata.h>
#include <dependency_graph/selection.h>
#include <possumwood_sdk/node_data.h>
#include <possumwood_sdk/undo_stack.h>

struct Actions {
	static possumwood::UndoStack::Action createNode(const dependency_graph::Metadata& meta, const std::string& name, const possumwood::NodeData& data);
	static possumwood::UndoStack::Action removeNode(dependency_graph::Node& node);

	static possumwood::UndoStack::Action connect(dependency_graph::Port& p1, dependency_graph::Port& p2);
	static possumwood::UndoStack::Action disconnect(dependency_graph::Port& p1, dependency_graph::Port& p2);

	static possumwood::UndoStack::Action cut(const dependency_graph::Selection& selection);
	static void copy(const dependency_graph::Selection& selection);
	static possumwood::UndoStack::Action paste(dependency_graph::Selection& selection);
	static possumwood::UndoStack::Action remove(const dependency_graph::Selection& selection);

	static void move(dependency_graph::Node& n, const QPointF& pos);
};
