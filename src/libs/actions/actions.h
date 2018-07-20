#pragma once

#include <dependency_graph/metadata.h>
#include <dependency_graph/selection.h>

#include "node_data.h"
#include "undo_stack.h"

namespace possumwood { namespace actions {

void createNode(dependency_graph::Network& current, const dependency_graph::MetadataHandle& meta,
                       const std::string& name, const possumwood::NodeData& data,
                       const dependency_graph::UniqueId& id = dependency_graph::UniqueId());
void removeNode(dependency_graph::NodeBase& node);

void connect(dependency_graph::Port& p1, dependency_graph::Port& p2);
void disconnect(dependency_graph::Port& p1, dependency_graph::Port& p2);

template<typename T>
void setValue(dependency_graph::Port& p, const T& value);
void setValue(dependency_graph::Port& p, const dependency_graph::BaseData& value);

void changeMetadata(dependency_graph::NodeBase& node, const dependency_graph::MetadataHandle& handle);

void cut(const dependency_graph::Selection& selection);
void copy(const dependency_graph::Selection& selection);
void paste(dependency_graph::Network& current, dependency_graph::Selection& selection, const possumwood::io::json& json);
void paste(dependency_graph::Network& current, dependency_graph::Selection& selection);
void remove(const dependency_graph::Selection& selection);

void move(const std::map<dependency_graph::NodeBase*, possumwood::NodeData::Point>& nodes);

////

template<typename T>
void setValue(dependency_graph::Port& p, const T& value) {
	const std::unique_ptr<const dependency_graph::BaseData> data(new dependency_graph::Data<T>(value));
	setValue(p, *data);
}

}}
