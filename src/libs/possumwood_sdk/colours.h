#pragma once

#include <array>
#include <typeindex>

namespace possumwood {

class Metadata;

class Colours {
  public:
	static const std::array<float, 3>& get(const std::type_index& type);

  private:
	/// registers a colour for a particular type (only in Metadata)
	static void registerColour(const std::type_index& type, const std::array<float, 3>& c);

	friend class Metadata;
};

}  // namespace possumwood
