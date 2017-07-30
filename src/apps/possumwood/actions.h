#pragma once

#include <dependency_graph/metadata.h>
#include <dependency_graph/selection.h>
#include <possumwood_sdk/node_data.h>

struct Actions {
	static void createNode(const dependency_graph::Metadata& meta, const std::string& name, const possumwood::NodeData& data);

	static void cut(const dependency_graph::Selection& selection);
	static void copy(const dependency_graph::Selection& selection);
	static dependency_graph::Selection paste();
	static void remove(const dependency_graph::Selection& selection);
};
