#include "io.h"

namespace dependency_graph { namespace io {

namespace {

// singleton pattern
std::function<bool(const BaseData& data)>& isSaveableCallback() {
	static std::function<bool(const BaseData& data)> fn;
	return fn;
}

}

void setIsSaveableCallback(std::function<bool(const BaseData& data)> fn) {
	isSaveableCallback() = fn;
}

bool isSaveable(const BaseData& data) {
	auto fn = isSaveableCallback();
	if(fn)
		return fn(data);
	return false;
}

} }
