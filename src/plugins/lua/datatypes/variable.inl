#pragma once

#include <string>

#include "state.h"
#include "variable.h"

namespace possumwood {
namespace lua {

template <typename T>
Variable::Variable(const std::string& name, const T& value) : m_name(name) {
	m_value = std::shared_ptr<const HolderBase>(new Holder<T>(value));
}

template <typename T>
Variable::Holder<T>::Holder(const T& value) : m_value(value) {
}

template <typename T>
const std::type_info& Variable::Holder<T>::type() const {
	return typeid(T);
}

template <typename T>
void Variable::Holder<T>::init(State& s, const std::string& name) const {
	s.globals()[name] = m_value;
}

template <typename T>
std::string Variable::Holder<T>::str() const {
	// ADL lookup - either use current namespace, or std, whichever comes first
	using std::to_string;
	return to_string(m_value);
}

template <typename T>
bool Variable::Holder<T>::equalTo(const HolderBase& v) const {
	const Holder<T>* tmp = dynamic_cast<const Holder<T>*>(&v);
	if(tmp)
		return m_value == tmp->m_value;
	return false;
}

}  // namespace lua
}  // namespace possumwood
