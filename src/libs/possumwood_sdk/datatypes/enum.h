#pragma once

#include <string>
#include <set>

#include <boost/filesystem/path.hpp>

#include <dependency_graph/data_traits.h>
#include <dependency_graph/io/json.h>

#include "actions/io.h"
#include "actions/traits.h"

namespace possumwood {

class Enum {
  public:
	Enum(std::initializer_list<std::string> options = std::initializer_list<std::string>());
	Enum(std::initializer_list<std::pair<std::string, int>> options);

	const std::string& value() const;
	int intValue() const;
	void setValue(const std::string& value);

	const std::vector<std::pair<std::string, int>>& options() const;

	Enum& operator=(const Enum& fn);

	bool operator==(const Enum& fn) const;
	bool operator!=(const Enum& fn) const;

	void fromJson(const ::dependency_graph::io::json& json);
	void toJson(::dependency_graph::io::json& json) const;

  private:
	std::pair<std::string, int> m_value;
	std::vector<std::pair<std::string, int>> m_options;
};

template <>
struct Traits<Enum> {
	static IO<Enum> io;

	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0, 0, 0}};
	}
};

std::ostream& operator << (std::ostream& out, const Enum& e);

}
