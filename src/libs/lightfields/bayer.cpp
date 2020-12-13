#include "bayer.h"

namespace lightfields {

namespace {

uint16_t getComponent(const nlohmann::json& json, const std::string& key) {
	auto it = json.find(key);
	if(it == json.end())
		throw std::runtime_error("Error reading Bayer component - missing '" + key + "' value.");
	return it->get<uint16_t>();
}

}  // namespace

Bayer::Value::Value() : b(0), gb(0), gr(0), r(0) {
}

Bayer::Value::Value(const nlohmann::json& json) {
	b = getComponent(json, "b");
	gb = getComponent(json, "gb");
	gr = getComponent(json, "gr");
	r = getComponent(json, "r");
}

uint16_t Bayer::Value::operator[](unsigned id) const {
	assert(id < 4 && "Bayer value ID should be in range 0..3");
	return reinterpret_cast<const uint16_t*>(this)[id];
}

}  // namespace lightfields
