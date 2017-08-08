#pragma once

#include <string>
#include <set>

#include <boost/filesystem/path.hpp>

#include <dependency_graph/data_traits.h>
#include <dependency_graph/io/json.h>

#include "../io.h"
#include "../traits.h"

namespace possumwood {

class Enum {
	public:
		Enum(std::initializer_list<std::string> options = std::initializer_list<std::string>());

		const std::string& value() const;
		void setValue(const std::string& value);

		const std::set<std::string>& options() const;

		Enum& operator = (const Enum& fn);

		bool operator == (const Enum& fn) const;
		bool operator != (const Enum& fn) const;

		void fromJson(const ::dependency_graph::io::json& json);
		void toJson(::dependency_graph::io::json& json) const;

	private:
		std::string m_value;
		std::set<std::string> m_options;
};

template<>
struct Traits<Enum> {
	static IO<Enum> io;
};

}
