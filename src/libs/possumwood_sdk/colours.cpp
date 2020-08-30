#include "colours.h"

#include <map>
#include <memory>

namespace possumwood {

namespace {
std::map<std::type_index, std::array<float, 3>>& colours() {
	static std::unique_ptr<std::map<std::type_index, std::array<float, 3>>> s_colurs;
	if(!s_colurs)
		s_colurs = std::unique_ptr<std::map<std::type_index, std::array<float, 3>>>(
		    new std::map<std::type_index, std::array<float, 3>>());
	return *s_colurs;
}
}  // namespace

const std::array<float, 3>& Colours::get(const std::type_index& type) {
	static const std::array<float, 3> s_default{1, 0, 0};

	auto it = colours().find(type);
	if(it == colours().end())
		return s_default;
	return it->second;
}

void Colours::registerColour(const std::type_index& type, const std::array<float, 3>& c) {
	colours().insert(std::make_pair(type, c));
}

}  // namespace possumwood
