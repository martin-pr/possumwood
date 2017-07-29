#include "actions.h"

#include <possumwood_sdk/app.h>

void Actions::createNode(const dependency_graph::Metadata& meta, const std::string& name, const possumwood::NodeData& data) {
	possumwood::App::instance().graph().nodes().add(meta, name, data);
}
