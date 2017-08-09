#include "decimater_module.h"

namespace {
	static std::size_t s_uniqueId;
}

DecimaterModule::DecimaterModule() : m_id(s_uniqueId++) {
}

DecimaterModule::DecimaterModule(const DecFn& fn) : m_id(s_uniqueId++) {
	m_fn = fn;
}

void DecimaterModule::operator() (OpenMesh::Decimater::DecimaterT<Mesh>& dec) const {
	m_fn(dec);
}

bool DecimaterModule::operator == (const DecimaterModule& mod) const {
	return m_id == mod.m_id;
}

bool DecimaterModule::operator != (const DecimaterModule& mod) const {
	return m_id != mod.m_id;
}

/////////


namespace possumwood {

namespace {

void toJson(::dependency_graph::io::json& json, const std::vector<DecimaterModule>& value) {
	assert(value.empty());
	json = "[]"_json;
}

void fromJson(const ::dependency_graph::io::json& json, std::vector<DecimaterModule>& value) {
	assert(json.empty());
}

}

IO<std::vector<DecimaterModule>> io(&toJson, &fromJson);

}
