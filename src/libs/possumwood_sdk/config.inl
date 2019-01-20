#include "config.h"

#include <cassert>
#include <iostream>

namespace possumwood {

template<typename T>
Config::Item::Item(const std::string& name, const std::string& group, const T& defaultValue, const Flags& f, const std::string& description) {
	m_name = name;
	m_group = group;
	m_description = description;
	m_flags = f;

	m_value = defaultValue;
	m_defaultValue = defaultValue;
}

template<typename T>
const T& Config::Item::defaultValue() const {
	return boost::get<T>(m_defaultValue);
}

template<typename T>
bool Config::Item::is() const {
	return boost::get<const T>(&m_defaultValue) != nullptr;
}

template<typename T>
const T Config::Item::as() const {
	return boost::get<T>(m_value);
}

template<typename T>
Config::Item& Config::Item::operator = (const T& val) {
	assert(is<T>());

	if(boost::get<T>(m_value) != val) {
		m_value = val;
		m_onChanged(*this);
	}

	return *this;
}

}
