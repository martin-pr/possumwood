#include "vec3.h"

namespace possumwood {

namespace {

void toJson(::possumwood::io::json& json, const Imath::Vec3<float>& value) {
	json["x"] = value.x;
	json["y"] = value.y;
	json["z"] = value.z;
}

void fromJson(const ::possumwood::io::json& json, Imath::Vec3<float>& value) {
	value.x = json["x"].get<float>();
	value.y = json["y"].get<float>();
	value.z = json["z"].get<float>();
}

}

IO<Imath::Vec3<float>> Traits<Imath::Vec3<float>>::io(&toJson, &fromJson);

}
