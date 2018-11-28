#include "symbol_table.h"

namespace possumwood {

void ExprSymbols::addConstant(const std::string& name, float value) {
	m_constants[name] = value;
}

ExprSymbols::const_iterator ExprSymbols::begin() const {
	return m_constants.begin();
}

ExprSymbols::const_iterator ExprSymbols::end() const {
	return m_constants.end();
}

bool ExprSymbols::operator == (const ExprSymbols& es) const {
	return m_constants == es.m_constants;
}

bool ExprSymbols::operator != (const ExprSymbols& es) const {
	return m_constants != es.m_constants;
}

std::ostream& operator << (std::ostream& out, const ExprSymbols& st) {
	out << "Variables:";

	for(auto& v : st)
		out << " " << v.first << "=" << v.second;

	return out;
}

}
