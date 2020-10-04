#pragma once

#include <algorithm>
#include <cassert>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <boost/iterator/indirect_iterator.hpp>

#include "property.h"

namespace possumwood {

template <typename T>
class Property;

/// A simple container class designed to hold properties of items of a polyhedron.
class Properties {
  public:
	Properties();

	Properties(const Properties& p);
	Properties& operator=(const Properties& p);

	typedef boost::indirect_iterator<std::vector<std::unique_ptr<PropertyBase>>::const_iterator> const_iterator;
	const_iterator begin() const;
	const_iterator end() const;
	const_iterator find(const std::string& name) const;

	template <typename T>
	Property<T>& addProperty(const std::string& name, const T& defaultValue);
	bool hasProperty(const std::string& name) const;
	void removeProperty(const std::string& name);

	template <typename T>
	Property<T>& property(const std::string& name);

	template <typename T>
	const Property<T>& property(const std::string& name) const;

	bool operator==(const Properties& p) const;
	bool operator!=(const Properties& p) const;

  protected:
  private:
	std::size_t addSingleItem();

	std::vector<std::unique_ptr<PropertyBase>> m_properties;

	template <typename T>
	friend class Property;
};

template <typename T>
Property<T>& Properties::addProperty(const std::string& name, const T& defaultValue) {
	assert(find(name) == end() && "property naming has to be unique");

	// add a new Property instance
	std::unique_ptr<PropertyBase> ptr(new Property<T>(name, defaultValue));
	m_properties.push_back(std::move(ptr));

	std::sort(m_properties.begin(), m_properties.end(),
	          [](const std::unique_ptr<PropertyBase>& v1, const std::unique_ptr<PropertyBase>& v2) {
		          return v1->name() < v2->name();
	          });

	return property<T>(name);
}

template <typename T>
Property<T>& Properties::property(const std::string& name) {
	auto it = find(name);
	assert(it != end());

	return dynamic_cast<Property<T>&>(*it);
}

template <typename T>
const Property<T>& Properties::property(const std::string& name) const {
	auto it = find(name);
	assert(it != end());

	return dynamic_cast<const Property<T>&>(*it);
}

}  // namespace possumwood

#include "property.inl"
