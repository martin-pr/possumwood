#include "vec3.h"

namespace possumwood {

namespace {

template <typename T>
void toJson(::nlohmann::json& json, const Imath::Vec3<T>& value) {
	json["x"] = value.x;
	json["y"] = value.y;
	json["z"] = value.z;
}

template <typename T>
void fromJson(const ::nlohmann::json& json, Imath::Vec3<T>& value) {
	value.x = json["x"].get<T>();
	value.y = json["y"].get<T>();
	value.z = json["z"].get<T>();
}

}  // namespace

IO<Imath::Vec3<float>> Traits<Imath::Vec3<float>>::io(&toJson<float>, &fromJson<float>);
IO<Imath::Vec3<int>> Traits<Imath::Vec3<int>>::io(&toJson<int>, &fromJson<int>);
IO<Imath::Vec3<unsigned>> Traits<Imath::Vec3<unsigned>>::io(&toJson<unsigned>, &fromJson<unsigned>);

}  // namespace possumwood
