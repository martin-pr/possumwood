#include "variable.h"

namespace possumwood { namespace lua {

Variable::Variable(const std::string& name) : m_name(name) {
}

Variable::Variable(const Variable& con) : m_name(con.m_name) {
}

Variable& Variable::operator = (const Variable& con) {
	m_name = con.m_name;
	return *this;
}

const std::string& Variable::name() const {
	return m_name;
}

bool Variable::operator == (const Variable& var) const {
	return name() == var.name() && type() == var.type() && equalTo(var);
}

bool Variable::operator != (const Variable& var) const {
	return name() != var.name() || type() != var.type() || !equalTo(var);
}

} }
