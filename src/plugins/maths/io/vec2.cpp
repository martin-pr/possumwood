#include "vec2.h"

namespace possumwood {

namespace {

void toJson(::possumwood::io::json& json, const Imath::Vec2<float>& value) {
	json["x"] = value.x;
	json["y"] = value.y;
}

void fromJson(const ::possumwood::io::json& json, Imath::Vec2<float>& value) {
	value.x = json["x"].get<float>();
	value.y = json["y"].get<float>();
}

}

IO<Imath::Vec2<float>> Traits<Imath::Vec2<float>>::io(&toJson, &fromJson);

}
