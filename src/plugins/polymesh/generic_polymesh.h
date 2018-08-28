#pragma once

#include <vector>
#include <memory>
#include <map>

#include "generic_container.h"

#include <boost/iterator/iterator_facade.hpp>

namespace possumwood {
namespace polymesh {

/// GenericPolymesh is built on top of 3 instances of GenericContainer.
/// Allows to set arbitrary attributes to all topology-defining concepts of a polygonal mesh - vertices, indices and polygons.
/// Items of each array are stored in GenericContainer instances, with topology information stored in separate index arrays.
/// Vertex, Index and Polygon classes fully hide the implementation detail behind an abstract interface.
/// Only iterator-based access is supported by this design (use *(begin()+n) instead [n]).
class GenericPolymesh {
	public:
		GenericPolymesh();

		GenericPolymesh(const GenericPolymesh& p);
		GenericPolymesh& operator = (const GenericPolymesh& p);

		class Index;

		class Vertex {
			public:
				template<typename T>
				const T& get(const GenericContainer<Vertex>::Handle& handle) const;

				template<typename T>
				T& get(const GenericContainer<Vertex>::Handle& handle);

				template<typename T>
				void set(const GenericContainer<Vertex>::Handle& handle, const T& value);

				std::size_t id() const;

			private:
				Vertex(GenericContainer<Vertex>* parent, std::size_t index);

				Vertex(const Vertex&) = default;
				Vertex& operator=(const Vertex&) = default;

				Vertex& operator += (long d);
				void operator++();

				GenericContainer<Vertex>* m_parent;
				std::size_t m_index;

			friend class GenericContainer<Vertex>;
			friend class Index;
		};

		class Vertices : public GenericContainer<Vertex> {
			public:
				iterator add();
		};

		const Vertices& vertices() const;
		Vertices& vertices();


		class Index {
			public:
				template<typename T>
				const T& get(const GenericContainer<Index>::Handle& handle) const;

				template<typename T>
				T& get(const GenericContainer<Index>::Handle& handle);

				template<typename T>
				void set(const GenericContainer<Index>::Handle& handle, const T& value);

				Vertex& vertex();
				const Vertex& vertex() const;

			private:
				Index(GenericContainer<Index>* parent, std::size_t index);
				Index(const Index&);

				Index& operator=(const Index&);

				Index& operator += (long d);
				void operator++();

				GenericContainer<Index>* m_parent;
				std::size_t m_index;
				std::unique_ptr<Vertex> m_vertex;

			friend class GenericContainer<Index>;
		};

		class Indices : public GenericContainer<Index> {
			private:
				Indices(GenericPolymesh* parent, const GenericContainer<Index>& value = GenericContainer<Index>());

				using GenericContainer<Index>::add;

				GenericPolymesh* m_parent;

			friend class GenericPolymesh;
		};

		const Indices& indices() const;
		Indices& indices();

		class Polygon {
			public:
				template<typename T>
				const T& get(const GenericContainer<Polygon>::Handle& handle) const;

				template<typename T>
				T& get(const GenericContainer<Polygon>::Handle& handle);

				template<typename T>
				void set(const GenericContainer<Polygon>::Handle& handle, const T& value);

				std::size_t size() const;

				typedef Indices::iterator iterator;
				iterator begin();
				iterator end();

				typedef Indices::const_iterator const_iterator;
				const_iterator begin() const;
				const_iterator end() const;

			private:
				Polygon(GenericContainer<Polygon>* parent, std::size_t index);

				Polygon(const Polygon&) = default;
				Polygon& operator=(const Polygon&) = default;

				Polygon& operator += (long d);
				void operator++();

				GenericPolymesh* m_parent;
				std::size_t m_index;

			friend class GenericContainer<Polygon>;
		};

		class Polygons : public GenericContainer<Polygon> {
			public:
				/// adds a new polygon based on a pair of iterators (should dereference to std::size_t)
				template<typename ITER>
				Polygons::iterator add(ITER begin, ITER end);

			private:
				Polygons(GenericPolymesh* parent, const GenericContainer<Polygon>& value = GenericContainer<Polygon>());

				using GenericContainer<Polygon>::add;

				GenericPolymesh* m_parent;

			friend class GenericPolymesh;
		};

		const Polygons& polygons() const;
		Polygons& polygons();

		bool operator ==(const GenericPolymesh& pm) const;
		bool operator !=(const GenericPolymesh& pm) const;

	protected:
	private:
		void addIndex(std::size_t index) {
			m_vertexIndices.push_back(index);
			m_indices.add();
		}

		Vertices m_vertices;
		Indices m_indices;
		Polygons m_polygons;

		std::vector<std::size_t> m_vertexIndices;
		std::vector<std::size_t> m_polygonPointers;

	friend class Polygons;
};

std::ostream& operator << (std::ostream& out, const GenericPolymesh& pm);

}
}
