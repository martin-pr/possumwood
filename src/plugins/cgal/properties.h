#pragma once

#include <string>
#include <set>
#include <vector>
#include <map>
#include <memory>
#include <cassert>

#include "property.h"
#include "property_item.h"

namespace possumwood {

template <typename T>
class Property;

/// A simple container class designed to hold properties of items of a polyhedron.
class Properties {
  public:
	Properties();

	Properties(const Properties& p);
	Properties& operator=(const Properties& p);

	template <typename T>
	Property<T>& addProperty(const std::string& name, const T& defaultValue);
	bool hasProperty(const std::string& name) const;
	void removeProperty(const std::string& name);

	template <typename T>
	Property<T>& property(const std::string& name);

	template <typename T>
	const Property<T>& property(const std::string& name) const;

	std::set<std::string> properties() const;

	bool operator==(const Properties& p) const;
	bool operator!=(const Properties& p) const;

  protected:
  private:
  	std::size_t addSingleItem();

	std::map<std::string, std::unique_ptr<PropertyBase>> m_properties;

	std::vector<PropertyItem> m_data;

	template <typename T>
	friend class Property;
};

template <typename T>
Property<T>& Properties::addProperty(const std::string& name, const T& defaultValue) {
	assert(m_properties.find(name) == m_properties.end() && "property naming has to be unique");

	// add a new Property instance
	std::unique_ptr<PropertyBase> ptr(new Property<T>(this, m_properties.size(), defaultValue));
	auto it = m_properties.insert(std::make_pair(name, std::move(ptr))).first;

	// and update all the data items to include this new property
	for(auto& i : m_data)
		i.addValue(defaultValue);

	return dynamic_cast<Property<T>&>(*it->second);
}

template <typename T>
Property<T>& Properties::property(const std::string& name) {
	auto it = m_properties.find(name);
	assert(it != m_properties.end());

	return dynamic_cast<Property<T>&>(*it->second);
}

template <typename T>
const Property<T>& Properties::property(const std::string& name) const {
	auto it = m_properties.find(name);
	assert(it != m_properties.end());

	return dynamic_cast<const Property<T>&>(*it->second);
}

}

#include "property.inl"
