#include "data.inl"

namespace dependency_graph {

Data::Data() {
}

void Data::assign(const Data& src) {
	assert(std::string(src.typeinfo().name()) == std::string(typeinfo().name()));
	m_data = src.m_data;
}

bool Data::isEqual(const Data& src) const {
	assert(m_data != nullptr && src.m_data != nullptr);

	return m_data->isEqual(*src.m_data);
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

std::string Data::toString() const {
	if(m_data == nullptr)
		return "(null)";
	return m_data->toString();
}

std::ostream& operator << (std::ostream& out, const Data& bd) {
	out << bd.toString();

	return out;
}

}
