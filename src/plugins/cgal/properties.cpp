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

bool Properties::hasProperty(const std::string& name) const {
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
	if(m_properties.size() != p.m_properties.size())
		return false;

	auto it1 = m_properties.begin();
	auto it2 = p.m_properties.begin();

	while(it1 != m_properties.end()) {
		if(it1->first != it2->first || *it1->second != *it2->second)
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
		if(it1->first != it2->first || *it1->second != *it2->second)
			return true;

		++it1;
		++it2;
	}

	return false;
}

std::size_t Properties::addSingleItem() {
	m_data.push_back(PropertyItem());

	for(auto& p : m_properties)
		m_data.back().addValue(p.second->makeValue());

	return m_data.size() - 1;
}

void Properties::removeProperty(const std::string& name) {
	auto it = m_properties.find(name);
	assert(it != m_properties.end());

	const std::size_t removedID = it->second->index();

	m_properties.erase(it);

	for(auto& p : m_properties)
		if(p.second->index() > removedID)
			p.second->setIndex(p.second->index() - 1);

	for(auto& d : m_data)
		d.removeValue(removedID);
}

}  // namespace possumwood
