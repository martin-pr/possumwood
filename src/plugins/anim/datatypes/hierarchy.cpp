#include "hierarchy.h"

#include <cassert>
#include <iostream>

using std::cout;
using std::endl;

namespace anim {

////////

Hierarchy::Item::Item(const std::string& n, int p, std::size_t chld_begin, std::size_t chld_end) :
	name(n), parent(p), children_begin(chld_begin), children_end(chld_end) {
}

////////

const Hierarchy::Item& Hierarchy::operator[](std::size_t index) const {
	assert(index < m_items.size());
	return m_items[index];
}

bool Hierarchy::empty() const {
	return m_items.empty();
}

size_t Hierarchy::size() const {
	return m_items.size();
}

std::size_t Hierarchy::indexOf(const Item& j) const {
	return (&j - &(*m_items.begin()));
}

void Hierarchy::addRoot(const std::string& name) {
	if(empty())
		// create a single root Item, with children "behind the end"
		m_items.push_back(Item{name, -1, 1, 1});
	else {
		// add a new Item at the beginning
		m_items.insert(m_items.begin(), Item{name, -1, 1, 2});
		// and update the children indices of all following Items
		for(auto it = m_items.begin()+1; it != m_items.end(); ++it) {
			++it->children_begin;
			++it->children_end;

			++it->parent;
		}
	}
}

std::size_t Hierarchy::addChild(const Item& j, const std::string& name) {
	// find the Item's last child - the position we'll be inserting into
	const std::size_t lastChild = j.children_end;
	assert(lastChild > indexOf(j));

	// update all children indices larger than the inserted number
	const std::size_t currentIndex = indexOf(j);
	for(std::size_t ji = 0; ji < m_items.size(); ++ji) {
		Item& i = m_items[ji];

		if(ji == currentIndex) {
			// update the m_end of Item's children to include this new Item
			++i.children_end;
			assert(lastChild == i.children_end-1);
		}
		else if(i.children_begin > lastChild) {
			++i.children_begin;
			++i.children_end;
		}
		else if((i.children_begin == lastChild) && (ji > currentIndex)) {
			++i.children_begin;
			++i.children_end;
		}

		if(i.parent >= (int)lastChild)
			++i.parent;
	}

	// figure out the new children position
	unsigned childPos = lastChild+1;
	while((childPos-1 < m_items.size()) && (m_items[childPos-1].parent <= (int)lastChild))
		++childPos;

	// and insert the Item
	m_items.insert(m_items.begin() + lastChild, Item{name, (int)currentIndex, childPos, childPos});

	// and return the index of newly inserted Item
	return lastChild;
}

Hierarchy::const_iterator Hierarchy::begin() const {
	return m_items.begin();
}

Hierarchy::const_iterator Hierarchy::end() const {
	return m_items.end();
}

Attributes& Hierarchy::attributes() {
	return m_attributes;
}

const Attributes& Hierarchy::attributes() const {
	return m_attributes;
}

Attributes& Hierarchy::itemAttributes(std::size_t index) {
	assert(index < m_items.size());
	return m_items[index].attrs;
}

const Attributes& Hierarchy::itemAttributes(std::size_t index) const {
	assert(index < m_items.size());
	return m_items[index].attrs;
}

}
