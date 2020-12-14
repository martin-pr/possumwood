#include "traits.h"

// explicit calls for concrete specialisation of numeric traits - will not be necessary
//   one the traits are implemented fully

#pragma GCC diagnostic warning "-Wunused-value"

void fooBar() {
	possumwood::Traits<bool>::io;
	possumwood::Traits<float>::io;
	possumwood::Traits<int>::io;
	possumwood::Traits<unsigned>::io;
}

namespace possumwood {

namespace {

void stringToJson(nlohmann::json& json, const std::string& f) {
	json = f;
}

void stringFromJson(const nlohmann::json& json, std::string& f) {
	f = json.get<std::string>();
}

}  // namespace

IO<std::string> Traits<std::string>::io(&stringToJson, &stringFromJson);

}  // namespace possumwood
