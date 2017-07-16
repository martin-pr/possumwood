#include "attribute.h"

namespace anim {

Attribute::Data::Data() {
}

Attribute::Data::~Data() {
}

///

Attribute::Attribute() {
}

Attribute::~Attribute() {
}

Attribute::Attribute(const Attribute& a) {
	if(a.m_data.get() != NULL)
		m_data = a.m_data->clone();
}

Attribute::Attribute(Attribute&& a) : m_data(std::move(a.m_data)) {
}

Attribute& Attribute::operator = (const Attribute& a) {
	if(a.m_data.get() != NULL)
		m_data = a.m_data->clone();

	return *this;
}

Attribute& Attribute::operator = (Attribute&& a) {
	m_data = std::move(a.m_data);

	return *this;
}

std::string Attribute::type() const {
	assert(!empty());
	return m_data->type();
}

std::string Attribute::toString() const {
	assert(!empty());
	return m_data->toString();
}

bool Attribute::empty() const {
	return m_data.get() == NULL;
}

Attribute& Attribute::operator = (const char* value) {
	*this = std::string(value);

	return *this;
}

}
