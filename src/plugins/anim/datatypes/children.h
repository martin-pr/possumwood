#pragma once

#include <cassert>

namespace anim {

/// an encapsulation of an iterator range for the children of a particular joint.
/// All joints are stored in a flat array, and ths class just returns two iterators
/// with the right offsets.
template <typename JOINT, typename CONTAINER>
class Children {
  public:
	typedef typename CONTAINER::const_iterator const_iterator;
	const_iterator begin() const;
	const_iterator end() const;

	typedef typename CONTAINER::iterator iterator;
	iterator begin();
	iterator end();

	bool valid() const;
	bool empty() const;

	std::size_t size() const;
	JOINT& operator[](std::size_t index);
	const JOINT& operator[](std::size_t index) const;

  private:
	Children();
	Children(std::size_t begin, std::size_t end, CONTAINER* joints);

	std::size_t m_begin, m_end;
	CONTAINER* m_joints;

	friend CONTAINER;
};

////

template <typename JOINT, typename CONTAINER>
Children<JOINT, CONTAINER>::Children() : m_begin(0), m_end(0), m_joints(NULL) {
}

template <typename JOINT, typename CONTAINER>
Children<JOINT, CONTAINER>::Children(std::size_t begin, std::size_t end, CONTAINER* joints)
    : m_begin(begin), m_end(end), m_joints(joints) {
}

template <typename JOINT, typename CONTAINER>
typename Children<JOINT, CONTAINER>::const_iterator Children<JOINT, CONTAINER>::begin() const {
	assert(valid());
	return m_joints->begin() + m_begin;
}

template <typename JOINT, typename CONTAINER>
typename Children<JOINT, CONTAINER>::const_iterator Children<JOINT, CONTAINER>::end() const {
	assert(valid());
	return m_joints->begin() + m_end;
}

template <typename JOINT, typename CONTAINER>
typename Children<JOINT, CONTAINER>::iterator Children<JOINT, CONTAINER>::begin() {
	assert(valid());
	return m_joints->begin() + m_begin;
}

template <typename JOINT, typename CONTAINER>
typename Children<JOINT, CONTAINER>::iterator Children<JOINT, CONTAINER>::end() {
	assert(valid());
	return m_joints->begin() + m_end;
}

template <typename JOINT, typename CONTAINER>
bool Children<JOINT, CONTAINER>::valid() const {
	return m_joints != NULL;
}

template <typename JOINT, typename CONTAINER>
bool Children<JOINT, CONTAINER>::empty() const {
	return (m_joints == NULL) || (m_begin == m_end);
}

template <typename JOINT, typename CONTAINER>
std::size_t Children<JOINT, CONTAINER>::size() const {
	assert(valid());
	return m_end - m_begin;
}

template <typename JOINT, typename CONTAINER>
JOINT& Children<JOINT, CONTAINER>::operator[](std::size_t index) {
	assert(valid());
	assert(index < m_joints->size());
	return (*m_joints)[index + m_begin];
}

template <typename JOINT, typename CONTAINER>
const JOINT& Children<JOINT, CONTAINER>::operator[](std::size_t index) const {
	assert(valid());
	assert(index < m_joints->size());
	return (*m_joints)[index + m_begin];
}

}  // namespace anim
