#include "variable.h"

#include "state.h"

namespace possumwood { namespace lua {

template<typename T>
PODVariable<T>::PODVariable(const std::string& name, const T& value) : Variable(name), m_value(value) {
}

template<typename T>
std::unique_ptr<Variable> PODVariable<T>::clone() const {
	return std::unique_ptr<Variable>(new PODVariable<T>(*this));
}

template<typename T>
const std::type_info& PODVariable<T>::type() const {
	return typeid(T);
}

template<typename T>
void PODVariable<T>::init(State& s) {
	s.globals()[name()] = m_value;
}

template<typename T>
bool PODVariable<T>::equalTo(const Variable& v) const {
	const PODVariable& vt = dynamic_cast<const PODVariable&>(v);
	return vt.m_value == m_value;
}

template<typename T>
std::string PODVariable<T>::str() const {
	return std::to_string(m_value);
}

} }
