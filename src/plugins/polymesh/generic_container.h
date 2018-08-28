#pragma once

#include <vector>
#include <memory>
#include <map>

#include "generic_base.h"

#include <boost/iterator/iterator_facade.hpp>

namespace possumwood {
namespace polymesh {

template<typename ITEM>
class GenericContainer : public GenericBase {
	public:
		/// Data access only via random access iterators
		class iterator : public	boost::iterator_facade<iterator, ITEM, boost::random_access_traversal_tag> {
			public:
				iterator();
				iterator(const iterator& i);
				iterator& operator = (const iterator& i);

			protected:
				void increment();
				bool equal(const iterator& i) const;
				ITEM& dereference() const;
				void advance(long);

			private:
				iterator(GenericContainer* parent, std::size_t index);

				std::unique_ptr<ITEM> m_item; // stored in a pointer to allow non-const dereference without const_cast

			friend class GenericContainer;
			friend class boost::iterator_core_access;
		};

		iterator begin();
		iterator end();

		/// Data access only via random access iterators
		class const_iterator : public boost::iterator_facade<const_iterator, const ITEM, boost::random_access_traversal_tag> {
			public:
				const_iterator();
				const_iterator(const const_iterator& i) = default;
				const_iterator& operator = (const const_iterator& i) = default;

			protected:
				void increment();
				bool equal(const const_iterator& i) const;
				const ITEM& dereference() const;
				void advance(long);

			private:
				const_iterator(GenericContainer* parent, std::size_t index);

				ITEM m_item;

			friend class GenericContainer;
			friend class boost::iterator_core_access;
		};

		const_iterator begin() const;
		const_iterator end() const;

	protected:
	private:
};

}
}
