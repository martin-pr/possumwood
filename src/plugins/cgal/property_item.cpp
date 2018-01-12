#include "property_item.inl"

namespace possumwood {

PropertyItem::PropertyItem() {
}

PropertyItem::PropertyItem(const PropertyItem& i) {
	for(auto& elem : i.m_values)
		m_values.push_back(std::move(elem->clone()));
}

PropertyItem& PropertyItem::operator = (const PropertyItem& i) {
	m_values.clear();
	for(auto& elem : i.m_values)
		m_values.push_back(std::move(elem->clone()));

	return *this;
}

PropertyItem::ValueBase::ValueBase() {
}

PropertyItem::ValueBase::~ValueBase() {
}

void PropertyItem::addValue(std::unique_ptr<PropertyItem::ValueBase>&& value) {
	m_values.push_back(std::move(value));
}

void PropertyItem::removeValue(std::size_t index) {
	assert(index < m_values.size());
	m_values.erase(m_values.begin() + index);
}

}
