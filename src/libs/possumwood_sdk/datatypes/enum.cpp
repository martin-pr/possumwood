#include "enum.h"

#include <algorithm>

namespace possumwood {

Enum::Enum(std::initializer_list<std::string> options, int defaultValue) {
	int counter = 0;
	for(auto& o : options)
		m_options.push_back(std::make_pair(o, counter++));

	if(!m_options.empty() && defaultValue < (int)m_options.size())
		m_value = m_options[defaultValue];
	else if(!m_options.empty())
		m_value = m_options[0];
}

Enum::Enum(std::initializer_list<std::pair<std::string, int>> options, int defaultValue) : m_options(options.begin(), options.end()) {
	if(!m_options.empty()) {
		m_value = m_options[0];

		for(auto& o : m_options)
			if(o.second == defaultValue)
				m_value = o;
	}
}

Enum::Enum(const Enum& fn) : m_value(fn.m_value), m_options(fn.m_options) {
}

const std::string& Enum::value() const {
	return m_value.first;
}

int Enum::intValue() const {
	return m_value.second;
}

void Enum::setValue(const std::string& value) {
	auto it = std::find_if(m_options.begin(), m_options.end(), [&value](const std::pair<std::string, int>& val) {return val.first == value;});
	if(it == m_options.end())
		throw std::runtime_error("Enum value " + value + " not found!");
	m_value = *it;
}

const std::vector<std::pair<std::string, int>>& Enum::options() const {
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

void Enum::fromJson(const ::possumwood::io::json& json) {
	setValue(json.get<std::string>());
}

void Enum::toJson(::possumwood::io::json& json) const {
	json = value();
}

std::ostream& operator << (std::ostream& out, const Enum& e) {
	out << e.value();
	return out;
}

///////

namespace {

void toJson(::possumwood::io::json& json, const Enum& value) {
	json = value.value();
}

void fromJson(const ::possumwood::io::json& json, Enum& value) {
	value.setValue(json.get<std::string>());
}
}

IO<Enum> Traits<Enum>::io(&toJson, &fromJson);
}
