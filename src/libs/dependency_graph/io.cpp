#include "io.h"

namespace dependency_graph { namespace io {

namespace {
	Callbacks s_callbacks;
}

void setCallbacks(const Callbacks& cb) {
	s_callbacks = cb;
}

void fromJson(const json& j, BaseData& data) {
	assert(s_callbacks.fromJson);

	s_callbacks.fromJson(j, data);
}

void toJson(json& j, const BaseData& data) {
	assert(s_callbacks.toJson);

	s_callbacks.toJson(j, data);
}

bool isSaveable(const BaseData& data) {
	assert(s_callbacks.isSaveable);

	return s_callbacks.isSaveable(data);
}

} }
