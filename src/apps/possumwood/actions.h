#pragma once

#include <dependency_graph/metadata.h>
#include <possumwood_sdk/node_data.h>

struct Actions {
	static void createNode(const dependency_graph::Metadata& meta, const std::string& name, const possumwood::NodeData& data);
};
