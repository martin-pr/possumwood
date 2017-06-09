#pragma once

#include <OpenEXR/ImathVec.h>

#include <dependency_graph/io/json.h>

// needs to be in the same namespace as the Vec3 - on some systems, the OpenEXR namespaces are versioned
#ifdef IMATH_INTERNAL_NAMESPACE
namespace IMATH_INTERNAL_NAMESPACE {
#else
namespace Imath {
#endif

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
