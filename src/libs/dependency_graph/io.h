#pragma once

#include "data.h"

namespace dependency_graph { namespace io {

bool isSaveable(const BaseData& data);

void setIsSaveableCallback(std::function<bool(const BaseData& data)> fn);

} }
