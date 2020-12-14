#pragma once

#include <actions/io.h>
#include <dependency_graph/data.h>

namespace dependency_graph {
namespace io {

bool isSaveable(const Data& data);

}
}  // namespace dependency_graph

namespace possumwood {
namespace io {

void fromJson(const nlohmann::json& j, dependency_graph::Data& data);
void toJson(nlohmann::json& j, const dependency_graph::Data& data);

}  // namespace io
}  // namespace possumwood
