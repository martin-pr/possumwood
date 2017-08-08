#include "mesh.h"

namespace possumwood {

namespace {

void toJson(::dependency_graph::io::json& json, const std::shared_ptr<const Mesh>& value) {
	// not saved, ever
}

void fromJson(const ::dependency_graph::io::json& json, std::shared_ptr<const Mesh>& value) {
	// not loaded, ever
}

}

IO<std::shared_ptr<const Mesh>> Traits<std::shared_ptr<const Mesh>>::io(&toJson, &fromJson);

}
