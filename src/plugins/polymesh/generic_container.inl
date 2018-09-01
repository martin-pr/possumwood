#include "generic_container.h"

#include "generic_array.inl"
#include "generic_base.inl"

namespace possumwood {
namespace polymesh {

template<typename ITEM>
GenericContainer<ITEM>::iterator::iterator() {
}

template<typename ITEM>
GenericContainer<ITEM>::iterator::iterator(GenericContainer* parent, std::size_t index) : m_item(new ITEM(parent, index)) {
}

template<typename ITEM>
GenericContainer<ITEM>::iterator::iterator(const iterator& i) : m_item(new ITEM(*i.m_item)) {
}

template<typename ITEM>
typename GenericContainer<ITEM>::iterator& GenericContainer<ITEM>::iterator::operator = (const iterator& i) {
	m_item = std::unique_ptr<ITEM>(new ITEM(i.m_item));

	return *this;
}

template<typename ITEM>
void GenericContainer<ITEM>::iterator::increment() {
	assert(m_item);
	++(*m_item);
}

template<typename ITEM>
void GenericContainer<ITEM>::iterator::advance(long d) {
	assert(m_item);
	(*m_item) += + d;
}

template<typename ITEM>
long GenericContainer<ITEM>::iterator::distance_to(const iterator& i) const {
	return *i.m_item - *m_item;
}

template<typename ITEM>
bool GenericContainer<ITEM>::iterator::equal(const iterator& i) const {
	return m_item->m_parent == i.m_item->m_parent && m_item->m_index == i.m_item->m_index;
}

template<typename ITEM>
ITEM& GenericContainer<ITEM>::iterator::dereference() const {
	assert(m_item);
	return *m_item;
}

template<typename ITEM>
typename GenericContainer<ITEM>::iterator GenericContainer<ITEM>::begin() {
	return iterator(this, 0);
}

template<typename ITEM>
typename GenericContainer<ITEM>::iterator GenericContainer<ITEM>::end() {
	return iterator(this, size());
}


////

template<typename ITEM>
GenericContainer<ITEM>::const_iterator::const_iterator() : m_item(nullptr, 0) {
}

template<typename ITEM>
GenericContainer<ITEM>::const_iterator::const_iterator(GenericContainer* parent, std::size_t index) : m_item(parent, index) {
}

template<typename ITEM>
void GenericContainer<ITEM>::const_iterator::increment() {
	++m_item;
}

template<typename ITEM>
void GenericContainer<ITEM>::const_iterator::advance(long d) {
	m_item += d;
}

template<typename ITEM>
long GenericContainer<ITEM>::const_iterator::distance_to(const const_iterator& i) const {
	return i.m_item - m_item;
}

template<typename ITEM>
bool GenericContainer<ITEM>::const_iterator::equal(const const_iterator& i) const {
	return m_item.m_parent == i.m_item.m_parent && m_item.m_index == i.m_item.m_index;
}

template<typename ITEM>
const ITEM& GenericContainer<ITEM>::const_iterator::dereference() const {
	return m_item;
}

template<typename ITEM>
typename GenericContainer<ITEM>::const_iterator GenericContainer<ITEM>::begin() const {
	return const_iterator(const_cast<GenericContainer*>(this), 0);
}

template<typename ITEM>
typename GenericContainer<ITEM>::const_iterator GenericContainer<ITEM>::end() const {
	return const_iterator(const_cast<GenericContainer*>(this), size());
}

}
}
