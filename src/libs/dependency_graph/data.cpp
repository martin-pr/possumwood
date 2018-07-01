#include "data.inl"

namespace dependency_graph {

std::map<std::string, std::function<std::unique_ptr<BaseData>()>>& BaseData::factories() {
	static std::map<std::string, std::function<std::unique_ptr<BaseData>()>> s_factories;
	return s_factories;
}

BaseData::BaseData() {
};

BaseData::~BaseData() {
};

std::unique_ptr<BaseData> BaseData::create(const std::string& type) {
	auto it = factories().find(type);

	if(it == factories().end()) {
		std::stringstream err;
		err << "Error instantiating type '" << type << "' - no registered factory found (plugin not loaded?)";

		throw std::runtime_error(err.str());
	}

	return it->second();
}

std::string BaseData::type() const {
	return dependency_graph::unmangledName(typeinfo().name());
}

////

Data<void>::Data() {
}

Data<void>::~Data() {
}

void Data<void>::assign(const BaseData& src) {
	assert(src.typeinfo() == typeid(void));
}

bool Data<void>::isEqual(const BaseData& src) const {
	return src.typeinfo() == typeid(void);
}

const std::type_info& Data<void>::typeinfo() const {
	return typeid(void);
}

std::unique_ptr<BaseData> Data<void>::clone() const {
	return std::unique_ptr<BaseData>(new Data<void>());
}

std::string Data<void>::toString() const {
	return "void";
}

std::ostream& operator << (std::ostream& out, const BaseData& bd) {
	out << bd.toString();

	return out;
}

BaseData::Factory<void> Data<void>::m_factory;

}
