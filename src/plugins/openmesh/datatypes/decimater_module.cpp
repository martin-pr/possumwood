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

/////////////////

namespace possumwood {

namespace {

void modToJson(::dependency_graph::io::json& json, const DecimaterModule& value) {
	// not saved, ever
}

void modFromJson(const ::dependency_graph::io::json& json, DecimaterModule& value) {
	// not loaded, ever
}

}

IO<DecimaterModule> Traits<DecimaterModule>::io(&modToJson, &modFromJson);

/////////////////

namespace {

void vecToJson(::dependency_graph::io::json& json, const std::vector<DecimaterModule>& value) {
	// not saved, ever
}

void vecFromJson(const ::dependency_graph::io::json& json, std::vector<DecimaterModule>& value) {
	// not loaded, ever
}

}

IO<std::vector<DecimaterModule>> Traits<std::vector<DecimaterModule>>::io(&vecToJson, &vecFromJson);


}
