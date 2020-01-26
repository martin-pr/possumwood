#include "data.inl"

namespace dependency_graph {

Data::Data() {
}

const std::type_info& Data::typeinfo() const {
	if(m_data == nullptr)
		return typeid(void);
	return m_data->typeinfo();
}

std::map<std::string, std::function<std::unique_ptr<Data>()>>& Data::factories() {
	static std::unique_ptr<std::map<std::string, std::function<std::unique_ptr<Data>()>>> s_factories;
	if(s_factories == nullptr)
		s_factories = std::unique_ptr<std::map<std::string, std::function<std::unique_ptr<Data>()>>>(new std::map<std::string, std::function<std::unique_ptr<Data>()>>());
	return *s_factories;
}

std::unique_ptr<Data> Data::create(const std::string& type) {
	auto it = factories().find(type);

	if(it == factories().end()) {
		std::stringstream err;
		err << "Error instantiating type '" << type << "' - no registered factory found (plugin not loaded?)";

		throw std::runtime_error(err.str());
	}

	return it->second();
}

std::unique_ptr<Data> Data::clone() const {
	return std::unique_ptr<Data>(new Data(*this));
}

std::string Data::type() const {
	return dependency_graph::unmangledName(typeinfo().name());
}

////

// TypedData<void>::TypedData() {
// }

// TypedData<void>::~TypedData() {
// }

// void TypedData<void>::assign(const Data& src) {
// 	assert(src.typeinfo() == typeid(void));
// }

// bool TypedData<void>::isEqual(const Data& src) const {
// 	return src.typeinfo() == typeid(void);
// }

// const std::type_info& TypedData<void>::typeinfo() const {
// 	return typeid(void);
// }

// std::unique_ptr<Data> TypedData<void>::clone() const {
// 	return std::unique_ptr<Data>(new TypedData<void>());
// }

// std::string TypedData<void>::toString() const {
// 	return "void";
// }

std::ostream& operator << (std::ostream& out, const Data& bd) {
	out << bd.toString();

	return out;
}

// Data::Factory<void> TypedData<void>::m_factory;

}
