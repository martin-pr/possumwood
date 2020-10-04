#pragma once

#include <memory>
#include <typeindex>

#include <boost/noncopyable.hpp>

#include "property_key.h"

namespace possumwood {

class Properties;

class PropertyBase {
  public:
	virtual ~PropertyBase();

	const std::string& name() const;
	const std::type_index& type() const;

	bool operator==(const PropertyBase& p) const;
	bool operator!=(const PropertyBase& p) const;

  protected:
	PropertyBase(const std::string& name, const std::type_index type);

	virtual std::unique_ptr<PropertyBase> clone() const = 0;

	virtual bool isEqual(const PropertyBase& p) const = 0;

	PropertyBase(const PropertyBase&) = default;
	PropertyBase& operator=(const PropertyBase&) = default;

  private:
	std::string m_name;
	std::type_index m_type;

	friend class Properties;
};

template <typename T>
class Property : public PropertyBase {
  public:
	const T& get(const PropertyKey& key) const;
	void set(PropertyKey& key, const T& value);
	void set(const PropertyKey& key, const T& value);

	typedef typename std::vector<T>::iterator iterator;
	iterator begin();
	iterator end();

  protected:
	Property(const std::string& name, const T& defaultValue);

	Property(const Property&) = default;
	Property& operator=(const Property&) = default;

	virtual std::unique_ptr<PropertyBase> clone() const override;

	virtual bool isEqual(const PropertyBase& p) const override;

  private:
	T m_defaultValue;
	std::vector<T> m_data;

	friend class Properties;
};
}  // namespace possumwood
