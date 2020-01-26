#pragma once

#include <dependency_graph/data.h>

#include <actions/io.h>

namespace dependency_graph { namespace io {

bool isSaveable(const Data& data);

} }

namespace possumwood { namespace io {

void fromJson(const json& j, dependency_graph::Data& data);
void toJson(json& j, const dependency_graph::Data& data);

} }
