#include "property_item.inl"

namespace possumwood {

PropertyItem::PropertyItem() {
}

PropertyItem::PropertyItem(const PropertyItem& i) {
	for(auto& elem : i.m_values)
		m_values.push_back(elem->clone());
}

PropertyItem& PropertyItem::operator = (const PropertyItem& i) {
	m_values.clear();
	for(auto& elem : i.m_values)
		m_values.push_back(elem->clone());

	return *this;
}

void PropertyItem::addValue(std::unique_ptr<PropertyItem::ValueBase>&& value) {
	m_values.push_back(std::move(value));
}

void PropertyItem::removeValue(std::size_t index) {
	assert(index < m_values.size());
	m_values.erase(m_values.begin() + index);
}

bool PropertyItem::operator == (const PropertyItem& i) const {
	if(m_values.size() != i.m_values.size())
		return false;

	auto it1 = m_values.begin();
	auto it2 = i.m_values.begin();

	while(it1 != m_values.end()) {
		if(**it1 != **it2)
			return false;

		++it1;
		++it2;
	}

	return true;
}

bool PropertyItem::operator != (const PropertyItem& i) const {
	if(m_values.size() != i.m_values.size())
		return true;

	auto it1 = m_values.begin();
	auto it2 = i.m_values.begin();

	while(it1 != m_values.end()) {
		if(**it1 != **it2)
			return true;

		++it1;
		++it2;
	}

	return false;
}

PropertyItem::ValueBase::ValueBase() {
}

PropertyItem::ValueBase::~ValueBase() {
}

bool PropertyItem::ValueBase::operator == (const ValueBase& v) const {
	return isEqual(v);
}

bool PropertyItem::ValueBase::operator != (const ValueBase& v) const {
	return !isEqual(v);
}

}
