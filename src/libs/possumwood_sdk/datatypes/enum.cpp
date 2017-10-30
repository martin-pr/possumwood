#include "enum.h"

namespace possumwood {

Enum::Enum(std::initializer_list<std::string> options) {
	int counter = 0;
	for(auto& o : options)
		m_options.insert(std::make_pair(o, counter++));
}

Enum::Enum(std::initializer_list<std::pair<std::string, int>> options) : m_options(options.begin(), options.end()) {
}

const std::string& Enum::value() const {
	return m_value.first;
}

int Enum::intValue() const {
	return m_value.second;
}

void Enum::setValue(const std::string& value) {
	auto it = m_options.find(value);
	assert(it != m_options.end());
	m_value = *it;
}

const std::map<std::string, int>& Enum::options() const {
	return m_options;
}

Enum& Enum::operator=(const Enum& fn) {
	if(m_options.empty())
		m_options = fn.m_options;
	m_value = fn.m_value;

	return *this;
}

bool Enum::operator==(const Enum& fn) const {
	return m_value == fn.m_value && m_options == fn.m_options;
}

bool Enum::operator!=(const Enum& fn) const {
	return m_value != fn.m_value || m_options != fn.m_options;
}

void Enum::fromJson(const ::dependency_graph::io::json& json) {
	setValue(json.get<std::string>());
}

void Enum::toJson(::dependency_graph::io::json& json) const {
	json = value();
}

///////

namespace {

void toJson(::dependency_graph::io::json& json, const Enum& value) {
	json = value.value();
}

void fromJson(const ::dependency_graph::io::json& json, Enum& value) {
	value.setValue(json.get<std::string>());
}
}

IO<Enum> Traits<Enum>::io(&toJson, &fromJson);
}
