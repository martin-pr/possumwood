#include "context.h"

namespace possumwood { namespace lua {

Context::Context() {
}

Context::~Context() {
}

Context::Context(const Context& con) {
	for(auto& v : con.m_variables)
		m_variables.push_back(v->clone());
}

Context& Context::operator = (const Context& con) {
	m_variables.clear();

	for(auto& v : con.m_variables)
		m_variables.push_back(v->clone());

	return *this;
}

bool Context::operator == (const Context& c) const {
	if(c.m_variables.size() != m_variables.size())
		return false;

	auto i1 = m_variables.begin();
	auto i2 = c.m_variables.begin();

	while(i1 != m_variables.end()) {
		if(**i1 != **i2)
			return false;

		++i1;
		++i2;
	}

	return true;
}

bool Context::operator != (const Context& c) const {
	if(c.m_variables.size() != m_variables.size())
		return true;

	auto i1 = m_variables.begin();
	auto i2 = c.m_variables.begin();

	while(i1 != m_variables.end()) {
		if(**i1 != **i2)
			return true;

		++i1;
		++i2;
	}

	return false;
}

void Context::addVariable(std::unique_ptr<Variable>&& var) {
	m_variables.push_back(std::move(var));
}

std::ostream& operator << (std::ostream& out, const Context& st) {
	std::cout << "Variables: ";
	for(auto& v : st.m_variables)
		std::cout << v->name() << " ";

	return out;
}

}}
