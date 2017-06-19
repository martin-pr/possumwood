#include "enum.h"

namespace possumwood {

Enum::Enum(std::initializer_list<std::string> options) : m_options(options) {
}

const std::string& Enum::value() const {
	return m_value;
}

void Enum::setValue(const std::string& value) {
	m_value = value;
}

const std::set<std::string>& Enum::options() const {
	return m_options;
}

Enum& Enum::operator = (const Enum& fn) {
	if(m_options.empty())
		m_options = fn.m_options;
	m_value = fn.m_value;

	return *this;
}

bool Enum::operator == (const Enum& fn) const {
	return m_value == fn.m_value && m_options == fn.m_options;
}

bool Enum::operator != (const Enum& fn) const {
	return m_value != fn.m_value || m_options != fn.m_options;
}

void Enum::fromJson(const ::dependency_graph::io::json& json) {
	setValue(json.get<std::string>());
}

void Enum::toJson(::dependency_graph::io::json& json) const {
	json = value();
}

}
