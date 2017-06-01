#pragma once

#include <OpenEXR/ImathVec.h>

#include <dependency_graph/io/json.h>

namespace Imath {

template<typename T>
void to_json(::dependency_graph::io::json& j, const Vec3<T>& g) {
	j["x"] = g.x;
	j["y"] = g.y;
	j["z"] = g.z;
}

template<typename T>
void from_json(const ::dependency_graph::io::json& j, Vec3<T>& g) {
	g.x = j["x"].get<float>();
	g.y = j["y"].get<float>();
	g.z = j["z"].get<float>();
}

}
