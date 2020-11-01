#pragma once

#include <set>

#include "mesh.h"

namespace possumwood {

class FaceSelection {
  public:
	class Item {
	  public:
		typedef std::set<possumwood::CGALPolyhedron::Facet_const_handle>::const_iterator const_iterator;
		typedef std::set<possumwood::CGALPolyhedron::Facet_const_handle>::iterator iterator;
		typedef possumwood::CGALPolyhedron::Facet_const_handle value_type;

		Item(const Mesh& mesh);

		const_iterator begin() const;
		const_iterator end() const;

		iterator begin();
		iterator end();

		bool empty() const;
		std::size_t size() const;

		void push_back(const possumwood::CGALPolyhedron::Facet_const_handle& value);
		bool contains(const possumwood::CGALPolyhedron::Facet_const_handle& value) const;

		// mesh is used only for operations on selection (grow, shrink...), not for anything else
		const Mesh& mesh() const;

	  private:
		std::set<possumwood::CGALPolyhedron::Facet_const_handle> m_faces;
		Mesh m_mesh;
	};

	bool empty() const;
	std::size_t size() const;

	Item& operator[](std::size_t index);
	const Item& operator[](std::size_t index) const;

	void push_back(const Item& item);

	typedef std::vector<Item>::const_iterator const_iterator;
	const_iterator begin() const;
	const_iterator end() const;

  private:
	std::vector<Item> m_items;
};

std::ostream& operator<<(std::ostream& out, const FaceSelection& sel);

}  // namespace possumwood
