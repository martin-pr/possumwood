#include "properties.h"

namespace possumwood {

Properties::Properties() {
}

Properties::Properties(const Properties& p) {
	for(auto& prop : p.m_properties) {
		std::unique_ptr<PropertyBase> property = prop.second->clone(this);
		m_properties.insert(std::make_pair(prop.first, std::move(property)));
	}

	for(auto& d : p.m_data)
		m_data.push_back(d);
}

Properties& Properties::operator=(const Properties& p) {
	m_properties.clear();

	for(auto& prop : p.m_properties) {
		std::unique_ptr<PropertyBase> property = prop.second->clone(this);
		m_properties.insert(std::make_pair(prop.first, std::move(property)));
	}

	m_data.clear();

	for(auto& d : p.m_data)
		m_data.push_back(d);

	return *this;
}

bool Properties::hasProperty(const std::string& name) {
	auto it = m_properties.find(name);
	return it != m_properties.end();
}

std::set<std::string> Properties::properties() const {
	std::set<std::string> result;
	for(auto& p : m_properties)
		result.insert(p.first);
	return result;
}

bool Properties::operator==(const Properties& p) const {
	return m_properties == p.m_properties;
}

bool Properties::operator!=(const Properties& p) const {
	return m_properties != p.m_properties;
}

std::size_t Properties::addSingleItem() {
	m_data.push_back(PropertyItem());

	for(auto& p : m_properties)
		m_data.back().addValue(p.second->makeValue());

	return m_data.size()-1;
}

void Properties::removeProperty(const std::string& name) {
	auto it = m_properties.find(name);
	assert(it != m_properties.end());

	const std::size_t removedID = it->second->index();

	m_properties.erase(it);

	for(auto& p : m_properties)
		if(p.second->index() > removedID)
			p.second->setIndex(p.second->index()-1);

	for(auto& d : m_data)
		d.removeValue(removedID);
}

}
