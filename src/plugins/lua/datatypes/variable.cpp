#include "variable.h"

namespace possumwood {
namespace lua {

const std::string& Variable::name() const {
	return m_name;
}

const std::type_info& Variable::type() const {
	return m_value->type();
}

bool Variable::operator==(const Variable& var) const {
	return name() == var.name() && type() == var.type() && m_value->equalTo(*var.m_value);
}

bool Variable::operator!=(const Variable& var) const {
	return name() != var.name() || type() != var.type() || !m_value->equalTo(*var.m_value);
}

void Variable::init(State& s) const {
	return m_value->init(s, m_name);
}

std::string Variable::str() const {
	return m_value->str();
}

}  // namespace lua
}  // namespace possumwood
