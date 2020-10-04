#include "properties.h"

namespace possumwood {

Properties::Properties() {
}

Properties::Properties(const Properties& p) {
	for(auto& prop : p.m_properties) {
		std::unique_ptr<PropertyBase> property = prop->clone();
		m_properties.push_back(std::move(property));
	}
}

Properties::const_iterator Properties::begin() const {
	return boost::make_indirect_iterator(m_properties.begin());
}

Properties::const_iterator Properties::end() const {
	return boost::make_indirect_iterator(m_properties.end());
}

Properties::const_iterator Properties::find(const std::string& name) const {
	auto it = std::find_if(m_properties.begin(), m_properties.end(),
	                       [name](const std::unique_ptr<PropertyBase>& val) { return val->name() == name; });

	return boost::make_indirect_iterator(it);
}

Properties& Properties::operator=(const Properties& p) {
	m_properties.clear();

	for(auto& prop : p.m_properties) {
		std::unique_ptr<PropertyBase> property = prop->clone();
		m_properties.push_back(std::move(property));
	}

	return *this;
}

bool Properties::hasProperty(const std::string& name) const {
	auto it = find(name);
	return it != end();
}

bool Properties::operator==(const Properties& p) const {
	if(m_properties.size() != p.m_properties.size())
		return false;

	auto it1 = m_properties.begin();
	auto it2 = p.m_properties.begin();

	while(it1 != m_properties.end()) {
		if(*it1 != *it2)
			return false;

		++it1;
		++it2;
	}

	return true;
}

bool Properties::operator!=(const Properties& p) const {
	if(m_properties.size() != p.m_properties.size())
		return true;

	auto it1 = m_properties.begin();
	auto it2 = p.m_properties.begin();

	while(it1 != m_properties.end()) {
		if(*it1 != *it2)
			return true;

		++it1;
		++it2;
	}

	return false;
}

void Properties::removeProperty(const std::string& name) {
	auto it = std::find_if(m_properties.begin(), m_properties.end(),
	                       [name](const std::unique_ptr<PropertyBase>& val) { return val->name() == name; });
	m_properties.erase(it);
}

}  // namespace possumwood
