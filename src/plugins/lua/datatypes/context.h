#pragma once

#include <memory>
#include <vector>
#include <map>
#include <functional>

#include <boost/range/iterator_range.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include <actions/traits.h>

#include "variable.h"
#include "state.h"

namespace possumwood { namespace lua {

class State;

class Context {
	public:
		Context();
		~Context();

		void addVariable(const Variable& v);
		void addModule(const std::string& name, const std::function<void(State&)>& registration);

		bool operator == (const Context& c) const;
		bool operator != (const Context& c) const;

		typedef std::vector<Variable>::const_iterator const_var_iterator;
		boost::iterator_range<const_var_iterator> variables() const;

		typedef boost::transform_iterator<
			std::function<const std::string&(const std::pair<std::string, std::function<void(State&)>>&)>,
			std::map<std::string, std::function<void(State&)>>::const_iterator
		> const_module_iterator;
		boost::iterator_range<const_module_iterator> modules() const;

	private:
		std::vector<Variable> m_variables;
		std::map<std::string, std::function<void(State&)>> m_modules;

	friend class State;

	friend std::ostream& operator << (std::ostream& out, const Context& st);
};

std::ostream& operator << (std::ostream& out, const Context& st);

}

template<>
struct Traits<lua::Context> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0.5, 0.5, 1}};
	}
};

}
