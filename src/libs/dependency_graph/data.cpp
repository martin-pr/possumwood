#include "data.inl"

namespace dependency_graph {

Data::Data() {
}

Data::Data(const Data& d) {
	m_data = d.m_data;
	assert(std::string(d.typeinfo().name()) == std::string(typeinfo().name()));
}

Data& Data::operator = (const Data& d) {
	// it should be possible to:
	// 1. assign values between the same types
	// 2. assign a value to a null (void) data - connecting void ports
	// 3. assign a null (void) to a value - disconnecting void ports
	assert(std::string(d.typeinfo().name()) == std::string(typeinfo().name()) || empty() || d.empty());
	m_data = d.m_data;
	return *this;
}

bool Data::operator == (const Data& d) const {
	assert(m_data != nullptr && d.m_data != nullptr);
	return m_data->isEqual(*d.m_data);
}

bool Data::operator != (const Data& d) const {
	assert(m_data != nullptr && d.m_data != nullptr);
	return !m_data->isEqual(*d.m_data);
}

const std::type_info& Data::typeinfo() const {
	if(m_data == nullptr)
		return typeid(void);
	return m_data->typeinfo();
}

std::map<std::string, std::function<Data()>>& Data::factories() {
	static std::unique_ptr<std::map<std::string, std::function<Data()>>> s_factories;
	if(s_factories == nullptr)
		s_factories = std::unique_ptr<std::map<std::string, std::function<Data()>>>(new std::map<std::string, std::function<Data()>>());
	return *s_factories;
}

Data Data::create(const std::string& type) {
	auto it = factories().find(type);

	if(it == factories().end()) {
		std::stringstream err;
		err << "Error instantiating type '" << type << "' - no registered factory found (plugin not loaded?)";

		throw std::runtime_error(err.str());
	}

	return it->second();
}

std::string Data::type() const {
	return dependency_graph::unmangledName(typeinfo().name());
}

std::string Data::toString() const {
	if(m_data == nullptr)
		return "(null)";
	return m_data->toString();
}

bool Data::empty() const {
	return m_data == nullptr;
}

std::ostream& operator << (std::ostream& out, const Data& bd) {
	out << bd.toString();

	return out;
}

}
