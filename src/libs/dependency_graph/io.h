#pragma once

#include "data.h"

namespace dependency_graph { namespace io {

extern void fromJson(const json& j, BaseData& data);
extern void toJson(json& j, const BaseData& data);
extern bool isSaveable(const BaseData& data);

} }
