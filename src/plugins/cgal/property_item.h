#pragma once

#include <vector>
#include <memory>

namespace possumwood {

class PropertySetup;

/// A rather naive implementation of property item, holding values for each property in a
/// setup.
class PropertyItem {
  public:
	class ValueBase {
	  public:
		virtual ~ValueBase();

		virtual std::unique_ptr<ValueBase> clone() const = 0;

		bool operator == (const ValueBase& v) const;
		bool operator != (const ValueBase& v) const;

	  protected:
	  	virtual bool isEqual(const ValueBase& v) const = 0;

		ValueBase();
	};

	template <typename T>
	class Value : public ValueBase {
	  public:
		Value(const T& val);
		virtual ~Value();

		T& operator*();
		const T& operator*() const;

	  protected:
		virtual std::unique_ptr<ValueBase> clone() const override;

		virtual bool isEqual(const ValueBase& v) const override;

	  private:
		T m_value;
	};

	PropertyItem();
	PropertyItem(const PropertyItem& i);
	PropertyItem& operator = (const PropertyItem& i);

	template <typename T>
	const T& get(std::size_t index) const;

	template <typename T>
	void set(std::size_t index, const T& value);

	template <typename T>
	void addValue(const T& value);

	void addValue(std::unique_ptr<PropertyItem::ValueBase>&& value);

	void removeValue(std::size_t index);

	bool operator == (const PropertyItem& i) const;
	bool operator != (const PropertyItem& i) const;

  private:
	std::vector<std::unique_ptr<ValueBase>> m_values;
};
}
