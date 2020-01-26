#pragma once

#include "data.h"

namespace dependency_graph { namespace io {

bool isSaveable(const Data& data);

void setIsSaveableCallback(std::function<bool(const Data& data)> fn);

} }
