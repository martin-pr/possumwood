#pragma once

#include <functional>
#include <map>
#include <string>

/// A simple class allowing to expand simple $XXX expressions in a string
class ExpressionExpansion {
  public:
	ExpressionExpansion();
	ExpressionExpansion(const std::initializer_list<std::pair<std::string, std::function<std::string()>>>& init);

	void addVariable(const std::string& name, std::function<std::string()> value);

	std::string expand(const std::string& input) const;

  private:
	std::map<std::string, std::function<std::string()>> m_variables;
};
