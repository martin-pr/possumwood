#pragma once

#include <dependency_graph/data.h>

namespace dependency_graph { namespace io {

void fromJson(const json& j, BaseData& data);
void toJson(json& j, const BaseData& data);

} }
