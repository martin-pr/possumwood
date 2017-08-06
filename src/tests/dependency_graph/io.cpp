#include "io.h"

#include <dependency_graph/rtti.h>

namespace dependency_graph { namespace io {

void fromJson(const json& j, BaseData& data) {
	if(data.type() == "float") {
		Data<float>& typed = dynamic_cast<Data<float>&>(data);
		typed.value = j.get<float>();
	}
	else if(data.type() == dependency_graph::unmangledTypeId<std::string>()) {
		Data<std::string>& typed = dynamic_cast<Data<std::string>&>(data);
		typed.value = j.get<std::string>();
	}
	else
		assert(false);
}

void toJson(json& j, const BaseData& data) {
	if(data.type() == "float") {
		const Data<float>& typed = dynamic_cast<const Data<float>&>(data);
		j = typed.value;
	}
	else if(data.type() == dependency_graph::unmangledTypeId<std::string>()) {
		const Data<std::string>& typed = dynamic_cast<const Data<std::string>&>(data);
		j = typed.value;
	}
	else
		assert(false);
}

} }
