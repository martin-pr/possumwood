#include "cgal/datatypes/meshes.h"

#include <boost/algorithm/string/join.hpp>

#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>

#include <GL/glew.h>
#include <GL/glu.h>

#include "render/datatypes/vertex_data.inl"
#include "generic_polymesh.inl"
#include "traits.h"

namespace possumwood {

template<typename VAL>
struct VertexExtractor {
	static const VAL& get(const possumwood::polymesh::GenericPolymesh::Polygon& poly, possumwood::polymesh::GenericPolymesh::Polygon::const_iterator i, const polymesh::GenericBase::Handle& handle) {
		return i->vertex().get<VAL>(handle);
	}
};

template<typename VAL>
struct IndexExtractor {
	static const VAL& get(const possumwood::polymesh::GenericPolymesh::Polygon& poly, possumwood::polymesh::GenericPolymesh::Polygon::const_iterator i, const polymesh::GenericBase::Handle& handle) {
		return i->get<VAL>(handle);
	}
};

template<typename VAL>
struct PolyExtractor {
	static const VAL& get(const possumwood::polymesh::GenericPolymesh::Polygon& poly, possumwood::polymesh::GenericPolymesh::Polygon::const_iterator i, const polymesh::GenericBase::Handle& handle) {
		return poly.get<VAL>(handle);
	}
};

namespace {

template<typename VAL, typename EXTRACTOR>
class Iterable {
	public:
		typedef VAL value_type;

		Iterable(std::shared_ptr<const possumwood::polymesh::GenericPolymesh> m, const polymesh::GenericBase::Handle& h) : mesh(m), handle(h), polyIterator(m->polygons().begin()), faceIndex(0) {
		}

		~Iterable() {
			assert(triIndex == 0);
			assert(faceIndex == 0);
			assert(polyIterator == mesh->polygons().end() || counter == 0);
		}

		void operator++() {
			assert(polyIterator != mesh->polygons().end());

			// inside a triangle
			if(triIndex < 2)
				++triIndex;

			// switching to another triangle
			else {
				// inside a single polygon
				triIndex = 0;
				++faceIndex;

				++counter;

				// circulated around the whole polygon - go to the next poly
				if(faceIndex == polyIterator->size()-2) {
					faceIndex = 0;
					++polyIterator;
				}
			}
		}

		VAL value() {
			assert(triIndex < 3);
			assert(faceIndex < polyIterator->size()-2);
			assert(polyIterator != mesh->polygons().end());

			VAL result;

			// first vertex of the triangle - always the first vertex of the polygon
			if(triIndex == 0)
				result = EXTRACTOR::get(*polyIterator, polyIterator->begin(), handle);

			// other vertices of the triangle
			else
				result = EXTRACTOR::get(*polyIterator, polyIterator->begin() + faceIndex + triIndex, handle);

			return result;
		}

	private:
		std::shared_ptr<const possumwood::polymesh::GenericPolymesh> mesh;
		polymesh::GenericBase::Handle handle;

		possumwood::polymesh::GenericPolymesh::Polygons::const_iterator polyIterator;
		std::size_t faceIndex = 0, triIndex = 0, counter = 0;
};

}

template<typename ITER>
void addVBO(possumwood::VertexData& vd, const possumwood::polymesh::GenericBase::Handle& handle,
	const ITER& _i, std::size_t count) {

	std::shared_ptr<ITER> i(new ITER(_i));

	vd.addVBO<typename ITER::value_type>(handle.name(), count, possumwood::VertexData::kStatic,
		[i, count](possumwood::Buffer<typename possumwood::VBOTraits<typename ITER::value_type>::element>& buffer,
			const possumwood::Drawable::ViewportState& vs) {

			for(std::size_t v=0;v<count;++v) {
				const auto& val = (*i).value();
				buffer.element(v) = val;

				++(*i);
			}
		}
	);
}

template<template<typename> class EXTRACTOR>
void makeVBO(possumwood::VertexData& vd, const possumwood::polymesh::GenericBase::Handle& handle,
	const std::shared_ptr<const polymesh::GenericPolymesh>& mesh, std::size_t count) {

	// this is SUPER UGLY - self-registered traits might be nicer
	if(handle.type() == typeid(float))
		addVBO(vd, handle, Iterable<float, EXTRACTOR<float>>(mesh, handle), count);

	else if(handle.type() == typeid(std::array<float, 2>))
		addVBO(vd, handle, Iterable<std::array<float, 2>, EXTRACTOR<std::array<float, 2>>>(mesh, handle), count);

	else if(handle.type() == typeid(std::array<float, 3>))
		addVBO(vd, handle, Iterable<std::array<float, 3>, EXTRACTOR<std::array<float, 3>>>(mesh, handle), count);

	else if(handle.type() == typeid(std::array<float, 4>))
		addVBO(vd, handle, Iterable<std::array<float, 4>, EXTRACTOR<std::array<float, 4>>>(mesh, handle), count);

	else if(handle.type() == typeid(int))
		addVBO(vd, handle, Iterable<int, EXTRACTOR<int>>(mesh, handle), count);

	else if(handle.type() == typeid(std::array<int, 2>))
		addVBO(vd, handle, Iterable<std::array<int, 2>, EXTRACTOR<std::array<int, 2>>>(mesh, handle), count);

	else if(handle.type() == typeid(std::array<int, 3>))
		addVBO(vd, handle, Iterable<std::array<int, 3>, EXTRACTOR<std::array<int, 3>>>(mesh, handle), count);

	else if(handle.type() == typeid(std::array<int, 4>))
		addVBO(vd, handle, Iterable<std::array<int, 4>, EXTRACTOR<std::array<int, 4>>>(mesh, handle), count);

	else
		throw std::runtime_error("Unsupported type " + std::string(handle.type().name()) + " for VBO conversion");

}

dependency_graph::OutAttr<std::shared_ptr<const possumwood::VertexData>> a_vd;
dependency_graph::InAttr<polymesh::GenericPolymesh> a_mesh;

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State result;

	// make a shared copy of the mesh
	std::shared_ptr<const polymesh::GenericPolymesh> mesh(
		new polymesh::GenericPolymesh(data.get(a_mesh)));

	// we're drawing triangles
	std::unique_ptr<possumwood::VertexData> vd(new possumwood::VertexData(GL_TRIANGLES));

	// first, figure out how many triangles we have
	//   there has to be a better way to do this, come on
	std::size_t triangleCount = 0;
	for(auto& p : mesh->polygons()) {
		assert(p.size() > 2);
		triangleCount += p.size()-2;
	}

	// and build the buffers
	if(triangleCount > 0) {
		// per-vertex buffers
		for(auto& handle : mesh->vertices().handles())
			makeVBO<VertexExtractor>(*vd, handle, mesh, triangleCount*3);

		// per-index buffers
		for(auto& handle : mesh->indices().handles())
			makeVBO<IndexExtractor>(*vd, handle, mesh, triangleCount*3);

		// per-index buffers
		for(auto& handle : mesh->polygons().handles())
			makeVBO<PolyExtractor>(*vd, handle, mesh, triangleCount*3);
	}

	data.set(a_vd, std::shared_ptr<const possumwood::VertexData>(vd.release()));

	return result;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_vd, "vertex_data");
	meta.addAttribute(a_mesh, "generic_mesh");

	meta.addInfluence(a_mesh, a_vd);

	meta.setCompute(&compute);
}

possumwood::NodeImplementation s_impl("polymesh/vertex_data", init);
}
