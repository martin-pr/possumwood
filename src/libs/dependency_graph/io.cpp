#include "io.h"

namespace dependency_graph { namespace io {

namespace {

// singleton pattern
std::function<bool(const Data& data)>& isSaveableCallback() {
	static std::function<bool(const Data& data)> fn;
	return fn;
}

}

void setIsSaveableCallback(std::function<bool(const Data& data)> fn) {
	isSaveableCallback() = fn;
}

bool isSaveable(const Data& data) {
	auto fn = isSaveableCallback();
	if(fn)
		return fn(data);
	return false;
}

} }
