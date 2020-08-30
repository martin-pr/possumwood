#pragma once

#include <memory>
#include <vector>

namespace possumwood {

/// An opaque key for referencing a single property value.
class PropertyKey {
  public:
	PropertyKey();

	bool isDefault() const;

  private:
	int m_index;

	template <typename T>
	friend class Property;
};
}  // namespace possumwood
