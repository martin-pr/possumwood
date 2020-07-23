#include "polyhedron.h"

#include "builder.h"

namespace possumwood {
namespace cgal {

void PolyhedronWrapper::Face::addVertex(std::size_t v) {
	m_vertices.push_back(v);
}

PolyhedronWrapper::PolyhedronWrapper(const std::string& name) : m_name(name) {
}

std::size_t PolyhedronWrapper::addPoint(float x, float y, float z) {
	m_points.push_back(possumwood::CGALKernel::Point_3(x, y, z));
	return m_points.size() - 1;
}

void PolyhedronWrapper::addFace(const Face& f) {
	m_faces.push_back(std::vector<std::size_t>());

	for(auto& vertex : f.m_vertices) {
		if(vertex >= m_points.size())
			throw std::runtime_error(
			    "Error building a polyhedron - vertex index out of bounds");
		m_faces.back().push_back(vertex);
	}
}

PolyhedronWrapper::operator Meshes() const {
	Meshes result;
	Mesh& mesh = result.addMesh(m_name);

	possumwood::CGALBuilder<possumwood::CGALPolyhedron::HalfedgeDS, typeof(m_points),
	                        typeof(m_faces)>
	    builder(m_points, m_faces);
	mesh.polyhedron().delegate(builder);

	return result;
}

}  // namespace cgal
}  // namespace possumwood
