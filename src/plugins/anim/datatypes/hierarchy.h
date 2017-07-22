#pragma once

#include <vector>
#include <string>

#include <boost/noncopyable.hpp>

#include "children.h"
#include "attributes.h"

namespace anim {

/// Hierarchy class describes a hierarchy of transformations (skeleton) as a flat array of named Joint
/// objects. It guarantees that the index of parent joint is always lower than index of children joints,
/// allowing for replacing recursive operations (e.g, world-to-local conversion) to simple iterations.
/// The internal representation of joint data might change in the future (the interface will probably not).
class Hierarchy {
	public:
		class Item {
			public:
				Item(const std::string& name, int parent, std::size_t chld_begin, std::size_t chld_end);

				std::string name;
				int parent;
				std::size_t children_begin, children_end;

			private:
				/// Private attributes - item instances and Hierarchy iterators are only exposed as const (non-const
				/// operations are fully managed by the Hierarchy class, to guarantee consistency). However,
				/// the attributes have to be accessible on a non-const Hierarchy instance as non-const, without
				/// allowing the user to change parents, children or other things on an Item. For that reason,
				/// they are exposed via itemAttributes() instead of directly on Item.
				Attributes attrs;

			friend class Hierarchy;
		};

		const Item& operator[](std::size_t index) const;

		bool empty() const;
		size_t size() const;

		void addRoot(const std::string& name);
		std::size_t addChild(const Item& i, const std::string& name);

		typedef std::vector<Item>::const_iterator const_iterator;
		const_iterator begin() const;
		const_iterator end() const;

		Attributes& attributes();
		const Attributes& attributes() const;

		Attributes& itemAttributes(std::size_t index);
		const Attributes& itemAttributes(std::size_t index) const;

	protected:
	private:
		std::size_t indexOf(const Item& j) const;

		std::vector<Item> m_items;
		Attributes m_attributes;
};

}
