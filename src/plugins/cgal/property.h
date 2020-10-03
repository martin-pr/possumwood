#pragma once

#include <boost/noncopyable.hpp>
#include <memory>

#include "property_item.h"
#include "property_key.h"

namespace possumwood {

class Properties;

class PropertyBase {
  public:
	virtual ~PropertyBase();

	std::size_t index() const;

	bool operator==(const PropertyBase& p) const;
	bool operator!=(const PropertyBase& p) const;

  protected:
	PropertyBase(Properties* parent, std::size_t index);

	Properties& parent();
	const Properties& parent() const;

	virtual std::unique_ptr<PropertyBase> clone(Properties* parent) const = 0;
	virtual std::unique_ptr<PropertyItem::ValueBase> makeValue() const = 0;

	void setIndex(std::size_t i);

	virtual bool isEqual(const PropertyBase& p) const = 0;

  private:
	Properties* m_parent;
	std::size_t m_index;

	friend class Properties;
};

template <typename T>
class Property : public PropertyBase {
  public:
	const T& get(const PropertyKey& key) const;
	void set(PropertyKey& key, const T& value);
	void set(const PropertyKey& key, const T& value);

  protected:
	Property(Properties* parent, std::size_t index, const T& defaultValue);

	virtual std::unique_ptr<PropertyBase> clone(Properties* parent) const override;
	virtual std::unique_ptr<PropertyItem::ValueBase> makeValue() const override;

	virtual bool isEqual(const PropertyBase& p) const override;

  private:
	T m_defaultValue;

	friend class Properties;
};
}  // namespace possumwood
