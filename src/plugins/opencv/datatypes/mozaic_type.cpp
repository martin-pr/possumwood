#include "mozaic_type.h"

namespace possumwood {

namespace {

std::string toString(const MozaicType& m) {
	switch(m) {
		case MozaicType::BG:
			return "BG";
		case MozaicType::GB:
			return "GB";
		case MozaicType::RG:
			return "RG";
		case MozaicType::GR:
			return "GR";
		default:
			return "Unknown";
	}
}

}  // namespace

std::ostream& operator<<(std::ostream& out, const MozaicType& f) {
	out << toString(f);
	return out;
}

/////////////////

namespace {

void toJson(::nlohmann::json& json, const MozaicType& value) {
	json = toString(value);
}

void fromJson(const ::nlohmann::json& json, MozaicType& value) {
	const std::string s = json.get<std::string>();
	if(s == "BG")
		value = MozaicType::BG;
	else if(s == "GB")
		value = MozaicType::GB;
	else if(s == "RG")
		value = MozaicType::RG;
	else if(s == "GR")
		value = MozaicType::GR;
	else
		throw std::runtime_error("Unknown mozaic type " + s);
}

}  // namespace

IO<MozaicType> Traits<MozaicType>::io(&toJson, &fromJson);

}  // namespace possumwood
