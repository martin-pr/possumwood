#pragma once

#include <string>
#include <map>

#include <actions/traits.h>

namespace possumwood {

class ExprSymbols {
	public:
		void addConstant(const std::string& name, float value);

		typedef std::map<std::string, float>::const_iterator const_iterator;
		const_iterator begin() const;
		const_iterator end() const;

		bool operator == (const ExprSymbols& es) const;
		bool operator != (const ExprSymbols& es) const;

	private:
		std::map<std::string, float> m_constants;
};

template<>
struct Traits<ExprSymbols> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0, 0, 1}};
	}
};

std::ostream& operator << (std::ostream& out, const ExprSymbols& st);

}
