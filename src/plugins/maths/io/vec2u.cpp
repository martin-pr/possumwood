#include "vec2u.h"

namespace possumwood {

namespace {

void toJson(::possumwood::io::json& json, const Imath::Vec2<unsigned>& value) {
	json["x"] = value.x;
	json["y"] = value.y;
}

void fromJson(const ::possumwood::io::json& json, Imath::Vec2<unsigned>& value) {
	value.x = json["x"].get<unsigned>();
	value.y = json["y"].get<unsigned>();
}

}

IO<Imath::Vec2<unsigned>> Traits<Imath::Vec2<unsigned>>::io(&toJson, &fromJson);

}
