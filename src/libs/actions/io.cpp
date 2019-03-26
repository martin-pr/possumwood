#include "io.h"

namespace {
	std::map<std::type_index, possumwood::IOBase::to_fn>& s_toFn() {
		static std::map<std::type_index, possumwood::IOBase::to_fn> s_map;
		return s_map;
	}

	std::map<std::type_index, possumwood::IOBase::from_fn>& s_fromFn() {
		static std::map<std::type_index, possumwood::IOBase::from_fn> s_map;
		return s_map;
	}
}

namespace possumwood {

IOBase::IOBase(const std::type_index& type, to_fn toJson, from_fn fromJson) : m_type(type) {
	// assert(s_toFn().find(type) == s_toFn().end() && "only one serialization per data type should be implemented");
	// assert(s_fromFn().find(type) == s_fromFn().end() && "only one serialization per data type should be implemented");

	s_toFn()[type] = toJson;
	s_fromFn()[type] = fromJson;
}

IOBase::~IOBase() {
	auto it1 = s_toFn().find(m_type);
	if(it1 != s_toFn().end())
		s_toFn().erase(it1);

	auto it2 = s_fromFn().find(m_type);
	if(it2 != s_fromFn().end())
		s_fromFn().erase(it2);
}

}

/////////////////////////////////////

namespace possumwood { namespace io {

void fromJson(const json& j, dependency_graph::BaseData& data) {
	auto it = s_fromFn().find(data.typeinfo());

	#ifndef NDEBUG
	if(it == s_fromFn().end())
		std::cout << "Error - no json serialization implemented for type " << data.type() << std::endl;
	assert(it != s_fromFn().end());
	#endif

	it->second(j, data);
}

void toJson(json& j, const dependency_graph::BaseData& data) {
	auto it = s_toFn().find(data.typeinfo());
	#ifndef NDEBUG
	if(it == s_toFn().end())
		std::cout << "Error - no json serialization implemented for type " << data.type() << std::endl;
	assert(it != s_toFn().end());
	#endif

	it->second(j, data);
}

} }

namespace dependency_graph { namespace io {

struct SaveableRegistration {
	SaveableRegistration() {
		setIsSaveableCallback([](const BaseData& data) {
			return s_toFn().find(data.typeinfo()) != s_toFn().end();
		});
	}

	~SaveableRegistration() {
		setIsSaveableCallback(std::function<bool(const BaseData& data)>());
	}
};

static SaveableRegistration s_saveableRegistration;

} }
