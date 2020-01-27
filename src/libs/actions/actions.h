#pragma once

#include <dependency_graph/metadata.h>
#include <dependency_graph/selection.h>
#include <dependency_graph/node_base.h>

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
void setValue(dependency_graph::Port& p, const dependency_graph::Data& value);

void changeMetadata(dependency_graph::NodeBase& node, const dependency_graph::MetadataHandle& handle);
void renameNode(dependency_graph::NodeBase& node, const std::string& name);

void cut(const dependency_graph::Selection& selection);
void copy(const dependency_graph::Selection& selection);
void paste(dependency_graph::Network& current, dependency_graph::Selection& selection);
dependency_graph::State paste(dependency_graph::Network& current, dependency_graph::Selection& selection, const std::string& content, bool haltOnError = true);
void remove(const dependency_graph::Selection& selection);

void move(const std::map<dependency_graph::NodeBase*, possumwood::NodeData::Point>& nodes);

dependency_graph::State fromJson(dependency_graph::Network& current, dependency_graph::Selection& selection, const possumwood::io::json& json, bool haltOnError = true);
possumwood::io::json toJson(const dependency_graph::Selection& selection = dependency_graph::Selection());
////

template<typename T>
void setValue(dependency_graph::Port& p, const T& value) {
	setValue(p, dependency_graph::Data(value));
}

}}
