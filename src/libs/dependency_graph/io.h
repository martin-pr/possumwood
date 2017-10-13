#pragma once

#include <functional>

#include "data.h"

namespace dependency_graph { namespace io {

struct Callbacks {
	std::function<void(const json& j, BaseData& data)> fromJson;
	std::function<void(json& j, const BaseData& data)> toJson;
	std::function<bool(const BaseData&)> isSaveable;
};

void setCallbacks(const Callbacks& cb);

void fromJson(const json& j, BaseData& data);
void toJson(json& j, const BaseData& data);
bool isSaveable(const BaseData& data);

} }
