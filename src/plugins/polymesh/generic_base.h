#pragma once

#include <vector>
#include <memory>
#include <set>
#include <typeindex>

#include "generic_array.h"

namespace possumwood {
namespace polymesh {

/// A simple type-masking data storage for particle data.
/// Each data record is stored separately in a flat array to allow for reasonably fast data access.
class GenericBase {
	public:
		GenericBase();
		virtual ~GenericBase();

		GenericBase(const GenericBase& gb);
		GenericBase& operator = (const GenericBase& gb);

		class Handles;

		/// A handle class, allowing to access data members in Items
		class Handle {
			public:
				const std::string& name() const;
				const std::type_index& type() const;

				bool operator ==(const Handle& b) const;
				bool operator !=(const Handle& b) const;

			private:
				Handle(const std::string& n, const std::type_index& type, std::size_t index);

				std::string m_name;
				std::type_index m_type;
				std::size_t m_index;

			friend class Handles;
			friend class GenericBase;
		};

		/// Handles container allows to iterate over all metadata (per-particle items)
		class Handles {
			private:
				struct Compare {
					typedef std::string is_transparent;
					bool operator()(const Handle& h1, const Handle& h2) const {
						return h1.name() < h2.name();
					}
					bool operator()(const std::string& h1, const Handle& h2) const {
						return h1 < h2.name();
					}
					bool operator()(const Handle& h1, const std::string& h2) const {
						return h1.name() < h2;
					}
				};

			public:
				template<typename T>
				const Handle& add(const std::string& name, const T& defaultValue);

				bool empty() const;
				std::size_t size() const;
				void clear();

				/// throws when not found
				const Handle& operator[](const std::string& name) const;

				typedef std::set<Handle, Compare>::const_iterator const_iterator;
				const_iterator begin() const;
				const_iterator end() const;
				const_iterator find(const std::string& name) const;

				bool operator ==(const Handles& b) const;
				bool operator !=(const Handles& b) const;

			private:
				Handles(GenericBase* parent, const std::set<Handle, Compare>& h = std::set<Handle, Compare>());

				std::set<Handle, Compare> m_handles;
				GenericBase* m_parent;

			friend class GenericBase;
		};

		/// Metadata access
		Handles& handles();
		/// Metadata access
		const Handles& handles() const;

		bool empty() const;
		std::size_t size() const;

		bool operator ==(const GenericBase& b) const;
		bool operator !=(const GenericBase& b) const;

		template<typename T>
		const T& get(const Handle& h, std::size_t index) const;

		template<typename T>
		T& get(const Handle& h, std::size_t index);

	protected:
		void clear();
		void resize(std::size_t size);
		std::size_t add();

	private:
		Handles m_handles;
		std::vector<std::unique_ptr<ArrayBase> > m_data;
		std::size_t m_size;
};

}
}
