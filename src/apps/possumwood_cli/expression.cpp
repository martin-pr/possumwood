#include "expression.h"

#include <boost/algorithm/string.hpp>

ExpressionExpansion::ExpressionExpansion() {
}

ExpressionExpansion::ExpressionExpansion(
    const std::initializer_list<std::pair<std::string, std::function<std::string()>>>& init) {
	for(auto& i : init)
		addVariable(i.first, i.second);
}

void ExpressionExpansion::addVariable(const std::string& name, std::function<std::string()> value) {
	m_variables[name] = value;
}

std::string ExpressionExpansion::expand(const std::string& input) const {
	std::string result = input;

	if(result.find('$') != std::string::npos)
		for(auto it = m_variables.rbegin(); it != m_variables.rend(); ++it)
			boost::replace_all(result, "$" + it->first, it->second());

	return result;
}
